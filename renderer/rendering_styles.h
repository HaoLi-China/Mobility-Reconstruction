#ifndef _RENDERER_RENDERING_STYLES_H_
#define _RENDERER_RENDERING_STYLES_H_

#include "../image/color.h"


struct PointStyle {
	PointStyle() : visible(false), color(0.0, 0.0, 0.0, 1.0), size(4) {}
	bool  visible ;
	Color color ;
	float size ;
} ;

std::ostream& operator<<(std::ostream& out, const PointStyle& ps) ;
std::istream& operator>>(std::istream& in, PointStyle& ps) ;

//________________________________________________________

struct EdgeStyle {
	EdgeStyle() : visible(false), color(1.0, 0.0, 0.0, 1.0), width(1) {}
	bool  visible ;
	Color color ;
	float width ;
} ;

std::ostream& operator<<(std::ostream& out, const EdgeStyle& es) ;
std::istream& operator>>(std::istream& in, EdgeStyle& es) ;

//________________________________________________________

struct SurfaceStyle {
	SurfaceStyle() : visible(false), color(0.33f, 0.67f, 1.0f, 0.5f) {}
	bool  visible ;
	Color color ;
} ;

std::ostream& operator<<(std::ostream& out, const SurfaceStyle& ss) ;
std::istream& operator>>(std::istream& in, SurfaceStyle& ss) ;


//________________________________________________________


struct SliceStyle {
public:
	SliceStyle() : visible(false), coord(0) { }
	bool	visible ;
	int		coord ;  // in [-250, 250]
} ;

#endif