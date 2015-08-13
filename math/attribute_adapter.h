
#ifndef _ATTRIBUTE_ADAPTER_H_
#define _ATTRIBUTE_ADAPTER_H_

#include "math_common.h"
#include "math_types.h"
#include "../basic/attribute.h"
#include "../basic/assertions.h"


class MATH_API AttributeAdapterBase {
public:
	enum PrimaryType {
		ATTR_UNDEFINED,
		ATTR_BOOLEAN,  ATTR_INTEGER,
		ATTR_FLOAT, ATTR_DOUBLE,
		ATTR_POINT2D, ATTR_VECTOR2D,
		ATTR_POINT3D, ATTR_VECTOR3D,
		ATTR_COMPLEX
	} ;
	enum SecondaryType {
		ATTR_ERROR, ATTR_VALUE,  ATTR_X,  ATTR_Y, ATTR_Z, ATTR_NORM
	} ;

public:
	AttributeAdapterBase() : primary_type_(ATTR_DOUBLE), secondary_type_(ATTR_VALUE) { 
		primary_type_ = ATTR_UNDEFINED ;
		secondary_type_ = ATTR_VALUE ;
		min_ = 0.0 ;
		max_ = 1.0 ;
		scale_ = 1.0 ;
		normalize_ = false ;
	}

	PrimaryType primary_type() const { return primary_type_ ; }

	SecondaryType secondary_type() const { return secondary_type_ ; }

	void set_range(double minv, double maxv, double scale = 1.0) {
		min_ = minv ;
		max_ = maxv ;
		scale_ = scale ;
		normalize_ = true ;
	}

	void unset_range() {
		normalize_ = false ;
	}

protected:
	double normalize(double v_in) const {
		double s = v_in ;
		if(normalize_) {
			s = (s - min_) / (max_ - min_) ;
			s = ogf_min(s, 1.0) ;
			s = ogf_max(s, 0.0) ;
			s *= scale_ ;
		}
		return s ;
	}
	SecondaryType parse_name(std::string& name) ;
	PrimaryType primary_type_ ;
	SecondaryType secondary_type_ ;
	bool normalize_ ;
	double min_ ;
	double max_ ;
	double scale_ ;
} ;

//_______________________________________________________________________________

