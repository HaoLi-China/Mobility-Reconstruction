
#ifndef _IMAGE_COLORMAP_H_
#define _IMAGE_COLORMAP_H_

#include "image_common.h"
#include "color.h"
#include "../basic/counted.h"


class IMAGE_API Colormap : public Counted {
public:
	typedef		Color_uint8		ColorCell ;

	Colormap(int size_in = 256) ;
	virtual ~Colormap() ;

	const ColorCell& color_cell(int index) const ;
	ColorCell& color_cell(int index) ;

	int size() const ;

	void set_color(int index, float r, float g, float b) ;

	void set_color(int index, float r, float g, float b, float a) ;

	void color_ramp_component(
		int component,
		int index1, Numeric::uint8 val1,
		int index2, Numeric::uint8 val2
		) ;

	void color_ramp_rgba(
		int index1, const Color& c1,
		int index2, const Color& c2
		) ;

	void color_ramp_rgb(
		int index1, const Color& c1,
		int index2, const Color& c2
		) ;

private:
	ColorCell* cells_ ;
	int size_ ;
} ;

typedef SmartPointer<Colormap> Colormap_var ;


//_________________________________________________________

inline const Colormap::ColorCell& Colormap::color_cell(
	int index
	) const {
		ogf_assert(index >= 0 && index < size_) ;
		return cells_[index] ;
}

inline Colormap::ColorCell& Colormap::color_cell(int index) {
	ogf_assert(index >= 0 && index < size_) ;
	return cells_[index] ;
}

inline int Colormap::size() const {
	return size_ ;
}

//_________________________________________________________


#endif

