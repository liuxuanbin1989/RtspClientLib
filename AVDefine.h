/********************************************************************************
   * COPYRIGHT   : Copyright(C) 2014-2016  T&W  Co., Ltd. All Rights Reserved.
   * FILE NAME   : AVDefine.h
   * DESCRIPTION : 音视频参数定义
   * AUTHOR      : liuxuanbin
   * VERSION     : 1.0
   * DATE        : 2015/03/05 
   * FIXED RECORD : 
   * DATE   VERSION     REVISIOR          ABSTRACT

   * NOTICE :
********************************************************************************/
#ifndef X_AV_DEFINE_H_
#define X_AV_DEFINE_H_

//========================================================
//帧头信息
//========================================================
//
//每帧数据:
//HV_FRAME_HEAD + [EXT_FRAME_HEAD] + Data
//

//音视频数据帧头
typedef struct _HV_FRAME_HEAD
{
	unsigned short	zeroFlag;				// 0
	unsigned char   oneFlag;				// 1
	unsigned char	streamFlag;				// 数据帧标志 FRAME_FLAG_VP，FRAME_FLAG_VI，FRAME_FLAG_A
	
	unsigned long	nByteNum;				//数据帧大小
	unsigned long	nTimestamp;				//时间戳
}HV_FRAME_HEAD;

//========================================================================
//扩展帧头
//========================================================================
#define	X_EXT_HEAD_FLAG	0x06070809
#define	X_EXT_TAIL_FLAG	0x0a0b0c0d

//视频参数
typedef struct _EXT_FRAME_VIDEO
{
	unsigned short	nVideoEncodeType;	//视频编码算法
	unsigned short	nVideoWidth;		//视频图像宽
	unsigned short	nVideoHeight;		//视频图像高
	unsigned char   cPal;               //制式
	unsigned char   cTwoFeild;          //是否是两场编码（如果是两场编码，PC端解码需做deinterlace）

	unsigned char   szReserve[8];
} EXT_FRAME_VIDEO;

//音频参数
typedef struct _EXT_FRAME_AUDIO
{
	unsigned short	nAudioEncodeType;	//音频编码算法
	unsigned short	nAudioChannels;		//通道数
	unsigned short	nAudioBits;			//位数
	unsigned char   szReserve[2];
	unsigned long	nAudioSamples;		//采样率 	
	unsigned long	nAudioBitrate;		//音频编码码率

} EXT_FRAME_AUDIO;

typedef union _EXT_FRAME_TYPE
{
	EXT_FRAME_VIDEO	szFrameVideo;
	EXT_FRAME_AUDIO	szFrameAudio;
} EXT_FRAME_TYPE;

typedef struct _EXT_FRAME_HEAD
{
    unsigned long	nStartFlag;			//扩展帧头起始标识
    unsigned short	nVer;				//版本
    unsigned short	nLength;			//扩展帧头长度
	EXT_FRAME_TYPE	szFrameInfo;		
	unsigned long   nTimestamp;			//以毫秒为单位的时间戳
	unsigned long	nEndFlag;			//扩展帧头起始标识

	bool CheckExtFlag()
	{
		return (X_EXT_HEAD_FLAG == nStartFlag && X_EXT_TAIL_FLAG == nEndFlag ) ? true:false;
	}
}EXT_FRAME_HEAD;

//=========================================
//数据帧类型标志
//=========================================
enum eFrameType
{
	MY_FRAME_TYPE_A = 0x0d,				//音频帧
	MY_FRAME_TYPE_I = 0x0e,				//视频的I帧
	MY_FRAME_TYPE_P = 0x0b				//视频的P帧
};

#define	FRAME_FLAG_A		MY_FRAME_TYPE_A
#define	FRAME_FLAG_VI		MY_FRAME_TYPE_I
#define	FRAME_FLAG_VP		MY_FRAME_TYPE_P

//=========================================
//视频编码算法
//=========================================
typedef enum  _X_VIDEO_ENCODER_E
{
	X_VENC_NONE   = 0x00,
	X_VENC_H264   = 0x01,
	X_VENC_MPEG4  = 0x02,
	X_VENC_MJPEG  = 0x03,
	X_VENC_JPEG   = 0x04,
}X_VIDEO_ENCODER_E;

//=========================================
//音频编码算法
//=========================================
typedef enum  _X_AUDIO_ENCODER_E
{
	X_AENC_NONE   = 0x00,
	X_AENC_G726   = 0x01,
	X_AENC_G722   = 0x02,
	X_AENC_G711A  = 0x03,
	X_AENC_ADPCM  = 0x04,
	X_AENC_MP3    = 0x05,
	X_AENC_PCM    = 0x06,
	X_AENC_G711U  = 0x07,
}X_AUDIO_ENCODER_E;