template <class RECORD> class AttributeAdapter : public AttributeAdapterBase {
public:
	typedef RECORD Record;
	typedef GenericAttributeManager<RECORD> AttributeManager;

protected:
	void check_types() ;

public:
	AttributeAdapter() { }
	AttributeAdapter(AttributeManager* manager, const std::string& name) {
		bind_if_defined(manager, name) ;
	}
	bool bind_if_defined(AttributeManager* manager, const std::string& name_in) {
		std::string name = name_in ;
		secondary_type_ = parse_name(name) ;
		primary_type_ = bind(manager, name) ;
		check_types() ;
		if(secondary_type_ == ATTR_ERROR) {
			unbind() ;
		}
		return is_bound() ;
	}

	bool is_bound() const { 
		return ( 
			primary_type_     != ATTR_UNDEFINED && 
			secondary_type_ != ATTR_ERROR 
			) ; 
	}

	void unbind() {
		unset_range() ;
		switch(primary_type_) {
			case ATTR_UNDEFINED:
				break ;
			case ATTR_BOOLEAN:  
				bool_attr_.unbind() ;
				break ;
			case ATTR_INTEGER:
				int_attr_.unbind() ;
				break ;
			case ATTR_FLOAT: 
				float_attr_.unbind() ;
				break ;
			case ATTR_DOUBLE:
				double_attr_.unbind() ;
				break ;
			case ATTR_POINT2D: 
				point2d_attr_.unbind() ;
				break ;
			case ATTR_VECTOR2D:
				vector2d_attr_.unbind() ;
				break ;
			case ATTR_POINT3D: 
				point3d_attr_.unbind() ;
				break ;
			case ATTR_VECTOR3D:
				vector3d_attr_.unbind() ;
				break ;
		}
		primary_type_ = ATTR_UNDEFINED ;
	}

	double operator[](const RECORD* r) {
		return normalize(value(r)) ;
	}

	double non_normalized(const RECORD* r ) {
		return value(r) ;
	}

protected:

	double value(const RECORD* r) {
		switch(primary_type_) {
			case ATTR_BOOLEAN:
				return bool_attr_[r] ? 1.0 : 0.0 ;
			case ATTR_INTEGER:
				return double(int_attr_[r]) ;
			case ATTR_FLOAT:
				return double(float_attr_[r]) ;
			case ATTR_DOUBLE:
				return double_attr_[r] ;
			case ATTR_VECTOR2D:
				switch(secondary_type_) {
			case ATTR_X:
				return vector2d_attr_[r].x ;
			case ATTR_Y:
				return vector2d_attr_[r].y ;
			case ATTR_NORM:
				return length(vector2d_attr_[r]) ;
			default:
				ogf_assert_not_reached ;
				}
			case ATTR_POINT2D:
				switch(secondary_type_) {
			case ATTR_X:
				return point2d_attr_[r].x ;
			case ATTR_Y:
				return point2d_attr_[r].y ;
			default:
				ogf_assert_not_reached ;
				}
			case ATTR_VECTOR3D:
				switch(secondary_type_) {
			case ATTR_X:
				return vector3d_attr_[r].x ;
			case ATTR_Y:
				return vector3d_attr_[r].y ;
			case ATTR_Z:
				return vector3d_attr_[r].z ;   
			case ATTR_NORM:
				return length(vector3d_attr_[r]) ;
			default:
				ogf_assert_not_reached ;
				}
			case ATTR_POINT3D:
				switch(secondary_type_) {
			case ATTR_X:
				return point3d_attr_[r].x ;
			case ATTR_Y:
				return point3d_attr_[r].y ;
			case ATTR_Z:
				return point3d_attr_[r].z ;                    
			default:
				ogf_assert_not_reached ;
				}
			default:
				ogf_assert_not_reached ;
		}
		ogf_assert_not_reached ;
		return 0.0 ;
	}


	PrimaryType bind(AttributeManager* manager, const std::string& name) {
		if(bool_attr_.bind_if_defined(manager, name)) {
			return ATTR_BOOLEAN ;
		}
		if(int_attr_.bind_if_defined(manager, name)) {
			return ATTR_INTEGER ;
		}
		if(float_attr_.bind_if_defined(manager, name)) {
			return ATTR_FLOAT ;
		}
		if(double_attr_.bind_if_defined(manager, name)) {
			return ATTR_DOUBLE ;
		}
		if(vector2d_attr_.bind_if_defined(manager, name)) {
			return ATTR_VECTOR2D ;
		}
		if(vector3d_attr_.bind_if_defined(manager, name)) {
			return ATTR_VECTOR3D ;
		}
		if(point2d_attr_.bind_if_defined(manager, name)) {
			return ATTR_POINT2D ;
		}
		if(point3d_attr_.bind_if_defined(manager, name)) {
			return ATTR_POINT3D ;
		}
		return ATTR_UNDEFINED ;
	}
private:
	Attribute<RECORD, bool> bool_attr_ ;
	Attribute<RECORD, int> int_attr_ ;
	Attribute<RECORD, float> float_attr_ ;
	Attribute<RECORD, double> double_attr_ ;
	Attribute<RECORD, vec2> point2d_attr_ ;
	Attribute<RECORD, vec2> vector2d_attr_ ;
	Attribute<RECORD, vec3> point3d_attr_ ;
	Attribute<RECORD, vec3> vector3d_attr_ ;
} ;

template<class RECORD> inline void 
AttributeAdapter<RECORD>::check_types() {
	switch(primary_type_) {
			case ATTR_BOOLEAN:
			case ATTR_INTEGER:
			case ATTR_FLOAT:
			case ATTR_DOUBLE:
				if(secondary_type_ != ATTR_VALUE) {
					secondary_type_ = ATTR_ERROR ;
				}
				break ;
			case ATTR_POINT2D:
				if(secondary_type_ == ATTR_NORM) {
					secondary_type_ = ATTR_ERROR ;
				}
			case ATTR_VECTOR2D:
			case ATTR_COMPLEX:
				if(
					secondary_type_ == ATTR_VALUE ||
					secondary_type_ == ATTR_Z
					) {
						secondary_type_ = ATTR_ERROR ;
				}
				break ;
			case ATTR_POINT3D:
				if(secondary_type_ == ATTR_NORM) {
					secondary_type_ = ATTR_ERROR ;
				}
			case ATTR_VECTOR3D:
				if(secondary_type_ == ATTR_VALUE) {
					secondary_type_ = ATTR_ERROR ;
				}
				break ;
			default:
				break ;
	}
}



#endif
