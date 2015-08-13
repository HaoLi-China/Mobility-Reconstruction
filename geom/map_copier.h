
#ifndef _GEOM_MAP_COPIER_H_
#define _GEOM_MAP_COPIER_H_

#include "geom_common.h"
#include "map.h"
#include "map_attributes.h"
#include "map_components.h"
#include "../basic/attribute_copier.h"

#include <string>
#include <set>


class MapBuilder ;

/**
* Copies from a Map or a MapComponent into a Map.
* Named attributes can be also copied. Note: halfedge
* attributes copy is not implemented yet.
*/

/*
Example usage:

Map* merge(const std::vector<Map*>& meshes) {
	if (meshes.empty())
		return nil;

	MapCopier copier ;
	copier.set_copy_all_attributes(true) ;

	Map* mesh = new Map;
	MapBuilder builder(mesh);

	builder.begin_surface() ;
	int cur_vertex_id = 0 ;
	int cur_tex_vertex_id = 0 ;
	for(unsigned int i=0; i<meshes.size(); ++i) {
		Map* cur = meshes[i];
		if(!cur)
			continue;

		MapVertexAttribute<int>		vertex_id(cur) ;
		MapTexVertexAttribute<int>	tex_vertex_id(cur) ;
		copier.copy(builder, cur, vertex_id, tex_vertex_id, cur_vertex_id, cur_tex_vertex_id) ;
	}

	builder.end_surface() ;
	return mesh;
}

void merge_into_source(Map* source, Map* another) {
	if (!source)
		return;

	if (!another)
		return;

	int cur_vertex_id = 0 ;
	int cur_tex_vertex_id = 0 ;

	MapVertexAttribute<int>		vertex_id(another) ;
	MapTexVertexAttribute<int>	tex_vertex_id(another) ;

	MapCopier copier ;
	copier.set_copy_all_attributes(true) ;
	MapBuilder builder(source);
	builder.begin_surface() ;
	copier.copy(builder, another, vertex_id, tex_vertex_id, cur_vertex_id, cur_tex_vertex_id) ;
	builder.end_surface() ;
}

*/


class GEOM_API MapCopier {
public:
	MapCopier() ;
	~MapCopier() ;

	void set_copy_all_attributes(bool x) {
		copy_all_attributes_ = x ; 
	}

	void declare_vertex_attribute_to_copy(const std::string& x) {
		vertex_attributes_to_copy_.insert(x) ;
	}

	void declare_tex_vertex_attribute_to_copy(const std::string& x) {
		tex_vertex_attributes_to_copy_.insert(x) ;
	}

	void declare_halfedge_attribute_to_copy(const std::string& x) {
		halfedge_attributes_to_copy_.insert(x) ;
	}

	void declare_facet_attribute_to_copy(const std::string& x) {
		facet_attributes_to_copy_.insert(x) ;
	}

	void clear_attributes_to_copy() {
		vertex_attributes_to_copy_.clear() ;
		tex_vertex_attributes_to_copy_.clear() ;
		halfedge_attributes_to_copy_.clear() ;
		facet_attributes_to_copy_.clear() ;
	}

	void copy(
		MapBuilder& to, Map* from, 
		MapVertexAttribute<int>& vertex_id, MapTexVertexAttribute<int>& tex_vertex_id, 
		int& cur_vertex_id, int& cur_tex_vertex_id
		) ;

	void copy(MapBuilder& to, Map* from) {
		MapVertexAttribute<int> vertex_id(from) ;
		MapTexVertexAttribute<int> tex_vertex_id(from) ;
		int cur_vertex_id = 0 ;
		int cur_tex_vertex_id = 0 ;
		copy(to, from, vertex_id, tex_vertex_id, cur_vertex_id, cur_tex_vertex_id) ;
	}

	void copy(
		MapBuilder& to, MapComponent* from,
		MapVertexAttribute<int>& vertex_id, MapTexVertexAttribute<int>& tex_vertex_id, 
		int& cur_vertex_id, int& cur_tex_vertex_id
		) ;

	void copy(MapBuilder& to, MapComponent* from) {
		MapVertexAttribute<int> vertex_id(from->map()) ;
		MapTexVertexAttribute<int> tex_vertex_id(from->map()) ;
		int cur_vertex_id = 0 ;
		int cur_tex_vertex_id = 0 ;
		copy(to, from, vertex_id, tex_vertex_id, cur_vertex_id, cur_tex_vertex_id) ;
	}

protected:

	// ------------------------------ copy attributes --------------------------------------------------------

	void copy_vertex_attributes(Map::Vertex* to, Map::Vertex* from) {
		copy_attributes(vertex_attribute_copiers_, to, from) ;
	}

	void copy_tex_vertex_attributes(Map::TexVertex* to, Map::TexVertex* from) {
		copy_attributes(tex_vertex_attribute_copiers_, to, from) ;
	}

	void copy_halfedge_attributes(Map::Halfedge* to, Map::Halfedge* from) {
		copy_attributes(halfedge_attribute_copiers_, to, from) ;
	}

	void copy_facet_attributes(Map::Facet* to, Map::Facet* from) {
		copy_attributes(facet_attribute_copiers_, to, from) ;
	}

	void bind_attribute_copiers(Map* destination, Map* source) ;

private:
	std::set<std::string> vertex_attributes_to_copy_ ;
	std::set<std::string> tex_vertex_attributes_to_copy_ ;
	std::set<std::string> halfedge_attributes_to_copy_ ;
	std::set<std::string> facet_attributes_to_copy_ ;
	bool copy_all_attributes_ ;

	std::vector< AttributeCopier<Map::Vertex> >		vertex_attribute_copiers_ ;
	std::vector< AttributeCopier<Map::Halfedge> >	halfedge_attribute_copiers_ ;
	std::vector< AttributeCopier<Map::Facet> >		facet_attribute_copiers_ ;
	std::vector< AttributeCopier<Map::TexVertex> >	tex_vertex_attribute_copiers_ ;

	Map* current_source_ ;
	Map* current_destination_ ;
} ;


#endif

