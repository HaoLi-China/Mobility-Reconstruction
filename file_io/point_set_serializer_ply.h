
#ifndef _POINT_SERIALIZER_PLY_H_
#define _POINT_SERIALIZER_PLY_H_

#include "file_io_common.h"
#include <string>

class PointSet;
class FILE_IO_API PointSetSerializer_ply
{
public:
	static std::string title() { return "[PointSetSerializer_ply]: "; }

	static PointSet* load(const std::string& file_name) ;
	static bool		 save(const std::string& file_name, const PointSet* pset) ;
} ;

#endif