//=========================================
//音频采样率
//=========================================
typedef enum  _X_AUDIO_SAMPLERATE_E
{
	AI_SAMPLE_RATE_NONE     = 0,  
	AI_SAMPLE_RATE_8000		= 8000,		/* 8kHz sampling rate		*/
	AI_SAMPLE_RATE_11025	= 11025,	/* 11.025kHz sampling rate	*/
	AI_SAMPLE_RATE_16000	= 16000,	/* 16kHz sampling rate		*/
	AI_SAMPLE_RATE_22050	= 22050,	/* 22.050kHz sampling rate	*/
	AI_SAMPLE_RATE_24000	= 24000,	/* 24kHz sampling rate		*/
	AI_SAMPLE_RATE_32000	= 32000,	/* 32kHz sampling rate		*/
	AI_SAMPLE_RATE_44100	= 44100,	/* 44.1kHz sampling rate	*/
	AI_SAMPLE_RATE_48000	= 48000,	/* 48kHz sampling rate		*/
}X_AUDIO_SAMPLERATE_E;

//=========================================
//音频编码码率(G726)
//=========================================
typedef enum  _X_AUDIO_BITRATE_E
{	
	AENC_BITRATE_NONE   = 0,
	AENC_BITRATE_16000	= 16000,		
	AENC_BITRATE_24000	= 24000,		
	AENC_BITRATE_32000	= 32000,		
	AENC_BITRATE_40000	= 40000,		
}_X_AUDIO_BITRATE_E;



