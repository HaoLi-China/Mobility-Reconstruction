
#include "texture.h"
#include "../opengl/opengl_info.h"
#include "../basic/logger.h"


Memory::byte Texture::i2r_[512];
Memory::byte Texture::i2g_[512];
Memory::byte Texture::i2b_[512];
Memory::byte Texture::i2a_[512];
int Texture::nb_colors_ ;
bool Texture::texture_initialized_ = false ;
bool Texture::encode_indexed_to_rgb_ = false ;
bool Texture::has_texture3d_ = false ;

//_________________________________________________________

Texture::Texture(int dimension)
: dimension_(dimension) 
{
	texture_id_ = 0 ;
	mipmap_ = false ;
	linear_filter_ = true ;
	switch(dimension) {
		case 1:
			texture_target_ = GL_TEXTURE_1D ;
			break ;
		case 2:
			texture_target_ = GL_TEXTURE_2D ;
			break ;
		case 3:
			texture_target_ = GL_TEXTURE_3D ;
			break ;
		default:
			ogf_assert(false) ;
	}

	colormap_low_value_ = 0.0;
	colormap_high_value_ = 1.0;
	colormap_ = nil;

	initialize() ;
	glGenTextures(1, &texture_id_) ;
}

Texture::Texture(int dimension, GLenum texture_target) 
: dimension_(dimension) 
{
	texture_id_ = 0 ;
	mipmap_ = false ;
	linear_filter_ = true ;
	texture_target_ = texture_target ;

	initialize() ;

	glGenTextures(1, &texture_id_) ;
}

Texture::~Texture() {
	GLuint id = texture_id() ;
	glDeleteTextures(1, &id);    
	texture_id_ = 0 ;
}

void Texture::initialize() {
	// Query GL driver, and detect drivers having
	//   bugged pixel transfer functions.
	if(!texture_initialized_) {
		bool verbose = false ;
		if(verbose) {
			Logger::out("Texture") << "GL Vendor: " << GLInfo::gl_vendor() << std::endl ;
			Logger::out("Texture") << "GL Renderer: " << GLInfo::gl_renderer() << std::endl ;
			Logger::out("Texture") << "GL Version: " << GLInfo::gl_version() << std::endl ;
			Logger::out("Texture") << "GL Extensions: " << GLInfo::gl_extensions() << std::endl ;
		}

		has_texture3d_ = glewIsSupported("GL_EXT_texture3D") != 0 ;
		texture_initialized_ = true ;

		// Note: for the moment, always use Indexed2RGB encoder
		if(true || GLInfo::gl_vendor() == "NVIDIA Corporation") {
			if (verbose) {
				Logger::out("Texture") << "Bugged NVIDIA GeForce driver detected" << std::endl ; 
				Logger::out("Texture") << "  using my Indexed2RGB encoder" << std::endl ; 
			}
			encode_indexed_to_rgb_ = true ;                
		}

		// GL_UNPACK_ALIGNMENT, GL_PACK_ALIGNMENT
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1) ;
	}
}


void Texture::setup_pixel_transfer_for_colormap(const Colormap* colormap) 
{
	nb_colors_ = colormap->size() ;
	ogf_assert(nb_colors_ <= 512) ;
	if(!encode_indexed_to_rgb_) {
		static float i2r[512] ;
		static float i2g[512] ;
		static float i2b[512] ;
		static float i2a[512] ;
		for(int i=0; i<nb_colors_; i++) {
			i2r[i] = float(colormap->color_cell(i).r()) / 255.0f ;
			i2g[i] = float(colormap->color_cell(i).g()) / 255.0f ;
			i2b[i] = float(colormap->color_cell(i).b()) / 255.0f ;
			i2a[i] = float(colormap->color_cell(i).a()) / 255.0f ;
		}
		glPixelMapfv(GL_PIXEL_MAP_I_TO_R, nb_colors_, i2r);
		glPixelMapfv(GL_PIXEL_MAP_I_TO_G, nb_colors_, i2g);
		glPixelMapfv(GL_PIXEL_MAP_I_TO_B, nb_colors_, i2b);
		glPixelMapfv(GL_PIXEL_MAP_I_TO_A, nb_colors_, i2a);
	} else {
		for(int i=0; i<nb_colors_; i++) {
			i2r_[i] = colormap->color_cell(i).r() ;
			i2g_[i] = colormap->color_cell(i).g() ;
			i2b_[i] = colormap->color_cell(i).b() ;
			i2a_[i] = colormap->color_cell(i).a() ;
		}
	}
}

