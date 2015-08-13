#include "plain_point_set_render.h"
#include "../geom/point_set.h"
#include "../geom/iterators.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif




PlainPointSetRender::PlainPointSetRender(PointSet* obj)
: Render(obj)
{
	use_color_attribute_ = true;

	points_as_spheres_ = false;

	vertices_style_.visible = true;
	vertices_style_.color = Color(85/255.0f, 170/255.0f, 1.0f);
	vertices_style_.size  = 2 ;

	anchors_style_.visible = true ;
	anchors_style_.color   = Color(1, 0, 0, 1) ;
	anchors_style_.size    = 6 ;
}


PlainPointSetRender::~PlainPointSetRender(void)
{
}


PointSet* PlainPointSetRender::target() const {
	return dynamic_cast<PointSet*>(object());
}


const PointStyle& PlainPointSetRender::vertices_style() const { 
	return vertices_style_ ; 
}

void PlainPointSetRender::set_vertices_style(const PointStyle& x) {
	vertices_style_ = x ; 
}

const PointStyle& PlainPointSetRender::anchors_style() const { 
	return anchors_style_ ;
}

void PlainPointSetRender::set_anchors_style(const PointStyle& x) {
	anchors_style_ = x ; 
}

void PlainPointSetRender::set_use_color_attribute(bool x) { 
	use_color_attribute_ = x ; 
}


void PlainPointSetRender::draw() {
	if(vertices_style_.visible) {
		if (points_as_spheres_ && has_points_shaders_) 
			activate_points_shaders();

		draw_point_set();

		if (points_as_spheres_ && has_points_shaders_) 
			deactivate_points_shaders();

	}
}


void PlainPointSetRender::draw_point_set() {
	if (use_color_attribute_)
		vertex_color_.bind_if_defined(target(), "color") ;

	bool has_normal = PointSetNormal::is_defined(target());
	if (has_normal)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	glPointSize(vertices_style_.size);
	glColor4fv(vertices_style_.color.data());

	glBegin(GL_POINTS);
	if (has_normal)	{
		PointSetNormal normals(target());
		if (vertex_color_.is_bound()) {
			PointSetColor	colors(target());
			FOR_EACH_VERTEX_CONST(PointSet, target(), it) {
				const vec3&  p = it->point();
				const vec3& n = normals[it];
				const Color& c = vertex_color_[it];
				glColor4fv(c.data());
				glNormal3dv(n.data());
				glVertex3dv(p.data());
			}
		} else  {
			glColor3fv(vertices_style_.color.data());
			FOR_EACH_VERTEX_CONST(PointSet, target(), it) {
				const vec3&  p = it->point();
				const vec3& n = normals[it];
				glNormal3dv(n.data());
				glVertex3dv(p.data());
			}
		} 
	}
	else {
		if (vertex_color_.is_bound()) {
			FOR_EACH_VERTEX_CONST(PointSet, target(), it) {
				const vec3&  p = it->point();
				const Color& c = vertex_color_[it];
				glColor4fv(c.data());
				glVertex3dv(p.data());
			}
		} else  {
			glColor3fv(vertices_style_.color.data());
			FOR_EACH_VERTEX_CONST(PointSet, target(), it) {
				const vec3&  p = it->point();
				glVertex3dv(p.data());
			}
		} 
	}
	glEnd();

	if(vertex_color_.is_bound()) {
		vertex_color_.unbind() ;
	}	
}

void PlainPointSetRender::blink() {
	vertices_style_.size += 1;
	target()->update() ;
    sleep(200);

	vertices_style_.size -= 1;
	target()->update() ;
}