//========================================================
//视频编码格式									
//========================================================
typedef enum  _ENCODE_VIDEO_TYPE
{
	EV_TYPE_NONE		= 0xFFFF,
	EV_TYPE_PAL_D1		= 0x00,		//PAL制D1		704 * 576
	EV_TYPE_PAL_HD1		= 0x01,		//PAL制HalfD1	704 * 288
	EV_TYPE_PAL_CIF		= 0x02,		//PAL制CIF		352 * 288
	EV_TYPE_VGA			= 0x03,		//VGA			640 * 480
	EV_TYPE_HVGA		= 0x04,		//HVGA			640 * 240
	EV_TYPE_CVGA		= 0x05,		//CVGA			320 * 240
	EV_TYPE_PAL_QCIF	= 0x06,		//PAL制QCIF		176 * 144
	EV_TYPE_QVGA		= 0x07,		//QVGA			160 * 120
	EV_TYPE_NTSC_D1		= 0x08,		//N制D1			704 * 480
	EV_TYPE_NTSC_HD1	= 0x09,		//N制HalfD1		704 * 240
	EV_TYPE_NTSC_CIF	= 0x0A,		//N制CIF		352 * 240
	EV_TYPE_NTSC_QCIF	= 0x0E,		//N制QCIF		176 * 120

	//H.264
	EV_H264_PAL_D1		= 0x10,		//H264_2,PAL制D1		704 * 576
	EV_H264_PAL_HD1		= 0x11,		//H264_2,PAL制HalfD1	704 * 288
	EV_H264_PAL_CIF		= 0x12,		//H264_2,PAL制CIF		352 * 288
	EV_H264_VGA			= 0x13,		//H264_2,VGA			640 * 480
	EV_H264_HVGA		= 0x14,		//H264_2,HVGA			640 * 240
	EV_H264_CVGA		= 0x15,		//H264_2,CVGA			320 * 240
	EV_H264_PAL_QCIF	= 0x16,		//H264_2,PAL制QCIF		176 * 144
	EV_H264_QVGA		= 0x17,		//H264_2,QVGA			160 * 120
	EV_H264_NTSC_D1		= 0x18,		//H264_2,N制D1			704 * 480
	EV_H264_NTSC_HD1	= 0x19,		//H264_2,N制HalfD1		704 * 240
	EV_H264_NTSC_CIF	= 0x1A,		//H264_2,N制CIF			352 * 240
	EV_H264_NTSC_QCIF	= 0x1E,		//H264_2,N制QCIF		176 * 120

	//标准MPEG4
	EV_MPEG4_PAL_D1		= 0x20,		//PAL制D1				704 * 576
	EV_MPEG4_PAL_HD1	= 0x21,		//PAL制HalfD1			704 * 288
	EV_MPEG4_PAL_CIF	= 0x22,		//PAL制CIF				352 * 288
	EV_MPEG4_VGA		= 0x23,		//VGA					640 * 480
	EV_MPEG4_HVGA		= 0x24,		//HVGA					640 * 240
	EV_MPEG4_CVGA		= 0x25,		//CVGA					320 * 240
	EV_MPEG4_PAL_QCIF	= 0x26,		//PAL制QCIF				176 * 144
	EV_MPEG4_QVGA		= 0x27,		//QVGA					160 * 120
	EV_MPEG4_NTSC_D1	= 0x28,		//N制D1					704 * 480
	EV_MPEG4_NTSC_HD1	= 0x29,		//N制HalfD1				704 * 240
	EV_MPEG4_NTSC_CIF	= 0x2A,		//N制CIF				352 * 240
	EV_MPEG4_NTSC_QCIF	= 0x2E,		//N制QCIF				176 * 120

	//MJPEG
	EV_MJPEG_PAL_D1     = 0x30,     //MJPEG,PAL制D1        704 * 576
	EV_MJPEG_PAL_HD1    = 0x31,     //MJPEG,PAL制HalfD1    704 * 288
	EV_MJPEG_PAL_CIF    = 0x32,     //MJPEG,PAL制CIF       352 * 288
	EV_MJPEG_VGA        = 0x33,     //MJPEG,VGA            640 * 480
	EV_MJPEG_HVGA       = 0x34,     //MJPEG,HVGA           640 * 240
	EV_MJPEG_CVGA       = 0x35,     //MJPEG,CVGA           320 * 240
	EV_MJPEG_PAL_QCIF   = 0x36,     //MJPEG,PAL制QCIF      176 * 144
	EV_MJPEG_QVGA       = 0x37,     //MJPEG,QVGA           160 * 120
	EV_MJPEG_NTSC_D1    = 0x38,     //MJPEG,N制D1          704 * 480
	EV_MJPEG_NTSC_HD1   = 0x39,     //MJPEG,N制HalfD1      704 * 240
	EV_MJPEG_NTSC_CIF   = 0x3A,     //MJPEG,N制CIF         352 * 240
	EV_MJPEG_NTSC_QCIF  = 0x3E,     //MJPEG,N制QCIF        176 * 120

	//JPEG
	EV_JPEG_PAL_D1      = 0x40,     //JPEG,PAL制D1        704 * 576
	EV_JPEG_PAL_HD1     = 0x41,     //JPEG,PAL制HalfD1    704 * 288
	EV_JPEG_PAL_CIF     = 0x42,     //JPEG,PAL制CIF       352 * 288
	EV_JPEG_VGA         = 0x43,     //JPEG,VGA            640 * 480
	EV_JPEG_HVGA        = 0x44,     //JPEG,HVGA           640 * 240
	EV_JPEG_CVGA        = 0x45,     //JPEG,CVGA           320 * 240
	EV_JPEG_PAL_QCIF    = 0x46,     //JPEG,PAL制QCIF      176 * 144
	EV_JPEG_QVGA        = 0x47,     //JPEG,QVGA           160 * 120
	EV_JPEG_NTSC_D1     = 0x48,     //JPEG,N制D1          704 * 480
	EV_JPEG_NTSC_HD1    = 0x49,     //JPEG,N制HalfD1      704 * 240
	EV_JPEG_NTSC_CIF    = 0x4A,     //JPEG,N制CIF         352 * 240
	EV_JPEG_NTSC_QCIF   = 0x4E,     //JPEG,N制QCIF        176 * 120

	//
	EA_G722_S16B16C1	= 0x0100,	//音频，G722
	EA_G711A_S16B16C1	= 0x0200,	//音频，G711A
	EA_G711MU_S16B16C1	= 0x0300,	//音频，G711MU
	EA_ADPCM_S16B16C1	= 0x0400,	//音频，ADPCM
	EA_G726_S16B16C1	= 0x0500,	//音频，G726
	EA_BUTT_S16B16C1	= 0x0600,	//音频，BUTT
	EA_MPT_S16B16C1		= 0x0700,	//音频，MPT
}ENCODE_VIDEO_TYPE;

//视频音频参数
typedef struct _AV_INFO
{
    //视频参数
	UINT			nVideoEncodeType;		//视频编码格式
    UINT			nVideoHeight;			//视频图像高
    UINT			nVideoWidth;			//视频图像宽
    //音频参数
	UINT			nAudioEncodeType;		//音频编码格式
    UINT			nAudioChannels;			//通道数
    UINT			nAudioBits;				//位数
    UINT			nAudioSamples;			//采样率
}AV_INFO,*PAV_INFO;

#define OUT_FMT_YUV_422  0x00000102
#define OUT_FMT_RGB_032  0x00000204
#define OUT_FMT_RGB_024  0x00000303
#define OUT_FMT_RGB_565  0x00000402
#define OUT_FMT_RGB_555  0x00000502
#define OUT_FMT_YUV_420  0x00000601

#endif