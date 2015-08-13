#ifndef SURFACE_TRIMMER_H
#define SURFACE_TRIMMER_H

#include "Geometry.h"

template<class Real >
void trim_mesh(
          std::vector< PlyValueVertex<Real> >& vertices,
          std::vector< std::vector<int> >& polygons,
		  Real trim_value, 
		  Real area_ratio,
		  bool triangulate,
		  int smooth_iteration = 0
		  );


#include "SurfaceTrimmer.inl"
#endif
