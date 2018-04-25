/********************************************************************************
   * COPYRIGHT   : Copyright(C) 2014-2016  T&W  Co., Ltd. All Rights Reserved.
   * FILE NAME   : StreamItem.cpp
   * DESCRIPTION : 负责RTSP处理模块
   * AUTHOR      : liuxuanbin
   * VERSION     : 1.0
   * DATE        : 2015/03/05 
   * FIXED RECORD : 
   * DATE   VERSION     REVISIOR          ABSTRACT

   * NOTICE :
********************************************************************************/
// StreamItem.cpp: implementation of the CStreamItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RtspStream.h"
#include "StreamItem.h"
#include "geth264videosize.h"
#include <string>
#include <stdio.h>
using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
  // called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// The main streaming routine (for each "rtsp://" URL):
void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

void usage(UsageEnvironment& env, char const* progName) {
  env << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
  env << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

static unsigned rtspClientCount = 0; // Counts how many streams (i.e., "RTSPClient"s) are currently in use.

void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL) {
  // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
  // to receive (even if more than stream uses the same "rtsp://" URL).
  RTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
  if (rtspClient == NULL) {
    env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
    return;
  }

  ++rtspClientCount;

  // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
  // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
  // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
  rtspClient->sendDescribeCommand(continueAfterDESCRIBE, ((ourRTSPClient*)rtspClient)->ourAuthenticator); 
}


// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) 
	{
	  ((ourRTSPClient*)rtspClient)->SetErrorInfo(-1);
      delete[] resultString;
      break;
    }

    char* const sdpDescription = resultString;   
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);
    setupNextSubsession(rtspClient);
	 ((ourRTSPClient*)rtspClient)->SetErrorInfo(0);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  // shutdownStream(rtspClient);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP False
//#define REQUEST_STREAMING_OVER_TCP False
 
void setupNextSubsession(RTSPClient* rtspClient) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
  
  scs.subsession = scs.iter->next();
  if (scs.subsession != NULL) {
    if (!scs.subsession->initiate()) {
     // env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
    } else {
     // env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
      if (scs.subsession->rtcpIsMuxed()) {
	//env << "client port " << scs.subsession->clientPortNum();
      } else {
	//env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
      }
      //env << ")\n";

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP,False,((ourRTSPClient*)rtspClient)->ourAuthenticator);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  if (scs.session->absStartTime() != NULL) {
    // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime(),1.0f,((ourRTSPClient*)rtspClient)->ourAuthenticator);
  } else {
    scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY,0.0f, -1.0f, 1.0f, ((ourRTSPClient*)rtspClient)->ourAuthenticator);
  }
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0)
	{
	   ((ourRTSPClient*)rtspClient)->SetErrorInfo(-1);
      env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
      break;
    }

    env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
    if (scs.subsession->rtcpIsMuxed()) {
      env << "client port " << scs.subsession->clientPortNum();
    } else {
      env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
    }
    env << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
    // after we've sent a RTSP "PLAY" command.)

    scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url());
	((DummySink*)scs.subsession->sink)->m_lpStreamCallBack = ((ourRTSPClient*)rtspClient)->m_lpStreamCallBack;
	((DummySink*)scs.subsession->sink)->m_pUserData = ((ourRTSPClient*)rtspClient)->m_pUserData;
      // perhaps use your own custom "MediaSink" subclass instead
    if (scs.subsession->sink == NULL) {
      env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
	  << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }

    env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
    scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 
    scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
				       subsessionAfterPlaying, scs.subsession);
    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
    }
	 ((ourRTSPClient*)rtspClient)->SetErrorInfo(0);
  } while (0);
  delete[] resultString;

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
  Boolean success = False;

  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
    // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
    // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
    // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      scs.duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
      scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";
    if (scs.duration > 0) {
      env << " (for up to " << scs.duration << " seconds)";
    }
    env << "...\n";

    success = True;
  } while (0);
  delete[] resultString;

  if (!success) {
    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
  }
}


// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL) return; // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  //shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
  UsageEnvironment& env = rtspClient->envir(); // alias

  env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData) {
  ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
  StreamClientState& scs = rtspClient->scs; // alias

  scs.streamTimerTask = NULL;

  // Shut down the stream:
  //shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) { 
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
      if (subsession->sink != NULL) {
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	if (subsession->rtcpInstance() != NULL) {
	  subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
	}

	someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*scs.session, NULL,((ourRTSPClient*)rtspClient)->ourAuthenticator);
    }
  }

 // env << *rtspClient << "Closing the stream.\n";
  Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

  if (--rtspClientCount == 0) {
    // The final stream has ended, so exit the application now.
    // (Of course, if you're embedding this code into your own application, you might want to comment this out,
    // and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
    //exit(exitCode);
  }
}


// Implementation of "ourRTSPClient":

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
  return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
			     int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1),
m_pUserData(NULL),
m_lpStreamCallBack(NULL),
m_nErrCode(-999)

{
	m_hWaitEvent = CreateEventA(NULL,  /* 默认属性 */  
		FALSE,  /* 自动复位 */  
		FALSE,  /* 初始状态为不触发 */  
		NULL    /* 没有名字 */  
                            );
}

ourRTSPClient::~ourRTSPClient() {
}

/*
void ourRTSPClient::parseDescribeInfo(char* resultString)
{
	string sRet = resultString;
	if (sRet.empty())
		return;

	char szTemp[10] = {0};
	string strTemp = szTemp;

	char* p1 = "a=rtpmap:96";
	int nPos1 = sRet.find(p1);
	if(nPos1 == string::npos)
		return;

	char* p2 = "//";//"\\r\\n";
	int nPos2 = sRet.find_first_of(p2, nPos1);
	if(nPos2 == string::npos)
		return;
//video type
	sRet.copy(szTemp, nPos2-nPos1-12, nPos1+12);	
	strTemp = szTemp;
	if (strTemp.compare("H264") == 0)
		m_avInfo.nVideoEncodeType = 0x01;

	p1 = "a=rtpmap:97";
	nPos1 = sRet.find(p1);
	if(nPos1 == string::npos)
		return;
	
	char* p3 = "-";
	nPos2 = sRet.find_first_of(p3, nPos1);
	if(nPos2 == string::npos)
		return;
//audio type	
	sRet.copy(szTemp, nPos2-nPos1-12, nPos1+12);
	strTemp = szTemp;
	if (strTemp.compare("G726") == 0)
		m_avInfo.nAudioEncodeType = EA_G726_S16B16C1;
	else if (strTemp.compare("G722") == 0)
		m_avInfo.nAudioEncodeType = EA_G722_S16B16C1;
//audio bits	
	nPos1 = sRet.find_first_of(p2, nPos2);
	if(nPos1 != string::npos)
	{
		memset(szTemp, 0, 10);
		sRet.copy(szTemp, nPos1 - nPos2 - 1, nPos2+1);
		strTemp = szTemp;
		m_avInfo.nAudioBits = atoi(strTemp.c_str());
	}
//audio samples
	nPos2 = sRet.find_first_of(p2, nPos1+1);
	if (nPos2 != string::npos)
	{
		memset(szTemp, 0, 10);
		sRet.copy(szTemp, nPos2 - nPos1 - 1, nPos1 + 1);
		strTemp = szTemp;
		m_avInfo.nAudioSamples = atoi(strTemp.c_str());
	}
}*/
// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) {
}

StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}


// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 1024*1024
//static char aryHeard[4] = {0, 0, 0, 0}; //hisi audio header
//static char g711AryHeard[4] = {0, 1, 160, 0};//hisi G711 audio header

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) {
  return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
  : MediaSink(env),
fSubsession(subsession),
m_lpStreamCallBack(NULL), 
m_pUserData(NULL),
m_IFrameSize(0)
{
  //fStreamId = strDup(streamId);
  fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE]; 
  m_pFrameBuf=new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
  memset(&m_AVInfo, 0, sizeof(AV_INFO));
  memset(m_aryHeard, 0, 4);
  InitMediaInfo();
}

DummySink::~DummySink()
{
	if (fReceiveBuffer)
	{
		delete[] fReceiveBuffer;
		fReceiveBuffer = NULL;
	}

	if(m_pFrameBuf)
	{
		delete[] m_pFrameBuf;
		m_pFrameBuf = NULL;
	}
// 	if (fStreamId)
// 	{
// 		delete[] fStreamId;
// 		fStreamId = NULL;
// 	}	
}

