/********************************************************************************
   * COPYRIGHT   : Copyright(C) 2014-2016  T&W  Co., Ltd. All Rights Reserved.
   * FILE NAME   : RtspStream.cpp
   * DESCRIPTION : RTSP网络库接口导出模块
   * AUTHOR      : liuxuanbin
   * VERSION     : 1.0
   * DATE        : 2015/03/05 
   * FIXED RECORD : 
   * DATE   VERSION     REVISIOR          ABSTRACT

   * NOTICE :
********************************************************************************/
// RtspStream.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "RtspStream.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CRtspStreamApp

BEGIN_MESSAGE_MAP(CRtspStreamApp, CWinApp)
	//{{AFX_MSG_MAP(CRtspStreamApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRtspStreamApp construction

CRtspStreamApp::CRtspStreamApp()
{
	// TODO: add construction code here,
	g_WatchVariable = 1;
	g_eventLoopMutex = CreateMutex(NULL, FALSE, NULL);
	g_LoopThread = NULL;
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CRtspStreamApp object

CRtspStreamApp theApp;

CStreamItem* g_StreamItem[MAX_STREAM_ITEM_COUNT];

/**********************************************************************
  * 函数名称    : StartRtspEventLoop
  * 功能描述    : 消息处理线程
  * 输入参数    : pUser 
  * 输出参数    : 无
  * 返 回 值      : 0   
  *						 
  * 其它说明    : 无
  * 修改日期    版本号     修改人      修改内容
  * -----------------------------------------------
  * 14/11/21     V1.0      liuxuanbin      XXXX
  **********************************************************************/
static DWORD WINAPI StartRtspEventLoop(LPVOID pUser)
{
	theApp.g_scheduler=BasicTaskScheduler::createNew();
	theApp.g_env = BasicUsageEnvironment::createNew(*theApp.g_scheduler);

	while (theApp.g_WatchVariable)
	{
        //theApp.g_env->taskScheduler().doEventLoop(&eventLoopWatchVariable);  
        WaitForSingleObject(theApp.g_eventLoopMutex,INFINITE);  
        theApp.g_env->taskScheduler().doEventLoop();  
        ReleaseMutex(theApp.g_eventLoopMutex);  
    }  

	return 0;
}

HANDLE g_hMutex = NULL;

/**********************************************************************
  * 函数名称    : RTSP_STREAM_Init
  * 功能描述    : 初始化RTSP网络
  * 输入参数    : 无   
  * 输出参数    : 无
  * 返 回 值      : 
  * 其它说明    : 无
  * 修改日期    版本号     修改人      修改内容
  * -----------------------------------------------
  * 14/11/21     V1.0      liuxuanbin      XXXX
  **********************************************************************/
RTSP_STREAM_API int RTSP_STREAM_Init()
{
	g_hMutex=CreateMutex(NULL,FALSE,NULL);
	for (int i=0;i<MAX_STREAM_ITEM_COUNT;i++)
	{
		g_StreamItem[i]=NULL;
	}

	//  rtsp do event loop

	theApp.g_LoopThread = CreateThread(NULL, 0, StartRtspEventLoop, NULL, 0, NULL);

	return 0;
}

/**********************************************************************
  * 函数名称    : RTSP_STREAM_Fini
  * 功能描述    : 反初始化RTSP网络
  * 输入参数    : 无   
  * 输出参数    : 无
  * 返 回 值      : 
  * 其它说明    : 无
  * 修改日期    版本号     修改人      修改内容
  * -----------------------------------------------
  * 14/11/21     V1.0      liuxuanbin      XXXX
  **********************************************************************/
RTSP_STREAM_API int RTSP_STREAM_Fini()
{
	theApp.g_WatchVariable = 0;  // quit the event loop
	if(g_hMutex)
	{
		CloseHandle(g_hMutex);
		g_hMutex=NULL;
	}
	
	if (theApp.g_eventLoopMutex)
	{
		WaitForSingleObject(theApp.g_eventLoopMutex, INFINITE);
		CloseHandle(theApp.g_eventLoopMutex);
		theApp.g_eventLoopMutex=NULL;
	}
	

	if(theApp.g_LoopThread)
	{
		CloseHandle(theApp.g_LoopThread);
		theApp.g_LoopThread = NULL;
	}

	return 0;
}

/**********************************************************************
  * 函数名称    : RTSP_STREAM_Open
  * 功能描述    : 打开音视频通道
  * 输入参数    : sRtspUrl rtsp服务器地址
							pStreamCB 数据回调
							lStreamHandle 会话句柄
							pUserData 用户数据指针
  * 输出参数    : 无
  * 返 回 值    : 0   表示成功
  *						-1  表示失败
  * 其它说明    : 无
  * 修改日期    版本号     修改人      修改内容
  * -----------------------------------------------
  * 14/11/21     V1.0      liuxuanbin      XXXX
  **********************************************************************/
RTSP_STREAM_API int RTSP_STREAM_Open(char* sRtspUrl,LPSreamDataCallBack pStreamCB, long &lStreamHandle, void* pUserData)
{
	WaitForSingleObject(g_hMutex,INFINITE);
	for (int i=0;i<MAX_STREAM_ITEM_COUNT;i++)
	{
		if (!g_StreamItem[i])
		{
			g_StreamItem[i]=new CStreamItem();
			if(!g_StreamItem[i])
				return -1;

			
			int errorNo = g_StreamItem[i]->Open(sRtspUrl,pStreamCB,pUserData);		
			if (errorNo != 0)
			{
				g_StreamItem[i]->Close();
				MEMORY_DELETE( g_StreamItem[i]);

				ReleaseMutex(g_hMutex);
				return -1;
			}

			lStreamHandle = (long)g_StreamItem[i];
			ReleaseMutex(g_hMutex);
			return 0;						
		}
	}
	ReleaseMutex(g_hMutex);
	return -1;
}

/**********************************************************************
  * 函数名称    : RTSP_STREAM_Close
  * 功能描述    : 关闭音视频通道
  * 输入参数    : lStreamHandle  会话句柄   
  * 输出参数    : 无
  * 返 回 值      : 0   表示成功
  *						  -1  表示失败
  * 其它说明    : 无
  * 修改日期    版本号     修改人      修改内容
  * -----------------------------------------------
  * 14/11/21     V1.0      liuxuanbin      XXXX
  **********************************************************************/
RTSP_STREAM_API int RTSP_STREAM_Close(long lStreamHandle)
{
	WaitForSingleObject(g_hMutex,INFINITE);

	for (int i=0;i<MAX_STREAM_ITEM_COUNT;i++)
	{
		if(g_StreamItem[i] == (CStreamItem*)lStreamHandle)
		{
			if (NULL == g_StreamItem[i] && !IsBadWritePtr(g_StreamItem[i], sizeof(CStreamItem)))
			{
				ReleaseMutex(g_hMutex);
				return -1;
			}
			 g_StreamItem[i]->Close();
			MEMORY_DELETE( g_StreamItem[i]);
		}		
		
	ReleaseMutex(g_hMutex);
	}
	
	return 0;
}

