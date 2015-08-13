
#include "map_serializer_stl.h"
#include "../basic/logger.h"
#include "../geom/map_builder.h"
#include "../geom/map_geometry.h"

#include <map>
#include <fstream>
#include <iostream>


MapSerializer_stl::MapSerializer_stl()
: eps_(FLT_MIN)
{
	read_supported_ = true;
	write_supported_ = true;

	header_ = "Solid STL Model, saved by Liangliang Nan";
}

bool MapSerializer_stl::streams_supported() const {
	return false ;
}

bool MapSerializer_stl::serialize_read(
									   const std::string& file_name, Map* mesh)
{
	if (!mesh) {
		Logger::err("MapSerializer") << "mesh is null" << std::endl;
		return false;
	}
	MapBuilder builder(mesh);

	STL_Type file_type = resolve_stl_type(file_name);
	switch (file_type)
	{
	case STL_ASCII:
		return read_stla(file_name, builder);
		break;

	case STL_BINARY:
		return read_stlb(file_name, builder);
		break;

	default: 
		return false;
		break;
	}

	return false;
}

bool MapSerializer_stl::serialize_write(
										const std::string& file_name, const Map* mesh) const  
{
	// binary or ascii ?
	if ((file_name.rfind(".stla") != std::string::npos) || 
		(file_name.rfind(".astl") != std::string::npos)) 
	{
		return write_stla(file_name, mesh);
	}
	else if (
		(file_name.rfind(".stlb") != std::string::npos) ||
		(file_name.rfind(".bstl") != std::string::npos))
	{
		return write_stlb(file_name, mesh);
	}
	else if (file_name.rfind(".stl") != std::string::npos) // default is binary
	{
		return write_stlb(file_name, mesh);
	}

	return false;
}

bool MapSerializer_stl::read_stla(
								  const std::string& file_name, AbstractMapBuilder& builder) 
{
	std::ifstream input(file_name.c_str()) ;
	if(input.fail()) {
		Logger::err("MapSerializer_stl") 
			<< "could not open file\'" 
			<< file_name << "\'" << std::endl ;
		return false ;
	}

	PointCmp cmp(eps_);
	typedef std::map<vec3, int, PointCmp> VertexMap;
	VertexMap vMap(cmp);
	int  index(0);  // the index of the current read vertex

	builder.begin_surface();

	while (!input.eof())
	{
		std::string line;
		getline(input, line);

		if (line.find("outer") != std::string::npos ||
			line.find("OUTER") != std::string::npos)
		{
			builder.begin_facet();

			for (unsigned int i=0; i<3; ++i)
			{
				getline(input, line);
				std::istringstream face_input(line) ;
				std::string dumy;
				double x, y, z;
				face_input >> dumy >> x >> y >> z;
				vec3 v(x, y, z);

				// has point been referenced before?
				VertexMap::iterator vMapIt = vMap.find(v);
				if ( vMapIt == vMap.end()) {
					// No : add vertex and remember idx/vector mapping
					builder.add_vertex(v) ;
					builder.add_vertex_to_facet(index) ;
					vMap[v] = index ++;
				} else { 
					// Yes : get index from map
					builder.add_vertex_to_facet(vMapIt->second) ;
				}
			}

			builder.end_facet();
		}
	}

	builder.end_surface();

	return true;
}

bool MapSerializer_stl::read_stlb(
								  const std::string& file_name, AbstractMapBuilder& builder) 
{
	std::ifstream input(file_name.c_str(), std::fstream::binary) ;
	if(input.fail()) {
		Logger::err("MapSerializer_stl") 
			<< "could not open file\'" 
			<< file_name << "\'" << std::endl ;
		return false ;
	}

	// check size of types
	if ((sizeof(float) != 4) || (sizeof(int) != 4)) {
		Logger::err("MapSerializer_stl") 
			<< "Wrong type size" << std::endl;
		return false;
	}

	// skip
	input.seekg(80, std::ifstream::beg);

	// read number of triangles
	int nT = 0;
	input.read((char*)&nT, 4);

	PointCmp cmp(eps_);
	typedef std::map<vec3, int, PointCmp> VertexMap;
	VertexMap vMap(cmp);
	int  index(0);  // the index of the current read vertex

	builder.begin_surface();

	// read triangles
	for (int count=0; count<nT; ++count) {
		// skip triangle normal
		input.seekg(12, std::ifstream::cur);

		builder.begin_facet();

		// triangle's vertices
		for (unsigned int i=0; i<3; ++i) {
			float xyz[3];                    // faster
			input.read((char*)&xyz, 12);
			vec3 v(xyz[0], xyz[1], xyz[2]);

			// has point been referenced before?
			VertexMap::iterator vMapIt = vMap.find(v);
			if ( vMapIt == vMap.end()) {
				// No : add vertex and remember idx/vector mapping
				builder.add_vertex(v) ;
				builder.add_vertex_to_facet(index) ;
				vMap[v] = index ++;	
			} else {
				// Yes : get index from map
				builder.add_vertex_to_facet(vMapIt->second) ;
			}
		}

		builder.end_facet();

		input.seekg(2, std::fstream::cur);
	}

	builder.end_surface();

	return true;
}


