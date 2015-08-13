#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_main_window.h"
#include "../../math/math_types.h"
#include "../../basic/logger.h"
#include "../../basic/progress.h"


class QLabel;
class PaintCanvas;
class QProgressBar;
class Map;
class Object;
class PointSet;
class QSplitter;

class MainWindow
	: public QMainWindow
	, public LoggerClient
	, public ProgressClient
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

	PaintCanvas* canvas() { return mainCanvas_; }
	//////////////////////////////////////////////////////////////

	virtual void out_message(const std::string& msg);
	virtual void warn_message(const std::string& msg);
	virtual void err_message(const std::string& msg);
	virtual void status_message(const std::string& msg, int timeout);
	virtual void notify_progress(int value);

	//////////////////////////////////////////////////////////////

	void addObject(Object* obj, bool activate, bool fit);
	void deleteObject(Object* obj, bool activate_another, bool fit);
	void activateObject(const Object* obj, bool fit);

	//////////////////////////////////////////////////////////////

	//void showCoordinateUnderMouse(const vec3& p, bool found);
	//void updateStatusBar();

	public Q_SLOTS:
	bool open();
	bool import();
	//bool save();

	void ChangeFrame(int index);


private:
	void createMenus();
	void createActions();
	void createActionsForFileMenu();

	bool doOpen(const QString &fileName);

	void setCurrentFile(const QString &fileName);

	QString strippedName(const QString &fullFileName);

	void hideOtherObjects(Object* obj);
	void showAllObjects();
	void removeAllObjects();

private:
	Ui::MainWindowClass ui;

	QSplitter* mainSplitter, *leftSplitter, *rightSplitter;
	QSlider* seqSlider;

	PaintCanvas* mainCanvas_;
	PaintCanvas* secondCanvas_;
	PaintCanvas* thirdCanvas_;

	QStringList		recentFiles_;
	QString			curFileName_;
	QString			curDataDirectory_;
	QString			curCameraConfigFileDirectory_;

	QProgressBar*	progress_bar_;

	QLabel *statusLabel_,
		*coordinateUnderMouseLabel_,
		*numCellsLabel_,
		*numVerticesLabel_,
		*numEdgesLabel_;


	enum { MaxRecentFiles = 5 };
	QAction *actionsRecentFile[MaxRecentFiles],
		*actionSeparator;

	bool	auto_focus_;
	bool    selected_only_;
	bool    highlighting_;

	QStringList allFileNames;
};

#endif // MAIN_WINDOW_H
