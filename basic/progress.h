#ifndef __BASIC_PROGRESS__
#define __BASIC_PROGRESS__

#include "basic_common.h"
#include "basic_types.h"


class ProgressClient ;

/**
* For internal use, client code should use UserProgress.
*/
class BASIC_API Progress {
public:
	Progress() ;
	virtual ~Progress() ;

	static Progress* instance() ;

	virtual void notify(int new_val) ;

	void set_client(ProgressClient* c) { client_ = c ; }

	void push() ;
	void pop() ;

	void cancel()            { canceled_ = true ;  }
	void clear_canceled()    { canceled_ = false ; }
	bool is_canceled() const { return canceled_ ;  }
private:
	static Progress* instance_ ;
	ProgressClient* client_ ;
	int  level_ ;
	bool canceled_ ;
} ;

//_________________________________________________________

/**
* For internal use, client code do not need to use this one.
*/
class BASIC_API ProgressClient {
public:
	virtual void notify_progress(int new_val) = 0;
	virtual ~ProgressClient() ;
} ;

//_________________________________________________________

class BASIC_API ProgressLogger {
public:
	ProgressLogger(int max_val = 100, const std::string& task_name = "", bool quiet = false) ;
	virtual ~ProgressLogger() ;

	virtual void notify(int new_val) ;
	virtual void next() ;
	bool is_canceled() const {
		return Progress::instance()->is_canceled() ;
	}
	void reset() { notify(0) ; }
	void reset(int max_val) ;

protected:
	virtual void update() ;

private:
	int max_val_ ;
	std::string task_name_ ;
	int cur_val_ ;
	int cur_percent_ ;
	bool quiet_ ;
} ;


#endif

