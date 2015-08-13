
#ifndef _MAP_SERIALIZER_PLY_H_
#define _MAP_SERIALIZER_PLY_H_

#include "file_io_common.h"
#include "map_serializer.h"


class FILE_IO_API MapSerializer_ply : public MapSerializer
{
public:
	MapSerializer_ply();

	virtual bool streams_supported() const ;

	virtual bool serialize_read(
		const std::string& file_name, Map* mesh) ; 

	virtual bool serialize_write(
		const std::string& file_name, const Map* mesh) const;

	//////////////////////////////////////////////////////////////////////////
	
	// test if face info are stored in the ply file (otherwise a point cloud).
	static int num_facet(const std::string& file_name);
} ;

#endif

