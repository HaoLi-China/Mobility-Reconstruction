
#include "traced.h"
#include <iostream>



int Traced::last_id_ = 0 ;

Traced::Traced() {
	last_id_++ ;
	id_ = last_id_ ;
	std::cout << "constr Traced # " << id_ << std::endl ;
}

Traced::Traced(const Traced& rhs) {
	last_id_++ ;
	id_ = last_id_ ;
	std::cout << "copy constr Traced # " << id_ 
		<< " from Traced # " << rhs.id_ << std::endl ;
}

Traced::~Traced() {
	std::cout << "destr Traced # " << id_ << std::endl ;    
}

Traced& Traced::operator=(const Traced& rhs) {
	std::cout << "operator= Traced # " << id_ 
		<< " from Traced # " << rhs.id_ << std::endl ;
	return *this ;
}