void Texture::setup_pixel_transfer_for_grayscale(int nb_colors )
{
	nb_colors_ = nb_colors ;
	ogf_assert(nb_colors_ <= 512) ;
	if(!encode_indexed_to_rgb_) {
		static float i2r[512] ;
		static float i2g[512] ;
		static float i2b[512] ;
		static float i2a[512] ;
		for(int i=0; i<nb_colors_; i++) {
			float c = float(i) / float(nb_colors - 1) ;
			i2r[i] = c ;  
			i2g[i] = c ;
			i2b[i] = c ;
			i2a[i] = c ; 
		}
		glPixelMapfv(GL_PIXEL_MAP_I_TO_R, nb_colors, i2r);
		glPixelMapfv(GL_PIXEL_MAP_I_TO_G, nb_colors, i2g);
		glPixelMapfv(GL_PIXEL_MAP_I_TO_B, nb_colors, i2b);
		glPixelMapfv(GL_PIXEL_MAP_I_TO_A, nb_colors, i2a);
	} else {
		for(int i=0; i<nb_colors_; i++) {
			Memory::byte c = Memory::byte( 
				255.0 * float(i) / float(nb_colors - 1) 
				) ;
			i2r_[i] = c ;  
			i2g_[i] = c ;
			i2b_[i] = c ;
			i2a_[i] = c ; 
		}
	}
}

void Texture::setup_float32_transfer_using_soft_colormap(const Colormap* colormap, float colormap_low_value, float colormap_high_value)
{
	colormap_low_value_ = colormap_low_value;
	colormap_high_value_ = colormap_high_value;
	colormap_ = colormap;
}

void Texture::setup_texture_parameters(bool repeat) 
{
	GLint repeat_mode = repeat ? GL_REPEAT : GL_CLAMP ;

	glEnable(texture_target_);
	glBindTexture(texture_target_, texture_id());
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
	glTexParameterf(texture_target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR) ;
	glTexParameterf(texture_target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR) ;
	glTexParameterf(texture_target_, GL_TEXTURE_WRAP_S, static_cast<GLfloat>(repeat_mode)) ;
	glTexParameterf(texture_target_, GL_TEXTURE_WRAP_T, static_cast<GLfloat>(repeat_mode)) ;
	glTexParameterf(texture_target_, GL_TEXTURE_WRAP_R, static_cast<GLfloat>(repeat_mode)) ;
}

