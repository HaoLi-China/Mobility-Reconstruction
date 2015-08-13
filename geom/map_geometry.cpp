
#include "map_geometry.h"
#include "map_attributes.h"
#include "map_copier.h"
#include "map_builder.h"


MapNormalizer::MapNormalizer(Map* map) {
	map_ = map ;
	center_ = vec3(0,0,0) ;
	double num = 0 ;
	{ FOR_EACH_VERTEX(Map, map_, it) {
		vec3 p = it-> point() ;
		center_ = vec3(
			center_.x + p.x,
			center_.y + p.y,
			center_.z + p.z
			) ;
		num++ ;
	}}
	center_ = vec3(
		center_.x / num,
		center_.y / num,
		center_.z / num
		) ;

	radius_ = 0 ;
	{ FOR_EACH_VERTEX(Map, map_, it) {
		vec3 v = it->point() - center_ ;
		radius_ = ogf_max(radius_, length(v)) ;
	}}

	normalized_radius_ = 1.0 ;
}    

void MapNormalizer::apply(double normalized_radius) {
	normalized_radius_ = normalized_radius ;
	{ FOR_EACH_VERTEX(Map, map_, it) {
		vec3 v = it->point() - center_ ;
		v = (normalized_radius_ / radius_) * v ;
		it->set_point(v) ;
	}}
}

void MapNormalizer::unapply() {
	{ FOR_EACH_VERTEX(Map, map_, it) {
		const vec3& p = it->point() ;
		vec3 v = p ;
		v = (radius_ / normalized_radius_) * v ;
		it->set_point(center_ + v) ;
	}}
}

void MapNormalizer::normalize_tex_coords() {
	double u_min =  Numeric::big_double ;
	double v_min =  Numeric::big_double ;
	double u_max = -Numeric::big_double ;
	double v_max = -Numeric::big_double ;

	{ FOR_EACH_VERTEX(Map, map_, it) {
		double u = it->halfedge()->tex_coord().x ;
		double v = it->halfedge()->tex_coord().y ;
		u_min = ::ogf_min(u_min, u) ;
		v_min = ::ogf_min(v_min, v) ;
		u_max = ::ogf_max(u_max, u) ;
		v_max = ::ogf_max(v_max, v) ;
	}} 

	if((u_max - u_min > 1e-6) && (v_max - v_min > 1e-6)) {
		double delta = ::ogf_max(u_max - u_min, v_max - v_min) ;
		{ FOR_EACH_VERTEX(Map, map_, it) {
			double u = it->halfedge()->tex_coord().x ;
			double v = it->halfedge()->tex_coord().y ;
			u = (u - u_min) / delta ;
			v = (v - v_min) / delta ;
			it->halfedge()->set_tex_coord(::vec2(u,v)) ;
		}}
	}
}

//________________________________________________________________________

namespace Geom {

	vec3 facet_normal(const Map::Facet* f) {
		vec3 result(0,0,0) ;
		Map::Halfedge* cir = f->halfedge();
		do {
			vec3 v0 = vector(cir) ;
			vec3 v1 = vector(cir->prev()->opposite()) ;
			vec3 n = cross(v0, v1) ;
			result = result + n ;
			cir = cir->next() ;
		} while(cir != f->halfedge()) ;
		result = normalize(result) ;
		return result ;
	}


	vec3 vertex_normal(const Map::Vertex* v) {
		vec3 result(0,0,0) ;
		Map::Halfedge* cir = v->halfedge();
		unsigned int count = 0 ;
		do {
			if (!cir->is_border()) {
				count++ ;
				vec3 v0 = vector(cir->next()) ;
				vec3 v1 = vector(cir->opposite());
				vec3 n = cross(v0, v1) ;
				result = result + n ;
			}
			cir = cir->next_around_vertex() ;
		} while (cir != v->halfedge());
		result = normalize(result);
		return result;
	}


	vec3 triangle_normal(const Map::Facet* f){
		ogf_assert(f->is_triangle()) ;
		vec3 result = (
			cross(vector(f->halfedge()->next()),
			vector(f->halfedge()->opposite()))
			) + (
			cross(vector(f->halfedge()),
			vector(f->halfedge()->prev()->opposite()))
			) + ( 
			cross(vector(f->halfedge()->next()->next()),
			vector(f->halfedge()->next()->opposite()))
			) ;
		result = normalize(result);
		return result;
	}

	Plane3d facet_plane(const Map::Facet* f) {
		return Plane3d(
			f->halfedge()->vertex()->point(), 
			f->halfedge()->next()->vertex()->point(), 
			f->halfedge()->next()->next()->vertex()->point()
			);
	}

