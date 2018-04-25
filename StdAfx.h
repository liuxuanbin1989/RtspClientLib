// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__8990DB2F_254B_4619_880A_CB16114CA0CA__INCLUDED_)
#define AFX_STDAFX_H__8990DB2F_254B_4619_880A_CB16114CA0CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#define  _RTSP_STREAM_EXPORTS
#include "AVDefine.h"
#include "IRtspStream.h"

#ifdef _DEBUG
#pragma comment(lib,"groupsock_d.lib")
#pragma comment(lib,"BasicUsageEnvironment_d.lib")
#pragma comment(lib,"UsageEnvironment_d.lib")
#pragma comment(lib,"liveMedia_d.lib")
#else
#pragma comment(lib,"groupsock.lib")
#pragma comment(lib,"BasicUsageEnvironment.lib")
#pragma comment(lib,"UsageEnvironment.lib")
#pragma comment(lib,"liveMedia.lib")
#endif


#define  MAX_STREAM_ITEM_COUNT 128

#define MEMORY_DELETE(x)  { try{ if(x) delete x; x=NULL; }catch(...) { }  }             //delete 内存
#define MEMORY_DELETE_EX(x)	 { try{ if(x) delete [] x; x=NULL; }catch(...) { } }        //delete 内存数组

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8990DB2F_254B_4619_880A_CB16114CA0CA__INCLUDED_)
