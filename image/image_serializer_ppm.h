
#ifndef ___IMAGE_SERIALIZER_PPM__
#define ___IMAGE_SERIALIZER_PPM__

#include "image_common.h"
#include "image_serializer.h"


class IMAGE_API ImageSerializer_ppm : public ImageSerializer {
public:
	virtual Image*	serialize_read(std::istream& stream) ;
	virtual bool	serialize_write(std::ostream& stream, const Image* image) ;

	virtual bool	read_supported() const ;
	virtual bool	write_supported() const ;
} ;


#endif

