#include "paint_canvas.h"
#include "main_window.h"
#include "../../3rd_QGLViewer/manipulatedCameraFrame.h"
#include "../../geom/map.h"
#include "../../geom/map_editor.h"
#include "../../geom/map_geometry.h"
#include "../../geom/point_set.h"
#include "../../geom/point_set_geometry.h"
#include "../../opengl/opengl_info.h"
#include "../../renderer/render.h"

using namespace std;

PaintCanvas::PaintCanvas(QWidget* parent, QGLFormat format, CanvasType type /*= CT_FRONT*/)
	:QGLViewer(format, parent)
	, coord_system_region_size_(150)
	, show_coord_sys_(true)
{
	main_window_ = dynamic_cast<MainWindow*>(parent);

	objects_manager_ = new ObjectsManager;

	//////////////////////////////////////////////////////////////////////////

	// Move camera according to viewer type (on X, Y or Z axis)
	// e.g. 'GL_FRONT' equals to 
	// 'camera()->setViewDirection(qglviewer::Vec(0.0, 1.0, 0.0));'
	switch (type) {
	case CT_ROTATION_FREE:	camera()->setPosition(qglviewer::Vec(1.0, 1.0, 1.0));	break;
	case CT_FRONT:			camera()->setPosition(qglviewer::Vec(0.0, -1.0, 0.0));	break;
	case CT_BACK:			camera()->setPosition(qglviewer::Vec(0.0, 1.0, 0.0));	break;
	case CT_LEFT:			camera()->setPosition(qglviewer::Vec(-1.0, 0.0, 0.0));	break;
	case CT_RIGHT:			camera()->setPosition(qglviewer::Vec(1.0, 0.0, 0.0));	break;
	case CT_TOP:			camera()->setPosition(qglviewer::Vec(0.0, 0.0, 1.0));	break;
	case CT_BOTTOM:			camera()->setPosition(qglviewer::Vec(0.0, 0.0, -1.0));	break;
	default:	break;
	}

	camera()->lookAt(sceneCenter());
	camera()->setType(qglviewer::Camera::PERSPECTIVE);
	camera()->showEntireScene();
}

PaintCanvas::~PaintCanvas() {
	// this is required by the following destruction of textures, shaders, etc.
	makeCurrent();
}

void PaintCanvas::init()
{
	Logger::out(title()) << "initializing..." << std::endl;

	//////////////////////////////////////////////////////////////////////////
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		// Problem: glewInit failed, something is seriously wrong. 
		Logger::err(title()) << glewGetErrorString(err) << std::endl;
	}
	Logger::out(title()) << "Using GLEW: " << GLInfo::glew_version() << std::endl;
	Logger::out(title()) << "GL Vendor: " << GLInfo::gl_vendor() << std::endl;
	Logger::out(title()) << "GL Renderer: " << GLInfo::gl_renderer() << std::endl;
	Logger::out(title()) << "GL Version: " << GLInfo::gl_version() << std::endl;

	Logger::out(title()) << "GLSL Version: " << GLInfo::glsl_version() << std::endl;
	//Logger::out(title()) << "GL_Extensions: " << GLInfo::gl_extensions() << std::endl;

	//////////////////////////////////////////////////////////////////////////

	setStateFileName("");

	// Default value is 0.005, which is appropriate for most applications. 
	// A lower value will prevent clipping of very close objects at the 
	// expense of a worst Z precision.
	camera()->setZNearCoefficient(0.005f);

	// Default value is square root of 3.0 (so that a cube of size 
	// sceneRadius() is not clipped).
	camera()->setZClippingCoefficient(std::sqrt(3.0f));

	camera()->setViewDirection(qglviewer::Vec(0.0, 1.0, 0.0));
	camera()->setType(qglviewer::Camera::PERSPECTIVE);
	showEntireScene();

	camera()->frame()->setSpinningSensitivity(/*1.0f*/100.0f);
	setMouseTracking(true);

	//////////////////////////////////////////////////////////////////////////
	// I like the inverse direction
	setShortcut(MOVE_CAMERA_LEFT, Qt::Key_Right);
	setShortcut(MOVE_CAMERA_RIGHT, Qt::Key_Left);
	setShortcut(MOVE_CAMERA_UP, Qt::Key_Down);
	setShortcut(MOVE_CAMERA_DOWN, Qt::Key_Up);

	setMouseBinding(Qt::ShiftModifier, Qt::LeftButton, CAMERA, SCREEN_ROTATE);
	setMouseBinding(Qt::ControlModifier, Qt::LeftButton, CAMERA, ZOOM_ON_REGION);

	//////////////////////////////////////////////////////////////////////////

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  //GL_ONE_MINUS_DST_ALPHA

	// for transparent 
	glAlphaFunc(GL_GREATER, 0);
	glEnable(GL_ALPHA_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//////////////////////////////////////////////////////////////////////////

	QColor bkgrd_color = Qt::white;
	setBackgroundColor(bkgrd_color);

	//////////////////////////////////////////////////////////////////////////

	float pos[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	// Setup light parameters
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); /*GL_FALSE*/

	// how specular reflection angles are computed. GL_TRUE will introduce artifact for glu tess with specular
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);

	// Makes specular lighting work in texture mapping mode.
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	//////////////////////////////////////////////////////////////////////////

	// specify the specular and shininess
	float specular[] = { 0.4f, 0.4f, 0.4f, 0.5f };
	float shininess = 64.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);

	////////////////////////////////////////////////////////////////////////////

	// make the back side different
	float back_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float back_shininess = 128;
	float ambient_back[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	glMaterialfv(GL_BACK, GL_SPECULAR, back_specular);
	glMaterialf(GL_BACK, GL_SHININESS, back_shininess);
	//glMaterialfv(GL_BACK, GL_AMBIENT, ambient_back);
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, ambient_back);

	////////////////////////////////////////////////////////////////////////

	// to use facet color, the GL_COLOR_MATERIAL should be enabled
	glEnable(GL_COLOR_MATERIAL);
	// to use material color, the GL_COLOR_MATERIAL should be disabled
	//glDisable(GL_COLOR_MATERIAL);
}

