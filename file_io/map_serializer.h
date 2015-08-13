
#ifndef _MAP_SERIALIZER_H_
#define _MAP_SERIALIZER_H_

#include "file_io_common.h"
#include "../geom/map.h"
#include "../basic/counted.h"
#include "../basic/smart_pointer.h"
#include <iostream>


class AbstractMapBuilder;

class FILE_IO_API MapSerializer : public Counted
{
public:
	typedef Map::Vertex		Vertex ;
	typedef Map::TexVertex		TexVertex ;
	typedef Map::Halfedge		Halfedge ;
	typedef Map::Facet			Facet ;

	typedef Map::Vertex_iterator	Vertex_iterator ;
	typedef Map::Halfedge_iterator	Halfedge_iterator ;
	typedef Map::Facet_iterator		Facet_iterator ;

public:
    MapSerializer()
		: read_supported_(false)
		, write_supported_(false)
	{} 

	virtual ~MapSerializer() {}

	virtual bool serialize_read(const std::string& file_name, Map* mesh) ;
	virtual bool serialize_write(const std::string& file_name, const Map* mesh) const ;

public:
	/**
	* checks whether the stream should be opened
	* in text or binary mode. Default returns false.
	*/
	virtual bool binary() const ;

	/**
	* checks whether reading and writing to streams is supported.
	*/
	virtual bool streams_supported() const ;

	/**
	* checks whether reading is implemented.
	*/
	virtual bool read_supported() const ;

	/**
	* checks whether writing is implemented.
	*/
	virtual bool write_supported() const ;

protected:
	virtual bool do_read(std::istream& in, AbstractMapBuilder& builder) ; 
	virtual bool do_write(std::ostream& out, const Map* mesh) const;

protected:
	bool read_supported_ ;
	bool write_supported_ ;

} ;

typedef SmartPointer<MapSerializer> MapSerializer_var ;


#endif

