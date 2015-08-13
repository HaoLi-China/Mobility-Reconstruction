#ifndef __KDTREE_KDTREE_SEARCH_ANN__
#define __KDTREE_KDTREE_SEARCH_ANN__

#include "kdtree_common.h"
#include "kdtree_search.h"



class KDTREE_API KdTreeSearch_ANN : public KdTreeSearch  {
public:
	KdTreeSearch_ANN();
	virtual ~KdTreeSearch_ANN();


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

	//___________________ radius search __________________________

	// Liangliang: it seems ANN's annkFRSearch() needs to specify k. 
	/* "It does two things. First, it computes the k nearest neighbors within the radius bound. 
		Second, it returns the total number of points lying within the radius bound. It is 
		permitted to set k = 0, in which case it only answers a range counting query. */
	void set_k_for_radius_search(int k) { k_for_radius_search_ = k; }

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
	double**		points_;

	void*	tree_;	
	int		k_for_radius_search_;
} ;

#endif


