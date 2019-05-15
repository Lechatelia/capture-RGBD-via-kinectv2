
#include <kinect.h>


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <opencv2\opencv.hpp>


#include <opencv2/core/core.hpp>  


#include <opencv2/highgui/highgui.hpp>  

#include "pthread.h"


using namespace cv;
using namespace std;
//#define DEPTH_8
BOOL Save = FALSE;
string s1 = "F:\\dataset\\sample-";
string s2 = "-rgb.png";
string s3 = "-depth.png";
string s4 = "-infrared.png";
// ��ȫ�ͷ�ָ��
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

void  * GetPicture(void * param)
{
	// ��ȡKinect�豸
	IKinectSensor* m_pKinectSensor;
	 
	HRESULT  hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return &hr;
	}

	IMultiSourceFrameReader* m_pMultiFrameReader= nullptr;
	if (m_pKinectSensor)
	{
		hr = m_pKinectSensor->Open();
		if (SUCCEEDED(hr))
		{
			// ��ȡ������Դ����ȡ��  
			hr = m_pKinectSensor->OpenMultiSourceFrameReader(
				FrameSourceTypes::FrameSourceTypes_Color |
				FrameSourceTypes::FrameSourceTypes_Infrared |
				FrameSourceTypes::FrameSourceTypes_Depth,
				&m_pMultiFrameReader);
		}
	}

	if (!m_pKinectSensor || FAILED(hr))
	{ 
		hr = E_FAIL;
		return &hr;
	}
	// ��������֡������
	IDepthFrameReference* m_pDepthFrameReference = nullptr;
	IColorFrameReference* m_pColorFrameReference = nullptr;
	IInfraredFrameReference* m_pInfraredFrameReference = nullptr;
	IInfraredFrame* m_pInfraredFrame = nullptr;
	IDepthFrame* m_pDepthFrame = nullptr;
	IColorFrame* m_pColorFrame = nullptr;
	// ����ͼƬ��ʽ
	Mat i_rgb(1080, 1920, CV_8UC4);      //ע�⣺�������Ϊ4ͨ����ͼ��Kinect������ֻ����Bgra��ʽ����
#ifdef DEPTH_8
	Mat i_depth(424, 512, CV_8UC1);
#else
	Mat i_depth(424, 512, CV_16UC1);
