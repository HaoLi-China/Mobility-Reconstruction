
#include "point_set_serializer_ply.h"
#include "rply.h"
#include "../basic/logger.h"
#include "../basic/basic_types.h"
#include "../geom/point_set.h"
#include "../geom/iterators.h"
#include "../basic/logger.h"
#include "../image/color.h"
#include <cassert>



class PlyPointSetLoad 
{
public:
	PointSet* load(const std::string& filename) {
		p_ply ply = ply_open(filename.c_str(), nil, 0, nil) ;
		if(ply == nil) {
			Logger::err("PlyPointSetLoad") << filename << ": could not open" << std::endl;
			return nil ;
		}

		if(!ply_read_header(ply)) {
			Logger::err("PlyPointSetLoad") << filename << ": invalid PLY file" << std::endl;
			ply_close(ply) ;
			return nil ;
		}


		point_set_ = new PointSet;

		current_vertex_ = 0 ;
		current_color_  = 0 ;
		current_normal_ = 0 ;
		check_for_colors_and_normals(ply) ;

		long nvertices = ply_set_read_cb(ply, "vertex", "x", PlyPointSetLoad::vertex_cb, this, 0) ;
		ply_set_read_cb(ply, "vertex", "y", PlyPointSetLoad::vertex_cb, this, 1) ;
		ply_set_read_cb(ply, "vertex", "z", PlyPointSetLoad::vertex_cb, this, 2) ;

		//progress_.reset(nvertices);
		create_vertices(nvertices) ;

		if(!ply_read(ply)) {
			Logger::err("PlyPointSetLoad") 
				<< filename << ": problem occurred while parsing PLY file" << std::endl;
			delete point_set_;
		}

		ply_close(ply) ;
		return end_point_set() ;
	}

protected:
	void check_for_colors_and_normals(p_ply ply) {
		p_ply_element element = nil ;

		bool has_r     = false ;
		bool has_g     = false ;
		bool has_b     = false ;

		bool has_red   = false ;
		bool has_green = false ;
		bool has_blue  = false ;

		bool has_diffuse_red   = false ;
		bool has_diffuse_green = false ;
		bool has_diffuse_blue  = false ;
		
		bool has_normals = false;
		bool has_normals_Neil = false;

		for(;;) {
			element = ply_get_next_element(ply, element) ;
			if(element == nil) { break ; }
			const char* elt_name = nil ;
			ply_get_element_info(element, &elt_name, nil) ;

			if(!strcmp(elt_name, "vertex")) {
				p_ply_property property = nil ;
				for(;;) {
					property = ply_get_next_property(element, property) ;
					if(property == nil) 
						break ;

					const char* prop_name = nil ;
					ply_get_property_info(property, &prop_name, nil, nil, nil) ;
					has_r = has_r || !strcmp(prop_name, "r") ;
					has_g = has_g || !strcmp(prop_name, "g") ;
					has_b = has_b || !strcmp(prop_name, "b") ;
					has_red   = has_red   || !strcmp(prop_name, "red")  ;
					has_green = has_green || !strcmp(prop_name, "green");
					has_blue  = has_blue  || !strcmp(prop_name, "blue") ;

					has_diffuse_red   = has_diffuse_red   || !strcmp(prop_name, "diffuse_red") ;
					has_diffuse_green = has_diffuse_green || !strcmp(prop_name, "diffuse_green") ;
					has_diffuse_blue  = has_diffuse_blue  || !strcmp(prop_name, "diffuse_blue") ;

					has_normals  = has_normals || !strcmp(prop_name, "nx");
					has_normals  = has_normals || !strcmp(prop_name, "ny");
					has_normals  = has_normals || !strcmp(prop_name, "nz");
					has_normals_Neil  = has_normals || !strcmp(prop_name, "vsfm_cnx");  // for Neil Smith's
					has_normals_Neil  = has_normals || !strcmp(prop_name, "vsfm_cny");
					has_normals_Neil  = has_normals || !strcmp(prop_name, "vsfm_cnz");
				}
			} 
		}

		if(has_r && has_g && has_b) {
			has_colors_ = true ;
			color_mult_ = 1.0 ;
			ply_set_read_cb(ply, "vertex", "r", PlyPointSetLoad::color_cb, this, 0) ;
			ply_set_read_cb(ply, "vertex", "g", PlyPointSetLoad::color_cb, this, 1) ;
			ply_set_read_cb(ply, "vertex", "b", PlyPointSetLoad::color_cb, this, 2) ;

		} else if(has_red && has_green && has_blue) {
			has_colors_ = true ;
			color_mult_ = 1.0 / 255.0 ;
			ply_set_read_cb(ply, "vertex", "red",   PlyPointSetLoad::color_cb, this, 0) ;
			ply_set_read_cb(ply, "vertex", "green", PlyPointSetLoad::color_cb, this, 1) ;
			ply_set_read_cb(ply, "vertex", "blue",  PlyPointSetLoad::color_cb, this, 2) ;
		} else if(has_diffuse_red && has_diffuse_green && has_diffuse_blue) {
			has_colors_ = true ;
			color_mult_ = 1.0 / 255.0 ;
			ply_set_read_cb(ply, "vertex", "diffuse_red",   PlyPointSetLoad::color_cb, this, 0) ;
			ply_set_read_cb(ply, "vertex", "diffuse_green", PlyPointSetLoad::color_cb, this, 1) ;
			ply_set_read_cb(ply, "vertex", "diffuse_blue",  PlyPointSetLoad::color_cb, this, 2) ;
		} else {
			has_colors_ = false ;
		}

		has_normals_ = has_normals;
		if (has_normals) {
			has_normals_ = true;
			ply_set_read_cb(ply, "vertex", "nx",   PlyPointSetLoad::normal_cb, this, 0) ;
			ply_set_read_cb(ply, "vertex", "ny",   PlyPointSetLoad::normal_cb, this, 1) ;
			ply_set_read_cb(ply, "vertex", "nz",   PlyPointSetLoad::normal_cb, this, 2) ;
		} else if (has_normals_Neil) {
			has_normals_ = true;
			ply_set_read_cb(ply, "vertex", "vsfm_cnx",   PlyPointSetLoad::normal_cb, this, 0) ;
			ply_set_read_cb(ply, "vertex", "vsfm_cny",   PlyPointSetLoad::normal_cb, this, 1) ;
			ply_set_read_cb(ply, "vertex", "vsfm_cnz",   PlyPointSetLoad::normal_cb, this, 2) ;
		} else
			has_normals_ = false;
		
		if (has_colors_)
			point_set_color_.bind(point_set_);
		if (has_normals_)
			point_set_normal_.bind(point_set_);
	}

