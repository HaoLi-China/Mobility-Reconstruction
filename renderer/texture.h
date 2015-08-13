
#ifndef _RENDERER_TEXTURE_H_
#define _RENDERER_TEXTURE_H_

#include "renderer_common.h"
#include "../image/image.h"
#include "../basic/counted.h"
#include "../basic/smart_pointer.h"

#include "../opengl/glew.h"



class RENDERER_API Texture : public Counted
{
public:
	enum FilteringMode { NO_FILTERING, LINEAR_FILTERING, MIPMAP } ;
	enum TexCoordMode  { CLAMP, REPEAT } ;

public:
	Texture(int dimension) ;
	Texture(int dimension, GLenum texture_target) ;

	virtual ~Texture() ;

	inline int dimension() const {
		return dimension_ ;
	}

	GLuint texture_id() const     { return texture_id_ ;     }
	GLenum texture_target() const { return texture_target_ ; }

	void bind();
	void unbind();

	void setup_texture_parameters(bool repeat = true) ;
	void setup_pixel_transfer_for_colormap(const Colormap* colormap) ;
	void setup_pixel_transfer_for_grayscale(int nb_colors) ;
	void setup_float32_transfer_using_soft_colormap(const Colormap* colormap, float lo_cmap_val, float hi_cmap_val);
	void setup_texture_data(
		int dim, 
		Image::ColorEncoding encoding, Memory::pointer,
		Texture::FilteringMode mode,
		int width, int height = 0, int depth = 0
		) ;

	bool mipmap() const { return mipmap_ ; }
	bool linear_filter() const { return linear_filter_ ; }

	static void initialize() ;
	static bool has_texture3d()  { 
		if(!texture_initialized_) {
			initialize() ;
		}
		return has_texture3d_ ; 
	}

protected:
	// Some GL drivers (such as the version 1.2.2. of the
	//  GeForce driver) have bugged pixel transfer functions.
	//  If such a driver is detected, this function is
	//  used instead.
	void indexed_to_rgb(
		int size_in,
		Memory::pointer data_in,
		Image::ColorEncoding encoding_out,
		Memory::pointer data_out
		) ;

	// Used to build displayable version of float texture
	void float32_to_rgb(
		int size_in,
		Memory::pointer data_in,
		Image::ColorEncoding encoding_out,
		Memory::pointer data_out);

private:
	int		dimension_ ;

	GLuint  texture_id_ ;
	GLenum  texture_target_ ;

	bool	mipmap_ ;
	bool	linear_filter_ ;

	// Use the indexed_to_rgb() function instead
	//   of OpenGL pixel transfer if a bugged driver
	//   is detected.
	static bool encode_indexed_to_rgb_ ;

	static bool texture_initialized_ ;
	static bool has_texture3d_ ;

	// Used by the indexed to rgb encoder.
	static int nb_colors_ ;
	static Memory::byte i2r_[512];
	static Memory::byte i2g_[512];
	static Memory::byte i2b_[512];
	static Memory::byte i2a_[512];

	// Used by float32_to_rgb() to define mapping from float texture subdomain to colormap
	float colormap_low_value_;
	float colormap_high_value_;
	const Colormap* colormap_;

} ;


typedef SmartPointer<Texture> Texture_var ;


#endif

