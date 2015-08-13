
#ifndef _GEOM_MAP_EDITOR_EXT_H_
#define _GEOM_MAP_EDITOR_EXT_H_

#include "geom_common.h"
#include "map_editor.h"



class GEOM_API MapEditorExt : public MapEditor  {
public:
	MapEditorExt(Map* m) 
		: MapEditor(m)		
	{}

	bool flip_edge(Map::Halfedge* h) ;
	bool is_flippable(Map::Halfedge* h) ;

	inline vec3 local_plane_coords(
		const vec3& p, const vec3& v0, 
		const vec3& v1, const vec3& plane_n,
		const vec3& plane_origin
		) ;

     /**
     * If a single border arrives at this edge,
     * connects the two corresponding edges.
     */
    bool zip_edge(Map::Vertex* from) ;

    /**
     * Recomputes vertex normals and facet normals
     * (if present) in the neighborhood of v.
     */
    void compute_normals_around_vertex(Map::Vertex* v) ;

    /**
     * Recomputes vertex normals and facet normals
     * (if present) in the neighborhood of f.
     */
    void compute_normals_around_facet(Map::Facet* f) ;

    /**
     * Recomputes vertex normals and facet normals
     * (if present) in the neighborhood of h.
     */
    void compute_normals_around_edge(Map::Halfedge* h) ;
    
    /**
     * Recursively adds edges in facet f until the pieces have
     * no more than max_nb_vertices. Inserted edges minimize their
     * lengthes and try to balance the size of the parts.
     */
    void subdivide_facet(Map::Facet* f, int max_nb_vertices = 3) ;

    /**
     * Creates a quad facet connecting the two specified edges. 
     * Precondition:
	 *		1) both are border edges:  h1->is_border() && h2->is_border()
	 *		2) they are not successive:  h1->next() != h2 && h2->next() != h1
     */
    Map::Facet* create_facet_between_edges(Map::Halfedge* h1, Map::Halfedge* h2) ;
} ;


inline bool MapEditorExt::flip_edge(Map::Halfedge* h) {
	if(!is_flippable(h))
		return false;

	Map::Halfedge* hopp = h->opposite();

	Map::Halfedge* h00 = h->prev();
	Map::Halfedge* h01 = h->next();

	Map::Halfedge* h10 = hopp->next();
	Map::Halfedge* h11 = hopp->prev();

	Map::Facet* f0 = h->facet();
	Map::Facet* f1 = hopp->facet();

	make_sequence(h, h11);
	make_sequence(h11, h01);
	make_sequence(h01, h);

	make_sequence(hopp, h00);
	make_sequence(h00, h10);
	make_sequence(h10, hopp);

	set_facet_on_orbit(h, f0);
	make_facet_key(h);

	set_facet_on_orbit(hopp, f1);
	make_facet_key(hopp);

	make_vertex_key(h, h10->vertex());
	make_vertex_key(hopp, h01->vertex());

	make_vertex_key(h00);
	make_vertex_key(h10);
	make_vertex_key(h01);
	make_vertex_key(h11);

	return true;
}

inline bool MapEditorExt::is_flippable(Map::Halfedge* h) {
	if (h->is_border_edge()) {
		return false;
	}
	if(!h->facet()->is_triangle()) {
		return false ;
	}
	if(!h->opposite()->facet()->is_triangle()) {
		return false ;
	}
	// check if the flipped edge is already present in the mesh
	if (h->next()->vertex()->is_connected(h->opposite()->next()->vertex())) {
		return false;
	}

	// the two edges involved in the flip
	vec3 plane_v0 = normalize(h->vertex()->point() - h->opposite()->vertex()->point());
	vec3 plane_v1 = normalize(h->opposite()->next()->vertex()->point() - h->next()->vertex()->point());		

	// the plane defined by the two edges
	vec3 plane_n = normalize(cross(plane_v1, plane_v0));

	// orthogonalize in-plane vectors
	plane_v0 = normalize(plane_v0 - dot(plane_v0, plane_v1) * plane_v1);
	vec3 plane_origin = h->next()->vertex()->point();

	// 2d coordinates in plane
	vec3 local_t = local_plane_coords(h->vertex()->point(), plane_v0, plane_v1, plane_n, plane_origin);
	vec3 local_b = local_plane_coords(h->opposite()->vertex()->point(), plane_v0, plane_v1, plane_n, plane_origin);
	vec3 local_l = local_plane_coords(h->next()->vertex()->point(), plane_v0, plane_v1, plane_n, plane_origin);
	vec3 local_r = local_plane_coords(h->opposite()->next()->vertex()->point(), plane_v0, plane_v1, plane_n, plane_origin);


	// check if edge intersections lies inside triangles pair (in plane)

	vec3 tb = local_t - local_b;
	vec3 lr = local_l - local_r;

	double ntb[2];
	double ctb;

	double nlr[2];
	double clr;

	ntb[0] = - tb[1];
	ntb[1] = tb[0];

	ctb = -(ntb[0] * local_t[0] + ntb[1] * local_t[1]);


	nlr[0] = - lr[1];
	nlr[1] = lr[0];

	clr = -(nlr[0] * local_l[0] + nlr[1] * local_l[1]);


	double det = ntb[0] * nlr[1] - nlr[0] * ntb[1];

	vec3 intersection(- (nlr[1] * ctb - ntb[1] * clr) / det, 
		- (-nlr[0] * ctb + ntb[0] * clr) / det, 
		0.0);


	double l0 = dot(intersection - local_r, lr) / dot(lr, lr);
	double l1 = dot(intersection - local_b, tb) / dot(tb, tb);

	return l0 > 0.0 && l0 < 1.0 && l1 > 0.0 && l1 < 1.0;
}

vec3 MapEditorExt::local_plane_coords(
	const vec3& p, const vec3& v0, 
	const vec3& v1, const vec3& plane_n,
	const vec3& plane_origin
	) 
{
	vec3 q(p + dot(plane_origin - p, plane_n) * plane_n);
	return vec3( dot(q - plane_origin, v0), dot(q - plane_origin, v1), 0.0);
}



#endif

