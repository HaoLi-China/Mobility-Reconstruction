
#include "map_enumerator.h"
#include "map.h"
#include "../basic/assertions.h"

#include <stack>

int MapEnumerator::enumerate_vertices(Map* map, Attribute<Map::Vertex, int>& id, int start, int step)
{
	ogf_assert(start >= 0) ;
	ogf_assert(step > 0) ;
	ogf_assert(
		id.attribute_manager() == map->vertex_attribute_manager()
		) ;
	int cur_id = start ;
	FOR_EACH_VERTEX(Map,map, it) {
		id[it] = cur_id ;
		cur_id += step ;
	}
	return (cur_id - start) / step ;
}

int MapEnumerator::enumerate_halfedges(Map* map, Attribute<Map::Halfedge, int>& id, int start, int step) 
{
	ogf_assert(start >= 0) ;
	ogf_assert(step > 0) ;
	ogf_assert(
		id.attribute_manager() == map->halfedge_attribute_manager()
		) ;
	int cur_id = start ;
	FOR_EACH_HALFEDGE(Map,map, it) {
		id[it] = cur_id ;
		cur_id += step ;
	}
	return (cur_id - start) / step ;
}

int MapEnumerator::enumerate_facets(Map* map, Attribute<Map::Facet, int>& id, int start, int step) 
{
	ogf_assert(start >= 0) ;
	ogf_assert(step > 0) ;
	ogf_assert(
		id.attribute_manager() == map->facet_attribute_manager()
		) ;
	int cur_id = start ;
	FOR_EACH_FACET(Map, map, it) {
		id[it] = cur_id ;
		cur_id += step ;
	}
	return (cur_id - start) / step ;
}

int MapEnumerator::enumerate_tex_vertices(Map* map, Attribute<Map::TexVertex, int>& id, int start, int step) 
{
	ogf_assert(start >= 0) ;
	ogf_assert(step > 0) ;
	ogf_assert(
		id.attribute_manager() == map->tex_vertex_attribute_manager()
		) ;

	{ FOR_EACH_HALFEDGE(Map, map, it) {
		id[it->tex_vertex()] = -1 ;
	}}
	int cur_id = start ;
	{ FOR_EACH_HALFEDGE(Map, map, it) {
		if(id[it->tex_vertex()] == -1) {
			id[it->tex_vertex()] = cur_id ;
			cur_id += step ;
		}
	}}
	return (cur_id - start) / step ;
}


static void propagate_connected_component(
	Map* map, Attribute<Map::Vertex, int>& id,
	Map::Vertex* v, int cur_id
	) 
{
	std::stack<Map::Vertex*> stack ;
	stack.push(v) ;

	while(!stack.empty()) {
		Map::Vertex* top = stack.top() ;
		stack.pop() ;
		if(id[top] == -1) {
			id[top] = cur_id ;
			Map::Halfedge* it = top->halfedge() ;
			do {
				Map::Vertex* cur = it->opposite()->vertex() ;
				if(id[cur] == -1) {
					stack.push(cur) ;
				}
				it = it->next_around_vertex() ;
			} while(it != top->halfedge()) ;
		}
	}
}



int MapEnumerator::enumerate_connected_components(Map* map, Attribute<Map::Vertex, int>& id)
{
	ogf_assert(
		id.attribute_manager() == map->vertex_attribute_manager()
		) ;
	{ FOR_EACH_VERTEX(Map,map,it) {
		id[it] = -1 ;
	}}
	int cur_id = 0 ;
	{ FOR_EACH_VERTEX(Map,map,it) {
		if(id[it] == -1) {
			propagate_connected_component(map, id, it, cur_id) ;
			cur_id++ ;
		}
	}}
	return cur_id ;
}


static void propagate_connected_component(
	Map* map, Attribute<Map::Facet, int>& id,
	Map::Facet* f, int cur_id
	) 
{
	std::stack<Map::Facet*> stack ;
	stack.push(f) ;

	while(!stack.empty()) {
		Map::Facet* top = stack.top() ;
		stack.pop() ;
		if(id[top] == -1) {
			id[top] = cur_id ;
			Map::Halfedge* it = top->halfedge() ;
			do {
				Map::Facet* cur = it->opposite()->facet() ;
				if(cur != nil && id[cur] == -1) {
					stack.push(cur) ;
				}
				it = it->next() ;
			} while(it != top->halfedge()) ;
		}
	}
}


int MapEnumerator::enumerate_connected_components(Map* map, Attribute<Map::Facet, int>& id)
{
	ogf_assert(id.attribute_manager() == map->facet_attribute_manager()) ;
	{ FOR_EACH_FACET(Map,map,it) {
		id[it] = -1 ;
	}}
	int cur_id = 0 ;
	{ FOR_EACH_FACET(Map,map,it) {
		if(id[it] == -1) {
			propagate_connected_component(map, id, it, cur_id) ;
			cur_id++ ;
		}
	}}
	return cur_id ;
}
