
#ifndef __IMAGE_IO_IMAGE_SERIALIZER_XPM__
#define __IMAGE_IO_IMAGE_SERIALIZER_XPM__

#include "image_common.h"
#include "image_serializer.h"




    class IMAGE_API ImageSerializer_xpm : public ImageSerializer {
    public:
        virtual Image* serialize_read(std::istream& stream) ;
        virtual bool read_supported() const ;
        virtual bool binary() const ;

    protected:
        Image* read_xpm_1_byte_per_pixel(
            int width, int height, int num_colors,
            std::istream& input
        ) ;

        Image* read_xpm_2_bytes_per_pixel(
            int width, int height, int num_colors,
            std::istream& input
        ) ;

        static char* next_xpm_data(std::istream& input) ;
    } ;



#endif

