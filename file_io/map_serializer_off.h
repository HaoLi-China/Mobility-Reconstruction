
#ifndef _MAP_SERIALIZER_OFF_H_
#define _MAP_SERIALIZER_OFF_H_

#include "file_io_common.h"
#include "map_serializer.h"


class FILE_IO_API MapSerializer_off : public MapSerializer
{
public:
	MapSerializer_off() ;

protected:
	virtual bool do_read(std::istream& in, AbstractMapBuilder& builder) ;        
	virtual bool do_write(std::ostream& out, const Map* mesh) const;

} ;



#endif

