
#include "image_io.h"
#include "image.h"
#include "image_serializer.h"
#include "image_serializer_bmp.h"
#include "image_serializer_ppm.h"
#include "image_serializer_xpm.h"
#include "../basic/logger.h"
#include "../basic/file_utils.h"



Image* ImageIO::read(const std::string& file_name) {
	ImageSerializer_var serializer = resolve_serializer(file_name);
	if (!serializer.is_nil())
		return serializer->serialize_read(file_name);
	else
		return nil;
}


bool ImageIO::save(const std::string& file_name, const Image* image) {
	ImageSerializer_var serializer = resolve_serializer(file_name);
	if (!serializer.is_nil())
		return serializer->serialize_write(file_name, image);
	else
		return false;
}

ImageSerializer* ImageIO::resolve_serializer(const std::string& file_name) {
	std::string extension = FileUtils::extension(file_name) ;
	String::to_lowercase(extension);
	if(extension.length() == 0) {
		Logger::err("ImageIO") << "No extension in file name" << std::endl ;
		return nil ;
	}

	ImageSerializer* serializer = nil;

	if ( extension == "bmp" )
		serializer = new ImageSerializer_bmp();
	else if ( extension == "ppm" )
		serializer = new ImageSerializer_ppm();
	else if ( extension == "xpm" ) 
		serializer = new ImageSerializer_xpm();
	else { 	
		Logger::err("ImageIO") << "Unknown image file format \'" << extension << "\'" << std::endl;
		return nil;
	}

	return serializer;
}