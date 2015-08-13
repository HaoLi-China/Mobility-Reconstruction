
#ifndef _MAP_SERIALIZER_PLY2_H_
#define _MAP_SERIALIZER_PLY2_H_

#include "file_io_common.h"
#include "map_serializer.h"


class FILE_IO_API MapSerializer_ply2 : public MapSerializer
{
public:
	MapSerializer_ply2();

protected:
	virtual bool do_read(std::istream& in, AbstractMapBuilder& builder) ;        
	virtual bool do_write(std::ostream& out, const Map* mesh) const ;        
} ;


#endif 

