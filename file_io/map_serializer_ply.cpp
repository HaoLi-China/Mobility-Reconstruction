
#include "map_serializer_ply.h"
#include "rply.h"
#include "../geom/map_builder.h"
#include "../basic/logger.h"
#include "../geom/map_enumerator.h"


//_________________________________________________________

class PlyMeshLoad {
public:
	PlyMeshLoad(AbstractMapBuilder& builder) : builder_(builder) { } 
	bool load(const std::string& filename) {
		p_ply ply = ply_open(filename.c_str(), nil, 0, nil) ;

		if(ply == nil) {
			Logger::err("PlyMeshLoad") << filename << ": could not open" << std::endl ;
			return false ;
		}

		if(!ply_read_header(ply)) {
			Logger::err("PlyMeshLoad") << filename << ": invalid PLY file" << std::endl ;
			ply_close(ply) ;
			return false ;
		}

		current_vertex_ = 0 ;
		current_color_  = 0 ;
		check_for_colors(ply) ;

		long nvertices = ply_set_read_cb(ply, "vertex", "x", PlyMeshLoad::vertex_cb, this, 0) ;
		ply_set_read_cb(ply, "vertex", "y", PlyMeshLoad::vertex_cb, this, 1) ;
		ply_set_read_cb(ply, "vertex", "z", PlyMeshLoad::vertex_cb, this, 2) ;

		long nfaces = ply_set_read_cb(ply, "face", "vertex_indices", PlyMeshLoad::face_cb, this, 0);
		if (nfaces == 0) {
			Logger::err("PlyMeshLoad") 
				<< "0 facet, maybe a point cloud file" << std::endl ;
			ply_close(ply) ;
			return false ;
		}

		ply_set_read_cb(ply, "tristrips", "vertex_indices", PlyMeshLoad::tristrip_cb, this, 0);

		builder_.begin_surface() ;
		builder_.create_vertices(nvertices, has_colors_) ;

		if(!ply_read(ply)) {
			Logger::err("PlyMeshLoad") 
				<< filename << ": problem occurred while parsing PLY file" << std::endl ;
			ply_close(ply) ;
			builder_.end_surface() ;
			return false ;
		}

		ply_close(ply) ;
		builder_.end_surface() ;

		if (nfaces == 0)
			return false;
		else
			return true ;
	}

protected:
	void check_for_colors(p_ply ply) {
		p_ply_element element = nil ;

		bool has_r     = false ;
		bool has_g     = false ;
		bool has_b     = false ;

		bool has_red   = false ;
		bool has_green = false ;
		bool has_blue  = false ;

		for(;;) {
			element = ply_get_next_element(ply, element) ;
			if(element == nil) { break ; }
			const char* elt_name = nil ;
			ply_get_element_info(element, &elt_name, nil) ;

			if(!strcmp(elt_name, "vertex")) {
				p_ply_property prop = nil ;
				for(;;) {
					prop = ply_get_next_property(element, prop) ;
					if(prop == nil) { break ; }
					const char* prop_name = nil ;
					ply_get_property_info(prop, &prop_name, nil, nil, nil) ;
					has_r = has_r || !strcmp(prop_name, "r") ;
					has_g = has_g || !strcmp(prop_name, "g") ;
					has_b = has_b || !strcmp(prop_name, "b") ;
					has_red   = has_red   || !strcmp(prop_name, "red") ;
					has_green = has_green || !strcmp(prop_name, "green") ;
					has_blue  = has_blue  || !strcmp(prop_name, "blue") ;
				}
			} 
		}

		if(has_r && has_g && has_b) {
			has_colors_ = true ;
			color_mult_ = 1.0 ;
			ply_set_read_cb(ply, "vertex", "r", PlyMeshLoad::color_cb, this, 0) ;
			ply_set_read_cb(ply, "vertex", "g", PlyMeshLoad::color_cb, this, 1) ;
			ply_set_read_cb(ply, "vertex", "b", PlyMeshLoad::color_cb, this, 2) ;
		} else if(has_red && has_green && has_blue) {
			has_colors_ = true ;
			color_mult_ = 1.0 / 255.0 ;
			ply_set_read_cb(ply, "vertex", "red",   PlyMeshLoad::color_cb, this, 0) ;
			ply_set_read_cb(ply, "vertex", "green", PlyMeshLoad::color_cb, this, 1) ;
			ply_set_read_cb(ply, "vertex", "blue",  PlyMeshLoad::color_cb, this, 2) ;
		} else {
			has_colors_ = false ;
		}
	}

	static PlyMeshLoad* plyload(p_ply_argument argument) {
		PlyMeshLoad* result = nil ;
		ply_get_argument_user_data(argument, (void**)(&result), nil) ;
		ogf_debug_assert(result != nil) ;
		return result ;
	}

	static int vertex_cb(p_ply_argument argument) {
		return plyload(argument)->add_vertex_data(argument) ;
	}