bool MapSerializer_stl::write_stla(
								   const std::string& file_name, const Map* mesh) const
{
	std::ofstream output(file_name.c_str()) ;
	if(output.fail()) {
		Logger::err("MapSerializer_stl") 
			<< "could not open file\'" 
			<< file_name << "\'" << std::endl ;
		return false ;
	}

	// header
	output << header_ << std::endl;

	MapFacetNormal normal(const_cast<Map*>(mesh)) ;
	// Output facets
	FOR_EACH_FACET_CONST(Map, mesh, it) {
		if (it->nb_vertices() == 3) {
			const Map::Halfedge* he = it->halfedge() ;
			const vec3& p0 = he->vertex()->point();
			const vec3& p1 = he->next()->vertex()->point();
			const vec3& p2 = he->next()->next()->vertex()->point();
			const vec3& n = normal[it];

			output << "  facet normal " << n.x << " " << n.y << " " << n.z << std::endl;
			output << "      outer loop" << std::endl;
			output << "          vertex " << p0 << std::endl;
			output << "          vertex " << p1 << std::endl;
			output << "          vertex " << p2 << std::endl;
		} else {
			Logger::warn("MapSerializer")
				<< "Skip non-triangle data" << std::endl;
		}

		output << "      endloop" << std::endl;
		output << "  endfacet" << std::endl;
	}

	output << "end solid";

	return true;
}

bool MapSerializer_stl::write_stlb(
								   const std::string& file_name, const Map* mesh) const
{
	// open file
	std::ofstream output(file_name.c_str(), std::fstream::binary) ;
	if(output.fail()) {
		Logger::err("MapSerializer_stl") 
			<< "could not open file\'" 
			<< file_name << "\'" << std::endl ;
		return false ;
	}

	// write header
	const char* h = header_.c_str();
	output.write(h, 80);

	// number of faces
	int nF = mesh->size_of_facets();
	output.write((char*)&nF, 4);

	// write face set
	vec3 p[3];
	const char ignor[2] = {' ', ' '};
	MapFacetNormal normal(const_cast<Map*>(mesh)) ;
	FOR_EACH_FACET_CONST(Map, mesh, it) {
		if (it->nb_vertices() == 3) {
			const Map::Halfedge* he = it->halfedge() ;
			p[0] = he->vertex()->point();
			p[1] = he->next()->vertex()->point();
			p[2] = he->next()->next()->vertex()->point();
			const vec3& n = normal[it];

			// normal
			for(int i=0; i<3; ++i) {
				const float& v = static_cast<float>(n[i]);
				output.write((char*)&v, 4);
			}

			// vertices
			for(int i=0; i<3; ++i) {
				for (int j=0; j<3; ++j) {
					const float& v = static_cast<float>(p[i][j]);
					output.write((char*)&v, 4);
				}
			}

			// space filler
			output.write(ignor, 2);
		} else { 
			Logger::err("MapSerializer")
				<< "Skip non-triangle data" << std::endl;
		}
	}

	return true;
}

MapSerializer_stl::STL_Type MapSerializer_stl::resolve_stl_type(
	const std::string& file_name) const
{
	// assume it's binary stl, then file size is known from #triangles
	// if size matches, it's really binary

	std::ifstream input(file_name.c_str(), std::fstream::binary) ;
	if(input.fail()) {
		Logger::err("MapSerializer_stl") 
			<< "could not open file\'" 
			<< file_name << "\'" << std::endl ;
		return NONE ;
	}

	// check size of types
	if ((sizeof(float) != 4) || (sizeof(int) != 4)) {
		Logger::err("MapSerializer_stl") 
			<< "Wrong type size" << std::endl;
		return NONE;
	}

	// skip
	input.seekg(80, std::ifstream::beg);
	if (input.fail()) {
		Logger::err("MapSerializer_stl") 
			<< "File is less than 80 bytes" << std::endl;
		return NONE;
	}

	// read number of triangles
	int nT = 0;
	input.read((char*)&nT, 4);

	// compute file size from nT
	size_t needed_size = 84 + nT*50; // 50 bytes per facet

	// get actual file size
	input.seekg(0, std::fstream::beg);
	size_t begin_pos = input.tellg();
	input.seekg(0, std::fstream::end);
	size_t end_pos = input.tellg();
	size_t file_size = end_pos - begin_pos;

	// if sizes match->it's STLB
	STL_Type type = STL_ASCII ;
	if (needed_size == file_size) {
		type = STL_BINARY ;
	} 

	// NOTE: many people may forget the last two type, so...
	if (needed_size == (file_size + 2)) {
		Logger::err("MapSerializer_stl")
			<< "STL file's triangle count does not match file size" << std::endl
			<< "Bytes needed: " << needed_size 
			<< ", available data: " << file_size << std::endl
			<< "Try to open it as STL binary file" << std::endl;
		type = STL_BINARY ;
	}

	return type ;
}


bool MapSerializer_stl::PointCmp::operator () ( 
	const vec3& v0, const vec3& v1 ) const
{
	if (std::fabs(v0[0] - v1[0]) <= eps_) {
		if (std::fabs(v0[1] - v1[1]) <= eps_) 
			return (v0[2] < v1[2] - eps_);
		else
			return (v0[1] < v1[1] - eps_);
	} else {
		return (v0[0] < v1[0] - eps_);
	}
}




