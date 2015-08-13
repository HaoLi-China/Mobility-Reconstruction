#ifndef _GEOM_POINT_SET_GEOMETRY_H_
#define _GEOM_POINT_SET_GEOMETRY_H_

#include "geom_common.h"
#include "../math/math_types.h"



class PointSet;

// Adds some functions related to PointSet to the Geom namespace.
namespace Geom {

	GEOM_API PointSet*	duplicate(const PointSet* pset);

    GEOM_API Box3d		bounding_box(const PointSet* pset) ;

}

#endif