	static int face_cb(p_ply_argument argument) {
		return plyload(argument)->add_face_data(argument) ;	    
	}

	static int tristrip_cb(p_ply_argument argument) {
		return plyload(argument)->add_tristrip_data(argument) ;	    
	}

	static int color_cb(p_ply_argument argument) {
		return plyload(argument)->add_color_data(argument) ;
	}

	int add_vertex_data(p_ply_argument argument) {
		long coord ;
		ply_get_argument_user_data(argument, nil, &coord);
		ogf_debug_assert(coord >= 0 && coord < 3) ;
		xyz_[coord] = double(ply_get_argument_value(argument)) ;
		if(coord == 2) { 
			builder_.set_vertex(current_vertex_, vec3(xyz_[0], xyz_[1], xyz_[2])) ;
			current_vertex_++ ; 
		}
		return 1;
	}

	int add_face_data(p_ply_argument argument) {
		long length, value_index;
		ply_get_argument_property(argument, nil, &length, &value_index);
		if(value_index < 0)  
			return 1; 
		int vertex_index = int(ply_get_argument_value(argument)) ;
		if(value_index == 0) {
			builder_.begin_facet() ;
		}
		builder_.add_vertex_to_facet(vertex_index) ;
		if(value_index == length-1) { 
			builder_.end_facet() ; 
		}
		return 1;
	}

	int add_tristrip_data(p_ply_argument argument) {
		long length, value_index;
		ply_get_argument_property(argument, nil, &length, &value_index);
		if(value_index < 0) { return 1; }
		int vertex_index = int(ply_get_argument_value(argument)) ;
		if(value_index == 0)  { begin_tristrip() ; }
		if(vertex_index >= 0) {
			add_to_tristrip(vertex_index) ;
		} else {
			end_tristrip() ; begin_tristrip() ;
		}
		if(value_index == length-1) { end_tristrip() ;   }
		return 1;
	}

	void begin_tristrip() {
		tristrip_p1_ = -1 ;
		tristrip_p2_ = -1 ;
		tristrip_odd_ = false ;
	}

	void end_tristrip() {
	}

	void add_to_tristrip(int vertex_index) {
		if(tristrip_p1_ == -1) { tristrip_p1_ = vertex_index ; return ; }
		if(tristrip_p2_ == -1) { tristrip_p2_ = vertex_index ; return ; }

		builder_.begin_facet() ;
		builder_.add_vertex_to_facet(tristrip_p1_) ;
		builder_.add_vertex_to_facet(tristrip_p2_) ;
		builder_.add_vertex_to_facet(vertex_index) ;
		builder_.end_facet() ;

		if(tristrip_odd_) {
			tristrip_p2_ = vertex_index ;
		} else {
			tristrip_p1_ = vertex_index ;
		}
		tristrip_odd_ = !tristrip_odd_ ;
	}

	int add_color_data(p_ply_argument argument) {
		long coord ;
		ply_get_argument_user_data(argument, nil, &coord);
		ogf_debug_assert(coord >= 0 && coord < 3) ;
		rgb_[coord] = double(ply_get_argument_value(argument)) * color_mult_ ;
		if(coord == 2) { 
			builder_.set_vertex_color(current_color_, Color(float(rgb_[0]), float(rgb_[1]), float(rgb_[2]))) ;
			current_color_++ ; 
		}
		return 1 ;
	}

protected:
	AbstractMapBuilder& builder_ ;

	double xyz_[3] ;
	unsigned int current_vertex_ ;

	bool has_colors_ ;
	double color_mult_ ;
	double rgb_[3] ;
	unsigned int current_color_ ;

	int tristrip_p1_ ;
	int tristrip_p2_ ;
	bool tristrip_odd_ ;
} ;

//__________________________________________________________


MapSerializer_ply::MapSerializer_ply()
{
	read_supported_ = true;
	write_supported_ = true;
}

bool MapSerializer_ply::streams_supported() const {
	return false ;
}

int MapSerializer_ply::num_facet(const std::string& file_name) {
	p_ply ply = ply_open(file_name.c_str(), nil, 0, nil) ;

	if(ply == nil) {
		Logger::err("MapSerializer_ply") << file_name << ": could not open" << std::endl ;
		return false ;
	}

	if(!ply_read_header(ply)) {
		Logger::err("MapSerializer_ply") << file_name << ": invalid PLY file" << std::endl ;
		ply_close(ply) ;
		return false ;
	}

	p_ply_element element = nil ;
	for(;;) {
		element = ply_get_next_element(ply, element) ;
		if(element == nil)
			break ;
		const char* elt_name = nil ;
		long num;
		ply_get_element_info(element, &elt_name, &num) ;

		if(!strcmp(elt_name, "face")) {
			if (num > 0) {
				ply_close(ply) ;
				return num;
			}
		} 
	}
	ply_close(ply) ;
	return 0;
}


class PlyMeshSave {
public:
	PlyMeshSave(const Map* map) 
		: map_(map)
		, color_mult_(255.0)
	{ }

