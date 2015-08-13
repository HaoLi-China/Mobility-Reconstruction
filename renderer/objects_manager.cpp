#include "objects_manager.h"
#include "../geom/point_set.h"
#include "../basic/logger.h"
#include "../opengl/opengl_info.h"

#include <algorithm>



ObjectsManager::~ObjectsManager() {
	clear();
}


bool ObjectsManager::has_object(const Object* obj) const {
	if (obj == nil) {
		Logger::warn(title()) << "null object" << std::endl;
		return false;
	}

	ObjectInfoMap::const_iterator pos = objects_info_.find(const_cast<Object*>(obj));
	if (pos == objects_info_.end()) {
		return false;
	} else
		return true;
}


bool ObjectsManager::has_object(const std::string& name) const {
	if (name.empty()) {
		Logger::warn(title()) << "object has empty name" << std::endl;
		return false;
	}

	ObjectInfoMap::const_iterator pos = objects_info_.begin();
	for (; pos != objects_info_.end(); ++pos) {
		Object* obj = pos->first;
		if (obj->name() == name)
			return true;
	}

	return false;
}


Object* ObjectsManager::get_object(const std::string& name) const {
	if (name.empty()) {
		Logger::warn(title()) << "empty object name" << std::endl;
		return nil;
	}

	ObjectInfoMap::const_iterator pos = objects_info_.begin();
	for (; pos != objects_info_.end(); ++pos) {
		Object* obj = pos->first;
		if (obj->name() == name)
			return obj;
	}

	return nil;
}


void ObjectsManager::add_object(Object* obj, bool make_activate) {
	if (obj == nil) {
		Logger::warn(title()) << "null object" << std::endl;
		return;
	}

	if (has_object(obj)) {
		Logger::warn(title()) << "object already exists (ignored)" << std::endl;
		return;
	} 

	objects_info_[obj].render_manager = new RenderManager(obj);
	objects_info_[obj].active = false;
	sorted_objects_list_.push_back(obj);

	if (make_activate)
		set_active(obj);
}


void ObjectsManager::delete_object(Object* obj, bool activate_another) {
	if (obj == nil) {
		Logger::warn(title()) << "null object" << std::endl;
		return;
	}

	if (!has_object(obj)) {
		Logger::warn(title()) << "object doesn't exists" << std::endl;
		return;
	} 

	objects_info_.erase(obj);
	delete obj;

//////////////////////////////////////////////////////////////////////////
	std::list<Object*>::iterator it = std::find(sorted_objects_list_.begin(), sorted_objects_list_.end(), obj);
	if (it != sorted_objects_list_.end())
		sorted_objects_list_.erase(it);
	else
		ogf_assert_not_reached;
//////////////////////////////////////////////////////////////////////////

	// make the last object in the list as active
	if (activate_another && !objects_info_.empty()) {
		Object* another = *(sorted_objects_list_.rbegin());
		set_active(another);
	}
}

void ObjectsManager::clear() {
	ObjectInfoMap::iterator pos = objects_info_.begin();
	for (; pos != objects_info_.end(); ++pos) {
		Object* obj = pos->first;
		delete obj;
	}
	objects_info_.clear();
	sorted_objects_list_.clear();
}


Object* ObjectsManager::active_object() const {
	if (objects_info_.empty()) {
		return nil;
	}

	//////////////////////////////////////////////////////////////////////////

	std::vector<Object*> objs;

	ObjectInfoMap::const_iterator pos = objects_info_.begin();
	for (; pos != objects_info_.end(); ++pos) {
		Object* obj = pos->first;
		if (pos->second.active)
			objs.push_back(obj);
	}

	// always check this
	if (objs.size() == 1)
		return objs[0];
	else {
		Logger::err(title()) << objs.size() << " active object(s)" << std::endl;
		return nil;
	}
}


RenderManager* ObjectsManager::active_render_manager() const {
	Object* obj = active_object();
	if (obj)
		return render_manager_of(obj);
	else
		return nil;
}


bool ObjectsManager::is_active(const Object* obj) const {
	if (obj == nil) {
		Logger::warn(title()) << "null object" << std::endl;
		return false;
	}

	ObjectInfoMap::const_iterator pos = objects_info_.find(const_cast<Object*>(obj));
	if (pos == objects_info_.end()) {
		Logger::warn(title()) << "object doesn't exists" << std::endl;
		return false;
	} 

	return pos->second.active;
}

void ObjectsManager::set_active(const Object* obj) {
	if (obj == nil) {
		Logger::warn(title()) << "null object" << std::endl;
		return;
	}

	ObjectInfoMap::iterator pos = objects_info_.find(const_cast<Object*>(obj));
	if (pos == objects_info_.end()) {
		Logger::warn(title()) << "object doesn't exists" << std::endl;
		return;
	} 

	pos = objects_info_.begin();
	for (; pos != objects_info_.end(); ++pos) {
		Object* o = pos->first;
		if (o == obj)
			pos->second.active = true;
		else
			pos->second.active = false;
	}
}


bool ObjectsManager::is_visible(const Object* obj) const {
	if (obj == nil) {
		Logger::warn(title()) << "null object" << std::endl;
		return false;
	}

	ObjectInfoMap::const_iterator pos = objects_info_.find(const_cast<Object*>(obj));
	if (pos == objects_info_.end()) {
		Logger::warn(title()) << "object doesn't exists" << std::endl;
		return false;
	} 

	return pos->second.render_manager->is_visible();
}


void ObjectsManager::set_visible(const Object* obj, bool v) {
	if (obj == nil) {
		Logger::warn(title()) << "null object" << std::endl;
		return;
	}

	ObjectInfoMap::iterator pos = objects_info_.find(const_cast<Object*>(obj));
	if (pos == objects_info_.end()) {
		Logger::warn(title()) << "object doesn't exists" << std::endl;
		return;
	} 
	else
		pos->second.render_manager->set_visible(v);
}

// return the render_manager of 'obj'
RenderManager* ObjectsManager::render_manager_of(const Object* obj) const {
	if (obj == nil) {
		Logger::warn(title()) << "null object" << std::endl;
		return nil;
	}

	ObjectInfoMap::const_iterator pos = objects_info_.find(const_cast<Object*>(obj));
	if (pos == objects_info_.end()) {
		Logger::warn(title()) << "object doesn't exists" << std::endl;
		return nil;
	} 

	return pos->second.render_manager;
}

std::vector<Object*> ObjectsManager::objects() const {
// 	std::vector<Object*> results;
// 	ObjectInfoMap::const_iterator pos = objects_info_.begin();
// 	for (; pos != objects_info_.end(); ++pos) 
// 		results.push_back(pos->first);
// 	return results;

	//////////////////////////////////////////////////////////////////////////

	std::vector<Object*> results(sorted_objects_list_.begin(), sorted_objects_list_.end());
	return results;

	//////////////////////////////////////////////////////////////////////////
}


// call draw() in your draw function
void ObjectsManager::draw() const {
	ObjectInfoMap::const_iterator pos = objects_info_.begin();
	for (; pos != objects_info_.end(); ++pos) {
		Object* obj = pos->first;

		if (dynamic_cast<PointSet*>(obj))
			glDisable(GL_MULTISAMPLE);
		else // object has facets
			glEnable(GL_MULTISAMPLE);

		pos->second.render_manager->draw();
	}
}