	static PlyPointSetLoad* plyload(p_ply_argument argument) {
		PlyPointSetLoad* result = nil ;
		ply_get_argument_user_data(argument, (void**)(&result), nil) ;
		ogf_assert(result != nil) ;
		return result ;
	}

	static int vertex_cb(p_ply_argument argument) {
		return plyload(argument)->add_vertex_data(argument) ;
	}

	static int color_cb(p_ply_argument argument) {
		return plyload(argument)->add_color_data(argument) ;
	}

	static int normal_cb(p_ply_argument argument) {
		return plyload(argument)->add_normal_data(argument) ;
	}

	int add_vertex_data(p_ply_argument argument) {

		long coord ;
		ply_get_argument_user_data(argument, nil, &coord);
		ogf_assert(coord >= 0 && coord < 3) ;
		xyz_[coord] = double(ply_get_argument_value(argument)) ;
		if(coord == 2) { 
			//progress_.notify(current_vertex_);
			PointSet::Vertex* v = point_set_->new_vertex(vec3(xyz_[0], xyz_[1], xyz_[2]));
			vertices_[current_vertex_] = v;
			current_vertex_++ ; 
		}
		return 1;
	}

	int add_color_data(p_ply_argument argument) {
		long coord ;
		ply_get_argument_user_data(argument, nil, &coord);
		ogf_assert(coord >= 0 && coord < 3) ;
		rgb_[coord] = double(ply_get_argument_value(argument)) * color_mult_ ;
		if(coord == 2) { 
			set_vertex_color(current_color_, Color(float(rgb_[0]), float(rgb_[1]), float(rgb_[2]))) ;
			current_color_++ ; 
		}
		return 1 ;
	}

	int add_normal_data(p_ply_argument argument) {
		long coord ;
		ply_get_argument_user_data(argument, nil, &coord);
		ogf_assert(coord >= 0 && coord < 3) ;
		normal_[coord] = double(ply_get_argument_value(argument));
		if(coord == 2) { 
			set_vertex_normal(current_normal_, vec3(float(normal_[0]), float(normal_[1]), float(normal_[2]))) ;
			current_normal_++ ; 
		}
		return 1 ;
	}

	//////////////////////////////////////////////////////////////////////////
	
	void create_vertices(unsigned int nb_vertices) {
		vertices_.resize(nb_vertices);
	}

	void set_vertex_color(unsigned int idx, const Color& c) {
		if(idx < 0 || idx >= int(vertices_.size())) {
			Logger::warn("PlyPointSetLoad") << "vertex index " << idx << " out of range" << std::endl;
			return ;
		}

		point_set_color_[vertices_[idx]] = c;
	}

	void set_vertex_normal(unsigned int idx, const vec3& n) {
		if(idx < 0 || idx >= int(vertices_.size())) {
			Logger::warn("PlyPointSetLoad") << "vertex index " << idx << " out of range" << std::endl;
			return ;
		}

		point_set_normal_[vertices_[idx]] = n;
	}

	PointSet* end_point_set() {
		PointSet* result = point_set_;

		point_set_ = nil;
  		point_set_normal_.unbind();
  		point_set_color_.unbind();

		vertices_.clear();
		current_vertex_ = 0;
		current_color_ = 0;
	
		return result;
	}

protected:
	//ProgressLogger	progress_;