// Draws a spiral
void PaintCanvas::draw()
{
	if (show_coord_sys_)
		drawCornerAxis();

	//////////////////////////////////////////////////////////////////////////

	objects_manager_->draw();
}

void PaintCanvas::update() {
	updateGL();
	//main_window_->updateStatusBar();
}

void PaintCanvas::immediate_update() {
	update();

	// This approach has significant drawbacks. For example, imagine you wanted to perform two such loops 
	// in parallel-calling one of them would effectively halt the other until the first one is finished 
	// (so you can't distribute computing power among different tasks). It also makes the application react
	// with delays to events. Furthermore the code is difficult to read and analyze, therefore this solution
	// is only suited for short and simple problems that are to be processed in a single thread, such as 
	// splash screens and the monitoring of short operations.
	QCoreApplication::processEvents();
}

Object* PaintCanvas::activeObject() const {
	return objects_manager_->active_object();
}

bool PaintCanvas::isActive(const Object* obj) const {
	return (activeObject() == obj);
}

void PaintCanvas::fitScreen() {
	Object* obj = activeObject();
	if (obj) {
		Box3d box;
		if (dynamic_cast<Map*>(obj))
			box = Geom::bounding_box(dynamic_cast<Map*>(obj));
		else if (dynamic_cast<PointSet*>(obj))
			box = Geom::bounding_box(dynamic_cast<PointSet*>(obj));

		qglviewer::Vec vmin(box.x_min(), box.y_min(), box.z_min());
		qglviewer::Vec vmax(box.x_max(), box.y_max(), box.z_max());

		//Vec vmin(-2.0f, -2.0f, -2.0f);
		//Vec vmax(2.0f, 2.0f, 2.0f);
		setSceneBoundingBox(vmin, vmax);
		showEntireScene();
	}
	updateGL();
}


void PaintCanvas::drawCornerAxis()
{
	glEnable(GL_MULTISAMPLE);

	// The viewport and the scissor are changed to fit the lower left
	// corner. Original values are saved.
	int viewport[4];
	int scissor[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetIntegerv(GL_SCISSOR_BOX, scissor);

	//////////////////////////////////////////////////////////////////////////

	// Axis viewport size, in pixels
	glViewport(0, 0, coord_system_region_size_, coord_system_region_size_);
	glScissor(0, 0, coord_system_region_size_, coord_system_region_size_);

	// The Z-buffer is cleared to make the axis appear over the
	// original image.
	glClear(GL_DEPTH_BUFFER_BIT);

	// Tune for best line rendering
	glLineWidth(3.0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixd(camera()->orientation().inverse().matrix());

	float axis_size = 0.9f; // other 0.2 space for drawing the x, y, z labels
	drawAxis(axis_size);

	// Draw text id
	glColor3f(0, 0, 0);

	// Liangliang: It seems the renderText() func disables multi-sample.
	// Is this a bug in Qt ?
	GLboolean anti_alias = glIsEnabled(GL_MULTISAMPLE);
	const_cast<PaintCanvas*>(this)->renderText(axis_size, 0, 0, "X");
	const_cast<PaintCanvas*>(this)->renderText(0, axis_size, 0, "Y");
	const_cast<PaintCanvas*>(this)->renderText(0, 0, axis_size, "Z");
	if (anti_alias)
		glEnable(GL_MULTISAMPLE);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	//////////////////////////////////////////////////////////////////////////

	// The viewport and the scissor are restored.
	glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

Map* PaintCanvas::mesh() const {
	return dynamic_cast<Map*>(activeObject());
}


PointSet* PaintCanvas::pointSet() const {
	return dynamic_cast<PointSet*>(activeObject());
}

void PaintCanvas::snapshotScreen() {
	bool need_hide = show_coord_sys_;
	if (need_hide)
		show_coord_sys_ = false;  // hide the coord system temporally

	saveSnapshot(false);

	if (need_hide) {
		show_coord_sys_ = true;
		updateGL();
	}
}

void PaintCanvas::snapshotScreen(const QString &filename) {
	bool need_hide = show_coord_sys_;
	if (need_hide)
		show_coord_sys_ = false;  // hide the coord system temporally

	saveSnapshot(filename);

	if (need_hide) {
		show_coord_sys_ = true;
		updateGL();
	}
}