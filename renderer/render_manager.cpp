#include "render_manager.h"
#include "plain_surface_render.h"
#include "scalar_surface_render.h"
#include "plain_point_set_render.h"
#include "scalar_point_set_render.h"
#include "../basic/logger.h"
#include "../geom/map.h"
#include "../geom/point_set.h"


RenderManager::RenderManager(Object* obj) : visible_(true) {
	set_target(obj);
}


void RenderManager::set_target(Object* obj) { 
	target_ = obj; 
	create_renders();
}


void RenderManager::create_renders() {
	renders_.clear();
	applicable_renders_.clear();

	if (dynamic_cast<Map*>(target())) {
		Map* map = dynamic_cast<Map*>(target());
		renders_[PLAIN_SURFACE_RENDER] = new PlainSurfaceRender(map);
		renders_[SCALAR_SURFACE_RENDER] = new ScalarSurfaceRender(map);
        set_render(PLAIN_SURFACE_RENDER);
		applicable_renders_.push_back("Plain");
		applicable_renders_.push_back("Scalar");
	} 
	else if (dynamic_cast<PointSet*>(target())) {
		PointSet* pset = dynamic_cast<PointSet*>(target());
		renders_[PLAIN_POINT_SET_RENDER] = new PlainPointSetRender(pset);
		renders_[SCALAR_POINT_SET_RENDER] = new ScalarPointSetRender(pset);
        set_render(PLAIN_POINT_SET_RENDER);
		applicable_renders_.push_back("Plain");
		applicable_renders_.push_back("Scalar");
	}
	else {
		Logger::err(title()) << "no renderer for this object" << std::endl;
	}
}


void RenderManager::set_render(RenderType t) {
	RenderMap::iterator it = renders_.find(t) ;
	if(it == renders_.end()) {
		Logger::err(title()) << ": no such render" << std::endl ;
		current_render_ = nil ;
		current_render_type_ = EMPTY_RENDER;
		return ;
	}
	else {
		current_render_ = it->second ;
		current_render_type_ = it->first;
	}
}


void RenderManager::set_render(const std::string& s) {
	if (dynamic_cast<Map*>(target())) {
		if (s == "Plain") 
			set_render(PLAIN_SURFACE_RENDER);
		else if (s == "Scalar") 
			set_render(SCALAR_SURFACE_RENDER);
		else
			Logger::err(title()) << "render \'" << s << "\' not applicable for current object" << std::endl;
	} 
	else if (dynamic_cast<PointSet*>(target())) {
		if (s == "Plain") 
			set_render(PLAIN_POINT_SET_RENDER);
		else if (s == "Scalar")
			set_render(SCALAR_POINT_SET_RENDER);
		else
			Logger::err(title()) << "render \'" << s << "\' not applicable for current object" << std::endl;
	}

	else {
		Logger::err(title()) << "no renderer for this object" << std::endl;
	}
}

void RenderManager::draw() {
	if(visible_ && current_render_ != nil) {
		current_render_->draw() ;
	}
}


std::vector<Render*> RenderManager::applicable_renders() const {
	std::vector<Render*> results;

	RenderMap::const_iterator it = renders_.begin();
	for (; it!=renders_.end(); ++it)
		results.push_back(it->second);

	return results;
}


const std::vector<std::string>& RenderManager::applicable_renders_names() const {
	return applicable_renders_;
}



