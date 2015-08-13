
#include "kdtree_search_ann.h"
#include "ANN/ANN.h"
#include <algorithm>

using namespace ANN;

#define get_tree(x) ((ANNkd_tree*)(x))

KdTreeSearch_ANN::KdTreeSearch_ANN()  {
	points_ = nil;
	points_num_ = 0;
	tree_ = nil;
	k_for_radius_search_ = 32;
}


KdTreeSearch_ANN::~KdTreeSearch_ANN() {
	if(points_)
		annDeallocPts(points_);

    delete get_tree(tree_);
	annClose(); 
}


void KdTreeSearch_ANN::begin()  {
	vertices_.clear();

	if(points_)
		annDeallocPts(points_);

    delete get_tree(tree_);
	tree_ = nil;
}


void KdTreeSearch_ANN::end()  {
	points_num_ = vertices_.size();
	points_ = annAllocPts(points_num_, 3);

	for(unsigned int i=0; i<points_num_; ++i) {
		const vec3& p = vertices_[i]->point();
		points_[i][0] = p[0];
		points_[i][1] = p[1];
		points_[i][2] = p[2];
	}

	tree_ = new ANNkd_tree(points_, points_num_, 3);
}


void KdTreeSearch_ANN::add_point(PointSet::Vertex* v)  {
	vertices_.push_back(v);
}


void KdTreeSearch_ANN::add_vertex_set(PointSet* vs)  {
	for(PointSet::Vertex_iterator it = vs->vertices_begin() ; it != vs->vertices_end() ; ++it)
		vertices_.push_back(it);
}


PointSet::Vertex* KdTreeSearch_ANN::find_closest_point(const vec3& p) const {
	ANNcoord ann_p[3];

	ANNidx closest_pt_ix;
	ANNdist closest_pt_dist;

	ann_p[0] = p[0];
	ann_p[1] = p[1];
	ann_p[2] = p[2];

	get_tree(tree_)->annkSearch(ann_p, 1, &closest_pt_ix, &closest_pt_dist);

	return vertices_[closest_pt_ix];
}

PointSet::Vertex* KdTreeSearch_ANN::find_closest_point(const vec3& p, double& squared_distance) const {
	ANNcoord ann_p[3];
	ann_p[0] = p[0];
	ann_p[1] = p[1];
	ann_p[2] = p[2];

	ANNidx closest_pt_ix;
	ANNdist closest_pt_dist;


	get_tree(tree_)->annkSearch(ann_p, 1, &closest_pt_ix, &closest_pt_dist);
	squared_distance = closest_pt_dist; // ANN uses squared distance internally

	return vertices_[closest_pt_ix];
}

void KdTreeSearch_ANN::find_closest_K_points(
	const vec3& p, unsigned int k, std::vector<PointSet::Vertex*>& neighbors
	)  const {
		ANNcoord ann_p[3];
		ann_p[0] = p[0];
		ann_p[1] = p[1];
		ann_p[2] = p[2];

		ANNidxArray  closest_pts_idx = new ANNidx[k];		// near neighbor indices
		ANNdistArray closest_pts_dists = new ANNdist[k];	// near neighbor distances
		get_tree(tree_)->annkSearch(ann_p, k, closest_pts_idx, closest_pts_dists);

		neighbors.resize(k);
		for (unsigned int i=0; i<k; ++i) {
			neighbors[i] = vertices_[ closest_pts_idx[i] ];
		}

		delete [] closest_pts_idx;
		delete [] closest_pts_dists;
}

void KdTreeSearch_ANN::find_closest_K_points(
	const vec3& p, unsigned int k, std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances
	)  const {
		ANNcoord ann_p[3];
		ann_p[0] = p[0];
		ann_p[1] = p[1];
		ann_p[2] = p[2];

		ANNidxArray  closest_pts_idx = new ANNidx[k];		// near neighbor indices
		ANNdistArray closest_pts_dists = new ANNdist[k];	// near neighbor distances
		get_tree(tree_)->annkSearch(ann_p, k, closest_pts_idx, closest_pts_dists);

		neighbors.resize(k);
		squared_distances.resize(k);
		for (unsigned int i=0; i<k; ++i) {
			neighbors[i] = vertices_[ closest_pts_idx[i] ];
			squared_distances[i] = closest_pts_dists[i]; // ANN uses squared distance internally
		}

		delete [] closest_pts_idx;
		delete [] closest_pts_dists;
}



void KdTreeSearch_ANN::find_points_in_radius(
	const vec3& p, double squared_radius, std::vector<PointSet::Vertex*>& neighbors
	)  const {
		ANNcoord ann_p[3];
		ann_p[0] = p[0];
		ann_p[1] = p[1];
		ann_p[2] = p[2];

		ANNidxArray  closest_pts_idx = new ANNidx[k_for_radius_search_];		// near neighbor indices
		ANNdistArray closest_pts_dists = new ANNdist[k_for_radius_search_];		// near neighbor distances
		int n = get_tree(tree_)->annkFRSearch(ann_p, squared_radius, k_for_radius_search_, closest_pts_idx, closest_pts_dists);
		
		int num = std::min(n, k_for_radius_search_);
		neighbors.resize(num);
		for (int i=0; i<num; ++i) {
			neighbors[i] = vertices_[ closest_pts_idx[i] ];
		}

		delete [] closest_pts_idx;
		delete [] closest_pts_dists;
}


void KdTreeSearch_ANN::find_points_in_radius(
	const vec3& p, double squared_radius, std::vector<PointSet::Vertex*>& neighbors, std::vector<double>& squared_distances
	)  const {
		ANNcoord ann_p[3];
		ann_p[0] = p[0];
		ann_p[1] = p[1];
		ann_p[2] = p[2];

		ANNidxArray  closest_pts_idx = new ANNidx[k_for_radius_search_];		// near neighbor indices
		ANNdistArray closest_pts_dists = new ANNdist[k_for_radius_search_];		// near neighbor distances
		int n = get_tree(tree_)->annkFRSearch(ann_p, squared_radius, k_for_radius_search_, closest_pts_idx, closest_pts_dists);
		
		int num = std::min(n, k_for_radius_search_);
		neighbors.resize(num);
		squared_distances.resize(num);
		for (int i=0; i<num; ++i) {
			neighbors[i] = vertices_[ closest_pts_idx[i] ];
			squared_distances[i] = closest_pts_dists[i]; // ANN uses squared distance internally
		}

		delete [] closest_pts_idx;
		delete [] closest_pts_dists;
}
