
#ifndef _RAW_ATTRIBUTE_STORE__
#define _RAW_ATTRIBUTE_STORE__

#include "basic_common.h"
#include "record_id.h"

#include <vector>
#include <set>


/**
* Base class for the low-level storage for attributes. Client
* code should not need to use this directly. The storage space
* is allocated in chunks, to make dynamic growing more efficient
* (i.e. without needing to copy the data).
*/
class BASIC_API RawAttributeStore {
public:
	enum { CHUNK_SIZE = RecordId::MAX_OFFSET + 1 } ;

	RawAttributeStore(unsigned int item_size) : item_size_(item_size) { }
	virtual void clear() ; 
	virtual ~RawAttributeStore() ;
	unsigned int item_size() const { return item_size_ ; }
	unsigned int nb_chunks() const { return (unsigned int)data_.size() ; }
	unsigned int capacity() const { 
		return (unsigned int) data_.size() * CHUNK_SIZE ;
	}

	Memory::pointer data(
		unsigned int chunk, unsigned int offset
		) const {
			ogf_attribute_assert(chunk < data_.size()) ;
			ogf_attribute_assert(offset < CHUNK_SIZE) ;
			return &(data_[chunk][offset * item_size_]) ;
	}

	Memory::pointer data(const Record& r) const {
		return data(r.record_id().chunk(), r.record_id().offset()) ;
	}

	virtual void grow() ;

private:
	unsigned int item_size_ ;
	std::vector<Memory::pointer> data_ ;
} ;




#endif

