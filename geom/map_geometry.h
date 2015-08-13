#ifndef _GEOM_MAP_GEOMETRY_H_
#define _GEOM_MAP_GEOMETRY_H_

#include "geom_common.h"
#include "map.h"
#include "../math/oriented_line.h"



class GEOM_API MapNormalizer {
public:
	MapNormalizer(Map* map) ;
	void apply(double normalized_radius = 1.0) ;
	void unapply() ;
	void normalize_tex_coords() ;

private:
	Map* map_ ;
	vec3 center_ ;
	double radius_ ;
	double normalized_radius_ ;
} ;

// Adds some functions related to Map to the Geom namespace.
namespace Geom {

	inline vec3 vector(const Map::Halfedge* h) {
		return h->vertex()->point() - h->prev()->vertex()->point();
	}
	inline vec2 vector2d(const Map::Halfedge* h){
		return h->tex_coord() - h->prev()->tex_coord();
	}

	inline Segment3d segment(const Map::Halfedge* h) {
		return Segment3d(h->vertex()->point(), h->prev()->vertex()->point());
	}
	inline Segment2d segment2d(const Map::Halfedge* h) {
		return Segment2d(h->tex_coord(), h->prev()->tex_coord());
	}

	GEOM_API vec3 facet_normal(const Map::Facet* f) ; 
	GEOM_API vec3 vertex_normal(const Map::Vertex* v) ;
	GEOM_API vec3 triangle_normal(const Map::Facet* f) ;

	// I assume the facet is planar
	GEOM_API Plane3d	facet_plane(const Map::Facet* f) ;

	/**
	* returns the barycenter of all TexVertices
	* associated with v.
	*/
	vec2 GEOM_API vertex_barycenter2d(const Map::Vertex* v) ;

	inline vec3 edge_barycenter(const Map::Halfedge* h) {
		return barycenter(
			h->vertex()->point(),
			h->opposite()->vertex()->point()
			) ;
	}


	inline vec2 edge_barycenter2d(const Map::Halfedge* h) {
		return barycenter(
			h->tex_coord(),
			h->prev()->tex_coord()
			) ;
	}

	inline vec3 line_plane_intersection(const vec3& L0, const vec3& L1, const vec3& P0, const vec3& P1, const vec3& P2)
	{
		vec3 N = cross(P1-P0, P2-P0);
		double r = dot(N, L0-P0)/dot(N, L1-L0);
		return L0+r*(L1-L0);
	}

	vec3 GEOM_API facet_barycenter(const Map::Facet* f) ;
	vec2 GEOM_API facet_barycenter2d(const Map::Facet* f) ;


	double GEOM_API facet_area(const Map::Facet* f) ;
	double GEOM_API facet_signed_area2d(const Map::Facet* f) ;

	inline double facet_area2d(const Map::Facet* f) {
		return ::fabs(facet_signed_area2d(f)) ;
	}

	double GEOM_API border_signed_area2d(const Map::Halfedge* h) ;


	inline double border_area2d(const Map::Halfedge* h) {
		return ::fabs(border_signed_area2d(h)) ;
	}

	inline double edge_length(const Map::Halfedge* h) {
		return length(vector(h)) ;
	}

	inline double edge_length2d(const Map::Halfedge* h) {
		return length(vector2d(h)) ;
	}

	// average edge length around $v$
	inline double average_edge_length(const Map::Vertex* v) {
		Map::Halfedge* cir = v->halfedge();
		unsigned int count = 0 ;
		double total_len = 0;
		do {
			total_len += edge_length(cir);
			count++ ;
			cir = cir->next_around_vertex() ;
		} while (cir != v->halfedge());
		return total_len / count;
	}

	inline double average_edge_length(const Map::Facet* f) {
		Map::Halfedge* cir = f->halfedge();
		unsigned int count = 0 ;
		double total_len = 0;
		do {
			total_len += edge_length(cir);
			count++ ;
			cir = cir->next() ;
		} while (cir != f->halfedge());
		return total_len / count;
	}

	inline double triangle_area(const Map::Facet* f) {
		ogf_assert(f->is_triangle()) ;
		Map::Halfedge* h1 = f->halfedge() ;
		Map::Halfedge* h2 = h1->next() ;
		Map::Halfedge* h3 = h2->next() ;            
		return triangle_area(
			h1->vertex()->point(),
			h2->vertex()->point(),
			h3->vertex()->point()
			) ;
	}

	inline double triangle_area2d(const Map::Facet* f) {
		ogf_assert(f->is_triangle()) ;
		Map::Halfedge* h1 = f->halfedge() ;
		Map::Halfedge* h2 = h1->next() ;
		Map::Halfedge* h3 = h2->next() ;            
		return triangle_area(
			h1->tex_coord(),
			h2->tex_coord(),
			h3->tex_coord()
			) ;
	}

	/**
	* Note: I am not sure of this one for
	* non-convex facets.
	*/
	bool GEOM_API line_intersects_facet(
		const OrientedLine& line,
		const Map::Facet* f
		) ;

	double GEOM_API border_length(Map::Halfedge* start) ;
	double GEOM_API border_length2d(Map::Halfedge* start) ;

	double GEOM_API map_area(const Map* map) ;
	double GEOM_API map_area2d(const Map* map) ;

	Box3d GEOM_API bounding_box(const Map* map) ;
	Box2d GEOM_API bounding_box2d(const Map* map) ;

	void  GEOM_API normalize_map_tex_coords(Map* map) ;

	Box3d GEOM_API border_bbox(Map::Halfedge* h) ;
	Box2d GEOM_API border_bbox2d(Map::Halfedge* h) ;

	GEOM_API Map*  duplicate(const Map* map);

	GEOM_API Map*  merge(const std::vector<Map*>& maps);
	GEOM_API void  merge_into_source(Map* source, Map* another);
}


#endif

