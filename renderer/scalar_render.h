
#ifndef _RENDERER_SCALAR_RENDER_H_
#define _RENDERER_SCALAR_RENDER_H_

#include "renderer_common.h"
#include "texture.h"


class Render;

class RENDERER_API ScalarRender
{
public:
	ScalarRender() ;
	virtual ~ScalarRender() {}

	const std::string& attribute() const { return attribute_name_; }
	void set_attribute(const std::string& x) ;

	const std::string& colormap_style() const { return colormap_style_ ; }
	void set_colormap_style(const std::string& x);

	bool use_texture() const { return use_texture_ ; }
	void set_use_texture(bool x) ;

	bool auto_range() const { return auto_range_ ; }
	void set_auto_range(bool x) ;

	double minimum() const { return minimum_ ; }
	void set_minimum(double x) ;

	double maximum() const { return maximum_ ; }
	void set_maximum(double x) ;

protected:
	void set_color(double normalized_attrib_value) ;
	void create_textures_if_needed() ;
	void read_colormap_image_if_needed() ;

protected:
	std::string		attribute_name_ ;
	bool			use_texture_ ;

	std::string		colormap_style_ ;
	Texture_var		colormap_texture_ ;
	Image_var		colormap_image_ ;
	bool			colormap_image_not_exist_;

	bool			auto_range_ ;
	double			minimum_ ;
	double			maximum_ ;
} ;

#endif
