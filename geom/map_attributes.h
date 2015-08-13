#ifndef _GEOM_MAP_ATTRIBUTES_H_
#define _GEOM_MAP_ATTRIBUTES_H_


#include "map.h"
#include "../math/attribute_adapter.h"
#include "../basic/counted.h"

#include <string>
#include <vector>
#include <iostream>

//_________________________________________________________


enum SurfaceAttributeLocation {
	on_vertices = 0,
	on_edges    = 1,        
	on_facets   = 2
} ;

//________________________________________________________

template <class T> 
class MapTexVertexAttribute : public Attribute<Map::TexVertex, T> {
public:
	typedef Attribute<Map::TexVertex, T> superclass ;

	void bind(Map* map, const std::string& name) {
		superclass::bind(
			map->tex_vertex_attribute_manager(), name
			) ;
	}

	void bind(Map* map) { 
		superclass::bind(map->tex_vertex_attribute_manager()) ; 
	}

	bool bind_if_defined(Map* map, const std::string& name) {
		return superclass::bind_if_defined(map->tex_vertex_attribute_manager(), name) ;
	}

	MapTexVertexAttribute() { }

	MapTexVertexAttribute(Map* map) {
		bind(map) ;
	}

	MapTexVertexAttribute(Map* map, const std::string& name) { 
		bind(map, name) ; 
	}

	static bool is_defined(Map* map, const std::string& name) {
		return superclass::is_defined(
			map->tex_vertex_attribute_manager(), name
			) ;
	}
} ;

//_________________________________________________________

template <class T> 
class MapVertexAttribute : public Attribute<Map::Vertex, T> {
public:
	typedef Attribute<Map::Vertex, T> superclass ;

	void bind(Map* map, const std::string& name) {
		superclass::bind(
			map->vertex_attribute_manager(), name 
			) ;
	}

	void bind(Map* map) { 
		superclass::bind(map->vertex_attribute_manager()) ; 
	}

	bool bind_if_defined(Map* map, const std::string& name) {
		return superclass::bind_if_defined(map->vertex_attribute_manager(), name) ;
	}

	MapVertexAttribute() { }

	MapVertexAttribute(Map* map) {
		bind(map) ;
	}

	MapVertexAttribute(Map* map, const std::string& name) { 
		bind(map, name) ;
	}

	static bool is_defined(Map* map, const std::string& name) {
		return superclass::is_defined(
			map->vertex_attribute_manager(), name
			) ;
	}
} ;

//_________________________________________________________

template <class T> 
class MapHalfedgeAttribute : public Attribute<Map::Halfedge, T> {
public:
	typedef Attribute<Map::Halfedge, T> superclass ;

	void bind(Map* map, const std::string& name) {
		superclass::bind(
			map->halfedge_attribute_manager(), name
			) ;
	}

	void bind(Map* map) { 
		superclass::bind(map->halfedge_attribute_manager()) ; 
	}

	bool bind_if_defined(Map* map, const std::string& name) {
		return superclass::bind_if_defined(map->halfedge_attribute_manager(), name) ;
	}

	MapHalfedgeAttribute() { }

	MapHalfedgeAttribute(Map* map) {
		bind(map) ;
	}

	MapHalfedgeAttribute(Map* map, const std::string& name) { 
		bind(map, name) ; 
	}

	static bool is_defined(Map* map, const std::string& name) {
		return superclass::is_defined(
			map->halfedge_attribute_manager(), name
			) ;
	}
} ;

//_________________________________________________________

template <class T> 
class MapFacetAttribute : public Attribute<Map::Facet, T> {
public:
	typedef Attribute<Map::Facet, T> superclass ;

	void bind(Map* map, const std::string& name) {
		superclass::bind(
			map->facet_attribute_manager(), name
			) ;
	}

	void bind(Map* map) { 
		superclass::bind(map->facet_attribute_manager()) ; 
	}

	bool bind_if_defined(Map* map, const std::string& name) {
		return superclass::bind_if_defined(map->facet_attribute_manager(), name) ;
	}

	MapFacetAttribute() { }

	MapFacetAttribute(Map* map) {
		bind(map) ;
	}

	MapFacetAttribute(Map* map, const std::string& name) { 
		bind(map, name) ; 
	}

	static bool is_defined(Map* map, const std::string& name) {
		return superclass::is_defined(
			map->facet_attribute_manager(), name
			) ;
	}
} ;


//_________________________________________________________

class MapTexVertexAttributeAdapter : public AttributeAdapter<Map::TexVertex> {
public:
	typedef AttributeAdapter<Map::TexVertex> superclass ;
	bool bind_if_defined(Map* map, const std::string& name) {
		return superclass::bind_if_defined(map->tex_vertex_attribute_manager(), name) ;
	}
} ;

class MapVertexAttributeAdapter : public AttributeAdapter<Map::Vertex> {
public:
	typedef AttributeAdapter<Map::Vertex> superclass ;
	bool bind_if_defined(Map* map, const std::string& name) {
		return superclass::bind_if_defined(map->vertex_attribute_manager(), name) ;
	}
} ;

class MapHalfedgeAttributeAdapter : public AttributeAdapter<Map::Halfedge> {
public:
	typedef AttributeAdapter<Map::Halfedge> superclass ;
	bool bind_if_defined(Map* map, const std::string& name) {
		return superclass::bind_if_defined(map->halfedge_attribute_manager(), name) ;
	}
} ;

class MapFacetAttributeAdapter : public AttributeAdapter<Map::Facet> {
public:
	typedef AttributeAdapter<Map::Facet> superclass ;
	bool bind_if_defined(Map* map, const std::string& name) {
		return superclass::bind_if_defined(map->facet_attribute_manager(), name) ;
	}
} ;


//=====================================================================
//
//                    standard map attributes
//
//=====================================================================

class MapVertexNormal : public MapVertexAttribute<vec3> {
public:
	typedef MapVertexAttribute<vec3> superclass ;
	MapVertexNormal() { }
	MapVertexNormal(Map* map) : superclass(map, "normal") { }
	void bind(Map* map) { superclass::bind(map, "normal");  }
	static bool is_defined(Map* map) {
		return superclass::is_defined(map, "normal") ;
	}
} ;

//_________________________________________________________
class MapVertexLock : public MapVertexAttribute<bool> {
public:
	typedef MapVertexAttribute<bool> superclass ;
	MapVertexLock() { }
	MapVertexLock(Map* map) : superclass(map, "lock") { }
	void bind(Map* map) { superclass::bind(map, "lock") ; }
	static bool is_defined(Map* map) {
		return superclass::is_defined(map,"lock") ;
	}
} ;

//_________________________________________________________

class MapFacetMaterialId : public MapFacetAttribute<int> {
public:
	typedef MapFacetAttribute<int> superclass ;
	MapFacetMaterialId() { }
	MapFacetMaterialId(Map* map) : superclass(map, "material_id") { }
	void bind(Map* map) { superclass::bind(map, "material_id") ; }
	static bool is_defined(Map* map) {
		return superclass::is_defined(map, "material_id") ;
	}
} ;

//___________________________________________________________________

class MapFacetNormal : public MapFacetAttribute<vec3> {
public:
	typedef MapFacetAttribute<vec3> superclass ;
	MapFacetNormal() { }
	MapFacetNormal(Map* map) : superclass(map, "normal") { }
	void bind(Map* map) { superclass::bind(map, "normal");  }
	static bool is_defined(Map* map) {
		return superclass::is_defined(map,"normal") ;
	}
} ;


#endif

