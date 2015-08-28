//HaoLi:Get depth data from Kinect

#include <strsafe.h>
#include "depth_basic.h"
#include "../geom/point_set.h"

/// <summary>
/// Constructor
/// </summary>
CDepthBasics::CDepthBasics() :
depth_width(512),
depth_height(424),
rgb_width(1920),
rgb_height(1080),
nDepthMinReliableDistance(500),
nDepthMaxDistance(USHRT_MAX),
m_pKinectSensor(NULL),
m_pDepthFrameReader(NULL)
{
	// create heap storage for color pixel data in RGBX format
	m_pColorRGBX = new RGBQUAD[rgb_width * rgb_height];
}


/// <summary>
/// Destructor
/// </summary>
CDepthBasics::~CDepthBasics()
{
	//release m_pDepthFrameReader
	SafeRelease(m_pDepthFrameReader);

	// close the Kinect Sensor
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}

	//release m_pKinectSensor
	SafeRelease(m_pKinectSensor);

	if (m_pColorRGBX)
	{
		delete[] m_pColorRGBX;
		m_pColorRGBX = NULL;
	}
}

void CDepthBasics::openScanner(){
	InitializeDefaultSensor();
}

void CDepthBasics::closeScanner(){
	//release m_pDepthFrameReader
	SafeRelease(m_pDepthFrameReader);

	// close the Kinect Sensor
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}

	//release m_pKinectSensor
	SafeRelease(m_pKinectSensor);
}

/// <summary>
/// Initializes the default Kinect sensor
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT CDepthBasics::InitializeDefaultSensor()
{
	HRESULT hr;
	HRESULT hr1;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return hr;
	}

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get the depth reader
		IDepthFrameSource* pDepthFrameSource = NULL;
		IColorFrameSource* pColorFrameSource = NULL;

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
			hr1 = m_pKinectSensor->get_CoordinateMapper(&pCoordinateMapper);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
		}

		if (SUCCEEDED(hr))
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);

		if (SUCCEEDED(hr))
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);

		SafeRelease(pColorFrameSource);
		SafeRelease(pDepthFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		//SetStatusMessage(L"No ready Kinect found!", 10000, true);
		return E_FAIL;
	}

	return hr;
}

/// <summary>
/// Main processing function
/// </summary>
bool CDepthBasics::GetPointsOfOneFrame(PointSet* pointSet)
{
	if (!m_pDepthFrameReader)
	{
		return false;
	}

	IDepthFrame* pDepthFrame = NULL;

	HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

	if (SUCCEEDED(hr))
	{
		UINT nBufferSize = 0;
		UINT16 *pBuffer = NULL;

		hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);

		if (SUCCEEDED(hr) && pBuffer)
		{
			CameraSpacePoint* csp = new CameraSpacePoint[depth_width * depth_height];
			HRESULT hr = pCoordinateMapper->MapDepthFrameToCameraSpace(depth_width * depth_height, pBuffer, depth_width * depth_height, csp);

			if (SUCCEEDED(hr)){
				for (int i = 0; i < depth_width * depth_height; i++)
				{
					if (csp[i].Z >= (nDepthMinReliableDistance / 1000.0) && csp[i].Z <= (nDepthMaxDistance / 1000.0)){
						pointSet->new_vertex(vec3(-csp[i].X, -csp[i].Y, csp[i].Z));
					}
				}
			}
			else{
				return false;
			}
			delete[] csp;
		}
		else{
			return false;
		}
	}
	else{
		return false;
	}

	SafeRelease(pDepthFrame);

	return true;
}

/// <summary>
/// Main processing function
/// </summary>
bool CDepthBasics::GetDataOfOneFrame(PointSet* pointSet, UINT16* depth_data, unsigned char *rgb)
{
	if (!m_pDepthFrameReader)
	{
		return false;
	}
	
	IColorFrame* pColorFrame = NULL;
	ColorImageFormat imageFormat = ColorImageFormat_None;
	UINT nBufferSize = 0;
	RGBQUAD *c_pBuffer = NULL;

	HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);
	
	if (SUCCEEDED(hr))
	{
		hr = pColorFrame->get_RawColorImageFormat(&imageFormat);

		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&c_pBuffer));
			}
			else if (m_pColorRGBX)
			{
				c_pBuffer = m_pColorRGBX;
				nBufferSize = rgb_width * rgb_height * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(c_pBuffer), ColorImageFormat_Bgra);
			}
			else
			{
				hr = E_FAIL;
			}
		}
		else{
			return false;
		}
		
		if (SUCCEEDED(hr) && c_pBuffer)
		{
			for (int i = 0; i < rgb_width * rgb_height; i++)
			{
				rgb[4 * i] = c_pBuffer[i].rgbRed;
				rgb[4 * i + 1] = c_pBuffer[i].rgbGreen;
				rgb[4 * i + 2] = c_pBuffer[i].rgbBlue;
				rgb[4 * i + 3] = 255;
			}
		}
	}
	else{
		return false;
	}
	
	SafeRelease(pColorFrame);

	IDepthFrame* pDepthFrame = NULL;

	hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);
	
	if (SUCCEEDED(hr))
	{
		UINT nBufferSize = 0;
		UINT16 *pBuffer = NULL;

		hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);

		if (SUCCEEDED(hr) && pBuffer)
		{
			for (int i = 0; i < depth_width * depth_height; i++)
			{
				depth_data[i] = pBuffer[i];
				//depth_data[i] = (pBuffer[i] >= nDepthMinReliableDistance) && (pBuffer[i] <= nDepthMaxDistance) ? pBuffer[i] : 0;
			}

			CameraSpacePoint* csp = new CameraSpacePoint[depth_width * depth_height];
			HRESULT hr = pCoordinateMapper->MapDepthFrameToCameraSpace(depth_width * depth_height, pBuffer, depth_width * depth_height, csp);

			if (SUCCEEDED(hr)){
				for (int i = 0; i < depth_width * depth_height; i++)
				{
					if (csp[i].Z >= (nDepthMinReliableDistance / 1000.0) && csp[i].Z <= (nDepthMaxDistance / 1000.0)){
						pointSet->new_vertex(vec3(-csp[i].X, -csp[i].Y, csp[i].Z));
					}
				}
			}
			else{
				return false;
			}
			delete[] csp;
		}
		else{
			return false;
		}
	}
	else{
		return false;
	}
	
	SafeRelease(pDepthFrame);
	return true;
}

int CDepthBasics::getDepthWidth(){
	return depth_width;
}

int CDepthBasics::getDepthHeight(){
	return depth_height;
}

int CDepthBasics::getRGBWidth(){
	return rgb_width;
}

int CDepthBasics::getRGBHeight(){
	return rgb_height;
}
