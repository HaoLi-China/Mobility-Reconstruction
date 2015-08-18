//HaoLi
#ifndef _KINECT_IO_COMMON_H_
#define _KINECT_IO_COMMON_H_


#include "../basic/basic_common.h"


# ifdef KINECT_IO_EXPORTS
#   define KINECT_IO_API  EXPORT_LIBRARY
# else
#   define KINECT_IO_API  IMPORT_LIBRARY
# endif


#endif