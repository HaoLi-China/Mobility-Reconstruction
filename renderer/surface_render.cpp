#include "surface_render.h"
#include "../basic/logger.h"
#include "../opengl/opengl_info.h"


SurfaceRender::SurfaceRender(Map* obj)
: Render(obj)
{
	smooth_shading_ = false;

	surface_style_.visible = true ;
	//surface_style_.color = Color(1.0f, 0.67f, 0.5f, 1.0f);
	surface_style_.color = Color(0.33f, 0.67f, 1.0f, 0.5f);

	mesh_style_.visible = false ;
	mesh_style_.color   = Color(0.0f, 0.0f, 0.0f, 1.0f); 
	mesh_style_.width   = 1 ;

	border_style_.visible = true ;
	border_style_.color   = Color(1.0f, 1.0f, 0.0f, 1.0f) ;
	border_style_.width   = 3 ;

	points_as_spheres_ = true;

	vertices_style_.visible = false ;
	vertices_style_.color   = Color(0, 1, 0, 1) ;
	vertices_style_.size    = 5 ;

	anchors_style_.visible = true ;
	anchors_style_.color   = Color(1, 0, 0, 1) ;
	anchors_style_.size    = 6 ;

	pins_style_.visible = true ;
	pins_style_.color   = Color(1, 0.33f, 1, 1) ;
	pins_style_.size    = 6 ;
}

Map* SurfaceRender::target() const {
	return dynamic_cast<Map*>(object());
}

void SurfaceRender::draw() {
	if (points_as_spheres_ && has_points_shaders_)
		activate_points_shaders();

	if(anchors_style_.visible)
		draw_anchors() ;

	if(pins_style_.visible) 
		draw_pins() ;

	if(vertices_style_.visible)
		draw_vertices() ;

	if (points_as_spheres_ && has_points_shaders_)
		deactivate_points_shaders();

	//////////////////////////////////////////////////////////////////////////

	if(surface_style_.visible) {
		if (mesh_style_.visible || border_style_.visible) {
			// 			glEnable(GL_POLYGON_OFFSET_FILL);
			// 			glPolygonOffset(1.0f, 1.0f);
			// Makes the depth coordinates of the filled primitives smaller, so that
			// displaying the mesh and the surface does not cause Z-fighting.
			glEnable(GL_POLYGON_OFFSET_FILL) ;
			glPolygonOffset(0.5f, -0.0001f) ;
		}

		draw_surface() ;

		if (mesh_style_.visible || border_style_.visible)
			glDisable(GL_POLYGON_OFFSET_FILL);
	}

	if(border_style_.visible)
		draw_border() ;

	if(mesh_style_.visible)
		draw_mesh() ;
}


void SurfaceRender::blink() {
	mesh_style_.visible = !mesh_style_.visible ;
	target()->update() ;
    sleep(200);

	mesh_style_.visible = !mesh_style_.visible ;
    target()->update() ;
}


bool SurfaceRender::smooth_shading() const { 
	return smooth_shading_ ; 
}

void SurfaceRender::set_smooth_shading(bool x) { 
	smooth_shading_ = x ; 
}

const SurfaceStyle& SurfaceRender::surface_style() const { 
	return surface_style_ ; 
}

void SurfaceRender::set_surface_style(const SurfaceStyle& x) { 
	surface_style_ = x ; 
}

const EdgeStyle& SurfaceRender::mesh_style() const { 
	return mesh_style_ ; 
}

void SurfaceRender::set_mesh_style(const EdgeStyle& x) { 
	mesh_style_ = x ; 
}

const EdgeStyle& SurfaceRender::border_style() const { 
	return border_style_ ; 
}

void SurfaceRender::set_border_style(const EdgeStyle& x) { 
	border_style_ = x ; 
}

const PointStyle& SurfaceRender::vertices_style() const { 
	return vertices_style_ ; 
}

void SurfaceRender::set_vertices_style(const PointStyle& x) {
	vertices_style_ = x ; 
}


const PointStyle& SurfaceRender::anchors_style() const { 
	return anchors_style_ ;
}

void SurfaceRender::set_anchors_style(const PointStyle& x) {
	anchors_style_ = x ; 
}


const PointStyle& SurfaceRender::pins_style() const { 
	return pins_style_ ;
}

void SurfaceRender::set_pins_style(const PointStyle& x) {
	pins_style_ = x ; 
}
