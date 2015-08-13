#include "main_window.h"
#include <QtWidgets/QApplication>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);


	// splash
	QPixmap pixmap("./Resources/splash.png");
	QSplashScreen splash(pixmap);
	splash.show();
	splash.showMessage("Starting...");
	app.processEvents();


	MainWindow window;
	window.show();

	splash.finish(&window);

	return app.exec();
}
