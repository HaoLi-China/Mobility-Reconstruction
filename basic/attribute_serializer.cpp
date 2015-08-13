

#include "attribute_serializer.h"



AttributeSerializer::SerializerMap* AttributeSerializer::type_to_serializer_ = nil ;
AttributeSerializer::SerializerMap* AttributeSerializer::name_to_serializer_  = nil ;
AttributeSerializer::StringMap*     AttributeSerializer::alias_to_name_       = nil ;    
AttributeSerializer::StringMap*     AttributeSerializer::type_to_name_        = nil ;    

void AttributeSerializer::initialize() {
	ogf_assert(type_to_serializer_ == nil) ;
	type_to_serializer_ = new SerializerMap ;
	ogf_assert(name_to_serializer_ == nil) ;
	name_to_serializer_ = new SerializerMap ;
	ogf_assert(alias_to_name_ == nil) ;
	alias_to_name_ = new StringMap ;
	ogf_assert(type_to_name_ == nil) ;
	type_to_name_ = new StringMap ;
}

void AttributeSerializer::terminate() {
	delete type_to_serializer_ ;
	type_to_serializer_ = nil ;
	delete name_to_serializer_ ;
	name_to_serializer_ = nil ;
	delete alias_to_name_ ;
	alias_to_name_ = nil ;
	delete type_to_name_ ;
	type_to_name_ = nil ;
}

AttributeSerializer* AttributeSerializer::resolve_by_type(const std::type_info& attribute_type) {
	ogf_assert(type_to_serializer_ != nil) ;
	SerializerMap::iterator it = type_to_serializer_->find(attribute_type.name()) ;
	if(it == type_to_serializer_->end()) {
		return nil ;
	}
	return it->second ;
}

AttributeSerializer* AttributeSerializer::resolve_by_name(const std::string& type_name_in) {
	ogf_assert(alias_to_name_ != nil) ;
	std::string type_name = type_name_in ;
	{
		StringMap::const_iterator it = alias_to_name_->find(type_name_in) ;
		if(it != alias_to_name_->end()) {
			type_name = it->second ;
		}
	}
	ogf_assert(name_to_serializer_ != nil) ;       
	SerializerMap::iterator it = name_to_serializer_->find(type_name) ;
	if(it == name_to_serializer_->end()) {
		return nil ;
	}
	return it->second ;
}


std::string AttributeSerializer::find_name_by_type(const std::type_info& attribute_type) {
	ogf_assert(type_to_name_ != nil) ; 
	StringMap::iterator it = type_to_name_->find(attribute_type.name()) ;
	if(it == type_to_name_->end()) {
		return "unknown" ;
	}
	return it->second ;
}

void AttributeSerializer::bind(
							   const std::type_info& attribute_type, const std::string& attribute_type_name, 
							   AttributeSerializer* serializer
							   ) 
{
	ogf_assert(resolve_by_type(attribute_type) == nil) ;
	ogf_assert(resolve_by_name(attribute_type_name) == nil) ;
	(*type_to_serializer_)[attribute_type.name()] = serializer ;
	(*name_to_serializer_)[attribute_type_name] = serializer ;
	(*type_to_name_)[attribute_type.name()] = attribute_type_name ;
}
