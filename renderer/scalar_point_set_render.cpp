
#include "scalar_point_set_render.h"
#include "texture_factory.h"
#include "../geom/iterators.h"
#include "../geom/point_set.h"
#include "../basic/logger.h"
#include "../basic/file_utils.h"
#include "../image/image_io.h"



ScalarPointSetRender::ScalarPointSetRender(PointSet* pset) 
: PlainPointSetRender(pset)
{
	use_color_attribute_ = true;
}


void ScalarPointSetRender::draw() {
	vertex_attr_.bind_if_defined(target(), attribute_name_) ;

	if(use_texture_) {
		create_textures_if_needed() ;
	}

	if( !vertex_attr_.is_bound() ) {
		PlainPointSetRender::draw() ;
	}
	else {
		double maximum = maximum_ ;
		double minimum = minimum_ ;
		if(auto_range_) {
			maximum = -Numeric::big_double ;
			minimum =  Numeric::big_double ;
			FOR_EACH_VERTEX(PointSet, target(), it) {
				maximum = ogf_max(maximum, vertex_attr_[it]) ;
				minimum = ogf_min(minimum, vertex_attr_[it]) ;
			}
			Logger::status() << "Value range: [" << minimum << ", " << maximum << "]" << std::endl ;
		}

		if(minimum == maximum) 
			maximum += 1.0 ;
		vertex_attr_.set_range(minimum, maximum) ;

		if (use_texture_ && colormap_texture_)
			colormap_texture_->bind();

		bool has_normal = PointSetNormal::is_defined(target());
		if (has_normal)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);

		glPointSize(vertices_style_.size);
		glBegin(GL_POINTS);
		if (has_normal)	{
			PointSetNormal normals(target());
			FOR_EACH_VERTEX_CONST(PointSet, target(), it) {
				double v = vertex_attr_[it] ;
				set_color(v) ;
				const vec3&  p = it->point();
				const vec3& n = normals[it];
				glNormal3dv(n.data());
				glVertex3dv(p.data());
			}
		} else {
			FOR_EACH_VERTEX_CONST(PointSet, target(), it) {
				double v = vertex_attr_[it] ;
				set_color(v) ;
				const vec3&  p = it->point();
				glVertex3dv(p.data());
			}
		}
		glEnd();
		vertex_attr_.unbind() ;
	}

	if (use_texture_ && colormap_texture_)
		colormap_texture_->unbind();
}
