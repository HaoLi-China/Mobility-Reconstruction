#include <iostream>
#include <QFileDialog>
#include <QLabel>
#include <QSplitter>
#include <QTextEdit>
#include <time.h>

#include "main_window.h"
#include "paint_canvas.h"
#include "scan_thread.h"

#include "../../basic/file_utils.h"
#include "../../geom/map.h"
#include "../../geom/point_set.h"
#include "../../file_io/map_io.h"
#include "../../file_io/point_set_io.h"
#include "../../file_io/point_set_serializer_ply.h"
#include "../../kinect_io/depth_basic.h"
#include "../../algo/point_set_normal_estimation.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, curDataDirectory_(".")
	, auto_focus_(false)
	, selected_only_(false)
	, highlighting_(false)
	, is_save_when_scanning(false)
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

	depthbc = new CDepthBasics();
	scanthread = new ScanThread();

	/////////////////////////////////////////////////////////////////////
	//createMenus();
	createActions();
	connect(seqSlider, SIGNAL(valueChanged(int)), this, SLOT(ChangeFrame(int)));
	connect(scanthread, SIGNAL(doScanSig()), this, SLOT(doScan()));

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
	connect(ui.actionScanByKinect2, SIGNAL(triggered()), this, SLOT(scan_by_kinect2()));
	connect(ui.actionStopScan, SIGNAL(triggered()), this, SLOT(stopScan()));
	connect(ui.actionSaveWhenScanning, SIGNAL(toggled(bool)), this, SLOT(set_save_when_scan_flag(bool)));
	connect(ui.actionComputeNormalsForFrames, SIGNAL(triggered()), this, SLOT(computeNormalsForFrames()));

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

bool MainWindow::doOpen(const QString &fileName, bool fit)
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
		addObject(obj, true, fit);

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

//HaoLi:remove all objects
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

//HaoLi:change depth frame
void MainWindow::ChangeFrame(int index){
	removeAllObjects();
	doOpen(allFileNames[index], false);
}

//HaoLi:save snap shot
void MainWindow::snapshot(){
	canvas()->snapshotScreen();
}

//HaoLi:export sequential snapshots
void MainWindow::export_sequential_snapshots(){
	seqSlider->setVisible(false);

	QString path = QFileDialog::getExistingDirectory(this,
		tr("Export Sequential Snapshots"),
		curDataDirectory_);

	if (path.isEmpty()){
		return;
	}

	for (int i = 0; i < allFileNames.size(); i++){
		removeAllObjects();
		doOpen(allFileNames[i],false);
		canvas()->snapshotScreen(path + "/" + QString::number(i, 10) + ".jpg");
	}
}

//HaoLi:scan by kinect2
void MainWindow::scan_by_kinect2(){
	seqSlider->setVisible(false);
	allFileNames.clear();

	if (is_save_when_scanning){
		QDir dir1("scan/depth_image");
		QFileInfoList file_list1 = dir1.entryInfoList(QDir::Files);

		for (int i = 0; i < file_list1.size(); ++i) {
			QFileInfo fileInfo = file_list1.at(i);
			QFile::remove("scan/depth_image/" + fileInfo.fileName());
		}

		QDir dir2("scan/rgb_image");
		QFileInfoList file_list2 = dir2.entryInfoList(QDir::Files);

		for (int i = 0; i < file_list2.size(); ++i) {
			QFileInfo fileInfo = file_list2.at(i);
			QFile::remove("scan/rgb_image/" + fileInfo.fileName());
		}

		QDir dir3("scan/point_cloud");
		QFileInfoList file_list3 = dir3.entryInfoList(QDir::Files);

		for (int i = 0; i < file_list3.size(); ++i) {
			QFileInfo fileInfo = file_list3.at(i);
			QFile::remove("scan/point_cloud/" + fileInfo.fileName());
		}
	}

	qglviewer::Vec vmin(-2.5f, -2.5f, 0.5f);
	qglviewer::Vec vmax(2.5f, 2.5f, 5.5f);
	canvas()->setSceneBoundingBox(vmin, vmax);
	canvas()->showEntireScene();

	cdepthbasic()->openScanner();
	scanthread->start();
}

