#ifndef __KDTREE_KDTREE_SEARCH_FLANN__
#define __KDTREE_KDTREE_SEARCH_FLANN__

#include "kdtree_common.h"
#include "kdtree_search.h"



class KDTREE_API KdTreeSearch_FLANN : public KdTreeSearch  {
public:
	KdTreeSearch_FLANN();
	virtual ~KdTreeSearch_FLANN();

	// 'checks' specifies the maximum leafs to visit when searching for neighbors. 
	// A higher value for this parameter would give better search precision, but also
	// take more time. For all leafs to be checked use the value FLANN_CHECKS_UNLIMITED. 
	// If automatic configuration was used when the index was created, the number 
	// of checks required to achieve the specified precision was also computed, to use 
	// that value specify FLANN_CHECKS_AUTOTUNED.
	// The default value is FLANN_CHECKS_AUTOTUNED. (Liangliang: 32 is also a good value).
	void set_checks(int chk);

	//______________ tree construction __________________________

	virtual void begin() ;
	virtual void add_point(PointSet::Vertex* v) ;
	virtual void add_vertex_set(PointSet* vs) ;
	virtual void end() ;

	//________________ closest point ____________________________

	// NOTE: *squared* distance is returned
	virtual PointSet::Vertex* find_closest_point(const vec3& p, double& squared_distance) const ;
	virtual PointSet::Vertex* find_closest_point(const vec3& p) const ;

	//_________________ K-nearest neighbors ____________________

	// NOTE: *squared* distances are returned
	virtual void find_closest_K_points(
		const vec3& p, unsigned int k, 
		std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances
		) const ;

	virtual void find_closest_K_points(
		const vec3& p, unsigned int k, 
		std::vector<PointSet::Vertex*>& neighbors
		) const ;

	//___________________ radius search ___________________________

	// fixed-radius kNN	search. Search for all points in the range.
	// NOTE: *squared* radius of query ball
	virtual void find_points_in_radius(const vec3& p, double squared_radius, 
		std::vector<PointSet::Vertex*>& neighbors
		) const ;

	virtual void find_points_in_radius(const vec3& p, double squared_radius, 
		std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances
		) const ;

protected:
	std::vector<PointSet::Vertex*> vertices_;
	unsigned int	points_num_;
	double*			points_;

	void*	tree_;
	int		checks_;
} ;

#endif


