#include "point_set_normal_estimation.h"
#include "../geom/point_set.h"
#include "../geom/iterators.h"
#include "../kd_tree/kdtree_search_eth.h"


void PointSetNormalEstimation::apply(PointSet* pointSet, bool smooth, unsigned int K_nei/* = 10*/, unsigned int K_nor/* = 10*/)
{
	PointSetNormal normals;
	if (!normals.is_defined(pointSet)) {
		normals.bind(pointSet);
	}
	
	KdTreeSearch_ETH kd_eth;	
	kd_eth.add_vertex_set(pointSet);
	kd_eth.end();

	FOR_EACH_VERTEX_CONST(PointSet, pointSet, it) {
		const vec3& p = it->point();

		std::vector<PointSet::Vertex*> neighbors;
		std::vector<vec3> points;
		kd_eth.find_closest_K_points(p, K_nei, neighbors);

		for (unsigned int j = 0; j < neighbors.size(); j++) {
			points.push_back(neighbors[j]->point());
		}

		GenericPlane3<Numeric::float64> plane;
		if (plane.FitToPoints(points)) {
			vec3 normal_plane = plane.normal();
			vec3 normal_tem(p[0], p[1], p[2]);
			if (normal_tem[0] * normal_plane[0] + normal_tem[1] * normal_plane[1] + normal_tem[2] * normal_plane[2]>0){
				normals[it] = -normal_plane;
			}
			else{
				normals[it] = normal_plane;
			}
		}
		else {
			normals[it] = vec3(1.0, 0.0, 0.0);
			std::cout << "Compute normals: wrong!!!!!!!" << std::endl;
		}
	}

	if (smooth) {
		// smooth the normals!!!!!!!!!!
	}
	kd_eth.begin();
}