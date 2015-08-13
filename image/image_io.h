
#ifndef __IMAGE_IO__
#define __IMAGE_IO__

#include "image_common.h"
#include "image.h"

#include <string>


class Image ;
class ImageSerializer ;

class IMAGE_API ImageIO
{
public:
	static Image*	read(const std::string& file_name);
	static bool		save(const std::string& file_name, const Image* image) ;

	static ImageSerializer* resolve_serializer(const std::string& file_name) ;

};


#endif

