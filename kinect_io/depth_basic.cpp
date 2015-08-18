//HaoLi:Get depth data from Kinect

#include <strsafe.h>
#include "depth_basic.h"
#include "../geom/point_set.h"

/// <summary>
/// Constructor
/// </summary>
CDepthBasics::CDepthBasics() :
    m_nStartTime(0),
    m_nLastCounter(0),
    m_nFramesSinceUpdate(0),
    m_fFreq(0),
    m_nNextStatusTime(0LL),
    m_bSaveScreenshot(false),
    m_pKinectSensor(NULL),
	m_pDepthFrameReader(NULL)
{
    LARGE_INTEGER qpf = {0};
    if (QueryPerformanceFrequency(&qpf))
    {
        m_fFreq = double(qpf.QuadPart);
    }

	InitializeDefaultSensor();
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
/// Handle new depth data
/// <param name="nTime">timestamp of frame</param>
/// <param name="pBuffer">pointer to frame data</param>
/// <param name="nWidth">width (in pixels) of input image data</param>
/// <param name="nHeight">height (in pixels) of input image data</param>
/// <param name="nMinDepth">minimum reliable depth</param>
/// <param name="nMaxDepth">maximum reliable depth</param>
/// </summary>
void CDepthBasics::ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nWidth, int nHeight, USHORT nMinDepth, USHORT nMaxDepth, PointSet* pointSet)
{
	if (!m_nStartTime)
	{
		m_nStartTime = nTime;
	}

	double fps = 0.0;

	LARGE_INTEGER qpcNow = { 0 };
	if (m_fFreq)
	{
		if (QueryPerformanceCounter(&qpcNow))
		{
			if (m_nLastCounter)
			{
				m_nFramesSinceUpdate++;
				fps = m_fFreq * m_nFramesSinceUpdate / double(qpcNow.QuadPart - m_nLastCounter);
			}
		}
	}

	//WCHAR szStatusMessage[64];
	//StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" FPS = %0.2f    Time = %I64d", fps, (nTime - m_nStartTime));

	//if (SetStatusMessage(szStatusMessage, 1000, false))
	//{
	//	m_nLastCounter = qpcNow.QuadPart;
	//	m_nFramesSinceUpdate = 0;
	//}

	// Make sure we've received valid data
	if (pBuffer)
	{
		CameraSpacePoint* csp = new CameraSpacePoint[nWidth * nHeight];
		HRESULT hr = pCoordinateMapper->MapDepthFrameToCameraSpace(nWidth * nHeight, pBuffer, nWidth * nHeight, csp);

		//printf("nMinDepth:%d\n", nMinDepth);
		//printf("nMaxDepth:%d\n", nMaxDepth);

		if (SUCCEEDED(hr)){
			// end pixel is start + width*height - 1
			//const CameraSpacePoint* cspEnd = csp + (nWidth * nHeight);

			//while (csp < cspEnd)
			for (int i = 0; i < nWidth * nHeight; i++)
			{
				//USHORT depth = static_cast<USHORT> (csp[i].Z);
				
				if (csp[i].Z >= (nMinDepth / 1000.0) && csp[i].Z <= (nMaxDepth / 1000.0)){
					pointSet->new_vertex(vec3(-csp[i].X, -csp[i].Y, csp[i].Z));
				}
			}
		}
		delete[] csp;
	}
}

/// <summary>
/// Main processing function
/// </summary>
void CDepthBasics::GetPointsOfOneFrame(PointSet* pointSet)
{
	if (!m_pDepthFrameReader)
	{
		return;
	}

	IDepthFrame* pDepthFrame = NULL;

	HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		USHORT nDepthMinReliableDistance = 0;
		USHORT nDepthMaxDistance = 0;
		UINT nBufferSize = 0;
		UINT16 *pBuffer = NULL;

		hr = pDepthFrame->get_RelativeTime(&nTime);

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
		}

		if (SUCCEEDED(hr))
		{
			// In order to see the full range of depth (including the less reliable far field depth)
			// we are setting nDepthMaxDistance to the extreme potential depth threshold
			nDepthMaxDistance = USHRT_MAX;

			// Note:  If you wish to filter by reliable depth distance, uncomment the following line.
			//// hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxDistance);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
		}

		if (SUCCEEDED(hr))
		{
			ProcessDepth(nTime, pBuffer, nWidth, nHeight, nDepthMinReliableDistance, nDepthMaxDistance, pointSet);
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pDepthFrame);
}
