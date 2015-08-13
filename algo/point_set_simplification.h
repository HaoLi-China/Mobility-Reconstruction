#ifndef _ALGO_POINT_SET_SIMPLIFICATION_H_
#define _ALGO_POINT_SET_SIMPLIFICATION_H_

#include "algo_common.h"
#include "../math/math_types.h"



class PointSet;

namespace PointSetTypes {
	class Vertex;
}

class ALGO_API PointSetSimplification {
public:
	// @k: number of nearest neighbors.
	static double	average_sapcing(PointSet* pset, int k = 6) ;

	// considers a regular grid covering the bounding box of the input point set, and clusters 
	// all points sharing the same cell of the grid by picking as representant one arbitrarily 
	// chosen point.
	// @epsilon (or cell size): tolerance value when merging 3D points.
	// return a list of vertices that can be removed.
	static std::vector<PointSetTypes::Vertex*>	grid_simplification(PointSet* pset, double epsilon) ; 
};

#endif

