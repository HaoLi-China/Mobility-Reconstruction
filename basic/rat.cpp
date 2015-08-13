
#include "rat.h"


void RAT::clear() {
	RawAttributeStore::clear() ;
	free_list_.forget() ;
}

RecordId RAT::new_record_id() {
	ogf_attribute_assert(!is_full()) ;
	RecordId result = free_list_ ;
	free_list_ = cell(free_list_) ;
	cell(result).unfree() ;
	return result ;
}

void RAT::delete_record_id(RecordId record) {
	RecordId& ref = cell(record) ;
	ogf_attribute_assert(!ref.is_free()) ;
	ref = free_list_ ;
	ref.free() ;
	free_list_ = record ;
}


void RAT::grow() {
	RawAttributeStore::grow() ;
	unsigned int chunk = nb_chunks() - 1 ;
	for(unsigned int i=0; i<CHUNK_SIZE-1; i++) {
		cell(chunk,i)=RecordId(chunk,i+1,true) ;
	}
	cell(chunk,CHUNK_SIZE-1) = free_list_ ;
	free_list_ = RecordId(chunk,0) ;
}

