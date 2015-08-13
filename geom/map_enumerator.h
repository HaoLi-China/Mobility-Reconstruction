
#ifndef _GEOM_MAP_ENUMERATOR_H_
#define _GEOM_MAP_ENUMERATOR_H_

#include "geom_common.h"
#include "map_attributes.h"


class Map;


class GEOM_API MapEnumerator {
public:
	/**
	* returns the number of vertices.
	*/
	static int enumerate_vertices(
		Map* map, Attribute<Map::Vertex, int>& id,
		int start = 0, int step = 1
		) ;

	/**
	* returns the number of halfedges.
	*/
	static int enumerate_halfedges(
		Map* map, Attribute<Map::Halfedge, int>& id,
		int start = 0, int step = 1
		) ;

	/**
	* returns the number of facets.
	*/
	static int enumerate_facets(
		Map* map, Attribute<Map::Facet, int>& id,
		int start = 0, int step = 1
		) ;

	/**
	* returns the number of texture vertices.
	*/
	static int enumerate_tex_vertices(
		Map* map, Attribute<Map::TexVertex, int>& id,
		int start = 0, int step = 1
		) ;

	/**
	* returns the number of connected components.
	*/
	static int enumerate_connected_components(
		Map* map, Attribute<Map::Vertex, int>& id
		) ;

	/**
	* returns the number of connected components.
	*/
	static int enumerate_connected_components(
		Map* map, Attribute<Map::Facet, int>& id
		) ;

};


#endif

