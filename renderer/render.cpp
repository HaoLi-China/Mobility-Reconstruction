#include "render.h"
#include "../basic/object.h"


Render::Render(Object* obj)
: object_(obj)
{ 
}


void Render::sleep(int milliseconds) {
#ifdef _WIN32
	::Sleep(milliseconds);
#else
	::usleep(milliseconds * 1000);
#endif
}
