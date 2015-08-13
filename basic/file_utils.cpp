
#include "file_utils.h"
#include "line_stream.h"

#include <iostream>
#include <fstream>
#include <cassert>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <direct.h> // for _mkdir
#include <sys/stat.h> // for _stat64
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#endif

// some code are copied from:
// "OpenSceneGraph - <osgDB/FileNameUtils>" 
// "OpenSceneGraph - <osgDB/FileUtils>"




namespace FileUtils {

	static const char * const PATH_SEPARATORS = "/\\";
	static unsigned int PATH_SEPARATORS_LEN = 2;



	//_______________________OS-dependent functions__________________________


	bool is_file(const std::string& filename) {
// set up for windows so acts just like unix access().
#ifndef F_OK
#define F_OK 4
#endif

		bool exist = false;
#ifdef WIN32
		exist = _access( filename.c_str(), F_OK ) == 0; // use '_waccess()' for Multi-Byte Character Set
#else
		exist = access( filename.c_str(), F_OK ) == 0;
#endif
		return (exist && !is_directory(filename));
	}



	bool is_directory(const std::string& path) {
		//std::string file_name = path;
		//if (file_name.empty())
		//	return false;
		//if (file_name[file_name.length() - 1] == '/') 
		//	file_name.erase(file_name.end() - 1);

		//WIN32_FIND_DATA file;
		//HANDLE file_handle;
		//file_handle = ::FindFirstFile(file_name.c_str(), &file);
		//if (file_handle==INVALID_HANDLE_VALUE ) {
		//	return false;
		//}
		//::FindClose(file_handle);
		//return ((file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ); 

		// Liangliang: not fully test for relative paths
		// set up _S_ISDIR()
#if !defined(S_ISDIR)
#  if defined( _S_IFDIR) && !defined( __S_IFDIR)
#    define __S_IFDIR _S_IFDIR
#  endif
#  define S_ISDIR(mode)    (mode&__S_IFDIR)
#endif

#ifdef WIN32
		struct _stat stbuf;
		if( _stat(path.c_str(), &stbuf ) == 0 ) { // _wstat()
#else
		struct stat64 stbuf;
		if( stat64( path.c_str(), &stbuf ) == 0 ) {
#endif
			if( S_ISDIR(stbuf.st_mode))
				return true;
			else
				return false;
		}
		return false;
	}


	bool create_directory(const std::string& dir) {
		if (is_directory(dir)) {
			std::cout << "directory \'" << dir << "\' already exists" << std::endl;
			return true;
		}

#ifdef WIN32
		if (_mkdir(dir.c_str()) < 0 ) { // use '_wmkdir()' for Multi-Byte Character Set
#else
		if (mkdir(dir.c_str(), 0755) != 0) {
#endif
			std::cerr << "could not mkdir" << dir << std::endl ;
			return false ;
		}

		return true ;
	}

	bool delete_directory(const std::string& path) {
#ifdef WIN32
		return (_rmdir(path.c_str()) == 0) ;
#else
		return (rmdir(path.c_str()) == 0) ;
#endif
	}

	bool delete_file(const std::string& filename) {
#ifdef WIN32
		return (_unlink(filename.c_str()) == 0) ;  // you can also use "remove()"
#else
		return (unlink(filename.c_str()) == 0) ;
#endif
	}


	std::string get_current_working_directory() {
		char buff[1024] ;
#ifdef WIN32
		return std::string(_getcwd(buff, 4096)) ;
#else
		return std::string(getcwd(buff, 4096)) ;
#endif
	}

	bool set_current_working_directory(const std::string& path) {
#ifdef WIN32
		return (_chdir(path.c_str()) == 0) ; // // use '_wchdir()' for Multi-Byte Character Set
#else
		return (chdir(path.c_str()) == 0) ;
#endif
	}

	bool rename_file(const std::string& old_name, const std::string& new_name) {
		if(is_file(new_name)) {
			return false ;
		}
		return (::rename(old_name.c_str(), new_name.c_str()) == 0) ;
	}

	unsigned int get_time_stamp(const std::string& filename)
	{
		struct stat buffer;
		if (!stat(filename.c_str(), &buffer))
            return (unsigned int)(buffer.st_mtime);
		return 0;
	}


	void get_directory_entries(const std::string& dir, std::vector<std::string>& contents) {
		if (!is_directory(dir)) {
			std::cerr << "directory \'" << dir << " \' does not exist" << std::endl; 
		}

#if defined(WIN32) && !defined(__CYGWIN__)

		std::string path = dir + "/*.*";
		_finddata_t data;
		intptr_t handle = _findfirst(path.c_str(), &data);
		if (handle != -1) {
			do {
				std::string name = data.name;
				if (name != "." && name != "..") // "." and ".." seems always there
					contents.push_back(name);
			}
			while (_findnext(handle, &data) != -1);

			_findclose(handle);
		}
#else
        DIR *handle = opendir(dir.c_str());
		if (handle)
		{
			dirent *rc;
			while((rc = readdir(handle))!=NULL)
			{
				contents.push_back(rc->d_name);
			}
			closedir(handle);
		}
#endif
	}


	//_______________________OS-independent functions__________________________


	// 
	std::string convert_to_lower_case(const std::string& str)
	{
		std::string lowcase_str(str);
		for(std::string::iterator itr=lowcase_str.begin();
			itr!=lowcase_str.end();
			++itr)
		{
			*itr = tolower(*itr);
		}
		return lowcase_str;
	}
	std::string convert_to_upper_case(const std::string& str)
	{
		std::string lowcase_str(str);
		for(std::string::iterator itr=lowcase_str.begin();
			itr!=lowcase_str.end();
			++itr)
		{
			*itr = toupper(*itr);
		}
		return lowcase_str;
	}

	std::string extension(const std::string& file_name) {
		std::string::size_type dot = file_name.find_last_of('.');
		std::string::size_type slash = file_name.find_last_of(PATH_SEPARATORS);
		if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) 
			return std::string("");

		return std::string(file_name.begin() + dot + 1, file_name.end());
	}

	std::string extension_in_lower_case(const std::string& file_name)
	{
		return convert_to_lower_case(extension(file_name));
	}

	std::string base_name(const std::string& file_name) {
		std::string simpleName = simple_name(file_name);
		return name_less_extension( simpleName );
	}

	std::string dir_name(const std::string& file_name) {
		std::string::size_type slash = file_name.find_last_of(PATH_SEPARATORS);
		if (slash == std::string::npos) 
			return std::string();
		else 
			return std::string(file_name, 0, slash);
	}

	std::string simple_name(const std::string& file_name) {
		std::string::size_type slash = file_name.find_last_of(PATH_SEPARATORS);
		if (slash == std::string::npos) 
			return file_name;
		else 
			return std::string(file_name.begin() + slash + 1, file_name.end());
	}


	// strip one level of extension from the filename.
	std::string name_less_extension(const std::string& file_name)
	{
		std::string::size_type dot = file_name.find_last_of('.');
		std::string::size_type slash = file_name.find_last_of(PATH_SEPARATORS);        // Finds forward slash *or* back slash
		if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) 
			return file_name;

		return std::string(file_name.begin(), file_name.begin() + dot);
	}


