
#ifndef _MAP_SERIALIZER_OBJ_H_
#define _MAP_SERIALIZER_OBJ_H_

#include "file_io_common.h"
#include "map_serializer.h"
#include "../image/color.h"


class FILE_IO_API MapSerializer_obj : public MapSerializer 
{
public:
	MapSerializer_obj();

	virtual bool serialize_read(
		const std::string& file_name, Map* mesh
		) ;

protected:
	virtual bool do_read(std::istream& input, AbstractMapBuilder& builder) ;        
	virtual bool do_write(std::ostream& output, const Map* mesh) const; 
	void read_mtl_lib(std::istream& input) ;

protected:
	std::string	current_directory_ ;

	typedef Color Material ;
	typedef std::map<std::string, Material> MaterialLib ;

	MaterialLib	material_lib_ ;
	Material	current_material_ ;
} ;


//_________________________________________________________

/**
* Extended obj file format, adds attributes.
*/
class FILE_IO_API MapSerializer_eobj : public MapSerializer_obj {
public:
	MapSerializer_eobj() ;

	virtual bool do_read(std::istream& input, AbstractMapBuilder& builder) ;        
	virtual bool do_write(std::ostream& output, const Map* mesh) const; 

} ;


#endif

