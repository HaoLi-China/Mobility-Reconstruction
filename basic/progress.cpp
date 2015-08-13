
#include "progress.h"
#include "assertions.h"


Progress* Progress::instance_ = nil ;

Progress::Progress() : client_(nil), level_(0), canceled_(false) {
}

Progress::~Progress() {
}

void Progress::push() { 
	level_++ ; 
	if(level_ == 1) {
		clear_canceled() ;
	}
}

void Progress::pop() {
	ogf_assert(level_ > 0) ;
	level_-- ;
}

void Progress::notify(int new_val) {
	if(client_ != nil && level_ < 2) {
		client_->notify_progress(new_val) ;
	}
}

Progress* Progress::instance() {
	if(instance_ == nil) {
		instance_ = new Progress ;
	}
	return instance_ ;
}


//_________________________________________________________


ProgressClient::~ProgressClient() {
	if (Progress::instance())
		delete Progress::instance();
}

//_________________________________________________________

ProgressLogger::ProgressLogger(int max_val, const std::string& task_name, bool quiet) 
: max_val_(max_val), task_name_(task_name), quiet_(quiet) 
{
	cur_val_ = 0 ; 
	cur_percent_ = 0 ;
	Progress::instance()->push() ;
	if(!quiet_) {
		Progress::instance()->notify(0) ;
	}
}

void ProgressLogger::reset(int max_val)  {
	max_val_ = max_val ;
	reset() ;
}

ProgressLogger::~ProgressLogger() {
    Progress::instance()->notify(100) ;
    Progress::instance()->notify(0) ;
    Progress::instance()->pop() ;
}

void ProgressLogger::next() {
	cur_val_++ ;
	update() ;
}

void ProgressLogger::notify(int new_val) {
	cur_val_ = new_val ;
	update() ;
}


void ProgressLogger::update() {
	int percent = cur_val_ * 100 / ogf_max(1, max_val_-1) ;
	if(percent != cur_percent_) {
		cur_percent_ = percent ;
		if(!quiet_) {
			Progress::instance()->notify(ogf_min(cur_percent_, 100)) ;
		}
	}
}

