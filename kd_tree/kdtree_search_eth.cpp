
#include "kdtree_search_eth.h"
#include "ETH_Kd_Tree/kdTree.h"




#define get_tree(x) ((kdtree::KdTree*)(x))


KdTreeSearch_ETH::KdTreeSearch_ETH()  {
	points_num_ = 0;
	tree_ = nil;
}


KdTreeSearch_ETH::~KdTreeSearch_ETH() {
    delete get_tree(tree_);
}


void KdTreeSearch_ETH::begin()  {
	vertices_.clear();

    delete get_tree(tree_);
	tree_ = nil;
}


void KdTreeSearch_ETH::end()  {
	points_num_ = vertices_.size();

	kdtree::Vector3D* points = new kdtree::Vector3D[points_num_];
	for(unsigned int i=0; i<points_num_; ++i) {
		const vec3& p = vertices_[i]->point();
		points[i].x = p.x;
		points[i].y = p.y;
		points[i].z = p.z;
	}

	unsigned int maxBucketSize = 16 ;	// number of points per bucket
	tree_ = new kdtree::KdTree(points, points_num_, maxBucketSize );
	delete [] points;
}


void KdTreeSearch_ETH::add_point(PointSet::Vertex* v)  {
	vertices_.push_back(v);
}


void KdTreeSearch_ETH::add_vertex_set(PointSet* vs)  {
	for(PointSet::Vertex_iterator it = vs->vertices_begin() ; it != vs->vertices_end() ; ++it)
		vertices_.push_back(it);
}


PointSet::Vertex* KdTreeSearch_ETH::find_closest_point(const vec3& p) const {
	kdtree::Vector3D v3d( p.x, p.y, p.z );
	get_tree(tree_)->setNOfNeighbours( 1 );
	get_tree(tree_)->queryPosition( v3d );

	unsigned int num = get_tree(tree_)->getNOfFoundNeighbours();
	if (num == 1) {
		return vertices_[ get_tree(tree_)->getNeighbourPositionIndex(0) ];
	} else
		return nil;
}

PointSet::Vertex* KdTreeSearch_ETH::find_closest_point(const vec3& p, double& squared_distance) const {
	kdtree::Vector3D v3d( p.x, p.y, p.z );
	get_tree(tree_)->setNOfNeighbours( 1 );
	get_tree(tree_)->queryPosition( v3d );

	unsigned int num = get_tree(tree_)->getNOfFoundNeighbours();
	if (num == 1) {
		squared_distance = get_tree(tree_)->getSquaredDistance(0);
		return vertices_[ get_tree(tree_)->getNeighbourPositionIndex(0) ];
	} else {
		std::cerr << "no point found" << std::endl;
		return nil;
	}
}

void KdTreeSearch_ETH::find_closest_K_points(
	const vec3& p, unsigned int k, std::vector<PointSet::Vertex*>& neighbors
	)  const {
		kdtree::Vector3D v3d( p.x, p.y, p.z );
		get_tree(tree_)->setNOfNeighbours( k );
		get_tree(tree_)->queryPosition( v3d );

		unsigned int num = get_tree(tree_)->getNOfFoundNeighbours();
		if (num == k) {
			neighbors.resize(k);
			for (unsigned int i=0; i<k; ++i) {
				neighbors[i] = vertices_[ get_tree(tree_)->getNeighbourPositionIndex(i) ];
			}		
		} else
			std::cerr << "less than " << k << " points found" << std::endl;
}

void KdTreeSearch_ETH::find_closest_K_points(
	const vec3& p, unsigned int k, std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances
	)  const {
		kdtree::Vector3D v3d( p.x, p.y, p.z );
		get_tree(tree_)->setNOfNeighbours( k );
		get_tree(tree_)->queryPosition( v3d );

		unsigned int num = get_tree(tree_)->getNOfFoundNeighbours();
		if (num == k) {
			neighbors.resize(k);
			squared_distances.resize(k);
			for (unsigned int i=0; i<k; ++i) {
				neighbors[i] = vertices_[ get_tree(tree_)->getNeighbourPositionIndex(i) ];
				squared_distances[i] = get_tree(tree_)->getSquaredDistance(i);
			}		
		} else
			std::cerr << "less than " << k << " points found" << std::endl;
}



