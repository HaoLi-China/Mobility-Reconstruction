#ifndef __KDTREE_KDTREE_SEARCH_ETH__
#define __KDTREE_KDTREE_SEARCH_ETH__

#include "kdtree_common.h"
#include "kdtree_search.h"



class KDTREE_API KdTreeSearch_ETH : public KdTreeSearch  {
public:
	KdTreeSearch_ETH();
	virtual ~KdTreeSearch_ETH();

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

	// fixed-radius kNN	search. Search for all points in the range.
	// NOTE: *squared* radius of query ball
	virtual void find_points_in_radius(const vec3& p, double squared_radius, 
		std::vector<PointSet::Vertex*>& neighbors
		) const ;

	virtual void find_points_in_radius(const vec3& p, double squared_radius, 
		std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances
		) const ;

	//____________________ cylinder range search _________________

	// Search for the nearest points whose distances to line segment $v1$-$v2$ are smaller 
	// than $radius$. If $bToLine$ is true, the points found are ordered by their distances 
	// to the line segment. Otherwise, they are ordered by their distances to $v1$.
	// NOTE: it is radius (instead of *squared* radius).
	unsigned int find_points_in_cylinder(
		const vec3& p1, const vec3& p2, double radius, 
		std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances, 
		bool bToLine = true
		) const ;

	unsigned int find_points_in_cylinder(
		const vec3& p1, const vec3& p2, double radius, 
		std::vector<PointSet::Vertex*>& neighbors, 
		bool bToLine = true
		) const ;
	
	//_______________________ cone range search __________________

	// Search for the nearest points $P_i$ with an cone from $v1$ to $v2$ defined by v1 and v2. 
	// As a result, the angle between $v1$$P_i$ and $v1$$v2$ is smaller than $angle_range$.
	// Search for the nearest points P_i where the angle between $v1$-P_i and $v1$-$v2$ is 
	// smaller than $angle$.
	// NOTE: angle is in radian.
	unsigned int find_points_in_cone(
		const vec3& eye, const vec3& p1, const vec3& p2, double angle_range, 
		std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances, 
		bool bToLine = true
		) const ;

	unsigned int find_points_in_cone(
		const vec3& eye, const vec3& p1, const vec3& p2, double angle_range, 
		std::vector<PointSet::Vertex*>& neighbors, 
		bool bToLine = true
		) const ;

protected:
	std::vector<PointSet::Vertex*> vertices_;
	unsigned int	points_num_;

	void*	tree_;
} ;

#endif


