
#include "map_components.h"
#include "map_enumerator.h"
#include "map_attributes.h"
#include "map_builder.h"
#include "map_geometry.h"

#include "map_copier.h" // for "Map* construct_map(const std::vector<MapComponent*>& components, bool copy_attributes);"

#include <set>
#include <stack>


Map* MapComponent::construct_map(bool triangles_only /* = false */) {
	MapComponent* comp = this;

	MapVertexAttribute<int> vertex_id(comp->map()) ;
	{
		int id = 0 ;
		FOR_EACH_VERTEX(MapComponent, comp, it) {
			vertex_id[it] = id ;
			id++ ;
		}
	}
	Map* result = new Map ;
	MapVertexLock from_is_locked(comp->map()) ;
	MapVertexLock to_is_locked(result) ;

	MapVertexAttribute<Map::Vertex*> orig_vertex(result, "orig_vertex") ;
	MapBuilder builder(result) ;
	builder.begin_surface() ;
	FOR_EACH_VERTEX(MapComponent, comp, it) {
		builder.add_vertex(it->point()) ;
		orig_vertex[builder.current_vertex()] = it ;
		to_is_locked[builder.current_vertex()] = from_is_locked[it] ;
	}
	FOR_EACH_FACET(MapComponent, comp, it) {
		std::vector<int> v ;
		Map::Halfedge* h = it->halfedge() ;
		do {
			v.push_back(vertex_id[h->vertex()]) ;
			h = h->next() ;
		} while(h != it->halfedge()) ;
		if(triangles_only) {
			for(unsigned int i=1; (i+1)<v.size(); i++) {
				builder.begin_facet() ;
				builder.add_vertex_to_facet(v[0]) ;
				builder.add_vertex_to_facet(v[i]) ;
				builder.add_vertex_to_facet(v[i+1]) ;
				builder.end_facet() ;
			}
		} else {
			builder.begin_facet() ;
			for(unsigned int i=0; i<v.size(); i++) {
				builder.add_vertex_to_facet(v[i]) ;
			}
			builder.end_facet() ;
		}
	}
	builder.end_surface() ;
	return result ;
}


MapComponentList MapComponentsExtractor::extract_components(
	Map* map
	) {
		Attribute<Map::Vertex,int> component_id( map->vertex_attribute_manager()	) ;
		int nb_components = MapEnumerator::enumerate_connected_components(map, component_id) ;
		MapComponentList result ;
		for(int i=0; i<nb_components; i++) {
			result.push_back(new MapComponent(map)) ;
		}

		FOR_EACH_VERTEX(Map, map, it) {
			set_target(result[component_id[it]]) ;
			vertices().push_back(it) ;
		}

		FOR_EACH_HALFEDGE(Map, map, it) {
			set_target(result[component_id[it->vertex()]]) ;
			halfedges().push_back(it) ;
		}

		FOR_EACH_FACET(Map, map, it) {
			set_target(result[component_id[it->halfedge()->vertex()]]) ;
			facets().push_back(it) ;
		}

		return result ;
}

MapComponentList MapComponentsExtractor::extract_components(
	Map* map, MapFacetAttribute<int>& component_id
	) {
		int nb_components = 0 ;
		FOR_EACH_FACET(Map, map, it) {
			nb_components = ogf_max(nb_components, component_id[it]) ;
		}
		nb_components++ ;
		MapComponentList result ;
		for(int i=0; i<nb_components; i++) {
			result.push_back(new MapComponent(map)) ;
		}

		FOR_EACH_VERTEX(Map, map, it) {
			Map::Halfedge* h = it->halfedge() ;
			int comp_id = (h->facet() != nil) ? component_id[h->facet()] : component_id[h->opposite()->facet()] ;
			set_target(result[comp_id]) ;
			vertices().push_back(it) ;
		}

		FOR_EACH_HALFEDGE(Map, map, it) {
			int comp_id = (it->facet() != nil) ? component_id[it->facet()] : component_id[it->opposite()->facet()] ;
			set_target(result[comp_id]) ;
			halfedges().push_back(it) ;
		}

		FOR_EACH_FACET(Map, map, it) {
			set_target(result[component_id[it]]) ;
			facets().push_back(it) ;
		}
		return result ;
}