/**********************************************************************
  * 函数名称    : InitMediaInfo
  * 功能描述    : 初始化媒体信息
  * 输入参数    : 无
  * 输出参数    : 无
  * 返 回 值      : 无   
  *						 
  * 其它说明    : 无
  * 修改日期    版本号     修改人      修改内容
  * -----------------------------------------------
  * 14/11/21     V1.0      liuxuanbin      XXXX
  **********************************************************************/
void DummySink::InitMediaInfo()
{
	if(strcmp(fSubsession.mediumName(), "video") == 0)
	{
		if (strcmp(fSubsession.codecName(), "H264") == 0)
		{
			m_AVInfo.nVideoEncodeType = 0x01;
		}
	}
	else if(strcmp(fSubsession.mediumName(), "audio") == 0)
	{
		if(strcmp(fSubsession.codecName(), "G726-16") == 0		// G.726, 16 kbps
			|| strcmp(fSubsession.codecName(), "G726-24") == 0 // G.726, 24 kbps
			|| strcmp(fSubsession.codecName(), "G726-32") == 0 // G.726, 32 kbps
		    || strcmp(fSubsession.codecName(), "G726-40") == 0 // G.726, 40 kbps
		   )
		{
			char szBits[3] = {0};
			memcpy(szBits, fSubsession.codecName() + 5, 2);
			int bits = atoi((const char*)szBits);

			m_AVInfo.nAudioEncodeType = X_AENC_G726;
			m_AVInfo.nAudioBits = bits;

			if (16 == bits)
				m_aryHeard[2] = 20;
			else if(24 == bits)
				m_aryHeard[2] = 30;
			else if(32 == bits)
				m_aryHeard[2] = 40;
			else if(40 == bits)
				m_aryHeard[2] = 50;
			else
				m_aryHeard[2] = 20;

			m_aryHeard[0] = 0;
			m_aryHeard[1] = 1;
		//	m_aryHeard[2] = 20;
			m_aryHeard[3] = 0;
		}
		else if( strcmp(fSubsession.codecName(), "G722") == 0) // G.722 audio (RFC 3551)
		{
			m_AVInfo.nAudioEncodeType = X_AENC_G722;
		}
		else if (strcmp(fSubsession.codecName(), "PCMA") == 0)
		{
			m_AVInfo.nAudioEncodeType = X_AENC_G711A;
			m_AVInfo.nAudioBits = 16;
		}
		else if (strcmp(fSubsession.codecName(), "PCMU") == 0)
		{
			m_AVInfo.nAudioEncodeType = X_AENC_G711U;
			m_AVInfo.nAudioBits = 16;
		}
		else if (strcmp(fSubsession.codecName(), "L8") == 0)
		{
			m_AVInfo.nAudioEncodeType = X_AENC_PCM;
		}
		else if (strcmp(fSubsession.codecName(), "DVI4") == 0)
		{
			m_AVInfo.nAudioEncodeType = X_AENC_ADPCM;
			m_AVInfo.nAudioBits = 16;
		}

		m_AVInfo.nAudioChannels = fSubsession.numChannels();
		m_AVInfo.nAudioSamples = fSubsession.rtpTimestampFrequency();
	}
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  DummySink* sink = (DummySink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

static char buf[4] = {0, 0, 0, 1};	//hisi h264 video header
/**********************************************************************
  * 函数名称    : afterGettingFrame
  * 功能描述    : 获取音视频数据
  * 输入参数    : frameSize		帧数据大小
							numTruncateBytes  
							presentationTime 时间戳
							durationlnMicroseconds 
  * 输出参数    : 无
  * 返 回 值      : 无   
  *						 
  * 其它说明    : 无
  * 修改日期    版本号     修改人      修改内容
  * -----------------------------------------------
  * 14/11/21     V1.0      liuxuanbin      XXXX
  **********************************************************************/
enum FrameType_e
{
	FRAME_P = 0x01,
	FRAME_I = 0x05,
	FRAME_S = 0x06,
	FRAME_SPS = 0x07,
	FRAME_PPS = 0x08,		
};

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {


  // Then continue, to request the next frame of data:
	continuePlaying();

	LONGLONG llTimeStamp=(LONGLONG)presentationTime.tv_sec*1000000+(LONGLONG)presentationTime.tv_usec; 

	if (0 == strcmp(fSubsession.mediumName(), "video"))
	{	
		int rate = fSubsession.videoFPS();
		int heigh = fSubsession.videoHeight();
		int witch = fSubsession.videoWidth();
		int num = fSubsession.rtpSource()->curPacketRTPSeqNum();
		int type = fReceiveBuffer[0]&0x1f;
		//TRACE("cur rtp seq num: %d ,heigh %d,witch %d\n", num, heigh, witch);
		if (m_lpStreamCallBack)
		{	
			memcpy(m_pFrameBuf + m_IFrameSize,buf,4);

			if (FRAME_SPS==type || FRAME_PPS==type || FRAME_S==type || FRAME_I==type)
			{
				memcpy(m_pFrameBuf + m_IFrameSize + 4, fReceiveBuffer,frameSize);
				m_IFrameSize += frameSize + 4;

				if (FRAME_I==type && FRAME_SPS == (m_pFrameBuf[4] &0x1f))
				{
					m_lpStreamCallBack(m_pFrameBuf, m_IFrameSize, FRAME_FLAG_VI, &m_AVInfo, llTimeStamp, m_pUserData);
					m_IFrameSize = 0;
				}
				else if (FRAME_SPS==type)
				{
					unsigned int width, height = 0;
					bool bRet = h264_decode_seq_parameter_set(fReceiveBuffer, frameSize,width, height);
					if (!bRet)
						return;

					if (width != m_AVInfo.nVideoWidth || height != m_AVInfo.nVideoHeight)
					{
						m_AVInfo.nVideoWidth = width;
						m_AVInfo.nVideoHeight = height;
					}
				}
			}
			else 
			{
				m_IFrameSize = 0;
				memcpy(m_pFrameBuf+4,fReceiveBuffer,frameSize);

				m_lpStreamCallBack(m_pFrameBuf,frameSize+4, FRAME_FLAG_VP, &m_AVInfo, llTimeStamp,m_pUserData);
			}
		}
	}
	else if(0 == strcmp(fSubsession.mediumName(), "audio"))
	{
		if (m_lpStreamCallBack)
		{
			int nAudioHeard = 0;
			if(m_AVInfo.nAudioEncodeType == X_AENC_G726)
			{
				memcpy(m_pFrameBuf, m_aryHeard, 4);
				nAudioHeard = 4;
			}
		
			memcpy(m_pFrameBuf + nAudioHeard, fReceiveBuffer, frameSize);
			m_lpStreamCallBack(m_pFrameBuf,frameSize+nAudioHeard, FRAME_FLAG_A, &m_AVInfo, llTimeStamp,m_pUserData);
		}
	}
	
}

Boolean DummySink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  
  
  return True;
}

Boolean parseRTSPURL(char const* url, char*& username, char*& password)
 {
	do {
		// Parse the URL as "rtsp://[<username>[:<password>]@]<server-address-or-name>[:<port>][/<stream-name>]"
		char const* prefix = "rtsp://";
		unsigned const prefixLength = 7;
		if (_strncasecmp(url, prefix, prefixLength) != 0) {
			
			break;
		}
		
		unsigned const parseBufferSize = 100;
		//char parseBuffer[parseBufferSize];
		char const* from = &url[prefixLength];
		
		// Check whether "<username>[:<password>]@" occurs next.
		// We do this by checking whether '@' appears before the end of the URL, or before the first '/'.
		username = password = NULL; // default return values
		char const* colonPasswordStart = NULL;
		char const* p;
		for (p = from; *p != '\0' && *p != '/'; ++p) {
			if (*p == ':' && colonPasswordStart == NULL) {
				colonPasswordStart = p;
			} else if (*p == '@') {
				// We found <username> (and perhaps <password>).  Copy them into newly-allocated result strings:
				if (colonPasswordStart == NULL) colonPasswordStart = p;
				
				char const* usernameStart = from;
				unsigned usernameLen = colonPasswordStart - usernameStart;
				username = new char[usernameLen + 1] ; // allow for the trailing '\0'
				for (unsigned i = 0; i < usernameLen; ++i) username[i] = usernameStart[i];
				username[usernameLen] = '\0';
				
				char const* passwordStart = colonPasswordStart;
				if (passwordStart < p) ++passwordStart; // skip over the ':'
				unsigned passwordLen = p - passwordStart;
				password = new char[passwordLen + 1]; // allow for the trailing '\0'
				for (unsigned j = 0; j < passwordLen; ++j) password[j] = passwordStart[j];
				password[passwordLen] = '\0';
				
				from = p + 1; // skip over the '@'
				break;
			}
		}
		
		return True;
	} while (0);
	
	return False;
}


extern CRtspStreamApp theApp;

CStreamItem::CStreamItem():
m_lpStreamCallBack(NULL),
m_pUserData(NULL),
m_pRtspClient(NULL),
m_bCloseClient(false)
{
}

CStreamItem::~CStreamItem()
{
	m_lpStreamCallBack=NULL;
	m_pUserData=NULL;
}

/**********************************************************************
  * 函数名称    : Open
  * 功能描述    : 请求RTSP连接
  * 输入参数    : sRtspURL  服务器地址
							pStreamCB 回调函数
							pUserData 用户数据指针
  * 输出参数    : 无
  * 返 回 值      : 0   表示成功
  *						  -1  表示失败
  * 其它说明    : 无
  * 修改日期    版本号     修改人      修改内容
  * -----------------------------------------------
  * 14/11/21     V1.0      liuxuanbin      XXXX
  **********************************************************************/
int CStreamItem::Open( char* sRtspUrl,LPSreamDataCallBack pStreamCB, void* pUserData )
{
	m_lpStreamCallBack=pStreamCB;
	m_pUserData=pUserData;	
	// open url

	m_pRtspClient=ourRTSPClient::createNew(*theApp.g_env, sRtspUrl, RTSP_CLIENT_VERBOSITY_LEVEL,sRtspUrl);
	if ( m_pRtspClient == NULL) {
		
		return -1;
	}
	
	m_pRtspClient->m_lpStreamCallBack=pStreamCB;
	m_pRtspClient->m_pUserData=pUserData;
	
	char* username;
    char* password; 
	parseRTSPURL(sRtspUrl, username, password);
	 m_pRtspClient->ourAuthenticator = new Authenticator(username, password);
	 delete[] username;
     delete[] password;
	// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
	// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
	// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:

	WaitForSingleObject(theApp.g_eventLoopMutex,INFINITE);
	m_pRtspClient->sendDescribeCommand(continueAfterDESCRIBE, m_pRtspClient->ourAuthenticator);	
	ReleaseMutex(theApp.g_eventLoopMutex);
//	Sleep(1000);
 
	while (m_pRtspClient->m_nErrCode == -999)
		Sleep(10);

	return m_pRtspClient->m_nErrCode;
}

/**********************************************************************
  * 函数名称    : CloseClientFun
  * 功能描述    : 关闭客户端连接(注册回调)
  * 输入参数    : data 数据指针   
  * 输出参数    : 无
  * 返 回 值      : 无   
  *						 
  * 其它说明    : 无
  * 修改日期    版本号     修改人      修改内容
  * -----------------------------------------------
  * 14/11/21     V1.0      liuxuanbin      XXXX
  **********************************************************************/
void CloseClientFun(void *data)
{ 
	if (!data)
	{
		return;
	}

	RTSPClient* pClient=(RTSPClient*)data;
	shutdownStream(pClient,0);

}

/**********************************************************************
  * 函数名称    : Close
  * 功能描述    : 关闭rtsp连接
  * 输入参数    : 无   
  * 输出参数    : 无
  * 返 回 值      : 0   
  *						 
  * 其它说明    : 无
  * 修改日期    版本号     修改人      修改内容
  * -----------------------------------------------
  * 14/11/21     V1.0      liuxuanbin      XXXX
  **********************************************************************/
int CStreamItem::Close()
{
	if (m_pRtspClient)
	{
		m_pRtspClient->m_lpStreamCallBack=NULL;
		m_pRtspClient->m_pUserData=NULL;

		WaitForSingleObject(theApp.g_eventLoopMutex,INFINITE);	
		theApp.g_scheduler->scheduleDelayedTask(1000*1000, CloseClientFun, m_pRtspClient);		
		ReleaseMutex(theApp.g_eventLoopMutex);
		delete m_pRtspClient->ourAuthenticator;
		m_pRtspClient->ourAuthenticator = NULL;
		m_pRtspClient=NULL;
	}
	//if(!m_bCloseClient)
	//	Sleep(100);

	return 0;	
}



