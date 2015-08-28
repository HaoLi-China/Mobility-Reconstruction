//HaoLi
#ifndef DEPTH_BASIC_H
#define DEPTH_BASIC_H

#include <windows.h>

// Kinect Header files
#include <Kinect.h>

#include "../math/vecg.h"
#include "kinect_io_common.h"

class PointSet;

class KINECT_IO_API CDepthBasics
{
public:
	CDepthBasics();
	~CDepthBasics();

	void openScanner();
	void closeScanner();

	//get points of one frame
	bool GetPointsOfOneFrame(PointSet* pointSet);

	//get depth image, rgb image and point cloud of one frame
	bool GetDataOfOneFrame(PointSet* pointSet, UINT16* depth_data, unsigned char *rgb);

	int getDepthWidth();
	int getDepthHeight();
	int getRGBWidth();
	int getRGBHeight();

private:
	//Depth image resolution
	int depth_width;
	int depth_height;

	//RGB image resolution
	int rgb_width;
	int rgb_height;

	USHORT nDepthMinReliableDistance;
	USHORT nDepthMaxDistance;

	// Current Kinect
	IKinectSensor*          m_pKinectSensor;

	// Depth reader
	IDepthFrameReader*      m_pDepthFrameReader;

	// color reader
	IColorFrameReader*      m_pColorFrameReader;

	ICoordinateMapper*      pCoordinateMapper;

	RGBQUAD* m_pColorRGBX;

	/// <summary>
	/// Initializes the default Kinect sensor
	/// </summary>
	/// <returns>S_OK on success, otherwise failure code</returns>
	HRESULT                 InitializeDefaultSensor();

	// Safe release for interfaces
	template<class Interface>
	void SafeRelease(Interface *& pInterfaceToRelease)
	{
		if (pInterfaceToRelease != NULL)
		{
			pInterfaceToRelease->Release();
			pInterfaceToRelease = NULL;
		}
	}
};

#endif