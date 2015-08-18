//HaoLi

#ifndef _SCAN_THREAD_H
#define _SCAN_THREAD_H

#include "QThread"
#include "QReadWriteLock"

class MainWindow;

class ScanThread : public QThread
{
	Q_OBJECT
public:
	ScanThread();
	ScanThread(MainWindow* main_window);
	~ScanThread();

	void stopScan();

signals:
	void doScanSig(int);


protected:
	void run();

private:
	MainWindow* main_window;
	bool isStop;
};

#endif