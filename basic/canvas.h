#ifndef _BASIC_CANVAS_H_
#define _BASIC_CANVAS_H_

#include "basic_common.h"
#include <string>

class Map;
class PointSet;
class CGraph;

class Canvas 
{
public:
	Canvas() {}
	virtual ~Canvas() {}

	virtual void update() = 0;
	virtual void immediate_update() = 0;

	// the active object
	virtual	Map*		mesh() const = 0;
	virtual	PointSet*	pointSet() const = 0;
	virtual	CGraph*	    whetGrid() const = 0;
};


#endif