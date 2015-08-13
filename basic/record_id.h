
#ifndef __RECORD_ID__
#define __RECORD_ID__

#include "basic_common.h"
#include "basic_types.h"
#include "assertions.h"
#include <iostream>

// #define OGF_ATTRIBUTE_CHECK

#ifdef OGF_ATTRIBUTE_CHECK
#define ogf_attribute_assert(x) ogf_assert(x)
#else
#define ogf_attribute_assert(x) 
#endif



//_________________________________________________________

/**
* Represents the unique id of a record created in an
* AttributeManager. Since attributes are allocated in
* chunks of 1024 elements, a RecordId is internally
* represented as a 32 bits number, divided into a 10 bits
* number representing the offset in a chunk, and as a 20 bits
* number referring to the chunk. The two remaining bits are 
* used for the management of the free list in the record 
* allocation table (RAT).
*/

class BASIC_API RecordId {

public:
	/** default constructor, creates a nil RecordId */
	RecordId() : data_(~Numeric::uint32(0)) { } ;
	bool is_nil() const { return data_ == ~Numeric::uint32(0); }
	/** make this RecordId nil */
	void forget() { data_ = ~Numeric::uint32(0); }
	RecordId(const RecordId& rhs) : data_(rhs.data_) { }
	RecordId& operator=(const RecordId& rhs) { 
		data_ = rhs.data_ ; return *this ;
	}


	/** 
	* only chunk and offset are compared, 
	* flags are not taken into account 
	*/
	bool operator==(const RecordId& rhs) const ;

	/** 
	* only chunk and offset are compared, 
	* flags are not taken into account 
	*/
	bool operator!=(const RecordId& rhs) const ;

protected:
	friend class RAT ;
	friend class RawAttributeStore ;
	friend class AttributeManager ;

	enum {
		OFFSET_BITS = 10,
		CHUNK_BITS  = 20
	} ;

	enum {
		OFFSET_SHIFT = 0,
		MAX_OFFSET   = (1 << OFFSET_BITS) - 1, 
		OFFSET_MASK  = (MAX_OFFSET << OFFSET_SHIFT)

	} ;

	enum {
		CHUNK_SHIFT = OFFSET_BITS,
		MAX_CHUNK   = (1 << CHUNK_BITS) - 1,
		CHUNK_MASK  = (MAX_CHUNK << CHUNK_SHIFT)
	} ;

	enum {
		FREE_SHIFT = OFFSET_BITS + CHUNK_BITS,
		FREE_MASK = (1 << FREE_SHIFT)
	} ;

	enum {
		MARKED_SHIFT = FREE_SHIFT + 1,
		MARKED_MASK = (1 << MARKED_SHIFT)
	} ;

	// --------------- Chunk and offset access -------------------

	RecordId(
		unsigned int chunk_in, unsigned int offset_in,
		bool free_in = false, bool marked_in = false
		) {
			ogf_attribute_assert(chunk_in <= MAX_CHUNK) ;
			ogf_attribute_assert(offset_in <= MAX_OFFSET) ;
			data_ = (chunk_in << CHUNK_SHIFT) |
				(offset_in << OFFSET_SHIFT)  ;
			if(free_in)   { free() ; }
			if(marked_in) { mark() ; }
	}

	RecordId(Numeric::uint32 data_in) : data_(data_in) {
	}

	unsigned int chunk() const { 
		return ((data_ & CHUNK_MASK) >> CHUNK_SHIFT) ;
	}

	unsigned int offset() const {
		return ((data_ & OFFSET_MASK) >> OFFSET_SHIFT) ;
	}

	// --------------- Flags -------------------------------------

	bool is_free() const   { return ((data_ & FREE_MASK) != 0);  } 
	void free()   { data_ |= FREE_MASK ;  }
	void unfree() { data_ &= ~FREE_MASK ; }

	bool is_marked() const { return ((data_ & MARKED_MASK) != 0); } 
	void mark()   { data_ |= MARKED_MASK ;  }
	void unmark() { data_ &= ~MARKED_MASK ; }

	// --------------- printing ----------------------------------

	friend std::ostream& operator<<(
		std::ostream& out, const RecordId& r
		) ;

	std::ostream& print(std::ostream& out = std::cerr) const {
		if(is_nil()) {
			out << "RecordId: nil" << std::endl ;
		} else {
			out << "RecordId: chunk=" << chunk() 
				<< " offset=" << offset() << " "
				<< (is_free() ? "free" : "!free") << " "
				<< (is_marked() ? "marked" : "!marked") ;
		}
		return out ;
	}

private:
	Numeric::uint32 data_;
} ;

//_________________________________________________________

/**
* Base class for all the types used to reference attributes.
*/
class BASIC_API Record {

protected:
	const RecordId& record_id() const { return record_id_ ; }
	RecordId& record_id() { return record_id_ ; }
	void set_record_id(RecordId id) { record_id_ = id ; }

	friend class AttributeManager ;
	friend class RawAttributeStore ;
private:
	RecordId record_id_ ;
} ;

//_________________________________________________________

inline bool RecordId::operator==(const RecordId& rhs) const {
	return 
		(chunk()  == rhs.chunk() )  &&
		(offset() == rhs.offset()) ;
}


inline bool RecordId::operator!=(const RecordId& rhs) const {
	return !((*this) == rhs) ;
}

inline std::ostream& operator<<(std::ostream& out, const RecordId& r) {
	r.print(out) ;
	return out ;
}


#endif


