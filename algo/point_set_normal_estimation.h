#ifndef _ALGOS_POINT_SET_NORMAL_ESTIMATION_H_
#define _ALGOS_POINT_SET_NORMAL_ESTIMATION_H_

#include "algo_common.h"
#include <string>

class PointSet;

class ALGO_API PointSetNormalEstimation
{
public:
	PointSetNormalEstimation(void);
	~PointSetNormalEstimation(void);

	static std::string title() { return "PoissonRecon"; }

	//////////////////////////////////////////////////////////////////////////

	static void apply(PointSet* pointSet, bool smooth, unsigned int K_nei = 10, unsigned int K_nor = 10);
};

#endif