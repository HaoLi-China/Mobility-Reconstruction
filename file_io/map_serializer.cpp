

#include "../basic/logger.h"
#include "../basic/assertions.h"
#include "../geom/map_builder.h"
#include "map_serializer.h"

#include <fstream>


bool MapSerializer::do_read(std::istream& in, AbstractMapBuilder& builder) 
{
	bool implemented = false ;
	ogf_assert(implemented) ;
	return false ;
}

bool MapSerializer::do_write(std::ostream& out, const Map* mesh) const 
{
	bool implemented = false ;
	ogf_assert(implemented) ;
	return false ;
}

bool MapSerializer::serialize_read(const std::string& file_name, Map* mesh) 
{
	if (!mesh) {
		Logger::err("MapSerializer") << "mesh is null" << std::endl;
		return false;
	}
	
	if (!read_supported_) {
		Logger::warn("MapSerializer")
			<< "Reading for this file format is NOT implemented"
			<< std::endl;
		return false;
	}
	std::fstream::openmode mode = binary() ?
		(std::fstream::in | std::fstream::binary) : std::fstream::in ;

	std::ifstream input(file_name.c_str(), mode) ;
	if(input.fail()) {
		Logger::err("MapSerializer") 
			<< "Could not open file\'" 
			<< file_name << "\'" 
			<< std::endl ;
		return false ;
	}

	MapBuilder builder(mesh);
	return do_read(input, builder);
}

bool MapSerializer::serialize_write(
									const std::string& file_name, const Map* mesh) const 
{
	if (!mesh) {
		Logger::warn("MapSerializer") << "Mesh is null" << std::endl;
		return false;
	}
	
	if (!write_supported_) {
		Logger::warn("MapSerializer") 
			<< "Writing for this file format is NOT implemented"
			<< std::endl;
		return false;
	}

	std::fstream::openmode mode = binary() ?
		(std::fstream::out | std::fstream::trunc | std::fstream::binary) :
	(std::fstream::out | std::fstream::trunc) ;

	std::ofstream output(file_name.c_str(), mode) ;

	if(output.fail()) {
		Logger::err("MapSerializer") << "Could not open file\'" 
			<< file_name << "\'" << std::endl ;
		return false ;
	}

	if(!binary())
		output.precision(16) ;

	return do_write(output, mesh) ;
}



bool MapSerializer::binary() const {
	return false ;
}

bool MapSerializer::streams_supported() const {
	return true ;
}

bool MapSerializer::read_supported() const {
	return read_supported_ ;
}

bool MapSerializer::write_supported() const {
	return write_supported_ ;
}
