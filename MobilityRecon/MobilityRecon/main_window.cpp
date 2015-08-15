#include <iostream>
#include <QFileDialog>
#include <QLabel>
#include <QSplitter>
#include <QTextEdit>
#include <time.h>

#include "main_window.h"
#include "paint_canvas.h"

#include "../../basic/file_utils.h"
#include "../../geom/map.h"
#include "../../geom/point_set.h"
#include "../../file_io/map_io.h"
#include "../../file_io/point_set_io.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, curDataDirectory_(".")
, auto_focus_(false)
, selected_only_(false)
, highlighting_(false)
{
	ui.setupUi(this);

	//////////////////////////////////////////////////////////////////////////

	Logger::initialize();
	Logger::instance()->register_client(this);
	Logger::instance()->set_value(Logger::LOG_REGISTER_FEATURES, "*"); // log everything
	Logger::instance()->set_value(Logger::LOG_FILE_NAME, "MobilityRecon.log");

	// Liangliang: added the time stamp in the log file
	time_t now = time(NULL); /* get current time; same as: time(&now)  */
	struct tm* timeinfo = localtime(&now);
	std::string tstr = asctime(timeinfo);
	Logger::out("MobilityRecon") << "--- started at: " << tstr.substr(0, tstr.length() - 1) << " ---" << std::endl;
	// use strftime() to define your own stamp format
	//size_t strftime(char *strDest, size_t maxsize, const char *format, const struct tm *timeptr);

	Progress::instance()->set_client(this);

	///////////////////////////////////////////////////////////////////

	// Setup the format to allow antialiasing if the graphic driver allow this.
	QGLFormat format = QGLFormat::defaultFormat();
	format.setSampleBuffers(true); // you can also call setOption(sampleBuffers)
	format.setSamples(8); // 8 is enough


	mainCanvas_ = new PaintCanvas(this, format, CT_FRONT);
	mainCanvas_->setAttribute(Qt::WA_MouseTracking);
	//setCentralWidget(mainCanvas_);	

	mainSplitter = new QSplitter(Qt::Horizontal, this);
	leftSplitter = new QSplitter(Qt::Vertical, mainSplitter);
	leftSplitter->addWidget(mainCanvas_);
	seqSlider = new QSlider(Qt::Horizontal, leftSplitter);
	seqSlider->setMaximum(0);
	seqSlider->setVisible(false);

	//need to modify
	secondCanvas_ = new PaintCanvas(this, format, CT_FRONT);
	secondCanvas_->setAttribute(Qt::WA_MouseTracking);

	thirdCanvas_ = new PaintCanvas(this, format, CT_FRONT);
	thirdCanvas_->setAttribute(Qt::WA_MouseTracking);

	rightSplitter = new QSplitter(Qt::Vertical, mainSplitter);
	rightSplitter->addWidget(secondCanvas_);
	rightSplitter->addWidget(thirdCanvas_);

	mainSplitter->addWidget(rightSplitter);

	setCentralWidget(mainSplitter);


	/////////////////////////////////////////////////////////////////////
	//createMenus();
	createActions();
	connect(seqSlider, SIGNAL(valueChanged(int)), this, SLOT(ChangeFrame(int)));

	setWindowState(Qt::WindowMaximized);
	setFocusPolicy(Qt::StrongFocus);
}

MainWindow::~MainWindow()
{
	Progress::instance()->set_client(nil);
	Logger::instance()->unregister_client(this);
	Logger::terminate();
}

//void MainWindow::showCoordinateUnderMouse(const vec3& p, bool found) {
//	QString coordString = "XYZ = [-, -, -]";
//	if (found)
//		coordString = QString("XYZ = [%1, %2, %3]").arg(p.x).arg(p.y).arg(p.z);
//	coordinateUnderMouseLabel_->setText(coordString);
//}

//void MainWindow::updateStatusBar()
//{
//	QString cell, vertices, edges;
//
//	Object* obj = mainCanvas_->activeObject();
//	if (dynamic_cast<Map*>(obj)) {
//		Map* mesh = dynamic_cast<Map*>(obj);
//		unsigned int num_faces = mesh->size_of_facets();
//		unsigned int num_vertices = mesh->size_of_vertices();
//		unsigned int num_edges = mesh->size_of_halfedges() / 2;
//
//		cell = QString("#faces: %1").arg(num_faces);
//		vertices = QString("#vertices: %1").arg(num_vertices);
//		edges = QString("#edges: %1").arg(num_edges);
//	}
//
//	else if (dynamic_cast<PointSet*>(obj)) {
//		PointSet* pset = dynamic_cast<PointSet*>(obj);
//		unsigned int num_vertices = pset->size_of_vertices();
//		cell = QString("#vertices: %1").arg(num_vertices);
//	}
//
//	numCellsLabel_->setText(cell);
//	numVerticesLabel_->setText(vertices);
//	numEdgesLabel_->setText(edges);
//}

