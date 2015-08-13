#include "scalar_render.h"
#include "texture_factory.h"
#include "../image/image_io.h"
#include "../basic/logger.h"
#include "../basic/file_utils.h"



ScalarRender::ScalarRender()
: colormap_texture_(nil)
, colormap_image_(nil)
, use_texture_(true)
, colormap_image_not_exist_(false)
, colormap_style_("default")
, attribute_name_("chart")
, minimum_( -1.0 )
, maximum_( 1.0 )
, auto_range_(true)
{   
}


void ScalarRender::set_attribute(const std::string& x) { 
	attribute_name_ = x ;
}

void ScalarRender::set_colormap_style(const std::string& x) { 
	colormap_texture_.forget() ;
	colormap_image_.forget();
	colormap_style_ = x ; 
	colormap_image_not_exist_ = false;
}

void ScalarRender::set_use_texture(bool x) { 
	use_texture_ = x; 
}

void ScalarRender::set_auto_range(bool x) { 
	auto_range_ = x ; 
}

void ScalarRender::set_minimum(double x) { 
	minimum_ = x ;
}

void ScalarRender::set_maximum(double x) { 
	maximum_ = x ; 
}


static Image* random_color_image(int num_colors) {
	Image* image = new Image(Image::RGB, num_colors, 2) ;
	for(int x=0; x<num_colors; x++) {
		Memory::byte r = Memory::byte(Numeric::random_float32() * 255.0) ;
		Memory::byte g = Memory::byte(Numeric::random_float32() * 255.0) ;
		Memory::byte b = Memory::byte(Numeric::random_float32() * 255.0) ;
		image->pixel_base(x,0)[0]  = r ;
		image->pixel_base(x,0)[1]  = g ;
		image->pixel_base(x,0)[2]  = b ;
		image->pixel_base(x,1)[0]  = r ;
		image->pixel_base(x,1)[1]  = g ;
		image->pixel_base(x,1)[2]  = b ;
	}
	return image;
}


void ScalarRender::read_colormap_image_if_needed() {
	if (colormap_image_ || colormap_image_not_exist_) 
		return;

	if (colormap_style_ == "random") {
		int num_colors = 20;
		colormap_image_ = random_color_image(num_colors);
		Logger::out("ScalarRender") << "random color image generated. "
			<< num_colors << " colors" << std::endl;
	} else {
		std::string dir = FileUtils::MeshStudio_resource_directory() + "/icons/";
		// 
		std::string filename = dir + colormap_style_ + "_hr.xpm" ;
		if (!FileUtils::is_file(filename))
			filename = dir + colormap_style_ + ".xpm" ;
		colormap_image_ = ImageIO::read(filename);

		if (colormap_image_) // tell the user
			Logger::out("ScalarRender") << "using colormap: \'" << FileUtils::base_name(filename) << " \'" << std::endl;
	}

	if (colormap_image_.is_nil()) {
		Logger::err("ScalarRender") << "read colormap image failed (file exist?)" << std::endl;
		colormap_image_not_exist_ = true;
		return;
	} 
}


void ScalarRender::create_textures_if_needed() {
	if(colormap_texture_.is_nil()) {
		read_colormap_image_if_needed();

		if (colormap_image_) {
			Texture::FilteringMode	filter = Texture::MIPMAP;
			Texture::TexCoordMode	mode = Texture::CLAMP;
			colormap_texture_ = TextureFactory::create_texture_from_image(colormap_image_, filter, mode);
			if (colormap_texture_.is_nil())
				Logger::err("ScalarSurfaceRender") << "failed creating colormap texture" << std::endl; 
		}
	}
}


void ScalarRender::set_color(double normalized_attrib_value) {
	if(use_texture_) {
		if (colormap_texture_)
			glTexCoord2d(normalized_attrib_value, 0.5) ;
		else {
			glColor4f(0.3f, 0.3f, 0.4f, 1.0f);
		}
	}
	else {
		read_colormap_image_if_needed();
		if (colormap_image_.is_nil())
			glColor4f(0.3f, 0.3f, 0.4f, 1.0f); 
		else {				
			if(colormap_image_->color_encoding() == Image::RGB) {
				int index = int(normalized_attrib_value * (colormap_image_->width() - 1)) ;
				ogf_clamp(index, 0, colormap_image_->width() - 1) ;
				Memory::byte* p = colormap_image_->pixel_base(index, 2) ;
				glColor3ubv(p) ;
			} 
			else {
				int index = int(normalized_attrib_value * (colormap_image_->colormap()->size() - 1)) ;
				ogf_clamp(index, 0, colormap_image_->colormap()->size() - 1) ;
				Color_uint8 c = colormap_image_->colormap()->color_cell(index);
				glColor4ubv(c.data()) ;
			}
		}
	}
}