#endif // DEPTH_8

	
	Mat i_ir(424, 512, CV_16UC1);

	UINT16 *depthData = new UINT16[424 * 512];
	IMultiSourceFrame* m_pMultiFrame = nullptr;
	int sample_id = 1;
	time_t now;
	struct tm tm_now;
	localtime_s(&tm_now, &now);
	struct tm tm_last;
	localtime_s(&tm_last, &now);
	tm_last.tm_sec -= 1;
	int Counter = 0;
	while (true)
	{
		// ��ȡ�µ�һ����Դ����֡
		
		time(&now);
		localtime_s(&tm_now, &now);

		hr = m_pMultiFrameReader->AcquireLatestFrame(&m_pMultiFrame);
		if (FAILED(hr) || !m_pMultiFrame)
		{
			//cout << "!!!" << endl;
			continue;
		}

		// �Ӷ�Դ����֡�з������ɫ���ݣ�������ݺͺ�������
		if (SUCCEEDED(hr))
			hr = m_pMultiFrame->get_ColorFrameReference(&m_pColorFrameReference);
		if (SUCCEEDED(hr))
			hr = m_pColorFrameReference->AcquireFrame(&m_pColorFrame);
		if (SUCCEEDED(hr))
			hr = m_pMultiFrame->get_DepthFrameReference(&m_pDepthFrameReference);
		if (SUCCEEDED(hr))
			hr = m_pDepthFrameReference->AcquireFrame(&m_pDepthFrame);
		if (SUCCEEDED(hr))
			hr = m_pMultiFrame->get_InfraredFrameReference(&m_pInfraredFrameReference);
		if (SUCCEEDED(hr))
			hr = m_pInfraredFrameReference->AcquireFrame(&m_pInfraredFrame);

		// color������ͼƬ��
		UINT nColorBufferSize = 1920 * 1080 * 4;
		if (SUCCEEDED(hr))
			hr = m_pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(i_rgb.data), ColorImageFormat::ColorImageFormat_Bgra);

		// depth������ͼƬ��
		if (SUCCEEDED(hr))
		{
#ifdef DEPTH_8
			hr = m_pDepthFrame->CopyFrameDataToArray(424 * 512, depthData);
			for (int i = 0; i < 512 * 424; i++)
			{
				// 0-255���ͼ��Ϊ����ʾ���ԣ�ֻȡ������ݵĵ�8λ
				BYTE intensity = static_cast<BYTE>(depthData[i] % 256);
				reinterpret_cast<BYTE*>(i_depth.data)[i] = intensity;
			}
#else
			// ʵ����16λunsigned int����
			hr = m_pDepthFrame->CopyFrameDataToArray(424 * 512, reinterpret_cast<UINT16*>(i_depth.data));
#endif // DEPTH_8

			

			
		}
			

		// infrared������ͼƬ��
		if (SUCCEEDED(hr))
		{
			hr = m_pInfraredFrame->CopyFrameDataToArray(424 * 512, reinterpret_cast<UINT16*>(i_ir.data));
		}

		// ��ʾ
		imshow("rgb", i_rgb);
		if (waitKey(1) == VK_ESCAPE)
			break;
		imshow("depth", i_depth);
		if (waitKey(1) == VK_ESCAPE)
			break;
		imshow("ir", i_ir);
		if (waitKey(1) == VK_ESCAPE)
			break;
		//waitKey(0);
		if (Save)
		{
			if (tm_now.tm_sec == tm_last.tm_sec)
			{
				Counter += 1;
			}
			else
			{
				tm_last = tm_now;
				Counter = 0;
			}
			/*cvSaveImage((s1 + to_string(sample_id) + s2).c_str(), &IplImage(i_rgb));
			cvSaveImage((s1 + to_string(sample_id) + s3).c_str(), &IplImage(i_depth));
			cvSaveImage((s1 + to_string(sample_id) + s4).c_str(), &IplImage(i_ir));
			sample_id += 1;*/
			char time[64] = { 0 };
			sprintf_s(time, "%02d%02d_%02d%02d%02d_%d",tm_now.tm_mon + 1,tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec,Counter);
			string Time(time);
			cvSaveImage((s1 + Time + s2).c_str(), &IplImage(i_rgb));
			cvSaveImage((s1 + Time + s3).c_str(), &IplImage(i_depth));
			cvSaveImage((s1 + Time + s4).c_str(), &IplImage(i_ir));
		}


		// �ͷ���Դ
		SafeRelease(m_pColorFrame);
		SafeRelease(m_pDepthFrame);
		SafeRelease(m_pInfraredFrame);
		SafeRelease(m_pColorFrameReference);
		SafeRelease(m_pDepthFrameReference);
		SafeRelease(m_pInfraredFrameReference);
		SafeRelease(m_pMultiFrame);
	}
	// �رմ��ڣ��豸
	cv::destroyAllWindows();
	m_pKinectSensor->Close();
}
int main()
{
	//GetPicture();
	pthread_t getpicture;
	pthread_create(&getpicture, NULL, GetPicture, NULL);
	while (1)
	{
		char cmd[25];
		std::cout << ("\n===========================\n");
		std::cout << ("s: Start\n");
		std::cout << ("q: Stop\n");
		std::cout << ("\n===========================\n");
		gets_s(cmd);
		if (strstr(cmd, "s") != NULL)
		{
			Save = TRUE;
			cout << "start save the images" << endl;
		}
		if (strstr(cmd, "q") != NULL)
		{
			Save = FALSE;
			cout << "stop savr the images" << endl;
		}

	}
	std::system("pause");
	return 0;
}