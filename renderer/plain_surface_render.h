
#ifndef _RENDERER_PLAIN_SURFACE_RENDER_H_
#define _RENDERER_PLAIN_SURFACE_RENDER_H_

#include "renderer_common.h"
#include "surface_render.h"


class RENDERER_API PlainSurfaceRender : public SurfaceRender 
{
public:
	PlainSurfaceRender(Map* obj) ;

	RenderType type() const { return PLAIN_SURFACE_RENDER; }

	bool use_color_attribute() const { return use_color_attribute_ ; }
	void set_use_color_attribute(bool x) ;

protected:
	virtual void draw_surface() ;
	virtual void draw_mesh() ;
	virtual void draw_border() ;
	virtual void draw_vertices() ;
	virtual void draw_anchors() ;
	virtual void draw_pins() ;

protected:
	bool	use_color_attribute_ ;
	MapFacetAttribute<Color>	facet_color_ ;
	MapVertexAttribute<Color>	vertex_color_ ;
} ;

#endif
