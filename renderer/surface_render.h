
#ifndef _RENDERER_SURFACE_RENDERER_H_
#define _RENDERER_SURFACE_RENDERER_H_

#include "renderer_common.h"
#include "render.h"
#include "rendering_styles.h"
#include "point_as_sphere.h"
#include "../geom/map_attributes.h"


class Map;

class RENDERER_API SurfaceRender : public Render, public PointAsSphere
{
public:
	SurfaceRender(Map* obj) ;

	Map* target() const;

	virtual void draw() ;
	virtual void blink() ;

	//___________________________________________________________

	bool smooth_shading() const ;
	void set_smooth_shading(bool x) ;

	const SurfaceStyle& surface_style() const ;
	void set_surface_style(const SurfaceStyle& x) ;

	const EdgeStyle& mesh_style() const ;
	void set_mesh_style(const EdgeStyle& x) ;

	const EdgeStyle& border_style() const ;
	void set_border_style(const EdgeStyle& x) ;

	const PointStyle& vertices_style() const ;
	void set_vertices_style(const PointStyle& x) ;

	const PointStyle& anchors_style() const ;
	void set_anchors_style(const PointStyle& x) ;
	
	const PointStyle& pins_style() const ;
	void set_pins_style(const PointStyle& x) ;


protected:
	virtual void draw_surface() = 0 ;
	virtual void draw_mesh() = 0 ;
	virtual void draw_vertices() = 0 ;
	virtual void draw_anchors() = 0 ;
	virtual void draw_border() = 0 ;
	virtual void draw_pins() = 0 ;

protected:
	bool         smooth_shading_ ;

	SurfaceStyle surface_style_ ;

	EdgeStyle    mesh_style_ ;
	EdgeStyle    border_style_ ;

	PointStyle   vertices_style_ ;
	PointStyle   anchors_style_ ;
	PointStyle   pins_style_ ;
} ;



#endif
