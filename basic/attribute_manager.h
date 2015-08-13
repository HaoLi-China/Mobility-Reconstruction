
#ifndef _ATTRIBUTE_MANAGER_H_
#define _ATTRIBUTE_MANAGER_H_

#include "basic_common.h"
#include "rat.h"
#include "attribute_store.h"

#include <set>
#include <map>
#include <vector>
#include <string>
#include <typeinfo>



class BASIC_API AttributeManager {
public:

	enum Mode { FIND=1, CREATE=2, FIND_OR_CREATE=3} ;

	AttributeManager() : size_(0) { }
	virtual ~AttributeManager() ;
	unsigned int capacity() { return rat_.capacity(); }
	unsigned int size() { return size_; }

	void clear() ;

	/**
	* creates new record attributes, and puts the resulting id
	* in the specified Record
	*/
	void new_record(Record* to) ;


	/**
	* creates new record attributes, initialized from the source record,
	* and puts the resulting id in the specified Record
	*/
	void new_record(Record* to, const Record* from) ;

	/**
	* copies all the attributes of the from Record to the to Record.
	*/
	void copy_record(Record* to, const Record* from) ;

	/**
	* destroys the record attributes corresponding to the
	* specified record.
	*/
	void delete_record(Record* record) ;

	void list_named_attributes(std::vector<std::string>& names) ;
	bool named_attribute_is_bound(const std::string& name) ;
	void delete_named_attribute(const std::string& name) ;

	virtual const std::type_info& record_type_id() const = 0 ;
	/* For an easy access to attribute type Jeanne 01/2010 */
	const std::type_info& resolve_named_attribute_type_id( const std::string& name ) ;

protected:

	/**
	* adds the AttributeStore to the list of managed
	* attributes, resizes it, and constructs
	* all the elements which are not in the free list.
	*/
	void register_attribute_store(AttributeStore* as) ;

	/**
	* detroys all the elements which are not in the
	* free list, and removes the AttributeStore from
	* the list of managed attributes.
	*/
	void unregister_attribute_store(AttributeStore* as) ;

	void bind_named_attribute_store(
		const std::string& name, AttributeStore* as
		) ;

	AttributeStore* resolve_named_attribute_store(
		const std::string& name
		) ;

	RAT& rat() { return rat_ ; }
	const RAT& rat() const { return rat_ ; }

	friend class AttributeStore ;
	friend class AttributeBase ;
	template <class RECORD> friend class AttributeCopier ;

private:
	RAT rat_ ;
	std::set<AttributeStore*> attributes_ ;
	std::map<std::string, AttributeStore_var> named_attributes_ ;

	int size_ ;
} ;


template <class RECORD> 
class GenericAttributeManager : public AttributeManager {
	virtual const std::type_info& record_type_id() const {
		return typeid(RECORD) ;
	}
} ;



#endif

