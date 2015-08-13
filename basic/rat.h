
#ifndef __RAT_H_
#define __RAT_H_

#include "basic_common.h" 
#include "raw_attribute_store.h"

/**
* RAT (Record Allocation Table) is used internally by
* the AttributeManager. It manages the free list.
*/

class BASIC_API RAT : public RawAttributeStore {
public:
	RAT() : RawAttributeStore(sizeof(RecordId)) { }

	bool is_full() const { return free_list_.is_nil() ; }

	virtual void clear() ; 

	/**
	* returns a new unique RecordId. If the RAT is full,
	* fails in an assertion check (grow() should be called
	* before).
	*/
	RecordId new_record_id() ;

	/**
	* adds a RecordId to the free list. It can
	* then be returned by a subsequent call to
	* new_record_id()
	*/
	void delete_record_id(RecordId record) ;

	/**
	* adds all the items of the new chunk to 
	* the free list.
	*/
	virtual void grow() ;

protected:
	RecordId& cell(unsigned int chunk, unsigned int offset) {
		return *reinterpret_cast<RecordId*>(
			RawAttributeStore::data(chunk,offset)
			) ;
	}

	RecordId& cell(RecordId index) {
		return cell(index.chunk(),index.offset()) ;
	}

	const RecordId& cell(unsigned int chunk, unsigned int offset) const {
		return *reinterpret_cast<RecordId*>(
			RawAttributeStore::data(chunk,offset)
			) ;
	}

	const RecordId& cell(RecordId index) const {
		return cell(index.chunk(),index.offset()) ;
	}

private:
	RecordId free_list_ ;

	friend class AttributeManager ;
} ;


#endif

