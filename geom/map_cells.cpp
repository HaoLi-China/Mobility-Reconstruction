
#include "map_cells.h"
#include "assert.h"


namespace MapTypes {

	//______________________________________

	bool Vertex::is_valid() const {
		return halfedge()->vertex() == this ;
	}

	void Vertex::assert_is_valid() const {
		ogf_assert(halfedge()->vertex() == this) ;
	}

	unsigned int Vertex::degree() const {
		unsigned int result = 0 ;
		Halfedge* it = halfedge() ;
		do {
			result++ ;
			it = it->next_around_vertex() ;
		} while(it != halfedge()) ;
		return result ;
	}

	bool Vertex::is_on_border() const {
		Halfedge* it = halfedge() ;
		do {
			if(it->is_border()) {
				return true ;
			}
			it = it->next_around_vertex() ;
		} while(it != halfedge()) ;
		return false ;
	}

	bool Vertex::is_connected(const Vertex* v) const {
		Halfedge* it = halfedge() ;
		do {
			if ( it->opposite()->vertex() == v )
				return true;

			it = it->next_around_vertex();
		} while(it != halfedge()) ;

		return false;
	}

	//______________________________________

	bool Halfedge::is_valid() const {
		return (
			(opposite()->opposite() == this) &&
			(next()->prev() == this) &&
			(prev()->next() == this) 
			) ;
	}

	void Halfedge::assert_is_valid() const {
		ogf_assert(opposite()->opposite() == this) ;
		ogf_assert(next()->prev() == this) ;
		ogf_assert(prev()->next() == this) ;
	}

	//______________________________________

	int Facet::degree() const {
		int result = 0 ;
		Halfedge* it = halfedge() ;
		do {
			result++ ;
			it = it->next() ;
		} while(it != halfedge()) ;
		return result ;
	}

	bool Facet::is_triangle() const {
		return ( halfedge()->next()->next()->next() == halfedge() ) ;
	}

	bool Facet::is_on_border() const {
		Halfedge* it = halfedge() ;
		do {
			if(it->opposite()->is_border()) {
				return true ;
			}
			it = it->next() ;
		} while(it != halfedge()) ;
		return false ;
	}

	bool Facet::is_valid() const {
		return (
			halfedge()->facet() == this &&
			degree() > 2
			) ;
	}

	void Facet::assert_is_valid() const {
		ogf_assert(halfedge()->facet() == this) ;
		ogf_assert(nb_edges() > 2) ;
	}

	//______________________________________

}
