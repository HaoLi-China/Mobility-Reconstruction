#include "map_io.h"
#include "../basic/logger.h"
#include "../basic/file_utils.h"
#include "../geom/map.h"
#include "../basic/stop_watch.h"
#include "map_serializer.h"
#include "map_serializer_obj.h"
#include "map_serializer_ply.h"
#include "map_serializer_ply2.h"
#include "map_serializer_off.h"
#include "map_serializer_stl.h"


Map* MapIO::read(const std::string& file_name)
{
	MapSerializer_var serializer = resolve_serializer(file_name);
	if (!serializer.is_nil()) {
		Map* mesh = new Map;

		StopWatch w;
		Logger::out(title()) << "reading file ..." << std::endl;

		if (serializer->serialize_read(file_name, mesh)) {
			Logger::out(title()) << "done. Time=" << w.elapsed() << std::endl;
			return mesh;
		}
		else {
			delete mesh;
			Logger::err(title()) << "reading file failed" << std::endl;
		}
	} 

	return nil;
}


bool MapIO::save(const std::string& file_name, const Map* mesh) 
{
	MapSerializer_var serializer = resolve_serializer(file_name);
	if (!serializer.is_nil()) {
		StopWatch w;
		Logger::out(title()) << "saving file ..." << std::endl;

		if (serializer->serialize_write(file_name, mesh))  {
			Logger::out(title()) << "done. Time=" << w.elapsed() << std::endl;
			return true;
		}
		else {
			Logger::err(title()) << "saving file failed" << std::endl;
			return false;
		}
	}

	return false;
}


MapSerializer* MapIO::resolve_serializer(const std::string& file_name) {
	std::string extension = FileUtils::extension(file_name) ;
	String::to_lowercase(extension);

	if(extension.length() == 0) {
		Logger::err(title()) << "No extension in file name" << std::endl ;
		return nil ;
	}

	MapSerializer* serializer = nil;

	if ( extension == "obj" )
		serializer = new MapSerializer_obj();
	else if ( extension == "eobj" )
		serializer = new MapSerializer_eobj();
	else if ( extension == "ply" )
		serializer = new MapSerializer_ply();
	else if ( extension == "ply2" ) 
		serializer = new MapSerializer_ply2();
	else if ( extension == "off" ) 
		serializer = new MapSerializer_off();
	else if ( extension.find("stl", 0) != std::string::npos ) 
		serializer = new MapSerializer_stl();
	else { 	
		Logger::err(title()) << "unknown file format" << std::endl;
		return nil;
	}

	return serializer;
}


int MapIO::ply_file_num_facet(const std::string& file_name) {
	return MapSerializer_ply::num_facet(file_name);
}