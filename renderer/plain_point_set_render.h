
#ifndef _RENDERER_PLAIN_POINT_SET_RENDERER_H_
#define _RENDERER_PLAIN_POINT_SET_RENDERER_H_

#include "renderer_common.h"
#include "rendering_styles.h"
#include "point_as_sphere.h"
#include "render.h"
#include "../geom/point_set.h"



class RENDERER_API PlainPointSetRender : public Render, public PointAsSphere
{
public:
	PlainPointSetRender(PointSet* obj);
	~PlainPointSetRender(void);

	RenderType type() const { return PLAIN_POINT_SET_RENDER; }

	bool use_color_attribute() const { return use_color_attribute_ ; }
	void set_use_color_attribute(bool x) ;

	const PointStyle& vertices_style() const ;
	void set_vertices_style(const PointStyle& x) ;

	const PointStyle& anchors_style() const ;
	void set_anchors_style(const PointStyle& x) ;

	PointSet* target() const;

	virtual void draw() ;
	virtual void blink() ;


protected:
	virtual void draw_point_set() ;

protected:
	PointStyle		vertices_style_;
	PointStyle		anchors_style_;

	bool			use_color_attribute_ ;
	PointSetColor	vertex_color_ ;
};



#endif