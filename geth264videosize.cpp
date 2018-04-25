/********************************************************************************
   * COPYRIGHT   : Copyright(C) 2014-2016  T&W  Co., Ltd. All Rights Reserved.
   * FILE NAME   : geth264videosize.cpp
   * DESCRIPTION : 解析H264获取视频宽高
   * AUTHOR      : liuxuanbin
   * VERSION     : 1.0
   * DATE        : 2015/03/05 
   * FIXED RECORD : 
   * DATE   VERSION     REVISIOR          ABSTRACT

   * NOTICE :
********************************************************************************/
//#include "StdAfx.h"
#include "geth264videosize.h"

#include <math.h>

unsigned int Ue(unsigned char *pBuff, unsigned int nLen, unsigned int &nStartBit)
{
	//计算0bit的个数
	unsigned int nZeroNum = 0;
	while (nStartBit < nLen * 8)
	{
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) //&:按位与，%取余
		{
		  break;
		}
		nZeroNum++;
		nStartBit++;
	}
	nStartBit ++;

	//计算结果
	unsigned long dwRet = 0;
	for (unsigned int i=0; i<nZeroNum; i++)
	{
		dwRet <<= 1;
		if (pBuff[nStartBit / 8] & (0x80 >> 
		(nStartBit % 8)))
		{
		  dwRet += 1;
		}
		nStartBit++;
	}
	return (1 << nZeroNum) - 1 + dwRet;
}


int Se(unsigned char *pBuff, unsigned int nLen, unsigned int &nStartBit)
{
	int UeVal=Ue(pBuff,nLen,nStartBit);
	double k=UeVal;
	int nValue=2;//ceil(k/2);//ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00
	if (UeVal % 2==0)
	nValue=-nValue;
	return nValue;
}


unsigned long u(unsigned int BitCount,unsigned char * buf,unsigned int &nStartBit)
{
	unsigned long dwRet = 0;
	for (unsigned int i=0; i<BitCount; i++)
	{
		dwRet <<= 1;
		if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
		  dwRet += 1;
		}
		nStartBit++;
	}
	return dwRet;
}


bool h264_decode_seq_parameter_set(unsigned char * buf,unsigned int nLen,unsigned int &Width,unsigned int &Height)
{
	unsigned int StartBit=0; 
	int forbidden_zero_bit=u(1,buf,StartBit);
	int nal_ref_idc=u(2,buf,StartBit);
	int nal_unit_type=u(5,buf,StartBit);
	int chroma_format_idc = -1;

	if(nal_unit_type==7)
	{
		int profile_idc=u(8,buf,StartBit);
		int constraint_set0_flag=u(1,buf,StartBit);//(buf[1] & 0x80)>>7;
		int	constraint_set1_flag=u(1,buf,StartBit);//(buf[1] & 0x40)>>6;
		int constraint_set2_flag=u(1,buf,StartBit);//(buf[1] & 0x20)>>5;
		int constraint_set3_flag=u(1,buf,StartBit);//(buf[1] & 0x10)>>4;
		int reserved_zero_4bits=u(4,buf,StartBit);
		int level_idc=u(8,buf,StartBit);

		int seq_parameter_set_id=Ue(buf,nLen,StartBit);

		if( profile_idc == 100 || profile_idc == 110 ||	profile_idc == 122 || profile_idc == 144 )
		{
			unsigned int chroma_format_idc=Ue(buf,nLen,StartBit);
			if( chroma_format_idc == 3 )
			{
				int residual_colour_transform_flag=u(1,buf,StartBit);
			}
		
			unsigned int bit_depth_luma_minus8=Ue(buf,nLen,StartBit);
			unsigned int bit_depth_chroma_minus8=Ue(buf,nLen,StartBit);
			int qpprime_y_zero_transform_bypass_flag=u(1,buf,StartBit);
			int seq_scaling_matrix_present_flag=u(1,buf,StartBit);

			int seq_scaling_list_present_flag[8];
			if( seq_scaling_matrix_present_flag )
			{
				for( int i = 0; i < 8; i++ )
				{
					seq_scaling_list_present_flag[i]=u(1,buf,StartBit);
				}
			}
		}

		unsigned int log2_max_frame_num_minus4=Ue(buf,nLen,StartBit);
		unsigned int pic_order_cnt_type=Ue(buf,nLen,StartBit);

		if( pic_order_cnt_type == 0 )
		{
			int log2_max_pic_order_cnt_lsb_minus4=Ue(buf,nLen,StartBit);
		}
		else if( pic_order_cnt_type == 1 )
		{
			int delta_pic_order_always_zero_flag=u(1,buf,StartBit);
			int offset_for_non_ref_pic=Se(buf,nLen,StartBit);
			int offset_for_top_to_bottom_field=Se(buf,nLen,StartBit);
			int num_ref_frames_in_pic_order_cnt_cycle=Ue(buf,nLen,StartBit);

			int *offset_for_ref_frame=new int[num_ref_frames_in_pic_order_cnt_cycle];
			for( int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
			offset_for_ref_frame[i]=Se(buf,nLen,StartBit);
			delete [] offset_for_ref_frame;
		}

		unsigned int num_ref_frames=Ue(buf,nLen,StartBit);
		int gaps_in_frame_num_value_allowed_flag=u(1,buf,StartBit);
		unsigned int pic_width_in_mbs_minus1=Ue(buf,nLen,StartBit);
		unsigned int pic_height_in_map_units_minus1=Ue(buf,nLen,StartBit);

		Width=(pic_width_in_mbs_minus1+1)*16;
		Height=(pic_height_in_map_units_minus1+1)*16;

		if (1920 == Width || 1080 == Height)
		{
			Width = 1920;
			Height = 1080;
		}

		return true;
	}
	else
		return false;
}

/*
int get_h264_video_size(void *buf, int &width, int &height)
{
	SPS sps;

	int CropUnitX, CropUnitY = 0;

	switch (sps.chroma_format_idc)
	 {
	 case 0:
		  CropUnitX = 1;
		  CropUnitY = 2-sps.frame_mbs_only_flag;
		 break;
	case 1:
		 CropUnitX = 2;
		 CropUnitY = 2*(2-sps.frame_mbs_only_flag);
		 break;
	 case 2:
		  CropUnitX = 2;
		  CropUnitY = 1*(2-sps.frame_mbs_only_flag);
		  break;
	 case 3:
		  CropUnitX = 1;
		  CropUnitY = 1*(2-sps.frame_mbs_only_flag);
		  break;
	default:
		 return -1;
		 break;
  }
  width  = sps.pic_width_in_mbs_minus1*16-CropUnitX*(sps.frame_crop_left_offset+sps.frame_crop_right_offset);
  height = sps.pic_height_in_map_units_minus1*16*(2-sps.frame_mbs_only_flag)-CropUnitY*(sps.frame_crop_top_offset+sps.frame_crop_bottom_offset);
  return 0;
}
*/
/*_*/


                            
 
