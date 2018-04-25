#if !defined(AFX_IRTSPSTREAM_H__7C529473_39BA_4256_9745_9C483E262BFA__INCLUDED_)
#define AFX_IRTSPSTREAM_H__7C529473_39BA_4256_9745_9C483E262BFA__INCLUDED_


#ifdef __cplusplus
extern "C" {  
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef	_RTSP_STREAM_EXPORTS
#define RTSP_STREAM_API __declspec(dllexport)
#else
#define RTSP_STREAM_API __declspec(dllimport)
#endif

//typedef int  (WINAPI *ChannelStreamCallback)(HANDLE hOpenChannel,void *pStreamData,DWORD dwClientID,void *pContext,ENCODE_VIDEO_TYPE encodeVideoType,HHAV_INFO *pAVInfo);

typedef int (CALLBACK *LPSreamDataCallBack)(unsigned char* lpBuf, int nBufSize, int nStreamFlag, AV_INFO *pAVInfo, LONGLONG llTimeStamp,void* pUser);


RTSP_STREAM_API int RTSP_STREAM_Init();

RTSP_STREAM_API int RTSP_STREAM_Fini();

/*
	return:  open handle   >=0 success ;  <0 failed
*/
RTSP_STREAM_API int RTSP_STREAM_Open(char* sRtspUrl,LPSreamDataCallBack pStreamCB, long &lStreamHandle, void* pUserData);

RTSP_STREAM_API int RTSP_STREAM_Close(long lStreamHandle);


#ifdef __cplusplus
}
#endif


#endif