//HaoLi:scanning
void MainWindow::doScan(){
	PointSet* pointSet = new PointSet;
	ushort *depth_data = new ushort[512 * 424];
	vecng<3, uchar> *rgb_data = new vecng<3, uchar>[1920 * 1080];
	
	//cdepthbasic()->GetPointsOfOneFrame(pointSet);
	bool isSucceed = cdepthbasic()->GetDataOfOneFrame(pointSet, depth_data, rgb_data);
	
	//printf("points num:%d\n", pointSet->size_of_vertices());

	if (isSucceed){
		Object* obj = nil;

		if (pointSet->size_of_vertices() > 100){
			obj = pointSet;
		}

		if (obj) {
			removeAllObjects();

			if (is_save_when_scanning){
				clock_t nowtime = clock();

				std::stringstream stream1;
				stream1 << "scan/" << "point_cloud/" << nowtime << ".ply";
				std::string pcfilename = stream1.str();

				std::stringstream stream2;
				stream2 << "scan/" << "depth_image/" << nowtime << ".depth";
				std::string depthfilename = stream2.str();

				std::stringstream stream3;
				stream3 << "scan/" << "rgb_image/" << nowtime << ".rgb";
				std::string rgbfilename = stream3.str();

				//doSavePointCloud(obj, pcfilename);
				doSaveDepthImage(depth_data, 512 , 424, depthfilename);
				doSaveRGBImage(rgb_data, 1920, 1080, rgbfilename);
			}

			addObject(obj, true, false);

			status_message("scanning", 500);
		}
		else {
			status_message("Failed", 500);
		}
	}
	else {
		status_message("Failed", 500);
	}

	delete[]rgb_data;
	delete[]depth_data;
}

//HaoLi:stop scan
void MainWindow::stopScan(){
	scanthread->stopScan();
	cdepthbasic()->closeScanner();
	//if (is_save_when_scanning){
	//	computeNormalForEachFrame();
	//}
}

//HaoLi:set saving flag when scanning
void MainWindow::set_save_when_scan_flag(bool flag){
	is_save_when_scanning = flag;
}

//HaoLi:savePointCloud
bool MainWindow::doSavePointCloud(Object* obj, std::string filename){
	bool bo;
	if (dynamic_cast<Map*>(obj)) {
		Map* map = dynamic_cast<Map*>(obj);
		//......
	}
	else if (dynamic_cast<PointSet*>(obj)) {
		PointSet* pset = dynamic_cast<PointSet*>(obj);
		bo = PointSetSerializer_ply::save(filename, pset);
	}

	return bo;
}

//HaoLi:saveDepthImage
bool MainWindow::doSaveDepthImage(ushort *depth_data, int depth_width, int depth_height, std::string filename){
	std::ofstream ofs(filename, std::ios::binary);
	ofs << depth_width << "\n";
	ofs << depth_height << "\n";

	/*for (int i = 0; i < depth_width*depth_height; i++){
		ofs << depth_data[i] << "\n";
	}*/
	ofs.write((char*)depth_data, depth_width*depth_height*sizeof(ushort));

	ofs.close();

	return true;
}

//HaoLi:saveRGBImage
bool MainWindow::doSaveRGBImage(vecng<3, uchar> *rgb_data, int rgb_width, int rgb_height, std::string filename){
	std::ofstream ofs(filename, std::ios::binary);
	ofs << rgb_width << "\n";
	ofs << rgb_height << "\n";

	ofs.write((char*)rgb_data, rgb_width*rgb_height*sizeof(vecng<3, uchar>));
	
	/*for (int i = 0; i < rgb_width; i += 3){
		for (int j = 0; j < rgb_height; j += 3){
		ofs << rgb[i * 640 + j].x << " " << rgb[i * 640 + j].y << " " << rgb[i * 640 + j].z << "\n";
		}
		}*/

	ofs.close();

	return true;
}

//HaoLi:compute normal for each frame
void  MainWindow::computeNormalsForFrames(){
	QStringList fileNames = QFileDialog::getOpenFileNames(this,
		tr("Choose files to read point cloud"), curDataDirectory_,
		tr("Point set format (*.ply)\n")
		);

	if (fileNames.isEmpty())
		return;

	QString save_path = QFileDialog::getExistingDirectory(this,
		tr("Choose dir to save point cloud"),
		curDataDirectory_);

	if (save_path.isEmpty()){
		return;
	}

	for (int i = 0; i < fileNames.size(); ++i) {
		status_message("Processing data, Wating...", 500);
		PointSet* pset = PointSetIO::read((fileNames[i]).toStdString());
		PointSetNormalEstimation::apply(pset, false, 50);
		QStringList strlist = (fileNames[i]).split("/");
		doSavePointCloud(pset, (save_path + "/" + strlist[strlist.size() - 1]).toStdString());

		delete pset;
		pset = NULL;
	}
}