void KdTreeSearch_ETH::find_points_in_radius(
	const vec3& p, double squared_radius, std::vector<PointSet::Vertex*>& neighbors
	)  const {
		kdtree::Vector3D v3d( p.x, p.y, p.z );
		get_tree(tree_)->queryRange( v3d, squared_radius, true );

		unsigned int num = get_tree(tree_)->getNOfFoundNeighbours();
		neighbors.resize(num);
		for (unsigned int i=0; i<num; ++i) {
			neighbors[i] = vertices_[ get_tree(tree_)->getNeighbourPositionIndex(i) ];
		}	
}


void KdTreeSearch_ETH::find_points_in_radius(
	const vec3& p, double squared_radius, std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances
	)  const {
		kdtree::Vector3D v3d( p.x, p.y, p.z );
		get_tree(tree_)->queryRange( v3d, squared_radius, true );

		unsigned int num = get_tree(tree_)->getNOfFoundNeighbours();
		neighbors.resize(num);
		squared_distances.resize(num);
		for (unsigned int i=0; i<num; ++i) {
			neighbors[i] = vertices_[ get_tree(tree_)->getNeighbourPositionIndex(i) ];
			squared_distances[i] = get_tree(tree_)->getSquaredDistance(i);
		}	
}


unsigned int KdTreeSearch_ETH::find_points_in_cylinder(
	const vec3& p1, const vec3& p2, double radius, 
	std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances, 
	bool bToLine
	) const {
		kdtree::Vector3D s( p1.x, p1.y, p1.z );
		kdtree::Vector3D t( p2.x, p2.y, p2.z );
		get_tree(tree_)->queryLineIntersection( s, t, radius, bToLine, true );

		unsigned int num = get_tree(tree_)->getNOfFoundNeighbours();

		neighbors.resize(num);
		squared_distances.resize(num);
		for (unsigned int i=0; i<num; ++i) {
			neighbors[i] = vertices_[ get_tree(tree_)->getNeighbourPositionIndex(i) ];
			squared_distances[i] = get_tree(tree_)->getSquaredDistance(i);
		}	

		return num;
}

unsigned int KdTreeSearch_ETH::find_points_in_cylinder(
	const vec3& p1, const vec3& p2, double radius, 
	std::vector<PointSet::Vertex*>& neighbors, 
	bool bToLine
	) const {
		kdtree::Vector3D s( p1.x, p1.y, p1.z );
		kdtree::Vector3D t( p2.x, p2.y, p2.z );
		get_tree(tree_)->queryLineIntersection( s, t, radius, bToLine, true );

		unsigned int num = get_tree(tree_)->getNOfFoundNeighbours();
		neighbors.resize(num);
		for (unsigned int i=0; i<num; ++i) {
			neighbors[i] = vertices_[ get_tree(tree_)->getNeighbourPositionIndex(i) ];
		}

		return num;
}


unsigned int KdTreeSearch_ETH::find_points_in_cone(
	const vec3& eye, const vec3& p1, const vec3& p2, double angle_range, 
	std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances, 
	bool bToLine
	) const {
		kdtree::Vector3D eye3d( eye.x, eye.y, eye.z );
		kdtree::Vector3D s( p1.x, p1.y, p1.z );
		kdtree::Vector3D t( p2.x, p2.y, p2.z ); 
		get_tree(tree_)->queryConeIntersection( eye3d, s, t, angle_range, bToLine, true );

		unsigned int num = get_tree(tree_)->getNOfFoundNeighbours();
		neighbors.resize(num);
		squared_distances.resize(num);
		for (unsigned int i=0; i<num; ++i) {
			neighbors[i] = vertices_[ get_tree(tree_)->getNeighbourPositionIndex(i) ];
			squared_distances[i] = get_tree(tree_)->getSquaredDistance(i);
		}

		return num;
}

unsigned int KdTreeSearch_ETH::find_points_in_cone(
	const vec3& eye, const vec3& p1, const vec3& p2, double angle_range, 
	std::vector<PointSet::Vertex*>& neighbors,
	bool bToLine
	) const {
		kdtree::Vector3D eye3d( eye.x, eye.y, eye.z );
		kdtree::Vector3D s( p1.x, p1.y, p1.z );
		kdtree::Vector3D t( p2.x, p2.y, p2.z );
		get_tree(tree_)->queryConeIntersection( eye3d, s, t, angle_range, bToLine, true);

		unsigned int num = get_tree(tree_)->getNOfFoundNeighbours();
		neighbors.resize(num);
		for (unsigned int i=0; i<num; ++i) {
			neighbors[i] = vertices_[ get_tree(tree_)->getNeighbourPositionIndex(i) ];
		}

		return num;
}
