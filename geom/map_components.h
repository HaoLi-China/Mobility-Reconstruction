
#ifndef _GEOM_MAP_COMPONENTS_H_
#define _GEOM_MAP_COMPONENTS_H_

#include "geom_common.h"
#include "map.h"
#include "map_attributes.h"
#include "../basic/pointer_iterator.h"

#include <vector>


/**
* represents a connected component of a Map.
* A MapComponent stores three lists of pointers to its
* vertices, halfedges and facets it contains.
*/
class GEOM_API MapComponent : public Counted {
public:

	MapComponent(Map* map) : map_(map) { }

	typedef Map::Vertex Vertex ;
	typedef Map::Halfedge Halfedge ;
	typedef Map::Facet Facet ;
	typedef Map::TexVertex TexVertex ;

	typedef Iterators::PointerIterator<std::vector<Vertex*>,Vertex>         Vertex_iterator ;
	typedef Iterators::PointerIterator<std::vector<Halfedge*>,Halfedge>		Halfedge_iterator ;
	typedef Iterators::PointerIterator<std::vector<Facet*>,Facet>			Facet_iterator ;
	typedef Iterators::PointerConstIterator<std::vector<Vertex*>,Vertex>	Vertex_const_iterator ;
	typedef Iterators::PointerConstIterator<std::vector<Halfedge*>,Halfedge> Halfedge_const_iterator;
	typedef Iterators::PointerConstIterator<std::vector<Facet*>,Facet>		Facet_const_iterator ;

	Vertex_iterator vertices_begin() { 
		return Vertex_iterator(vertices_.begin()); 
	}

	Vertex_iterator vertices_end() { 
		return Vertex_iterator(vertices_.end()); 
	}

	Halfedge_iterator halfedges_begin() { 
		return Halfedge_iterator(halfedges_.begin()); 
	}

	Halfedge_iterator halfedges_end() { 
		return Halfedge_iterator(halfedges_.end()); 
	}

	Facet_iterator facets_begin() { 
		return Facet_iterator(facets_.begin()); 
	}

	Facet_iterator facets_end() { 
		return Facet_iterator(facets_.end()); 
	}

	Vertex_const_iterator vertices_begin() const { 
		return Vertex_const_iterator(vertices_.begin()); 
	}

	Vertex_const_iterator vertices_end() const { 
		return Vertex_const_iterator(vertices_.end()); 
	}

	Halfedge_const_iterator halfedges_begin() const { 
		return Halfedge_const_iterator(halfedges_.begin()); 
	}

	Halfedge_const_iterator halfedges_end() const { 
		return Halfedge_const_iterator(halfedges_.end()); 
	}

	Facet_const_iterator facets_begin() const { 
		return Facet_const_iterator(facets_.begin()); 
	}

	Facet_const_iterator facets_end() const { 
		return Facet_const_iterator(facets_.end()); 
	}

	unsigned int size_of_vertices() const {
		return (unsigned int) vertices_.size() ;
	}

	unsigned int size_of_halfedges() const {
		return (unsigned int) halfedges_.size() ;
	}

	unsigned int size_of_facets() const {
		return (unsigned int) facets_.size() ;
	}

	Map* map() const { return map_ ; }

	Map* construct_map(bool triangles_only = false) ;

protected:
	friend class MapComponentMutator ;
	std::vector<Vertex*>& vertices() { return vertices_ ; }
	std::vector<Halfedge*>& halfedges() { return halfedges_ ; }
	std::vector<Facet*>& facets() { return facets_ ; }

private:
	Map* map_ ;
	std::vector<Vertex*>	vertices_ ;
	std::vector<Halfedge*>	halfedges_ ;
	std::vector<Facet*>		facets_ ;
} ;

typedef SmartPointer<MapComponent> MapComponent_var ;
typedef std::vector<MapComponent_var> MapComponentList ;

//_________________________________________________________

/**
* Base class for objects allowed to modify a MapComponent.
* It gives direct access to the lists stored by a component.
*/
class GEOM_API MapComponentMutator {
public:

	typedef MapComponent::Vertex Vertex ;
	typedef MapComponent::Halfedge Halfedge ;
	typedef MapComponent::Facet Facet ;
	typedef MapComponent::TexVertex TexVertex ;


	MapComponentMutator(MapComponent* target = nil) : target_(target) { }
	~MapComponentMutator() { target_ = nil ;  }
	MapComponent* target() { return target_ ; }
	void set_target(MapComponent* m) { target_ = m ; }

protected:
	std::vector<Vertex*>& vertices() { return target_->vertices(); }
	std::vector<Halfedge*>& halfedges() { return target_->halfedges(); }
	std::vector<Facet*>& facets() { return target_->facets(); }

private:
	MapComponent* target_ ;
} ;

//_________________________________________________________

/**
* creates the list of the connected components of a Map.
*/
class GEOM_API MapComponentsExtractor : public MapComponentMutator {
public:
	MapComponentList extract_components(Map* map) ;
	MapComponentList extract_components(Map* map, MapFacetAttribute<int>& component_id) ;
	MapComponent* extract_component(Map* map, Map::Facet* from) ;
} ;

//_________________________________________________________

namespace Geom {

	GEOM_API double component_area(const MapComponent* comp) ;

	GEOM_API double component_signed_area2d(const MapComponent* comp) ;
	inline double component_area2d(const MapComponent* comp) {
		return ::fabs(component_signed_area2d(comp)) ;
	}

	GEOM_API double component_border_length(const MapComponent* comp) ;

	GEOM_API Box3d  component_bbox(const MapComponent* mapc) ;
	GEOM_API Box2d  component_bbox2d(const MapComponent* mapc) ;

	GEOM_API void  translate_component(MapComponent* comp, const vec3& v) ;
	GEOM_API void  translate_component2d(MapComponent* comp, const vec2& v) ;

	GEOM_API void  normalize_component_tex_coords(MapComponent* comp) ;

	GEOM_API Map*  construct_map(const std::vector<MapComponent*>& components, bool copy_attributes) ;
} ;

#endif