	PointSet*		point_set_;
	PointSetNormal	point_set_normal_ ;
	PointSetColor	point_set_color_ ;

	double			xyz_[3] ;
	unsigned int	current_vertex_ ;

	std::vector<PointSet::Vertex*>	vertices_;

	bool			has_colors_ ;
	double			color_mult_ ;
	double			rgb_[3] ;
	unsigned int	current_color_ ;

	bool			has_normals_;
	unsigned int	current_normal_ ;
	double			normal_[3];
} ;

//__________________________________________________________

PointSet*	PointSetSerializer_ply::load(const std::string& file_name) {
	PlyPointSetLoad loader ;
	return  loader.load(file_name) ;
}


class PlyPointSetSave {
public:
	PlyPointSetSave(const PointSet* obj) 
		: object_(obj)
		, color_mult_(255.0)
	{ }

	bool save(const std::string& filename) {
		p_ply ply = ply_create(filename.c_str(), PLY_LITTLE_ENDIAN, nil, 0, nil) ;

		if(ply == nil) {
			Logger::err("PlyPointSetSave") << filename << ": could not open" << std::endl;
			return false ;
		}

		//////////////////////////////////////////////////////////////////////////

		if (!ply_add_comment(ply, "saved by liangliang.nan@gmail.com")) {
			Logger::err("PlyPointSetSave") << "unable to add comment" << std::endl;
			ply_close(ply) ;
			return false ;
		}

		int num_v = object_->size_of_vertices();
		if (!ply_add_element(ply, "vertex", num_v)) {
			Logger::err("PlyPointSetSave") << "unable to add element \'vertex\'" << std::endl;
			ply_close(ply) ;
			return false ;
		}

		e_ply_type length_type, value_type;
		length_type = value_type = static_cast<e_ply_type>(-1);
		std::string pos[3] = { "x", "y", "z" };
		for (unsigned int i=0; i<3; ++i) {
			if (!ply_add_property(ply, pos[i].c_str(), PLY_FLOAT, length_type, value_type)) {
				Logger::err("PlyPointSetSave") << "unable to add property \'" << pos[i] << "\'" << std::endl;
				ply_close(ply) ;
				return false ;
			}
		}
	
		PointSetNormal	normals;
		PointSetColor	colors;
		bool has_normals = PointSetNormal::is_defined(const_cast<PointSet*>(object_));
		bool has_colors = PointSetColor::is_defined(const_cast<PointSet*>(object_));	
		if (has_normals) {
			normals.bind(const_cast<PointSet*>(object_));
			std::string normal[3] = { "nx", "ny", "nz" };
			for (unsigned int i=0; i<3; ++i) {
				if (!ply_add_property(ply, normal[i].c_str(), PLY_FLOAT, length_type, value_type)) {
					Logger::err("PlyPointSetSave") << "unable to add property \'" << pos[i] << "\'" << std::endl;
					ply_close(ply) ;
					return false ;
				}
			}
		}
		if (has_colors) {
			colors.bind(const_cast<PointSet*>(object_));
			std::string color[4] = { "red", "green", "blue"};
			for (unsigned int i=0; i<3; ++i) {
				if (!ply_add_property(ply, color[i].c_str(), PLY_UCHAR, length_type, value_type)) {
					Logger::err("PlyPointSetSave") << "unable to add property \'" << color[i] << "\'" << std::endl;
					ply_close(ply) ;
					return false ;
				}
			}
		}
		
		if(!ply_write_header(ply)) {
			Logger::err("PlyPointSetSave") << filename << ": invalid PLY file" << std::endl;
			ply_close(ply) ;
			return false ;
		}

		//////////////////////////////////////////////////////////////////////////

// 		ProgressLogger progress(object_->size_of_vertices());
		int count = 0;
		FOR_EACH_VERTEX_CONST(PointSet, object_, it) {
// 			progress.notify(count);

			const PointSet::Vertex* v = it;
			const vec3& p = v->point();
			ply_write(ply, p.x);
			ply_write(ply, p.y);
			ply_write(ply, p.z);

			if (has_normals) {
				const vec3& n = normals[v];
				ply_write(ply, n.x);
				ply_write(ply, n.y);
				ply_write(ply, n.z);
			} 
			
			if (has_colors) {
				const Color& c = colors[v];
				double r = c.r() * color_mult_;	ogf_clamp(r, 0.0, 255.0);
				double g = c.g() * color_mult_;	ogf_clamp(g, 0.0, 255.0);
				double b = c.b() * color_mult_;	ogf_clamp(b, 0.0, 255.0);
				ply_write(ply, r);
				ply_write(ply, g);
				ply_write(ply, b);
			}

			++count;
		}
		
		ply_close(ply);
		return true ;
	}

protected:
	const PointSet*	object_;
	double			color_mult_;
} ;


bool PointSetSerializer_ply::save(const std::string& file_name, const PointSet* obj) {
	PlyPointSetSave plysave(obj) ;
	return  plysave.save(file_name) ;
}