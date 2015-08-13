#include "point_set_simplification.h"
#include "../geom/point_set.h"
#include "../geom/iterators.h"
#include "../basic/logger.h"
#include "../kd_tree/kdtree_search_eth.h"


/// Utility class for grid_simplify_point_set():
/// LessEpsilonPoints defines a 3D points order: two points are equal
/// iff they belong to the same cell of a grid of cell size = epsilon.
template <class Vertex>
class LessEpsilonPoints {
public:
	LessEpsilonPoints (double epsilon) : m_epsilon (epsilon) {
		ogf_assert(epsilon > 0);
	}

	// Round points to multiples of m_epsilon, then compare.
	bool operator() (const Vertex* a, const Vertex* b) const {
		const vec3& a_n = a->point();
		const vec3& b_n = b->point();

		vec3 rounded_a(round_epsilon(a_n.x, m_epsilon), round_epsilon(a_n.y, m_epsilon), round_epsilon(a_n.z, m_epsilon));
		vec3 rounded_b(round_epsilon(b_n.x, m_epsilon), round_epsilon(b_n.y, m_epsilon), round_epsilon(b_n.z, m_epsilon));

		//return (rounded_a < rounded_b);
		if (rounded_a.x < rounded_b.x)
			return true;
		else if (rounded_a.x == rounded_b.x) {
			if (rounded_a.y < rounded_b.y)
				return true;
			else if (rounded_a.y == rounded_b.y) {
				if (rounded_a.z < rounded_b.z)
					return true;
			}
		}

		return false;
	}

private:
	// Round number to multiples of epsilon
	static inline double round_epsilon(double value, double epsilon) {
		return std::floor(value/epsilon) * epsilon;
	}

private:
	double m_epsilon;
};


//////////////////////////////////////////////////////////////////////////


std::vector<PointSet::Vertex*> PointSetSimplification::grid_simplification(PointSet* pset, double epsilon) {
	ogf_assert(epsilon > 0);

	// Merges points which belong to the same cell of a grid of cell size = epsilon.
	// points_to_keep will contain 1 point per cell; the others will be in points_to_remove.
    std::set< PointSet::Vertex*, LessEpsilonPoints<PointSet::Vertex> > points_to_keep(epsilon);
	std::vector<PointSet::Vertex*> points_to_remove;
	FOR_EACH_VERTEX(PointSet, pset, it) {
		PointSet::Vertex* p = it;
        std::pair< std::set<PointSet::Vertex*, LessEpsilonPoints<PointSet::Vertex> >::iterator, bool> result = points_to_keep.insert(p);
		if (!result.second) // if not inserted
			points_to_remove.push_back(p);
	}

	return points_to_remove;
}



double PointSetSimplification::average_sapcing(PointSet* pset, int k/* = 6*/) {
 	KdTreeSearch_var kdtree = new KdTreeSearch_ETH;

	kdtree->begin();
	kdtree->add_vertex_set(pset);
	kdtree->end();

	double total = 0;
	FOR_EACH_VERTEX_CONST(PointSet, pset, it) {
		const vec3& p = it->point();
		std::vector<PointSet::Vertex*> neighbors;
		std::vector<double> sqr_distances;
		kdtree->find_closest_K_points(p, k+1, neighbors, sqr_distances);  // k+1 to exclude itself

		// in case we get less than k+1 neighbors
		double avg = 0;
		for (unsigned int i=1; i<sqr_distances.size(); ++i) { // starts from 1 to exclude itself
			avg += std::sqrt(sqr_distances[i]);
		}
		total += (avg / neighbors.size());
	}

	return (total / pset->size_of_vertices());
}

