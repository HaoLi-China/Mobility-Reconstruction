
#ifndef _RENDERER_RENDER_H_
#define _RENDERER_RENDER_H_

#include "renderer_common.h"
#include "render_types.h"
#include "../basic/counted.h"

#include <string>



class Object;

class RENDERER_API Render : public Counted
{
public:
	Render(Object* obj) ;
	virtual ~Render() {}

	virtual RenderType type() const = 0;

	virtual void draw() = 0 ;
	virtual void blink() = 0;

	Object* object() const { return object_; }

    void sleep(int milliseconds);

private:
	Object* object_;
} ;


typedef SmartPointer<Render>  Render_var;

#endif
