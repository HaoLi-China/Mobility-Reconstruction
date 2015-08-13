
#ifndef _GEOM_MAP_TOPOLOGY_H_
#define _GEOM_MAP_TOPOLOGY_H_

#include "geom_common.h"

class Map;
class MapComponent ;

//_________________________________________________________

/**
* computes some topological invariants of a MapComponent.
*/
class GEOM_API MapComponentTopology
{
public:
	MapComponentTopology(const MapComponent* comp) ;

	/* returns the Euler-Poincare characteristic,
	* xi = 2 for a sphere, xi = 1 for a disc. */
	int euler_poincare() const ;

	/** returns 0 for a closed surface. */
	int number_of_borders()  const  { return number_of_borders_ ;   }

	/** returns the number of edges in the largest border. */
	int largest_border_size() const { return largest_border_size_ ; }

	bool is_closed() const { return number_of_borders_ == 0 ; }
	bool is_almost_closed( int max_border_size = 3 ) const ;

	bool is_sphere()   const { return (number_of_borders() == 0) && (euler_poincare() == 2) ; }        
	bool is_disc()     const { return (number_of_borders() == 1) && (euler_poincare() == 1) ; }
	bool is_cylinder() const { return (number_of_borders() == 2) && (euler_poincare() == 0) ; }
	bool is_torus()    const { return (number_of_borders() == 0) && (euler_poincare() == 0) ; }


private:
	const MapComponent* component_ ;

	int number_of_borders_ ;
	int largest_border_size_ ;
} ;

//_________________________________________________________

#endif

