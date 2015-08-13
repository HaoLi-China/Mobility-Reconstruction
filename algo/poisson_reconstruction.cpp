#include "poisson_reconstruction.h"
#include "../geom/map.h"
#include "../geom/map_builder.h"
#include "../geom/point_set.h"
#include "../basic/logger.h"
#include "../basic/timer.h"

#include "../3rd_poisson_recon/MarchingCubes.h"
#include "../3rd_poisson_recon/Octree.h"
#include "../3rd_poisson_recon/MultiGridOctreeData.h"
#include "../3rd_poisson_recon/SurfaceTrimmer.h"

#ifdef _WIN32
#include <omp.h>
#endif

#define Real	float


PoissonReconstruction::PoissonReconstruction(void)
: octree_depth_(8)
, samples_per_node_(1.0f)
, triangulate_mesh_(false)  // it seems the code has bugs, so I use my triangulation
{
	// other default parameters
	full_depth_ = 5;
	voxelDepth_ = octree_depth_;
	cgDepth_ = 0;
	scale_ = 1.1f;
	pointWeight_ = 4.0f;
	gsIter_ = 8;
	threads_ = omp_get_num_procs();
	Logger::out("PoissonRecon") << "number of threads: " << threads_ << std::endl;

	confidence_ = false;
	normalWeight_ = false;
	verbose_ = false;
}

PoissonReconstruction::~PoissonReconstruction(void) {
}


template<class Vertex>
Map* convert_to_map(CoredFileMeshData<Vertex>& mesh, const std::string& density_attr_name) {
	int num_ic_pts = mesh.inCorePoints.size();
	int num_ooc_pts = mesh.outOfCorePointCount();
	int num_face = mesh.polygonCount();
	if (num_face <=0) {
		Logger::err("PoissonRecon") << "reconstructed mesh has 0 facet" << std::endl;
		return nil;
	}

	Map* result = new Map;
	MapVertexAttribute<float> density(result, density_attr_name);

	MapBuilder builder(result);
	builder.begin_surface();

	Real min_density = FLT_MAX;
	Real max_density = -FLT_MAX;
	mesh.resetIterator();
	for (int i=0; i<num_ic_pts; ++i) {
		const Vertex& v = mesh.inCorePoints[i];
		const Point3D<Real>& pt = v.point;
		builder.add_vertex(vec3(pt.coords[0], pt.coords[1], pt.coords[2]));

		density[builder.current_vertex()] = v.value;
		min_density = std::min(min_density, v.value);
		max_density = std::max(max_density, v.value);
	}
	for (int i=0; i<num_ooc_pts; ++i) {
		Vertex v;
		mesh.nextOutOfCorePoint(v);
		const Point3D<Real>& pt = v.point;
		builder.add_vertex(vec3(pt.coords[0], pt.coords[1], pt.coords[2]));

		density[builder.current_vertex()] = v.value;
		min_density = std::min(min_density, v.value);
		max_density = std::max(max_density, v.value);
	}

	for (int i=0; i<num_face; ++i) {
		std::vector<CoredVertexIndex> vertices;
		mesh.nextPolygon(vertices);

		builder.begin_facet();
		for (unsigned int j=0; j<vertices.size(); ++j) {
			int id = vertices[j].idx;
			if (!vertices[j].inCore)
				id += num_ic_pts;
			builder.add_vertex_to_facet(id);
		}
		builder.end_facet();
	}

	builder.end_surface();

 	Logger::out("PoissonRecon") 
 		<< "vertex attribute 'density' added. [" 
		<< clip_precision(min_density, 2) << ", " << clip_precision(max_density, 2) << "]" << std::endl;

	return result;
}


