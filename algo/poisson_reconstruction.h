
#ifndef _ALGOS_POISSON_RECONSTRUCTION_H_
#define _ALGOS_POISSON_RECONSTRUCTION_H_

#include "algo_common.h"
#include <string>

class Map;
class PointSet;

class ALGO_API PoissonReconstruction
{
public:
	PoissonReconstruction(void);
	~PoissonReconstruction(void);

	static std::string title() { return "PoissonRecon"; }

	//////////////////////////////////////////////////////////////////////////
	// reconstruction
	void set_octree_depth(int d) { octree_depth_ = d; }
	void set_sampers_per_node(float s) { samples_per_node_ = s; }
	Map* apply(const PointSet* pset, const std::string& density_attr_name = "density");

	// trimming
	static Map* trim(Map* mesh, const std::string& density_attr_name, float trim_value, float area_ratio, bool triangulate, int smooth);

public:
	// these parameters that usually do not need to change
	void set_full_depth(int v) { full_depth_ = v; }
	void set_voxel_depth_(int v) { voxelDepth_ = v; }
	void set_cg_depth(int v) { cgDepth_ = v; }
	void set_scale(float v) { scale_ = v; }
	void set_point_weight(float	v) { pointWeight_ = v; }
	void set_gs_iter(int v) { gsIter_ = v; }
	void set_confidence(bool v) { confidence_ = v; }
	void set_normal_weight(bool	v) { normalWeight_ = v; }
	void set_verbose(bool v) { verbose_ = v; }

private:
	/*
	This integer is the maximum depth of the tree that will be used for surface 
	reconstruction. Running at depth d corresponds to solving on a voxel grid 
	whose resolution is no larger than 2^d x 2^d x 2^d. Note that since the 
	reconstructor adapts the octree to the sampling density, the specified 
	reconstruction depth is only an upper bound.
	The default value for this parameter is 8.
	*/
	unsigned int octree_depth_;

	/*
	This integer specifies the depth beyond depth the octree will be adapted. 
	At coarser depths, the octree will be complete, containing all 2^d x 2^d x 2^d nodes.
	The default value for this parameter is 5.
	*/
	unsigned int full_depth_; // adaptive octree depth

	/*
	This floating point value specifies the minimum number of sample points that 
	should fall within an octree node as the octree construction is adapted to 
	sampling density. For noise-free samples, small values in the range [1.0 - 5.0] 
	can be used. For more noisy samples, larger values in the range [15.0 - 20.0] 
	may be needed to provide a smoother, noise-reduced, reconstruction.
	The default value is 1.0.
	*/
	float	samples_per_node_;

	bool	triangulate_mesh_;

private:
	int		voxelDepth_;
	int		cgDepth_;
	float	scale_;
	float	pointWeight_;
	int     gsIter_;
	int		threads_;
	bool	confidence_;
	bool	normalWeight_;
	bool	verbose_;
};



#endif