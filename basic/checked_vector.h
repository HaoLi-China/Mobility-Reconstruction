
#ifndef __BASIC_CONTAINERS_CHECKED_VECTOR__
#define __BASIC_CONTAINERS_CHECKED_VECTOR__


#include "assertions.h"
#include <vector>


/**
* checked_vector is a drop-in replacement of std::vector
* with an operator[] that does bound checking.
*/
template <class T>  
class checked_vector : public std::vector<T> {
public:
	typedef checked_vector<T> thisclass ;
	typedef std::vector<T> baseclass ;

	checked_vector() { }
	checked_vector(const thisclass& rhs) : baseclass(rhs) { }
	checked_vector(const baseclass& rhs) : baseclass(rhs) { }
	checked_vector(size_t nb) : baseclass(nb) { }
	checked_vector(size_t nb, const T& val) : baseclass(nb, val) { }

	thisclass& operator=(const thisclass& rhs) {
		baseclass::operator=(rhs) ; return *this ;
	}

	thisclass& operator=(const baseclass& rhs) {
		baseclass::operator=(rhs) ; return *this ;
	}

	const T& operator[](size_t idx) const {
		ogf_assert(idx < baseclass::size()) ;
		return baseclass::operator[](idx) ;
	}

	T& operator[](size_t idx) {
		ogf_assert(idx < baseclass::size()) ;
		return baseclass::operator[](idx) ;
	}

} ;


#endif
