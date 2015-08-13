
#include "image_serializer_ppm.h"
#include "image.h"
#include "../basic/logger.h"

#include <sstream>


Image* ImageSerializer_ppm::serialize_read(std::istream& in) {
	// Read header
	char buff[256] ;

	std::string magic ;
	in >> magic ;
	if(magic != "P6") {
		Logger::err("PPM loader: cannot load this type of PPM") ;
		return nil ;
	}
	// read end of line
	in.getline(buff, 256) ;

	// read comments (# CREATOR etc...)
	do {
		in.getline(buff, 256) ;
	} while(buff[0] == '#') ;

	int width, height, max_comp_value ;
	std::istringstream line_in(buff) ;
	line_in >> width >> height ;
	in >> max_comp_value ;
	if(width < 1 || height < 1) {
		Logger::err("PPM loader") << "invalid image size: " 
			<< width << " x " 
			<< height << std::endl ;
		return nil ;
	}
	if(max_comp_value != 255) {
		Logger::err(
			"PPM loader: invalid max component value (should be 255)"
			) ;
		return nil ;
	}
	in.getline(buff, 256) ;

	int image_size = width * height ;
	Image* result = new Image(Image::RGB, width, height) ;
	for(int i=0; i<image_size; i++) {
		char r,g,b ;
		in.get(r) ;
		in.get(g) ;
		in.get(b) ;


		result->base_mem()[3*i]   = r ;
		result->base_mem()[3*i+1] = g ;
		result->base_mem()[3*i+2] = b ;
	}
	flip_image(*result) ;
	return result ;
}

bool ImageSerializer_ppm::read_supported() const {
	return true ;
}

bool ImageSerializer_ppm::serialize_write(
	std::ostream& out, const Image* image
	) {

		if(
			image->color_encoding() != Image::RGB &&
			image->color_encoding() != Image::RGBA
			) {
				Logger::err("Image") 
					<< "PPM writer implemented for RGB or RGBA images only"
					<< std::endl ;
				return false ;
		}

		out << "P6 " << std::endl ;
		out << "# CREATOR: Meshop" << std::endl ;
		out << image->width() << " " << image->height() << std::endl ;
		out << 255 << std::endl ;

		int image_height = image->height() ;
		int image_width = image->width() ;

		switch(image->color_encoding()) {
		case Image::RGB: 
			for(int i=image_height-1; i>=0; i--) {
				for(int j = 0; j < image_width; j++){
					out.put(image->base_mem()[3*(i*image_width + j)]) ;
					out.put(image->base_mem()[3*(i*image_width + j)+1]) ;
					out.put(image->base_mem()[3*(i*image_width + j)+2]) ;
				}
			}			 
			break ;
		case Image::RGBA: 
			for(int i=image_height-1; i>=0; i--) {
				for(int j = 0; j < image_width; j++){
					out.put(image->base_mem()[4*(i*image_width + j)]) ;
					out.put(image->base_mem()[4*(i*image_width + j)+1]) ;
					out.put(image->base_mem()[4*(i*image_width + j)+2]) ;
				}
			}
			break ;
		default: 
			ogf_assert(false) ;
			break ;
		}
		return true ;
}


bool ImageSerializer_ppm::write_supported() const {
	return true ;
}