void Texture::setup_texture_data(
								 int dim,
								 Image::ColorEncoding encoding_in, 
								 Memory::pointer base_mem_in,
								 Texture::FilteringMode mode,
								 int width, int height, int depth) 
{
	const int static_buffer_size = 1024*1024 ;
	static Memory::byte static_buffer[static_buffer_size * 4] ;
	Memory::pointer dynamic_buffer = nil ;
	Memory::pointer base_mem = base_mem_in ;
	Image::ColorEncoding encoding = encoding_in ;

	mipmap_ = (mode == Texture::MIPMAP) ;
	linear_filter_ = (mode != Texture::NO_FILTERING) ;

	if(height == 0) {
		height++ ;
	}
	if(depth == 0) {
		depth++ ;
	}

	int size = width * height * depth ;

	if(encode_indexed_to_rgb_ && (encoding == Image::GRAY || encoding == Image::INDEXED)) {
		if(dim == 3 && encoding == Image::GRAY) {
		} 
		else
		{
			encoding = Image::RGBA ;
			if(size <= static_buffer_size) {
				base_mem = static_buffer ;
			} else {
				dynamic_buffer = new Memory::byte[size*4] ;
				base_mem = dynamic_buffer ;
			}
			indexed_to_rgb(size, base_mem_in, Image::RGBA, base_mem) ;
		}
	}

	switch(dim) 
	{
	case 1:
		switch(encoding) 
		{
		case Image::GRAY :
		case Image::INDEXED :
			gluBuild1DMipmaps(
				GL_TEXTURE_1D, GL_RGBA, width,
				GL_COLOR_INDEX, GL_UNSIGNED_BYTE, base_mem
				) ;
			break ;
		case Image::RGB :
			gluBuild1DMipmaps(
				GL_TEXTURE_1D, GL_RGB, width, 
				GL_RGB, GL_UNSIGNED_BYTE, base_mem
				) ;
			break ;
		case Image::RGBA :
			gluBuild1DMipmaps(
				GL_TEXTURE_1D, GL_RGBA, width,
				GL_RGBA, GL_UNSIGNED_BYTE, base_mem
				) ;
			break ;
		case Image::FLOAT32 :
			{				
				Memory::pointer colormapped_texture_values = new Memory::byte[4 * width];
				float32_to_rgb(size, base_mem_in, Image::RGBA, colormapped_texture_values);

				gluBuild1DMipmaps(
					GL_TEXTURE_1D, GL_RGBA, width,
					GL_RGBA, GL_UNSIGNED_BYTE, colormapped_texture_values
					) ; 

				delete[] colormapped_texture_values;
				break ;
			}
		default: {
			bool implemented = false ;
			ogf_assert(implemented) ;
				 }
		}
		break ;
	case 2:
		switch(encoding) 
		{
		case Image::GRAY :
		case Image::INDEXED :
			if(mipmap_) {
				gluBuild2DMipmaps(
					GL_TEXTURE_2D, GL_RGBA, width, height,
					GL_COLOR_INDEX, GL_UNSIGNED_BYTE, base_mem
					) ;
			} else {
				glTexImage2D(
					GL_TEXTURE_2D, 0, GL_RGBA, width, height,
					0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, base_mem
					) ;
			}
			break ;
		case Image::RGB :
			if(mipmap_) {
				gluBuild2DMipmaps(
					GL_TEXTURE_2D, GL_RGB, width, height, 
					GL_RGB, GL_UNSIGNED_BYTE, base_mem
					) ;
			} else {
				glTexImage2D(
					GL_TEXTURE_2D, 0, GL_RGB, width, height,
					0, GL_RGB, GL_UNSIGNED_BYTE, base_mem
					) ;
			}
			break ;
		case Image::RGBA :
			if(mipmap_) {
				gluBuild2DMipmaps(
					GL_TEXTURE_2D, GL_RGBA, width, height,
					GL_RGBA, GL_UNSIGNED_BYTE, base_mem
					) ;
			} else {
				glTexImage2D(
					GL_TEXTURE_2D, 0, GL_RGBA, width, height,
					0, GL_RGBA, GL_UNSIGNED_BYTE, base_mem
					) ;
			}
			break ;
		case Image::RGB_FLOAT32: 
			glTexImage2D(
				GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGB32_NV, width, height,
				0, GL_RGB, GL_FLOAT, base_mem
				) ;
			break ;
		case Image::RGBA_FLOAT32: 
			glTexImage2D(
				GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA32_NV, width, height,
				0, GL_RGBA, GL_FLOAT, base_mem
				) ;
			break ;
		case Image::FLOAT32 :
			{		
				Memory::pointer colormapped_texture_values = new Memory::byte[4 * width * height];
				float32_to_rgb(size, base_mem_in, Image::RGBA, colormapped_texture_values);

				if(mipmap_) {
					gluBuild2DMipmaps(
						GL_TEXTURE_2D, GL_RGBA, width, height,
						GL_RGBA, GL_UNSIGNED_BYTE, colormapped_texture_values
						) ; 
				} else {
					glTexImage2D(
						GL_TEXTURE_2D, 0, GL_RGBA, width, height,
						0, GL_RGBA, GL_UNSIGNED_BYTE, colormapped_texture_values 
						) ;
				}

				delete[] colormapped_texture_values;
				break ;
			}
		default: 
			{
				bool implemented = false ;
				ogf_assert(implemented) ;
			} break ;
		}
		break ;
	case 3:
		// Note: gluBuild3DMipmaps does not seem to exist under Windows	   
#ifdef WIN32
		mipmap_ = false ;
#endif
		switch(encoding) {
	case Image::GRAY : 
		if(mipmap_) {
#ifndef WIN32		   
			gluBuild3DMipmaps(
				GL_TEXTURE_3D, GL_LUMINANCE, width, height, depth,
				GL_LUMINANCE, GL_UNSIGNED_BYTE, base_mem
				) ;
#endif		   
		} else {
			glTexImage3DEXT(
				GL_TEXTURE_3D, 0, GL_LUMINANCE, width, height, depth,
				0, GL_LUMINANCE, GL_UNSIGNED_BYTE, base_mem
				) ;
		}
		break ;
	case Image::INDEXED :
		if(mipmap_) {
#ifndef WIN32
			gluBuild3DMipmaps(
				GL_TEXTURE_3D, GL_RGBA, width, height, depth,
				GL_RGB, GL_UNSIGNED_BYTE, base_mem
				) ;
#endif
		} else {
			glTexImage3DEXT(
				GL_TEXTURE_3D, 0, GL_RGB, width, height, depth,
				0, GL_RGB, GL_UNSIGNED_BYTE, base_mem
				) ;
		}
		break ;
	case Image::RGB :
		if(mipmap_) {
#ifndef WIN32
			gluBuild3DMipmaps(
				GL_TEXTURE_3D, GL_RGB, width, height, depth,
				GL_RGB, GL_UNSIGNED_BYTE, base_mem
				) ; 
#endif
		} else {
			glTexImage3DEXT(
				GL_TEXTURE_3D, 0, GL_RGB, width, height, depth,
				0, GL_RGB, GL_UNSIGNED_BYTE, base_mem
				) ;
		}
		break ;
	case Image::RGBA :
		if(mipmap_) {
#ifndef WIN32
			gluBuild3DMipmaps(
				GL_TEXTURE_3D, GL_RGBA, width, height, depth,
				GL_RGBA, GL_UNSIGNED_BYTE, base_mem
				) ; 
#endif
		} else {
			glTexImage3DEXT(
				GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth,
				0, GL_RGBA, GL_UNSIGNED_BYTE, base_mem
				) ;
		}
		break ;
	case Image::FLOAT32 :
		{				
			Memory::pointer colormapped_texture_values = new Memory::byte[4 * width * height * depth];
			float32_to_rgb(size, base_mem_in, Image::RGBA, colormapped_texture_values);

			if(mipmap_) {
#ifndef WIN32
				gluBuild3DMipmaps(
					GL_TEXTURE_3D, GL_RGBA, width, height, depth,
					GL_RGBA, GL_UNSIGNED_BYTE, colormapped_texture_values
					) ; 
#endif
			} else {
				glTexImage3DEXT(
					GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth,
					0, GL_RGBA, GL_UNSIGNED_BYTE, colormapped_texture_values 
					) ;
			}

			delete[] colormapped_texture_values;
			break ;
		}
	default: {
		bool implemented = false ;
		ogf_assert(implemented) ;
			 }
		}
		break ;
	}
	delete[] dynamic_buffer ;
}

