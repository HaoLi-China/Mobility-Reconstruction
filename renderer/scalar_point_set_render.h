
#ifndef __SCALAR_POINT_SET_RENDER__
#define __SCALAR_POINT_SET_RENDER__

#include "renderer_common.h"
#include "plain_point_set_render.h"
#include "scalar_render.h"
#include "../geom/point_set.h"
#include "texture.h"


class RENDERER_API ScalarPointSetRender : public PlainPointSetRender, public ScalarRender {
public:
	ScalarPointSetRender(PointSet* obj) ;

	RenderType type() const { return SCALAR_POINT_SET_RENDER; }

	virtual void draw() ;


protected:
	PointSetAttributeAdapter vertex_attr_ ;

} ;



#endif