Map* PoissonReconstruction::apply(const PointSet* pset, const std::string& density_attr_name) {
	if (!pset) {
		Logger::err(title()) << "null point cloud" << std::endl;
		return nil;
	}

	if (!PointSetNormal::is_defined(const_cast<PointSet*>(pset))) {
		Logger::err(title()) << "normals are required" << std::endl;
		return nil;
	}
	PointSetNormal normals(const_cast<PointSet*>(pset));

 	TreeNodeData::NodeCount = 0;
	Octree<Real> tree;
	tree.threads = threads_;
	OctNode<TreeNodeData>::SetAllocator(MEMORY_ALLOCATOR_BLOCK_SIZE);

	int maxSolveDepth = octree_depth_;
	int kernelDepth = octree_depth_ - 2;

	//////////////////////////////////////////////////////////////////////////

	Timer t, t_total; t.start(), t_total.start();
	Logger::out(title()) << "Running Screened Poisson Reconstruction (Version 6.13)" << std::endl;

	//////////////////////////////////////////////////////////////////////////	

	tree.maxMemoryUsage = 0;

    std::list< Point3D<Real> > pts;
    std::list< Point3D<Real> > nms;
	FOR_EACH_VERTEX_CONST(PointSet, pset, it) {
		const vec3& p = it->point();
		const vec3& n = normals[it];
		Point3D<Real> pt(p.x, p.y, p.z);
		Point3D<Real> nm(n.x, n.y, n.z);
		pts.push_back(pt);
		nms.push_back(nm);
	}

	Octree<Real>::PointInfo* pointInfo = new Octree<Real>::PointInfo();
	Octree<Real>::NormalInfo* normalInfo = new Octree<Real>::NormalInfo();
	std::vector<Real>* kernelDensityWeights = new std::vector<Real>();
	std::vector<Real>* centerWeights = new std::vector<Real>();

	int adaptiveExponent = 1;
	int boundaryType = 1;
    int pointCount = tree.SetTree< Point3D<Real> >(
		pts, nms, cgDepth_, octree_depth_, full_depth_, kernelDepth, samples_per_node_, scale_, confidence_, normalWeight_,
		pointWeight_, adaptiveExponent, *pointInfo, *normalInfo, *kernelDensityWeights, *centerWeights, boundaryType);
	pts.clear();
	nms.clear();

	Logger::out(title()) << "Tree built. " << t.time() << " seconds, " << clip_precision(tree.maxMemoryUsage, 2) << " MB memory" << std::endl;
	pset->immediate_update();

	//////////////////////////////////////////////////////////////////////////

	double maxMemoryUsage = tree.maxMemoryUsage;
	t.reset();

	tree.maxMemoryUsage = 0;
	Pointer(Real)constraints = tree.SetLaplacianConstraints(*normalInfo);
	delete normalInfo;

	Logger::out(title()) << "Constraints set. " << t.time() << " seconds, " << clip_precision(tree.maxMemoryUsage, 1) << " MB memory" << std::endl;
	maxMemoryUsage = std::max<double>(maxMemoryUsage, tree.maxMemoryUsage);
	pset->immediate_update();

	//////////////////////////////////////////////////////////////////////////

	bool showResidual = false;
	Real solverAccuracy = 1e-3f;
	Pointer(Real) solution = tree.SolveSystem(*pointInfo, constraints, showResidual, gsIter_, maxSolveDepth, cgDepth_, solverAccuracy);
	delete pointInfo;
	FreePointer(constraints);
	Logger::out(title()) << "Linear system solved. " << t.time() << "  seconds, " << clip_precision(tree.maxMemoryUsage, 1) << " MB memory" << std::endl;
	maxMemoryUsage = std::max< double >(maxMemoryUsage, tree.maxMemoryUsage);
	pset->immediate_update();

	//////////////////////////////////////////////////////////////////////////

	if (verbose_) 
		tree.maxMemoryUsage = 0;
	t.reset();
	Real isoValue = tree.GetIsoValue(solution, *centerWeights);
	delete centerWeights;
	Logger::out(title()) << "Iso-Value: " << isoValue << ". " << t.time() << " seconds" << std::endl;

	//////////////////////////////////////////////////////////////////////////

	t.reset();
	tree.maxMemoryUsage = 0;
	bool nonManifold = false;

 #ifdef DISABLE_DEPTH_VALUE  // NOTE: disabling depth value will at the same time disable the trimmer.
    CoredFileMeshData< PlyVertex<Real> > mesh;		// without the estimated depth values of the iso-surface vertices
 #else
    CoredFileMeshData< PlyValueVertex<Real> > mesh;	// with the estimated depth values of the iso-surface vertices
 #endif

 	tree.GetMCIsoSurface(
		kernelDensityWeights ? GetPointer(*kernelDensityWeights) : NullPointer<Real>(), 
		solution, 
		isoValue, 
		mesh, 
		true, 
		!nonManifold, 
		!triangulate_mesh_
		);
	delete kernelDensityWeights;
	kernelDensityWeights = nil;
	Logger::out(title()) << "Mesh extracted. " << t.time() << " seconds, " << clip_precision(tree.maxMemoryUsage, 1) << " MB memory" << std::endl;
	pset->immediate_update();

	maxMemoryUsage = std::max<double>(maxMemoryUsage, tree.maxMemoryUsage);

	//////////////////////////////////////////////////////////////////////////

	Map* result = convert_to_map(mesh, density_attr_name);
	Logger::out(title()) << "Total reconstruction: " << t_total.time() << " seconds, " << clip_precision(maxMemoryUsage, 1) << " MB memory" << std::endl;
	
	return result; 
}



