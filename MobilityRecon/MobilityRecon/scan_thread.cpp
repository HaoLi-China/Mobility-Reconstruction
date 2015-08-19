//HaoLi:Thread for kinect scanning

#include "scan_thread.h"
#include "main_window.h"

ScanThread::ScanThread(){
	//this->main_window = nil;
	isStop = false;
}

//ScanThread::ScanThread(MainWindow* main_window){
//	this->main_window = main_window;
//	isStop = false;
//}

ScanThread::~ScanThread(){

}

void ScanThread::stopScan(){
	isStop = true;
}

void ScanThread::run(){

	/*if (!main_window){
		return;
	}*/

	isStop = false;

	while (!isStop){
		emit doScanSig();
		msleep(20);
	}
}