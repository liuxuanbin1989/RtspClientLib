/********************************************************************************
   * COPYRIGHT   : Copyright(C) 2014-2016  T&W  Co., Ltd. All Rights Reserved.
   * FILE NAME   : StreamItem.h
   * DESCRIPTION : 负责RTSP处理模块
   * AUTHOR      : liuxuanbin
   * VERSION     : 1.0
   * DATE        : 2015/03/05 
   * FIXED RECORD : 
   * DATE   VERSION     REVISIOR          ABSTRACT

   * NOTICE :
********************************************************************************/
// StreamItem.h: interface for the CStreamItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STREAMITEM_H__AE91F627_99F2_4F64_AC30_9EC18F6FC2CE__INCLUDED_)
#define AFX_STREAMITEM_H__AE91F627_99F2_4F64_AC30_9EC18F6FC2CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "RTSPCommon.hh"
// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class StreamClientState {
public:
	StreamClientState();
	virtual ~StreamClientState();
	
public:
	MediaSubsessionIterator* iter;
	MediaSession* session;
	MediaSubsession* subsession;
	TaskToken streamTimerTask;
	double duration;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient: public RTSPClient {
public:
	static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel = 0,
		char const* applicationName = NULL,
		portNumBits tunnelOverHTTPPortNum = 0);
	void SetErrorInfo(const int nError){ m_nErrCode = nError;}
protected:
	ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
	virtual ~ourRTSPClient();
	
public:
	StreamClientState scs;
	LPSreamDataCallBack m_lpStreamCallBack;
	void*               m_pUserData;
	int                    m_nErrCode;
	HANDLE           m_hWaitEvent;
	Authenticator* ourAuthenticator;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink: public MediaSink {
public:
	static DummySink* createNew(UsageEnvironment& env,
		MediaSubsession& subsession, // identifies the kind of data that's being received
		char const* streamId = NULL); // identifies the stream itself (optional)
	
private:
	DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
    // called only by "createNew()"
	virtual ~DummySink();
	
	static void afterGettingFrame(void* clientData, unsigned frameSize,
		unsigned numTruncatedBytes,
		struct timeval presentationTime,
		unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
		struct timeval presentationTime, unsigned durationInMicroseconds);
	void InitMediaInfo();
private:
	// redefined virtual functions:
	virtual Boolean continuePlaying();
	
private:
	u_int8_t* fReceiveBuffer;
	MediaSubsession& fSubsession;
//	char* fStreamId;
	char m_aryHeard[4];
public:
	LPSreamDataCallBack m_lpStreamCallBack;
	void*					   m_pUserData;
	u_int8_t*               m_pFrameBuf;
	unsigned int         m_IFrameSize;
	AV_INFO					m_AVInfo;
};

class CStreamItem  
{
public:
	CStreamItem();
	virtual ~CStreamItem();

	int Open(char* sRtspUrl,LPSreamDataCallBack pStreamCB, void* pUserData);
	int Close();
	RTSPClient* GetRtspClient(){return m_pRtspClient;} 

public:
	bool			  m_bCloseClient;
private:
	LPSreamDataCallBack m_lpStreamCallBack;
	void*               m_pUserData;
	ourRTSPClient*      m_pRtspClient;
	int                 m_nLongTimeNoData;
	int                 m_nLastRecvTime;

};

#endif // !defined(AFX_STREAMITEM_H__AE91F627_99F2_4F64_AC30_9EC18F6FC2CE__INCLUDED_)
