#include "object.h"
#include "canvas.h"
#include "logger.h"


Object::Object() {}


Object::~Object() {}


void Object::update() const	{ 
	if (canvas_)	
		canvas_->update(); 
	else
		Logger::err("Object") << "you should assign canvas" << std::endl;
}

void Object::immediate_update() const { 
	if (canvas_)	
		canvas_->immediate_update(); 
	else
		Logger::err("Object") << "you should assign canvas" << std::endl;
}