	vec2 vertex_barycenter2d(const Map::Vertex* v) {
		double x = 0 ;
		double y = 0 ;
		double nb = 0 ;
		Map::Halfedge* it = v->halfedge() ;
		do {
			const vec2& p = it->tex_coord() ;
			x += p.x ;
			y += p.y ;
			nb++ ;
			it = it->next_around_vertex() ;
		} while(it != v->halfedge()) ;
		return vec2(x/nb, y/nb) ;
	}

	vec3 facet_barycenter(const Map::Facet* f){
		double x=0 ; double y=0 ; double z=0 ;
		int nb_vertex = 0 ;
		Map::Halfedge* cir = f->halfedge();
		do {
			nb_vertex++;
			x+= cir->vertex()->point().x ;
			y+= cir->vertex()->point().y ;
			z+= cir->vertex()->point().z ;
			cir = cir->next();
		} while (cir != f->halfedge());
		return  vec3(
			x / double(nb_vertex),
			y / double(nb_vertex),
			z / double(nb_vertex)
			) ;
	}


	vec2 facet_barycenter2d(const Map::Facet* f){
		double x=0 ; double y=0 ; 
		int nb_vertex = 0 ;
		Map::Halfedge* cir = f->halfedge();
		do {
			nb_vertex++;
			x+= cir->tex_coord().x ;
			y+= cir->tex_coord().y ;
			cir = cir->next();
		} while (cir != f->halfedge());
		return vec2(
			x / double(nb_vertex),
			y / double(nb_vertex)
			) ;
	}


	/*
	// I do not trust this one for the moment ...
	double facet_area(const Map::Facet* f) {
	vec3 n = facet_normal(f) ;
	vec3 w(0,0,0) ;
	Map::Halfedge* it = f->halfedge() ;
	do {
	vec3 v1(
	it-> vertex()-> point().x,
	it-> vertex()-> point().y,
	it-> vertex()-> point().z
	) ;
	vec3 v2(
	it-> next()-> vertex()-> point().x,
	it-> next()-> vertex()-> point().y,
	it-> next()-> vertex()-> point().z
	) ;
	w = w + (v1 ^ v2) ;
	it = it->next() ;
	} while(it != f->halfedge()) ;
	return 0.5 * ::fabs(w * n) ;
	}
	*/

	double facet_area(const Map::Facet* f) {
		double result = 0 ;
		Map::Halfedge* h = f->halfedge() ;
		const vec3& p = h->vertex()->point() ;
		h = h->next() ;
		do {
			result += triangle_area(
				p,
				h->vertex()->point(),
				h->next()->vertex()->point() 
				) ;
			h = h->next() ;
		} while(h != f->halfedge()) ;
		return result ;
	}



	double facet_signed_area2d(const Map::Facet* f) {
		double result = 0 ;
		Map::Halfedge* it = f->halfedge() ;
		do {
			const vec2& t1 = it-> tex_coord() ;
			const vec2& t2 = it-> next()-> tex_coord() ;
			result += t1.x * t2.y - t2.x * t1.y ;
			it = it->next() ;
		} while(it != f->halfedge()) ;
		result /= 2.0 ;
		return result ;
	}


	/*
	double facet_signed_area2d(const Map::Facet* f) {
	double result = 0 ;
	Map::Halfedge* h = f->halfedge() ;
	const vec2& p = h->tex_coord() ;
	h = h->next() ;
	do {
	result += triangle_signed_area(
	p,
	h->tex_coord(),
	h->next()->tex_coord()
	) ;
	h = h->next() ;
	} while(h != f->halfedge()) ;
	return result ;
	}
	*/


	double border_signed_area2d(const Map::Halfedge* h) {
		ogf_assert(h->is_border()) ;
		double result = 0 ;
		const Map::Halfedge* it = h ;
		do {
			const vec2& t1 = it-> tex_coord() ;
			const vec2& t2 = it-> next()-> tex_coord() ;
			result += t1.x * t2.y - t2.x * t1.y ;
			it = it->next() ;
		} while(it != h) ;
		result /= 2.0 ;
		return result ;
	}


	bool line_intersects_facet(
		const OrientedLine& line,
		const Map::Facet* f
		) {
			// Uses Plucker coordinates (see OrientedLine)
			Sign face_sign = ZERO ;
			Map::Halfedge* h = f->halfedge() ;
			do {
				OrientedLine edge_line(
					h->vertex()->point(), h->opposite()->vertex()->point()
					) ;
				Sign cur_sign = OrientedLine::side(line, edge_line) ;
				if(
					cur_sign != ZERO && face_sign != ZERO &&
					cur_sign != face_sign
					) {
						return false ;
				}
				if(cur_sign != ZERO) {
					face_sign = cur_sign ;
				}
				h = h->next() ;
			} while(h != f->halfedge()) ;
			return true ;
	}

