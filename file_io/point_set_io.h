#ifndef _POINT_SET_IO_H_
#define _POINT_SET_IO_H_

#include "file_io_common.h"

#include <iostream>
#include <vector>


class PointSet;

class FILE_IO_API PointSetIO
{
public:
	static std::string title() { return "PointSetIO"; }

	// for both point cloud and mesh
	static PointSet* read(const std::string& file_name);

	// save the point set to a file. return false if failed.
	static bool	save(const std::string& file_name, const PointSet* point_set);

protected:
	static void load_xyz(PointSet* pointSet, const std::string& file_name) ;
	static void save_xyz(const PointSet* pointSet, const std::string& file_name) ;
	static void load_bxyz(PointSet* pointSet, const std::string& file_name) ;
	static void save_bxyz(const PointSet* pointSet, const std::string& file_name) ;

	static void load_pn(PointSet* pointSet, const std::string& file_name) ;
	static void save_pn(const PointSet* pointSet, const std::string& file_name) ;
	static void load_bpn(PointSet* pointSet,  const std::string& file_name) ;
	static void save_bpn(const PointSet* pointSet,  const std::string& file_name) ;
	
	// each line with point, normal and color: (x, y, z, nx, ny, nz, r, g, b)
	static void load_pnc(PointSet* pointSet, const std::string& file_name);
	static void save_pnc(const PointSet* pointSet, const std::string& file_name);
	static void load_bpnc(PointSet* pointSet, const std::string& file_name);
	static void save_bpnc(const PointSet* pointSet, const std::string& file_name);

	//get columns of the file 
	static int get_file_cols(const std::string& file_name);
};

#endif