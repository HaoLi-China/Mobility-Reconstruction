
#ifndef _BASIC_GENERIC_ATTRIBUTES_IO_H_
#define _BASIC_GENERIC_ATTRIBUTES_IO_H_

#include "basic_common.h"
#include "attribute_manager.h"
#include "attribute_serializer.h"
#include <iostream>


template <class T> 
inline bool get_serializable_attributes(
	GenericAttributeManager<T>* manager, std::vector<SerializedAttribute<T> >& attributes,
	std::ostream& out, const std::string& location, const std::string& attribute_kw = "# attribute"
	) {
		bool result = false ;
		std::vector<std::string> names ;
		manager->list_named_attributes(names) ;
		for(unsigned int i=0; i<names.size(); i++) {
			attributes.push_back(SerializedAttribute<T>()) ;
			attributes.rbegin()->bind(manager, names[i]) ;
			if(attributes.rbegin()->is_bound()) {
				std::cerr << "Attribute " << names[i] << " on " << location << " : " 
					<< attributes.rbegin()->type_name() << std::endl ;
				out << attribute_kw << " " << names[i] << " " << location << " " 
					<< attributes.rbegin()->type_name() << std::endl ;
				result = true ;
			} else {
				std::cerr << "Attribute " << names[i] << " on " << location 
					<< " is not serializable" << std::endl ;
				attributes.pop_back() ;
			}
		}
		return result ;
}

template <class T> 
inline void serialize_read_attributes(
	std::istream& in, const T* item, std::vector<SerializedAttribute<T> >& attributes
	) {
		for(unsigned int i=0; i<attributes.size(); i++) {
			in >> attributes[i][item] ;
		}
}

template <class T> 
inline void serialize_write_attributes(
	std::ostream& out, const T* item, std::vector<SerializedAttribute<T> >& attributes
	) {
		for(unsigned int i=0; i<attributes.size(); i++) {
			out << attributes[i][item] << " " ;
		}
}


#endif