void Texture::indexed_to_rgb(
							 int size_in,
							 Memory::pointer data_in,
							 Image::ColorEncoding encoding_out,
							 Memory::pointer data_out) 
{
	switch(encoding_out) {
		case Image::RGB :
			{
				for(int i=0; i < size_in; i++) {
					Memory::byte index = data_in[i] ;
					data_out[3*i  ] = i2r_[index] ; 
					data_out[3*i+1] = i2g_[index] ; 
					data_out[3*i+2] = i2b_[index] ; 
				}
			}
			break ;
		case Image::RGBA :
			{
				for(int i=0; i < size_in; i++) {
					Memory::byte index = data_in[i] ;
					data_out[4*i  ] = i2r_[index] ; 
					data_out[4*i+1] = i2g_[index] ; 
					data_out[4*i+2] = i2b_[index] ; 
					data_out[4*i+3] = i2a_[index] ; 
				}
			}
			break ;
		default:
			{
				Logger::err("Texture") 
					<< "indexed_to_rgb(): invalid texture format" 
					<< std::endl ;
				bool ok = false ;
				ogf_assert(ok) ;
			}
			break ;
	}
}

void Texture::float32_to_rgb(
							 int size_in,
							 Memory::pointer data_in,
							 Image::ColorEncoding encoding_out,
							 Memory::pointer data_out
							 ) 
{
	// colormap not given -- fill with black

	if(!colormap_)
	{
		Logger::err("float32_to_rgb") << "No colormap assigned" << std::endl;

		switch(encoding_out) 
		{
		case Image::RGB :
			for(int i = 0 ; i < 3 * size_in ; i++)
				data_out[i] = 0;
			break;

		case Image::RGBA :
			for(int i = 0 ; i < 4 * size_in ; i++)
				data_out[i] = 0;

			break ;
		default:
			Logger::err("GLTexture") 
				<< "float32_to_rgb(): invalid texture format" 
				<< std::endl ;
			bool ok = false ;
			ogf_assert(ok) ;

			break ;
		}

		return;
	}


	// colormap given

	float colormap_remap_factor = (colormap_->size() - 1) / (colormap_high_value_ - colormap_low_value_);

	switch(encoding_out) 
	{
	case Image::RGB :
		for(int i=0; i < size_in; i++) 
		{
			float value = ((float*)data_in)[i];
			float remapped_value = (value - colormap_low_value_) * colormap_remap_factor;
			int remapped_value_int = (int)remapped_value;

			ogf_clamp(remapped_value_int, 0, colormap_->size() - 1);

			unsigned char r = colormap_->color_cell(remapped_value_int).r();
			unsigned char g = colormap_->color_cell(remapped_value_int).g();
			unsigned char b = colormap_->color_cell(remapped_value_int).b();

			data_out[3*i  ] = r;
			data_out[3*i+1] = g;
			data_out[3*i+2] = b;
		}
		break;

	case Image::RGBA :
		for(int i=0; i < size_in; i++) 
		{
			float value = ((float*)data_in)[i];
			float remapped_value = (value - colormap_low_value_) * colormap_remap_factor;
			int remapped_value_int = (int)remapped_value;

			ogf_clamp(remapped_value_int, 0, colormap_->size() - 1);

			unsigned char r = colormap_->color_cell(remapped_value_int).r();
			unsigned char g = colormap_->color_cell(remapped_value_int).g();
			unsigned char b = colormap_->color_cell(remapped_value_int).b();
			unsigned char a = colormap_->color_cell(remapped_value_int).a();

			data_out[4*i  ] = r;
			data_out[4*i+1] = g;
			data_out[4*i+2] = b;
			data_out[4*i+3] = a;
		}

		break ;
	default:
		Logger::err("GLTexture") 
			<< "float32_to_rgb(): invalid texture format" 
			<< std::endl ;
		bool ok = false ;
		ogf_assert(ok) ;

		break ;
	}
}


void Texture::bind() {
	glDisable(GL_TEXTURE_1D) ;
	glDisable(GL_TEXTURE_2D) ;
	glDisable(GL_TEXTURE_3D) ;

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(texture_target_);
	glBindTexture(texture_target_, texture_id_) ;
	if(mipmap_) {
		glTexParameteri(texture_target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR) ;
		glTexParameteri(texture_target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR) ;
	} else if(linear_filter_) {
		glTexParameteri(texture_target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR) ;
		glTexParameteri(texture_target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR) ;
	} else {
		glTexParameteri(texture_target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST) ;
		glTexParameteri(texture_target_, GL_TEXTURE_MAG_FILTER, GL_NEAREST) ;
	}

	glColor3f(1.0, 1.0, 1.0) ;
}

void Texture::unbind() {
	glDisable(GL_TEXTURE_1D) ;
	glDisable(GL_TEXTURE_2D) ;
	glDisable(GL_TEXTURE_3D) ;

	glColor3f(1.0, 1.0, 1.0) ;
}