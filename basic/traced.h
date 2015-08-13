
#ifndef __BASIC_DEBUG_TRACED__
#define __BASIC_DEBUG_TRACED__

#include "basic_common.h"
#include "counted.h"


/**
*
* This class can be used to debug containers.
* Each instance is provided with a unique id,
* and its constructor, copy contructor, destructor 
* and operator = issue a message on the standard
* output.
*
*/

class BASIC_API Traced : public virtual Counted {

public:
	Traced() ;
	Traced(const Traced& rhs) ;
	virtual ~Traced() ;

	Traced& operator=(const Traced& rhs) ;

private:
	int id_ ;
	static int last_id_ ;
} ;


#endif
