
#ifndef _RENDERER_TEXTURE_FACTORY_H_
#define _RENDERER_TEXTURE_FACTORY_H_

#include "renderer_common.h"
#include "texture.h"

#include <string>

class Canvas;
class Image;

class RENDERER_API TextureFactory
{
public:

	static Texture* create_texture_from_file(
		const std::string& file_name,
		Texture::FilteringMode filt_mode = Texture::MIPMAP, 
		Texture::TexCoordMode tex_coord_mode = Texture::REPEAT
		);

	static Texture* create_texture_from_image(
		const Image* image,
		Texture::FilteringMode filter_mode  = Texture::MIPMAP,
		Texture::TexCoordMode  coord_mode = Texture::REPEAT,
		float float_remap_low_point = 0.0,
		float float_remap_high_point = 1.0
		);
} ;

#endif
