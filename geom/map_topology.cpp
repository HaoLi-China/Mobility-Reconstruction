
#include "map_topology.h"
#include "map_components.h"
#include "../basic/attribute.h"
#include "../geom/map.h"




//_________________________________________________________

MapComponentTopology::MapComponentTopology( const MapComponent* comp) 
{
	component_ = comp ;

	// Compute number_of_borders_

	Attribute<Map::Halfedge,bool> is_marked(
		comp->map()->halfedge_attribute_manager()
		) ;

	number_of_borders_ = 0 ;
	largest_border_size_ = 0 ;

	{ FOR_EACH_HALFEDGE_CONST(MapComponent, component_, it) {
		const Map::Halfedge* cur = it ;
		if(cur->is_border() && !is_marked[cur]) {
			number_of_borders_++ ;
			int border_size = 0 ;
			do {
				border_size++ ;
				is_marked[cur] = true ;
				cur = cur->next() ;
			} while(cur != it) ;
			largest_border_size_ = ogf_max(
				largest_border_size_, border_size
				) ;
		}
	}}
}


int MapComponentTopology::euler_poincare() const {
	// xi = #vertices + #facets - #edges
	// #edges = #halfedges / 2 
	return 
		int(component_->size_of_vertices())        - 
		int(component_->size_of_halfedges() / 2)   +
		int(component_->size_of_facets())          ;
}

bool MapComponentTopology::is_almost_closed(int max_border_size) const {
	if(component_->size_of_facets() == 1) {
		return false ;
	}
	return largest_border_size_ <= max_border_size ;
}

//_________________________________________________________



