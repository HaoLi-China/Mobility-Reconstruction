
#include "raw_attribute_store.h"
#include <typeinfo>
#include <string.h>


void RawAttributeStore::clear() {
	for(
		std::vector<Memory::pointer>::iterator 
		it=data_.begin(); it!=data_.end(); it++
		) {
			// Paranoid stuff: reset freed memory to zero
#ifdef OGF_ATTRIBUTE_CHECK
			Memory::clear(*it, CHUNK_SIZE * item_size_) ;
#endif
			delete[] *it ;
	}
	data_.clear() ;
}

RawAttributeStore::~RawAttributeStore() {
	clear() ;
}

void RawAttributeStore::grow() {
	Memory::pointer chunk = new Memory::byte[
		CHUNK_SIZE * item_size_
	] ;
	// Paranoid stuff: initialize allocated memory to zero
#ifdef OGF_ATTRIBUTE_CHECK
	Memory::clear(chunk, CHUNK_SIZE * item_size_) ;
#endif
	data_.push_back(chunk) ;
	//        std::cerr << "RawAttributeStore (" << typeid(*this).name() 
	//                  << ") grow" << std::endl ;
}