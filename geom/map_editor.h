
#ifndef _GEOM_MAP_EDITOR_H_
#define _GEOM_MAP_EDITOR_H_

#include "geom_common.h"
#include "map.h"
#include "map_attributes.h"


class MapComponent ;

class GEOM_API MapEditor : public MapMutator {
public:
	MapEditor(Map* target = nil) ; 
	virtual void set_target(Map* target) ;

	// _________________ CGAL interface __________

	Halfedge* make_polygon(int nb) ;
	Halfedge* make_triangle() ;
	Halfedge* make_triangle(const vec3& p1, const vec3& p2, const vec3& p3) ;

	/** 
	* destroys the facet referred two by h.
	* precondition: !h->is_border() 
	*/
	void make_hole(Halfedge* h) ;

	/**
	* fills a hole (i.e. a border).
	* precondition: h->is_border()
	*/
	Halfedge* fill_hole(Halfedge* h) ;
	void fill_holes(int max_size = -1) ;

	Halfedge* add_vertex_and_facet_to_border(Halfedge* h, Halfedge* g) ;
	Halfedge* add_facet_to_border(Halfedge* h, Halfedge* g) ;

	bool can_split_facet(Halfedge* h, Halfedge* g) ;
	bool split_facet(Halfedge* h, Halfedge* g) ;

	bool can_join_facets(Halfedge* h) ;
	bool join_facets(Halfedge* h) ;
 
	// return the new vertex, nil if fail
	Vertex* create_center_vertex(Facet* f) ;
	// return the new facet, nil if fail
	Facet*  erase_center_vertex(Vertex* v) ;

	/**
	* Insert a new vertex in the edge referred to by h.
	* If triangulate is set to true, triangulate the affected facets.
	*/
	Vertex* split_edge(Halfedge* h, bool triangulate = false, double alpha = 0.5) ;
	bool can_join_edges(Vertex* v) ;
	bool join_edges(Vertex* v) ;

	void erase_facet(Halfedge* h) ;
	void erase_connected_component(Halfedge* h) ;

	// _________________ Additional functions ____      

	/**
	* Checks wether the two specified half-edges can be glued,
	* from a topological point of view. h0 and h1 should point 
	* in reverse direction, and should be on the
	* border. TODO: check if it can work for polygons.
	*/
	bool can_glue(Halfedge* h0, Halfedge* h1) ;

	/**
	* h0 and h1 should point in reversed direction, and 
	* should be on the border. 
	* @param do_merge_vertices if set to false, the operation does not
	*   affect the TexVertices attached to the concerned Vertices.
	*/
	bool glue(
		Halfedge* h0, Halfedge* h1, 
		bool do_merge_tex_vertices = true
		) ;


	/**
	* It is not allowed to collapse an edge of a
	*  triangle that has its two other edges on the border.
	*/
	// TODO: check if it works for polygons.
	bool can_collapse_edge(Halfedge* h) ;

	bool collapse_edge(Halfedge* h) ;

	/**
	* An edge on the border cannot be unglued. One of the
	* extremities of the edge should be on the border.
	*/
	bool can_unglue(Halfedge* h) ;

	/**
	* Separates the two halfedges indicated by h, and
	* generates new vertices as needed.
	* If h is already on the border, does nothing.
	*/
	bool unglue(Halfedge* h, bool check = true) ;

	/** Checks whether a Vertex has several TexVertices */
	bool vertex_is_split(const Vertex* v) ;

	/** Replace all TexVertices linked with a Vertex with a single one */
	void merge_tex_vertices(const Vertex* v) ;
	void merge_all_tex_vertices() ;

	/** creates one tex vertex per corner */
	void split_all_tex_vertices() ;

	/** 
	* Replace all TexVertices linked with two Vertices with 
	* a single one 
	*/
	void merge_tex_vertices(const Vertex* v, const Vertex* v2) ;

	/**
	* inverts the orientation of all the facets.
	*/
	void inside_out(bool reorient_normal = true) ;

	/**
	* inverts the orientation of all the facets of a component.
	*/
	void inside_out(MapComponent* comp, bool reorient_normal = true) ;


	//_____________ triangulation _________________

	/** 
	* Triangulates a facet by creating triangles
	* radiating from start->vertex(). 
	* Note that the facets should be convex.
	*/
	void naive_triangulate_facet(Halfedge* start) ;

	/**
	* triangulates all the facets, by radiating around one of the 
	* vertices of each facet. Note that the facets should be convex.
	*/
	void naive_triangulation();

	/**
	* triangulates all the facets by inserting a vertex in the center
	* of the each facet. 
	*/
	void center_triangulation();


	//_____________ copy attributes _______________

	void copy_attributes(Vertex* to, Vertex* from) ;
	void copy_attributes(Halfedge* to, Halfedge* from) ;
	void copy_attributes(Facet* to, Facet* from) ;
	void copy_attributes(TexVertex* to, TexVertex* from) ;

protected:

	//_________________ utilities ____________________


	/**
	* Removes faces having only two edges.
	* @param f0 is an interior halfedge of the face to be removed.
	*/
	void remove_null_face(Halfedge* f0) ;

	/**
	* Checks wheter the vertices pointed by h0 and h1 can be
	* merged. It is called twice by can_glue(), once per 
	* orientation of the edges.
	*/
	bool can_merge_vertices(Halfedge* h0, Halfedge* h1) ;

	/**
	* To be explained by Nico.
	* Note: should be called with both (h0,h1) and with (h1,h0)
	*/
	bool orbits_are_compatible(
		Halfedge* h0, Halfedge* h1
		) ;

	/**
	* Checks the existence of an half_edge e such that
	* e->vertex() = v1 and e->opposite()->vertex() = v2
	*/
	bool halfedge_exists_between_vertices(
		Vertex* v1, Vertex* v2
		) ;

	bool halfedges_on_same_vertex(
		Halfedge* h1, Halfedge* h2
		) ;

	bool halfedges_on_same_facet(
		Halfedge* h1, Halfedge* h2
		) ;

	void reorient_facet(Map::Halfedge* first) ;

protected:
	MapFacetAttribute<bool> border_facet_ ;  
	MapVertexNormal			vertex_normal_ ;
	MapVertexLock			is_locked_ ; 
} ;



#endif

