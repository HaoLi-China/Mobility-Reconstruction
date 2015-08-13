
#ifndef _RENDERER_RENDER_MANAGER_H_
#define _RENDERER_RENDER_MANAGER_H_

#include "renderer_common.h"
#include "render.h"
#include "../basic/counted.h"
#include "../basic/smart_pointer.h"

#include <map>


class Object;

// each object has its own RenderManager that 
// supports different rendering mode.
class RENDERER_API RenderManager : public Counted
{
public:
	RenderManager(Object* obj) ;
	virtual ~RenderManager() {}

	static std::string title() { return "RenderManager"; }

	Object* target() const { return target_ ; }
	void	set_target(Object* obj) ;

	bool    is_visible() const { return visible_; }
	void	set_visible(bool v) { visible_ = v; }

	void	set_render(RenderType t);
	void    set_render(const std::string& s);
	
	Render*		current_render() { return current_render_ ; }
	RenderType  current_render_type() const { return current_render_type_ ;}

	void draw();

	std::vector<Render*>			applicable_renders() const ;
	const std::vector<std::string>& applicable_renders_names() const ;

protected:
	void create_renders();

private:
	Object*		target_ ;
	bool		visible_ ;

	Render*		current_render_ ;
	RenderType  current_render_type_ ;

	typedef std::map<RenderType, Render_var> RenderMap ;
	RenderMap	renders_ ;

	std::vector<std::string> applicable_renders_;
};


typedef SmartPointer<RenderManager>		RenderManager_var ;


#endif
