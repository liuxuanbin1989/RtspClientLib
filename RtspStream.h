/********************************************************************************
   * COPYRIGHT   : Copyright(C) 2014-2016  T&W  Co., Ltd. All Rights Reserved.
   * FILE NAME   : RtspStream.h
   * DESCRIPTION : standard definition
   * AUTHOR      : liuxuanbin
   * VERSION     : 1.0
   * DATE        : 2015/03/05 
   * FIXED RECORD : 
   * DATE   VERSION     REVISIOR          ABSTRACT

   * NOTICE :
********************************************************************************/
// RtspStream.h : main header file for the RTSPSTREAM DLL
//

#if !defined(AFX_RTSPSTREAM_H__B0DE1A63_56F5_4F84_A66D_C2AD4FE635F1__INCLUDED_)
#define AFX_RTSPSTREAM_H__B0DE1A63_56F5_4F84_A66D_C2AD4FE635F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRtspStreamApp
// See RtspStream.cpp for the implementation of this class
//

#include "StreamItem.h"


class CRtspStreamApp : public CWinApp
{
public:
	CRtspStreamApp();
	int									   g_WatchVariable;
	TaskScheduler*			  g_scheduler;
	UsageEnvironment*	 g_env;
	HANDLE							g_eventLoopMutex;
	HANDLE							g_LoopThread;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRtspStreamApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CRtspStreamApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};






/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RTSPSTREAM_H__B0DE1A63_56F5_4F84_A66D_C2AD4FE635F1__INCLUDED_)
