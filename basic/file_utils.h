
#ifndef _BASIC_FILE_UTAILS_H_
#define _BASIC_FILE_UTAILS_H_

#include "basic_common.h"
#include <string>
#include <vector>

// "OpenSceneGraph - <osgDB/FileNameUtils>" has great implementation and documentation

namespace FileUtils {

	// ------------------- OS-dependent functions -------------------

	bool BASIC_API is_file(const std::string& filename) ;
	bool BASIC_API is_directory(const std::string& filename) ;    
	bool BASIC_API create_directory(const std::string& path) ; // Warning: path should be absolute.
	bool BASIC_API delete_directory(const std::string& path) ;
	
	bool BASIC_API delete_file(const std::string& filename) ;
	void BASIC_API get_directory_entries(const std::string& dir, std::vector<std::string>& entries);
	
	std::string BASIC_API get_current_working_directory() ;
	bool BASIC_API set_current_working_directory(const std::string& path) ;
	
	bool BASIC_API rename_file(const std::string& old_name, const std::string& new_name) ;
	unsigned int BASIC_API get_time_stamp(const std::string& filename) ;


	// ------------------- OS-independent functions -------------------

	std::string BASIC_API convert_to_lower_case(const std::string& str);
	std::string BASIC_API convert_to_upper_case(const std::string& str);

	/** Gets the parent path from full name (Ex: /a/b/c.Ext => /a/b). */
	std::string BASIC_API dir_name(const std::string& file_name) ;
	/** Gets the extension without dot (Ex: /a/b/c.Ext => Ext). */
	std::string BASIC_API extension(const std::string& file_name) ;
	/** Gets the lowercase extension without dot (Ex: /a/b/c.Ext => ext). */
	std::string BASIC_API extension_in_lower_case(const std::string& filename);

	/** Gets file name with extension (Ex: /a/b/c.Ext => c.Ext). */
	std::string BASIC_API simple_name(const std::string& file_name) ;
	/** Gets file name without path and last extension (Ex: c:/file.ext1.ext2 => file.ext1; /a/b/c.Ext => c). */
	std::string BASIC_API base_name(const std::string& file_name) ;
	
	/** Gets file path without last extension (Ex: /a/b/c.Ext => /a/b/c ; file.ext1.ext2 => file.ext1). */
	std::string BASIC_API name_less_extension(const std::string& file_name);
	/** Gets file path without all extensions (Ex: /a/b/c.Ext => /a/b/c ; file.ext1.ext2 => file). */
	std::string BASIC_API name_less_all_extensions(const std::string& file_name);

	/** Gets root part of a path ("/" or "C:"), or an empty string if none found. */
	std::string BASIC_API get_path_root(const std::string& path);
	/** Tests if path is absolute, as !get_path_root(path).empty(). */
	bool		BASIC_API is_absolute_path(const std::string& path);

	void BASIC_API get_directory_entries(const std::string& dir, std::vector<std::string>& entries, bool recursive) ;
	void BASIC_API get_files(const std::string& dir, std::vector<std::string>& files, bool recursive = false) ;
	void BASIC_API get_subdirectories(const std::string& dir, std::vector<std::string>& subs, bool recursive = false) ;

	bool BASIC_API copy_file(const std::string& original, const std::string& copy);
	bool BASIC_API file_contains_string(const std::string& file_name, const std::string& x) ;


	//////////////////////////////////////////////////////////////////////////

	// this is only for MeshStudio
	std::string BASIC_API MeshStudio_resource_directory();
}




#endif
