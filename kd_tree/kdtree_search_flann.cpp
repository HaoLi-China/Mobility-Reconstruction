
#include "kdtree_search_flann.h"
#include "FLANN/flann.hpp"



#define get_tree(x) ((const flann::Index< flann::L2<double> > *)(x))

KdTreeSearch_FLANN::KdTreeSearch_FLANN()  {
	points_ = nil;
	points_num_ = 0;
	tree_ = nil;
	
	//checks_ = 32;
	checks_ = flann::FLANN_CHECKS_AUTOTUNED;
}


KdTreeSearch_FLANN::~KdTreeSearch_FLANN() {
	if(points_)
		delete [] points_;
    delete get_tree(tree_);
}

void KdTreeSearch_FLANN::set_checks(int chk) {
	checks_ = chk;
}

void KdTreeSearch_FLANN::begin()  {
	vertices_.clear();

	if(points_)
		delete [] points_;

    delete get_tree(tree_);
	tree_ = nil;
}


void KdTreeSearch_FLANN::end()  {
	points_num_ = vertices_.size();
	points_ = new double[points_num_ * 3];

	for(unsigned int i=0; i<points_num_; ++i) {
		const vec3& p = vertices_[i]->point();
		points_[i*3  ] = p.x;
		points_[i*3+1] = p.y;
		points_[i*3+2] = p.z;
	}

	flann::Matrix<double> dataset(points_, points_num_, 3);

	// construct a single kd-tree optimized for searching lower dimensionality data (for example 3D point clouds)
	flann::Index< flann::L2<double> >* tree = new flann::Index< flann::L2<double> >(dataset, flann::KDTreeSingleIndexParams());
	tree->buildIndex();
	
	tree_ = tree;
	// NOTE: dataset.ptr() == points_, and points_ is deleted in the destructor
	//delete[] dataset.ptr();  
}


void KdTreeSearch_FLANN::add_point(PointSet::Vertex* v)  {
	vertices_.push_back(v);
}


void KdTreeSearch_FLANN::add_vertex_set(PointSet* vs)  {
	for(PointSet::Vertex_iterator it = vs->vertices_begin() ; it != vs->vertices_end() ; ++it)
		vertices_.push_back(it);
}


PointSet::Vertex* KdTreeSearch_FLANN::find_closest_point(const vec3& p) const {
	double dist = 0;
	return find_closest_point(p, dist);
}

PointSet::Vertex* KdTreeSearch_FLANN::find_closest_point(const vec3& p, double& squared_distance) const {
	flann::Matrix<double> query(const_cast<double*>(p.data()), 1, 3);

	std::vector< std::vector<int> >		indices;
	std::vector< std::vector<double> >	dists;

	get_tree(tree_)->knnSearch(query, indices, dists, 1, flann::SearchParams(checks_));

	squared_distance = dists[0][0];
	return vertices_[ indices[0][0] ];
}

void KdTreeSearch_FLANN::find_closest_K_points(
	const vec3& p, unsigned int k, std::vector<PointSet::Vertex*>& neighbors
	)  const {
	std::vector<double> squared_distances;
	return find_closest_K_points(p, k, neighbors, squared_distances);
}

void KdTreeSearch_FLANN::find_closest_K_points(
	const vec3& p, unsigned int k, std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances
	)  const {
		flann::Matrix<double> query(const_cast<double*>(p.data()), 1, 3);

		std::vector< std::vector<int> >		indices;
		std::vector< std::vector<double> >	dists;

		get_tree(tree_)->knnSearch(query, indices, dists, k, flann::SearchParams(checks_));

		size_t num = indices[0].size();
		neighbors.resize(num);
		squared_distances.resize(num);
		for (size_t i=0; i<num; ++i) {
			neighbors[i] = vertices_[ indices[0][i] ];
			squared_distances[i] = dists[0][i];
		}
}


void KdTreeSearch_FLANN::find_points_in_radius(
	const vec3& p, double squared_radius, std::vector<PointSet::Vertex*>& neighbors
	)  const {
		std::vector<double> squared_distances;
		return find_points_in_radius(p, squared_radius, neighbors, squared_distances);
}


void KdTreeSearch_FLANN::find_points_in_radius(
	const vec3& p, double squared_radius, std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances
	)  const {
		flann::Matrix<double> query(const_cast<double*>(p.data()), 1, 3);

		std::vector< std::vector<int> >		indices;
		std::vector< std::vector<double> >	dists;

		get_tree(tree_)->radiusSearch(query, indices, dists, squared_radius, flann::SearchParams(checks_));

		size_t num = indices[0].size();
		neighbors.resize(num);
		squared_distances.resize(num);
		for (size_t i=0; i<num; ++i) {
			neighbors[i] = vertices_[ indices[0][i] ];
			squared_distances[i] = dists[0][i];
		}
}
