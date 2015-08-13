
#include "texture_factory.h"
#include "../opengl/opengl_info.h"
#include "../basic/logger.h"
#include "../image/image_io.h"


Texture* TextureFactory::create_texture_from_image(
	const Image* image, 
	Texture::FilteringMode mode, 
	Texture::TexCoordMode tex_coord_mode,
	float float_remap_low_point,
	float float_remap_high_point
	)
{
	if(image->dimension() == 3 && !Texture::has_texture3d()) {
		Logger::warn("Render") 
			<< "texture 3D not supported by this GPU"
			<< std::endl ;
		return nil ;
	}

	switch(image->color_encoding())
	{
	case Image::INT16 :
	case Image::INT32 :
	case Image::FLOAT64 :
		{
			bool implemented = false ;
			ogf_assert(implemented) ;
		}
	default:
		break ;
	}

	GLenum texture_target = 0 ;
	switch(image->dimension()) {
	case 1:
		texture_target = GL_TEXTURE_1D ;
		break ;
	case 2:
		texture_target = GL_TEXTURE_2D ;
		break ;
	case 3:
		texture_target = GL_TEXTURE_3D ;
		break ;
	default:
		ogf_assert_not_reached ;
		break ;
	}

	if( image->color_encoding() == Image::RGB_FLOAT32 ||
		image->color_encoding() == Image::RGBA_FLOAT32 )
	{
		// TODO: check for extension floating point buffer
		ogf_assert(image->dimension() == 2) ;
		texture_target = GL_TEXTURE_RECTANGLE_NV ;			
	}

	Texture* result = new Texture(image->dimension(), texture_target) ;
	bool repeat = (tex_coord_mode == Texture::REPEAT) ;
	result->setup_texture_parameters(repeat) ;

	switch(image->color_encoding()) {
		case Image::GRAY :
			result->setup_pixel_transfer_for_grayscale(256) ;
			break ;
		case Image::INDEXED :
			result->setup_pixel_transfer_for_colormap(image->colormap()) ;
			break ;
		case Image::FLOAT32:
			result->setup_float32_transfer_using_soft_colormap(
				image->colormap(), 
				float_remap_low_point, 
				float_remap_high_point);
			break;
		default:
			break ;
	}

	result->setup_texture_data(
		image->dimension(), image->color_encoding(), 
		image->base_mem(), mode,
		image->width(), image->height(), image->depth()
		) ;

	result->unbind();
	return result ;
}


Texture* TextureFactory::create_texture_from_file(
	const std::string& file_name,
	Texture::FilteringMode filt_mode, 
	Texture::TexCoordMode tex_coord_mode
	) 
{
	if(file_name.length() == 0) {
		return nil ;
	}
	Image_var image = ImageIO::read(file_name) ;
	if(image.is_nil())
		return nil ;

	return create_texture_from_image(image, filt_mode, tex_coord_mode) ;
}