//void MainWindow::createMenus() {
//	/*actionSeparator = ui.menuFile->addSeparator();
//
//	QList<QAction*> actions;
//	for (int i = 0; i < MaxRecentFiles; ++i)
//	actions.push_back(actionsRecentFile[i]);
//	ui.menuFile->insertActions(ui.actionExit, actions);*/
//}

void MainWindow::createActions() {
	// file menu
	createActionsForFileMenu();
}

void MainWindow::createActionsForFileMenu() {
	//////////////////////////////////////////////////////////////////////////
	//for (int i = 0; i < MaxRecentFiles; ++i) {
	//	actionsRecentFile[i] = new QAction(this);
	//	actionsRecentFile[i]->setVisible(false);
	//	connect(actionsRecentFile[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
	//}

	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(open()));
	connect(ui.actionImport, SIGNAL(triggered()), this, SLOT(import()));
	connect(ui.actionSnapshot, SIGNAL(triggered()), this, SLOT(snapshot()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionExportSequentialSnapshots, SIGNAL(triggered()), this, SLOT(export_sequential_snapshots()));

	ui.actionExit->setShortcut(QString("Ctrl+Q"));
}


bool MainWindow::open()
{
	seqSlider->setVisible(false);
	allFileNames.clear();

	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open file"), curDataDirectory_,
		tr("Supported format (*.ply *.obj *.eobj *.off *.stl *.ply2 *.xyz *.bxyz *.pn *.bpn *.pnc *.bpnc *.mesh *.meshb *.tet)\n"
		"Mesh format (*.ply *.obj *.eobj *.off *.stl *.ply2)\n"
		"Point set format (*.ply *.xyz *.bxyz *.pn *.bpn *.pnc *.bpnc)\n"
		"All format (*.*)")
		);

	if (fileName.isEmpty())
		return false;

	if (fileName == curFileName_)
		return false;

	return doOpen(fileName);
}

bool MainWindow::import()
{
	seqSlider->setVisible(true);

	allFileNames.clear();
	removeAllObjects();
	
	allFileNames = QFileDialog::getOpenFileNames(this,
		tr("Import file"), curDataDirectory_,
		tr("Supported format (*.ply *.obj *.eobj *.off *.stl *.ply2 *.xyz *.bxyz *.pn *.bpn *.pnc *.bpnc *.mesh *.meshb *.tet)\n"
		"Mesh format (*.ply *.obj *.eobj *.off *.stl *.ply2)\n"
		"Point set format (*.ply *.xyz *.bxyz *.pn *.bpn *.pnc *.bpnc)\n"
		"All format (*.*)")
		);

	if (allFileNames.isEmpty())
		return false;
	
	seqSlider->setMaximum(allFileNames.size() - 1);
	seqSlider->setValue(0);
	return doOpen(allFileNames[0]);
}

bool MainWindow::doOpen(const QString &fileName)
{
	if (curFileName_ == fileName)
		return false;

	std::string name = fileName.toStdString();
	std::string ext = FileUtils::extension(name);
	String::to_lowercase(ext);

	bool is_ply_mesh = false;
	if (ext == "ply") {
		is_ply_mesh = (MapIO::ply_file_num_facet(name) > 0);
	}

	Object* obj = nil;
	if ((ext == "ply" && is_ply_mesh) || ext == "obj" || ext == "eobj" || ext == "off" || ext == "stl" || ext == "ply2") {
		obj = MapIO::read(name);
	}
	else {
		obj = PointSetIO::read(name);
	}

	if (obj) {
		obj->set_name(fileName.toStdString());
		addObject(obj, true, true);

		//updateStatusBar();

		setCurrentFile(fileName);
		setWindowTitle(tr("%1[*] - %2").arg(strippedName(fileName)).arg(tr("MobilityRecon")));

		status_message("File loaded", 500);

		return true;
	}
	else {
		status_message("Open failed", 500);
		return false;
	}
}

void MainWindow::setCurrentFile(const QString &fileName)
{
	curFileName_ = fileName;
	curDataDirectory_ = fileName.left(fileName.lastIndexOf("/") + 1); // path includes "/"

	setWindowModified(false);

	QString shownName = "Untitled";
	if (!curFileName_.isEmpty()) {
		shownName = strippedName(curFileName_);
		recentFiles_.removeAll(curFileName_);
		recentFiles_.prepend(curFileName_);
		//updateRecentFileActions();
	}

	setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("AutoScanSimulator")));
}