	bool save(const std::string& filename) {
		if(map_ == nil) {
			Logger::err("PlyMeshSave") << "mesh is null" << std::endl ;
			return false ;
		}

		p_ply ply = ply_create(filename.c_str(), PLY_LITTLE_ENDIAN, nil, 0, nil) ;

		if(ply == nil) {
			Logger::err("PlyMeshSave") << filename << ": could not open" << std::endl ;
			return false ;
		}

		//////////////////////////////////////////////////////////////////////////

		if (!ply_add_comment(ply, "saved by liangliang.nan@gmail.com")) {
			Logger::err("PlyMeshSave") << "unable to add comment" << std::endl ;
			ply_close(ply) ;
			return false ;
		}

		int num_v = map_->size_of_vertices();
		if (!ply_add_element(ply, "vertex", num_v)) {
			Logger::err("PlyMeshSave") << "unable to add element \'vertex\'" << std::endl ;
			ply_close(ply) ;
			return false ;
		}
		
		e_ply_type length_type, value_type;
		length_type = value_type = static_cast<e_ply_type>(-1);
		std::string pos[3] = { "x", "y", "z" };
		for (unsigned int i=0; i<3; ++i) {
			if (!ply_add_property(ply, pos[i].c_str(), PLY_FLOAT, length_type, value_type)) {
				Logger::err("PlyMeshSave") << "unable to add property \'" << pos[i] << "\'" << std::endl ;
				ply_close(ply) ;
				return false ;
			}
		}

		MapVertexAttribute<Color> vertex_color;
		vertex_color.bind_if_defined(const_cast<Map*>(map_), "color") ;
		if (vertex_color.is_bound()) {
			std::string color[4] = { "red", "green", "blue", "alpha" };
			for (unsigned int i=0; i<4; ++i) {
				if (!ply_add_property(ply, color[i].c_str(), PLY_UCHAR, length_type, value_type)) {
					Logger::err("PlyMeshSave") << "unable to add property \'" << color[i] << "\'" << std::endl ;
					ply_close(ply) ;
					return false ;
				}
			}
		}
		
		int num_f = map_->size_of_facets();
		if (!ply_add_element(ply, "face", num_f)) {
			Logger::err("PlyMeshSave") << "unable to add element \'face\'" << std::endl ;
			ply_close(ply) ;
			return false ;
		}
		if (!ply_add_property(ply, "vertex_indices", PLY_LIST, PLY_UCHAR, PLY_INT)) {
			Logger::err("PlyMeshSave") << "unable to add property \'vertex_indices\'" << std::endl ;
			ply_close(ply) ;
			return false ;
		}

		if(!ply_write_header(ply)) {
			Logger::err("PlyMeshSave") << filename << ": invalid PLY file" << std::endl ;
			ply_close(ply) ;
			return false ;
		}

		//////////////////////////////////////////////////////////////////////////
		
		FOR_EACH_VERTEX_CONST(Map, map_, it) {
			const vec3& p = it->point();
			ply_write(ply, p.x);
			ply_write(ply, p.y);
			ply_write(ply, p.z);
			if (vertex_color.is_bound()) {
				const Color& c = vertex_color[it];
				double r = c.r() * color_mult_;	ogf_clamp(r, 0.0, 255.0);
				double g = c.g() * color_mult_;	ogf_clamp(g, 0.0, 255.0);
				double b = c.b() * color_mult_;	ogf_clamp(b, 0.0, 255.0);
				double a = c.a() * color_mult_; ogf_clamp(a, 0.0, 255.0);
				ply_write(ply, r);
				ply_write(ply, g);
				ply_write(ply, b);
				ply_write(ply, a);
			}
		}

		// ply files numbering starts with 0
		Attribute<Map::Vertex, int> vertex_id(map_->vertex_attribute_manager());
		MapEnumerator::enumerate_vertices(const_cast<Map*>(map_), vertex_id, 0);
		FOR_EACH_FACET_CONST(Map, map_, it) {
			ply_write(ply, it->nb_vertices());
			Map::Halfedge* h = it->halfedge();
			do 
			{
				int id = vertex_id[h->vertex()];
				ply_write(ply, id);
				h = h->next();
			} while (h != it->halfedge());
		}

		ply_close(ply);
		return true ;
	}

protected:
	const Map*	map_;
	double		color_mult_;
} ;


//////////////////////////////////////////////////////////////////////////

bool MapSerializer_ply::serialize_read(const std::string& file_name, Map* mesh) {
	if (!mesh) {
		Logger::err("MapSerializer") << "mesh is null" << std::endl;
		return false;
	}
	MapBuilder builder(mesh);

	PlyMeshLoad plyload(builder) ;
	return  plyload.load(file_name) ;
}


bool MapSerializer_ply::serialize_write(const std::string& file_name, const Map* mesh) const {
	PlyMeshSave plysave(mesh) ;
	return  plysave.save(file_name) ;
}