	// strip all extensions from the filename.
	std::string name_less_all_extensions(const std::string& file_name) {
		// Finds start serach position: from last slash, or the begining of the string if none found
		std::string::size_type startPos = file_name.find_last_of(PATH_SEPARATORS);  // Finds forward slash *or* back slash
		if (startPos == std::string::npos) 
			startPos = 0;
		std::string::size_type dot = file_name.find_first_of('.', startPos);        // Finds *FIRST* dot from start pos
		if (dot == std::string::npos) 
			return file_name;

		return std::string(file_name.begin(), file_name.begin() + dot);
	}

	std::string get_path_root(const std::string& path) {
		// Test for unix root
		if (path.empty()) 
			return "";
		if (path[0] == '/') 
			return "/";
		// Now test for Windows root
		if (path.length() < 2) 
			return "";
		if (path[1] == ':') 
			return path.substr(0, 2);        // We should check that path[0] is a letter, but as ':' is invalid in paths in other cases, that's not a problem.
		return "";
	}

	bool is_absolute_path(const std::string& path) {
		// Test for unix root
		if (path.empty()) 
			return false;
		if (path[0] == '/') 
			return true;
		// Now test for Windows root
		if (path.length() < 2) 
			return false;
		return 
			path[1] == ':';        // We should check that path[0] is a letter, but as ':' is invalid in paths in other cases, that's not a problem.
	}

	void get_directory_entries(
		const std::string& dir, std::vector<std::string>& result, bool recursive
		) 
	{
		get_directory_entries(dir, result) ;
		if(recursive) {
			for(unsigned int i=0; i<result.size(); i++) {
				std::string path = dir + "/" + result[i];
				if(is_directory(path)) {
					std::vector<std::string> entries;
					get_directory_entries(path, entries) ;
					for (unsigned int j=0; j<entries.size(); ++j) 
						result.push_back(result[i] + "/" + entries[j]);
				}
			}
		}
	}

	void get_files(const std::string& dir, std::vector<std::string>& result, bool recursive) {
		std::vector<std::string> entries ;
		get_directory_entries(dir, entries, recursive) ;
		for(unsigned int i=0; i<entries.size(); i++) {
			std::string name = dir + "/" + entries[i];
			if(is_file(name)) {
				result.push_back(name) ;
			}
		}
	}

	void get_subdirectories(const std::string& dir, std::vector<std::string>& result, bool recursive) {
		std::vector<std::string> entries ;
		get_directory_entries(dir, entries, recursive) ;
		for(unsigned int i=0; i<entries.size(); i++) {
			std::string name = dir + "/" + entries[i];
			if(is_directory(name)) {
				result.push_back(name) ;
			}
		}
	}

	bool copy_file(const std::string& original, const std::string& copy) {
		std::ifstream in(original.c_str());
		if (!in)
			return false;
		std::ofstream out(copy.c_str());
		LineInputStream lis(in);
		while(!lis.eof()) {
			lis.get_line();
			out << lis.current_line() << std::endl ;
		}
		return true;
	}

	bool file_contains_string(const std::string& file_name, const std::string& x) {
		std::ifstream in(file_name.c_str()) ;
		std::string buff ;
		while(in) {
			getline(in, buff) ;
			if (buff.find(x) != std::string::npos)
				return true ;
		}
		return false ;
	}



	std::string MeshStudio_resource_directory() {
		std::string dir = "./resource";
		if (FileUtils::is_directory(dir)) {
			return dir;
		}	
        else if (FileUtils::is_directory("../resource"))
            return "../" + dir;
        else if (FileUtils::is_directory("../../resource"))
            return "../../" + dir;

        else if (FileUtils::is_directory("../src/resource"))
            return "../src/" + dir;
        else if (FileUtils::is_directory("../../src/resource"))
            return "../../src/" + dir;
        else if (FileUtils::is_directory("../../../src/resource"))
            return "../../../src/" + dir;

        return dir;
	}

}