QString MainWindow::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void MainWindow::out_message(const std::string& msg) {
	/*plainTextEditOutput->moveCursor(QTextCursor::End);
	plainTextEditOutput->insertPlainText(QString::fromStdString(msg));
	plainTextEditOutput->repaint();
	plainTextEditOutput->update();*/
}


void MainWindow::warn_message(const std::string& msg) {
	/*plainTextEditOutput->moveCursor(QTextCursor::End);
	plainTextEditOutput->insertPlainText(QString::fromStdString(msg));
	plainTextEditOutput->repaint();
	plainTextEditOutput->update();*/
}


void MainWindow::err_message(const std::string& msg) {
	/*plainTextEditOutput->moveCursor(QTextCursor::End);
	plainTextEditOutput->insertPlainText(QString::fromStdString(msg));
	plainTextEditOutput->repaint();
	plainTextEditOutput->update();*/
}


void MainWindow::status_message(const std::string& msg, int timeout) {
	statusBar()->showMessage(QString::fromStdString(msg), timeout);
}


void MainWindow::notify_progress(int value) {
	/*progress_bar_->setValue(value);
	progress_bar_->setTextVisible(value != 0);
	mainCanvas_->immediate_update();*/
}

void MainWindow::hideOtherObjects(Object* cur) {
	const std::vector<Object*>& objects = canvas()->objectsManager()->objects();
	for (int i = 0; i < objects.size(); ++i) {
		Object* obj = objects[i];
		if (selected_only_ && obj != cur)
			canvas()->objectsManager()->set_visible(obj, false);
		else
			canvas()->objectsManager()->set_visible(obj, true);
	}
}


void MainWindow::showAllObjects() {
	const std::vector<Object*>& objects = canvas()->objectsManager()->objects();
	for (int i = 0; i < objects.size(); ++i) {
		Object* obj = objects[i];
		canvas()->objectsManager()->set_visible(obj, true);
	}
}

//remove all objects
void MainWindow::removeAllObjects() {
	const std::vector<Object*>& objects = canvas()->objectsManager()->objects();
	for (int i = 0; i < objects.size(); ++i) {
		Object* obj = objects[i];
		canvas()->objectsManager()->delete_object(obj, false);  // after this, there will be an active one, or no object exists 
	}
}

void MainWindow::addObject(Object* obj, bool activate, bool fit) {
	if (obj) {
		obj->set_canvas(canvas());
		canvas()->objectsManager()->add_object(obj, activate);

		if (selected_only_)
			hideOtherObjects(obj);
		else
			showAllObjects();

		if (fit)
			canvas()->fitScreen();
		else
			canvas()->updateGL();

		//updateObjectPanel();
		//updateRenderingPanel();
		//updateStatusBar();
	}
}

void MainWindow::deleteObject(Object* obj, bool activate_another, bool fit) {
	canvas()->objectsManager()->delete_object(obj, activate_another);  // after this, there will be an active one, or no object exists 

	Object* act = canvas()->objectsManager()->active_object();
	if (selected_only_)
		hideOtherObjects(act);
	else
		showAllObjects();

	if (fit)
		canvas()->fitScreen();
	else
		canvas()->updateGL();

	//updateObjectPanel();
	//updateRenderingPanel();
	//updateStatusBar();
}


// the active object
void MainWindow::activateObject(const Object* obj, bool fit) {
	canvas()->objectsManager()->set_active(obj);
	if (fit)
		canvas()->fitScreen();
	else
		canvas()->updateGL();

	//updateObjectPanel();
	//updateRenderingPanel();
	//updateStatusBar();
}

// change depth frame
void MainWindow::ChangeFrame(int index){
	removeAllObjects();
	doOpen(allFileNames[index]);
}

//save snap shot
void MainWindow::snapshot(){
	canvas()->snapshotScreen();
}

//export sequential snapshots
void MainWindow::export_sequential_snapshots(){
	seqSlider->setVisible(false);

	QString path = QFileDialog::getExistingDirectory(this,
		tr("Export Sequential Snapshots"),
		curDataDirectory_);

	if(path.isEmpty()){
		return;
	}

	for (int i = 0; i < allFileNames.size(); i++){
		removeAllObjects();
		doOpen(allFileNames[i]);
		canvas()->snapshotScreen(path + "/" + QString::number(i, 10) + ".jpg");
	}
}