
#ifndef _MAP_SERIALIZER_STL_H_
#define _MAP_SERIALIZER_STL_H_

#include "file_io_common.h"
#include "map_serializer.h"

#include <cfloat>  // for FLT_MIN


class FILE_IO_API MapSerializer_stl : public MapSerializer
{
public:
	MapSerializer_stl() ;

	virtual bool streams_supported() const ;

	virtual bool serialize_read(
		const std::string& file_name, Map* mesh) ; 

	virtual bool serialize_write(
		const std::string& file_name, const Map* mesh) const;

	/** Set the threshold to be used for considering two point to be equal.
	Can be used to merge small gaps */
	void set_epsilon(double eps) { eps_=eps; }

	// Returns the threshold to be used for considering two point to be equal.
	double epsilon() const { return eps_; }

private:
	enum STL_Type { STL_ASCII, STL_BINARY, NONE };
	STL_Type resolve_stl_type(const std::string& file_name) const;

	bool  read_stla(const std::string& file_name, AbstractMapBuilder& builder) ;
	bool  read_stlb(const std::string& file_name, AbstractMapBuilder& builder) ;

	bool  write_stla(const std::string& file_name, const Map* mesh) const;
	bool  write_stlb(const std::string& file_name, const Map* mesh) const;

private:
	class PointCmp
	{
	public:
		PointCmp(double eps=FLT_MIN) : eps_(eps) {}
		bool operator()(const vec3& v0, const vec3& v1) const ;
	private:
		double eps_;
	};

private:
	double		eps_;

	std::string header_ ;
} ;


#endif

