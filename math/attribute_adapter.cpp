
#include "attribute_adapter.h"


AttributeAdapterBase::SecondaryType AttributeAdapterBase::parse_name(std::string& name) {
	int point_pos = name.find('.')  ;
	if(point_pos < 0) {
		return ATTR_VALUE ;
	}
	std::string extension = name.substr(point_pos+1, name.length() - point_pos - 1) ;
	name = name.substr(0, point_pos) ;
	if(extension == "x" || extension == "real") {
		return ATTR_X ;
	} else if(extension == "y" || extension == "imag" || extension == "imaginary") {
		return ATTR_Y ;
	} else if(extension == "z") {
		return ATTR_Z ;
	} else if(extension == "norm" || extension == "modulus" || extension == "length") {
		return ATTR_NORM ;
	} 
	return ATTR_ERROR ;
}