Map* PoissonReconstruction::trim(
								 Map* mesh, 
								 const std::string& density_attr_name, 
								 float trim_value, 
								 float area_ratio, 
								 bool triangulate, 
								 int smooth) 
{
	if (!mesh)
		return nil;

	if (MapVertexAttribute<float>::is_defined(mesh, density_attr_name) == false) {
		Logger::err(title()) << "density is not available" << std::endl;
		return nil;
	}

	MapVertexAttribute<float> density(mesh, density_attr_name);
	Attribute<Map::Vertex, int> vertex_id(mesh->vertex_attribute_manager());
    std::vector< PlyValueVertex<Real> >	vertices;
    std::vector< std::vector<int> >		polygons;
	int id = 0;
	FOR_EACH_VERTEX_CONST(Map, mesh, it) {
		const vec3& p = it->point();
		PlyValueVertex<Real> v;
		v.point = Point3D<Real>(p.x, p.y, p.z);
		v.value = density[it];
		vertices.push_back(v);

		vertex_id[it] = id;
		++id;
	}

	FOR_EACH_FACET_CONST(Map, mesh, it) {
		std::vector<int> plg;
		Map::Halfedge* jt = it->halfedge() ;
		do {
			int id = vertex_id[jt->vertex()];
			plg.push_back(id);
			jt = jt->next() ;
		} while(jt != it->halfedge()) ;
		polygons.push_back(plg);
	}
	
	Timer t; t.start();
	Logger::out(title()) << "Running Surface Trimmer (V5)" << std::endl;
	trim_mesh(vertices, polygons, Real(trim_value), Real(area_ratio), triangulate, smooth);

	//////////////////////////////////////////////////////////////////////////

	Map* trimmed_mesh = new Map;
	density.bind(trimmed_mesh, density_attr_name);

	MapBuilder builder(trimmed_mesh);
	builder.begin_surface();

	for (size_t i=0; i<vertices.size(); ++i) {
		const Point3D<Real>& pt = vertices[i].point;
		builder.add_vertex(vec3(pt.coords[0], pt.coords[1], pt.coords[2]));
		Map::Vertex* v = builder.current_vertex();
		density[v] = vertices[i].value;
	}

	for (size_t i=0; i<polygons.size(); ++i) {
		const std::vector<int>& plg = polygons[i];
		builder.begin_facet();
		for (unsigned int j=0; j<plg.size(); ++j) {
			builder.add_vertex_to_facet(plg[j]);
		}
		builder.end_facet();
	}

	builder.end_surface();

	int removed_num = mesh->size_of_facets() - trimmed_mesh->size_of_facets();
	Logger::out(title()) << "Done. Time: " << t.time() << " seconds" << std::endl;

	return trimmed_mesh;
}
