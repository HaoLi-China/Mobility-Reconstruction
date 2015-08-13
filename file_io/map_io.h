#ifndef _MAP_IO_H_
#define _MAP_IO_H_

#include "file_io_common.h"
#include "../basic/basic_types.h"

#include <string>


class Map;
class MapSerializer;

class FILE_IO_API MapIO
{
public:
	static std::string title() { return "MapIO"; }

	static Map*	read(const std::string& file_name);
	static bool	save(const std::string& file_name, const Map* mesh) ;

	static MapSerializer* resolve_serializer(const std::string& file_name) ;

	// test if face info are stored in the ply file (otherwise a point cloud).
	static int ply_file_num_facet(const std::string& file_name);
};


#endif // _FILE_IO_H_