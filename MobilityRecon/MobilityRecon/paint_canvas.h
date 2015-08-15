#ifndef PAINTCANVAS_H
#define PAINTCANVAS_H


#include "../../opengl/glew.h"
#include "../../3rd_QGLViewer/qglviewer.h"
#include "../../image/color.h"
#include "../../math/math_types.h"
#include "../../basic/canvas.h"
#include "../../renderer/texture.h"
#include "../../renderer/objects_manager.h"

class Object;
class Map;
class PointSet;
class MainWindow;
class Render;

class MainWindow;

enum CanvasType
{
	CT_FRONT, CT_BACK,
	CT_LEFT, CT_RIGHT,
	CT_TOP, CT_BOTTOM,
	CT_ROTATION_FREE
};

class PaintCanvas : public QGLViewer, public Canvas
{
	Q_OBJECT

public:
	PaintCanvas(QWidget* parent, QGLFormat format, CanvasType type = CT_FRONT);
	~PaintCanvas();

public:
	std::string title() const { return "PaintCanvas"; }

	//////////////////////////////////////////////////////////////////////////
	void update();
	void immediate_update();

	//////////////////////////////////////////////////////////////////////////

	// the active object
	Object* activeObject() const;
	bool    isActive(const Object* obj) const;

	ObjectsManager* objectsManager() const { return objects_manager_; }

	// the active object
	Map*		mesh() const;
	PointSet*	pointSet() const;
	CGraph*		whetGrid(void) const { return nil; }

protected:
	virtual void draw();
	virtual void init();


	public Q_SLOTS:
	void fitScreen();
	void snapshotScreen();
	void snapshotScreen(const QString &filename);

private:
	void drawCornerAxis();

private:
	MainWindow* main_window_;
	int coord_system_region_size_;
	bool show_coord_sys_;

	ObjectsManager_var	objects_manager_;
};


#endif