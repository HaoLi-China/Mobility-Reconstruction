
#ifndef _RENDERER_OBJECTS_MANAGER_H_
#define _RENDERER_OBJECTS_MANAGER_H_

#include "renderer_common.h"
#include "render_manager.h"
#include "../basic/counted.h"
#include "../basic/smart_pointer.h"

#include <map>
#include <list>



class Object;

// maintain a set of object (each object has its own MenderManager 
// that supports different rendering mode).
class RENDERER_API ObjectsManager : public Counted
{
public:
	ObjectsManager() {}
	~ObjectsManager() ;

	static std::string title() { return "ObjectsManager"; }

	//----------------------------------------------

	void add_object(Object* obj, bool make_activate = true);
	void delete_object(Object* obj, bool activate_another = true);

	// delete all object
	void clear();

	std::vector<Object*> objects() const ;
	unsigned int num_object() const { return static_cast<unsigned int>(objects().size()); }

	bool has_object(const Object* obj) const ;
	bool has_object(const std::string& name) const ;
	Object* get_object(const std::string& name) const ;

	Object* active_object() const ;
	RenderManager* active_render_manager() const ;

	// return the render_manager of 'obj'
	RenderManager* render_manager_of(const Object* obj) const ;

	bool is_active(const Object* obj) const ;
	void set_active(const Object* obj) ;

	bool is_visible(const Object* obj) const ;
	void set_visible(const Object* obj, bool v);

	// call draw() in your draw function
	void draw() const ;

private:
	struct ObjectInfo {
		RenderManager_var   render_manager;
		bool				active;
	};

	typedef std::map<Object*, ObjectInfo> ObjectInfoMap ;
	ObjectInfoMap	objects_info_ ;

	//////////////////////////////////////////////////////////////////////////
	// NOTE: you can delete this. 
	// It is only used for maintaining the order of the objects.
	std::list<Object*>  sorted_objects_list_;
};


typedef SmartPointer<ObjectsManager>	ObjectsManager_var ;


#endif
