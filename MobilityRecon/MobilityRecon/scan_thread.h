//HaoLi

#ifndef _SCAN_THREAD_H
#define _SCAN_THREAD_H

#include "QThread"
#include "QReadWriteLock"

//class MainWindow;

class ScanThread : public QThread
{
	Q_OBJECT
public:
	ScanThread();
	//ScanThread(MainWindow* main_window);
	~ScanThread();

	void stopScan();

signals:
	void doScanSig();


protected:
	void run();

private:
	//MainWindow* main_window;
	bool isStop;
};

class SaveRGBThread : public QThread
{
	Q_OBJECT
public:
	SaveRGBThread(uchar *rgb_data, int rgb_width, int rgb_height, std::string filename);
	SaveRGBThread();
	~SaveRGBThread();
signals:
	void releaseSelf(SaveRGBThread *srgbt);

protected:
	void run();

private:
	uchar *rgb_data;
	int rgb_width;
	int rgb_height;
	std::string filename;
};

class SaveDepthThread : public QThread
{
	Q_OBJECT
public:
	SaveDepthThread(ushort *depth_data, int depth_width, int depth_height, std::string filename);
	SaveDepthThread();
	~SaveDepthThread();
signals:
	void releaseSelf(SaveDepthThread *sdt);

protected:
	void run();

private:
	ushort *depth_data;
	int depth_width;
	int depth_height;
	std::string filename;
};

#endif