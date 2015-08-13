
#ifndef _MOBILITY_COMMON_H_
#define _MOBILITY_COMMON_H_


#include "../basic/basic_common.h"


# ifdef MOBILITY_EXPORTS
#   define MOBILITY_API  EXPORT_LIBRARY
# else
#   define MOBILITY_API  IMPORT_LIBRARY
# endif


#endif