	double border_length(Map::Halfedge* start) {
		ogf_assert(start->is_border()) ;
		double result = 0 ;
		Map::Halfedge* cur = start ;
		do {
			result += edge_length(cur) ;
			cur = cur->next() ;
		} while(cur != start) ;
		return result ;
	}

	double border_length2d(Map::Halfedge* start) {
		ogf_assert(start->is_border()) ;
		double result = 0 ;
		Map::Halfedge* cur = start ;
		do {
			result += edge_length2d(cur) ;
			cur = cur->next() ;
		} while(cur != start) ;
		return result ;
	}

	double map_area(const Map* map) {
		double result = 0 ;
		FOR_EACH_FACET_CONST(Map, map, it) {
			result += facet_area(it) ;
		}
		return result ;
	}

	double map_area2d(const Map* map) {
		double result = 0 ;
		FOR_EACH_FACET_CONST(Map, map, it) {
			result += facet_area2d(it) ;
		}
		return result ;
	}

	Box3d bounding_box(const Map* map) {
		ogf_assert(map->size_of_vertices() > 0);
		Box3d result ;
		FOR_EACH_VERTEX_CONST(Map, map, it) {
			result.add_point(it->point()) ;
		}
		return result ;
	}

	Box2d bounding_box2d(const Map* map) {
		Box2d result ;
		FOR_EACH_HALFEDGE_CONST(Map, map, it) {
			result.add_point(it->tex_coord()) ;
		}
		return result ;
	}

	Box3d border_bbox(Map::Halfedge* h) {
		Box3d result ;
		Map::Halfedge* cur = h ;
		do {
			result.add_point(cur->vertex()->point()) ;
			cur = cur->next() ;
		} while(cur != h) ;
		return result ;
	}

	Box2d border_bbox2d(Map::Halfedge* h) {
		Box2d result ;
		Map::Halfedge* cur = h ;
		do {
			result.add_point(cur->tex_coord()) ;
			cur = cur->next() ;
		} while(cur != h) ;
		return result ;
	}


	void normalize_map_tex_coords(Map* map) {
		if(map->size_of_vertices() == 0) {
			return ;
		}
		Box2d B = bounding_box2d(map) ;
		double sx = (B.width() > 0) ? 1.0/B.width() : 0.0 ;
		double sy = (B.height() > 0) ? 1.0/B.height() : 0.0 ;
		std::set<Map::TexVertex*> tex_vertices ;
		FOR_EACH_HALFEDGE(Map, map, it) {
			tex_vertices.insert(it->tex_vertex()) ;
		}
		for(std::set<Map::TexVertex*>::iterator it = tex_vertices.begin(); it != tex_vertices.end(); it++) {
			Map::TexVertex* cur = *it ;
			double x = cur->tex_coord().x ;
			double y = cur->tex_coord().y ;
			cur->set_tex_coord(
				vec2(
				sx * (x - B.x_min()),
				sy * (y - B.y_min())
				)
				) ;
		}
	}

	Map* duplicate(const Map* map) {
		if (!map)
			return nil;

		Map* result = new Map;
		if(result != nil) {
			MapBuilder builder(result) ;
			MapCopier copier ;
			copier.set_copy_all_attributes(true) ;
			builder.begin_surface() ;
			copier.copy(builder, const_cast<Map*>(map)) ;
			builder.end_surface() ;
		}
		return result ;
	}


	Map* merge(const std::vector<Map*>& maps) {
		if (maps.empty())
			return nil;

		MapCopier copier ;
		copier.set_copy_all_attributes(true) ;

		Map* mesh = new Map;
		MapBuilder builder(mesh);

		builder.begin_surface() ;
		int cur_vertex_id = 0 ;
		int cur_tex_vertex_id = 0 ;
		for(unsigned int i=0; i<maps.size(); ++i) {
			Map* cur = maps[i];
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

		MapCopier copier ;
		copier.set_copy_all_attributes(true) ;

		MapBuilder builder(source);

		builder.begin_surface() ;
		int cur_vertex_id = 0 ;
		int cur_tex_vertex_id = 0 ;
		MapVertexAttribute<int>		vertex_id(another) ;
		MapTexVertexAttribute<int>	tex_vertex_id(another) ;
		copier.copy(builder, another, vertex_id, tex_vertex_id, cur_vertex_id, cur_tex_vertex_id) ;

		builder.end_surface() ;
	}
}