MapComponent* MapComponentsExtractor::extract_component(
	Map* map, Map::Facet* from
	) {
		Attribute<Map::Vertex,int> component_id(
			map->vertex_attribute_manager()
			) ;
		MapEnumerator::enumerate_connected_components(map, component_id) ;
		MapComponent* result = new MapComponent(map) ;
		set_target(result) ;

		int comp_id = component_id[from->halfedge()->vertex()] ;

		{FOR_EACH_VERTEX(Map, map, it) {
			if(component_id[it] == comp_id) {
				vertices().push_back(it) ;
			}
		}}

		{FOR_EACH_HALFEDGE(Map, map, it) {
			if(component_id[it->vertex()] == comp_id) {
				halfedges().push_back(it) ;
			}
		}}

		{FOR_EACH_FACET(Map, map, it) {
			if(component_id[it->halfedge()->vertex()] == comp_id) {
				facets().push_back(it) ;
			}
		}}

		return result ;
}

//_______ Geometry ________________________________________________________

namespace Geom {

	double component_area(const MapComponent* comp) {
		double result = 0 ;
		FOR_EACH_FACET_CONST(MapComponent, comp, it) {
			result += facet_area(it) ;
		}
		return result ;
	}        

	double component_signed_area2d(const MapComponent* comp) {
		double result = 0 ;
		FOR_EACH_FACET_CONST(MapComponent, comp, it) {
			result += facet_signed_area2d(it) ;
		}
		return result ;
	}

	double component_border_length(const MapComponent* comp) {
		double result = 0.0 ;
		FOR_EACH_HALFEDGE_CONST(MapComponent, comp, it) {
			if(it->is_border()) {
				result += edge_length(it) ;
			}
		}

		return result ;
	}

	Box3d component_bbox(const MapComponent* mapc) {
		Box3d result ;
		FOR_EACH_VERTEX_CONST(MapComponent, mapc, it) {
			result.add_point(it->point()) ;
		}
		return result ;
	}

	Box2d component_bbox2d(const MapComponent* mapc) {
		Box2d result ;
		FOR_EACH_HALFEDGE_CONST(MapComponent, mapc, it) {
			result.add_point(it->tex_vertex()->tex_coord()) ;
		}
		return result ;
	}

	void translate_component(MapComponent* comp, const vec3& v) {
		FOR_EACH_VERTEX(MapComponent, comp, it) {
			vec3 new_p = it->point() + v ;
			it->set_point(new_p) ;
		}
	}
	void translate_component2d(MapComponent* comp, const vec2& v) {
		FOR_EACH_VERTEX(MapComponent, comp, it) {
			vec2 new_p = it->halfedge()->tex_coord() + v ;
			it->halfedge()->set_tex_coord(new_p) ;
		}
	}

	//______________________________________________________

	Map* construct_map(const std::vector<MapComponent*>& components, bool copy_attributes) {
		if (components.empty())
			return nil;
		
		MapCopier copier ;
		copier.set_copy_all_attributes(copy_attributes) ;

		Map* mesh = new Map;
		MapBuilder builder(mesh);
		builder.begin_surface() ;

		int cur_vertex_id = 0 ;
		int cur_tex_vertex_id = 0 ;
		for(unsigned int i=0; i<components.size(); ++i) {
			MapComponent* comp = components[i];
			MapVertexAttribute<int> vertex_id(comp->map()) ;
			MapTexVertexAttribute<int> tex_vertex_id(comp->map()) ;
			copier.copy(builder, comp, vertex_id, 
				tex_vertex_id, cur_vertex_id, cur_tex_vertex_id
				) ;
		}
		builder.end_surface() ;
		return mesh;
	}

}  //  namespace Geom 


