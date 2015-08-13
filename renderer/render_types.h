
#ifndef _RENDERER_RENDER_TYPES_H_
#define _RENDERER_RENDER_TYPES_H_

#include "renderer_common.h"

enum RenderType {

	//--------------- Map ----------------

	PLAIN_SURFACE_RENDER,		// "plain_surface_render"
	SCALAR_SURFACE_RENDER,		// "scalar_surface_render"
	VECTORS_SURFACE_RENDER,		// "vectors_surface_render"
	TEXTURED_SURFACE_RENDER,	// "textured_surface_render"

	//------------- PointSet --------------

	PLAIN_POINT_SET_RENDER,		//	"plain_point_set_render"
	SCALAR_POINT_SET_RENDER,	//	"scalar_point_set_render"

	//-------------- CGraph ---------------

	PLAIN_WHET_GRID_RENDER,		// plain_whet_grid_render
	SCALAR_WHET_GRID_RENDER,	// scalar_whet_grid_render
	VECTORS_WHET_GRID_RENDER,	// vectors_whet_grid_render

	// -------------------------------------

	EMPTY_RENDER

};



#endif
