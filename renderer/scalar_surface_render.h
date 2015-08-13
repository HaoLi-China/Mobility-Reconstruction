
#ifndef _RENDERER_SCALAR_SURFACE_RENDER_H_
#define _RENDERER_SCALAR_SURFACE_RENDER_H_

#include "renderer_common.h"
#include "plain_surface_render.h"
#include "scalar_render.h"
#include "texture.h"
#include "../geom/map_attributes.h"


class RENDERER_API ScalarSurfaceRender : public PlainSurfaceRender, public ScalarRender
{
public:
	enum EdgeAttributeStyle { sticks, arrows, corners, dual } ;

	ScalarSurfaceRender(Map* obj) ;

	RenderType type() const { return SCALAR_SURFACE_RENDER; }

	SurfaceAttributeLocation location() const { return location_ ; }
	void set_location(SurfaceAttributeLocation loc) ;

	EdgeAttributeStyle edge_attribute_style() const { return edge_attr_style_; }
	void set_edge_attribute_style(EdgeAttributeStyle x) ;


protected:
	virtual void draw() ;
	virtual void draw_surface() ;
	virtual void draw_mesh() ;
	virtual void draw_border() ;     

	void draw_with_vertex_attribute() ;
	void draw_with_edge_attribute() ;
	void draw_with_facet_attribute() ;

protected:
	SurfaceAttributeLocation	location_ ;
	EdgeAttributeStyle			edge_attr_style_ ;

	MapVertexAttributeAdapter	vertex_attr_ ;
	MapHalfedgeAttributeAdapter	halfedge_attr_ ;
	MapFacetAttributeAdapter	facet_attr_;
} ;

#endif
