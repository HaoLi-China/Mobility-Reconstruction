
#ifndef _ATTRIBUTE_STORE_H_
#define _ATTRIBUTE_STORE_H_


#include "basic_common.h"
#include "raw_attribute_store.h"
#include "attribute_life_cycle.h"
#include "smart_pointer.h"
#include "counted.h"

#include <typeinfo>



class AttributeManager ;
class Record ;

/**
* stores an attribute, and knows how to construct,copy,destroy
* instances of the attribute. This class should not be used
* directly by client code.
*/
class BASIC_API AttributeStore : public Counted, public RawAttributeStore {
public:

	AttributeStore(
		AttributeLifeCycle* life_cycle,
		AttributeManager* manager = nil
		) : RawAttributeStore( life_cycle->item_size() ), 
		life_cycle_(life_cycle), manager_(nil) {
			bind(manager) ;
	}

	virtual ~AttributeStore() ;

	void construct(
		Memory::pointer addr, Record* record = 0
		) {
			life_cycle_->construct(addr,record) ;
	}

	void destroy(
		Memory::pointer addr, Record* record = 0
		) {
			life_cycle_->destroy(addr,record) ;
	}

	void copy(
		Memory::pointer lhs, Record* record_lhs,
		Memory::pointer rhs, const Record* record_rhs
		) {
			life_cycle_->copy(lhs,record_lhs,rhs,record_rhs) ;
	}

	void copy_construct(
		Memory::pointer lhs, Record* record_lhs,
		Memory::pointer rhs, const Record* record_rhs
		) {
			life_cycle_->copy_construct(lhs,record_lhs,rhs,record_rhs) ;
	}

	void bind(AttributeManager* manager) ;
	AttributeManager* attribute_manager() const { return manager_; }

	virtual const std::type_info& attribute_type_id() const = 0 ;

	/** returns an empty AttributeStore() of the same type. */
	virtual AttributeStore* clone() = 0 ;

protected:
	AttributeLifeCycle_var life_cycle_ ;
	AttributeManager* manager_ ;
} ;

typedef SmartPointer<AttributeStore> AttributeStore_var ;

//_________________________________________________________

/**
* A typed AttributeStore, templated by the 
* Record class and the Attribute class. This
* is used for static and dynamic type checking
* in the AttributeManager.
*/
template <class ATTRIBUTE> 
class GenericAttributeStore : public AttributeStore {
public:
	GenericAttributeStore(
		AttributeLifeCycle* life_cycle,
		AttributeManager* manager = nil
		) : AttributeStore(life_cycle, manager) { 
	}        
	virtual ~GenericAttributeStore() { }
	virtual const std::type_info& attribute_type_id() const {
		return typeid(ATTRIBUTE) ;
	}
	virtual AttributeStore* clone() {
		return new GenericAttributeStore<ATTRIBUTE>(life_cycle_) ;
	}
} ;



#endif

