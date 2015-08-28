//HaoLi:Thread for kinect scanning

#include <fstream>
#include "scan_thread.h"
#include "../../math/vecg.h"

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

SaveRGBThread::SaveRGBThread(){
	
}

SaveRGBThread::SaveRGBThread(uchar *rgb_data, int rgb_width, int rgb_height, std::string filename){
	this->rgb_data = rgb_data;
	this->rgb_width = rgb_width;
	this->rgb_height = rgb_height;
	this->filename = filename;
}

SaveRGBThread::~SaveRGBThread(){

}

void SaveRGBThread::run(){
	std::ofstream ofs(filename, std::ios::binary);
	ofs << rgb_width << "\n";
	ofs << rgb_height << "\n";

	vecng<3, uchar> *rgb = new vecng<3, uchar>[rgb_width*rgb_height];

	for (int i = 0; i < rgb_width*rgb_height; i++){
		rgb[i].x = rgb_data[4 * i];
		rgb[i].y = rgb_data[4 * i + 1];
		rgb[i].z = rgb_data[4 * i + 2];
	}

	ofs.write((char*)rgb, rgb_width*rgb_height*sizeof(vecng<3, uchar>));

	delete[]rgb;
	delete[]rgb_data;
	ofs.close();

	emit releaseSelf(this);
}

SaveDepthThread::SaveDepthThread(){

}

SaveDepthThread::SaveDepthThread(ushort *depth_data, int depth_width, int depth_height, std::string filename){
	this->depth_data = depth_data;
	this->depth_width = depth_width;
	this->depth_height = depth_height;
	this->filename = filename;
}

SaveDepthThread::~SaveDepthThread(){

}

void SaveDepthThread::run(){
	std::ofstream ofs(filename, std::ios::binary);
	ofs << depth_width << "\n";
	ofs << depth_height << "\n";

	ofs.write((char*)depth_data, depth_width*depth_height*sizeof(ushort));

	delete []depth_data;
	ofs.close();

	emit releaseSelf(this);
}