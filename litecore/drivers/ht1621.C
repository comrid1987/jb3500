#if HT1621_ENABLE
#include <drivers/ht1621.h>


//**************************************************************************
//    
//	DESCRIPTION: 71M651x POWER METER - LCD VIM-808-DP or VIM-808-2 Descriptors.
// 
// 	AUTHOR:  MTF
//
//  HISTORY: See end of file 
//
//**************************************************************************
//               
// File:  LCD_VIM808.C  
//               
//                                            COM0        COM1        COM2     	 COM3
//         +-----a-----+                   1A - SEG0   1B - SEG0   1C- SEG0   DP1- SEG0   PIN1
//         |    01h    |                   1F - SEG1   1G - SEG1   1E- SEG1   1D - SEG1   PIN2
//   COM0  |           |                   2A - SEG2   2B - SEG2   2C- SEG2   DP2- SEG2   PIN3
//         f 20h   02h b                   2F - SEG3   2G - SEG3   2E- SEG3   2D - SEG3   PIN4
//   ______|___________|_______            3A - SEG4   3B - SEG4   3C- SEG4   DP3- SEG4   PIN5
//         |           |                   3F - SEG5   3G - SEG5   3E- SEG5   3D - SEG5   PIN6
//   COM1  +-----g-----+                   4A - SEG6   4B - SEG6   4C- SEG6   DP4- SEG6   PIN7
//         |    40h    |                   4F - SEG7   4G - SEG7   4E- SEG7   4D - SEG7   PIN8
//         e           c                   5A - SEG8   5B - SEG8   5C- SEG8   DP5- SEG8   PIN9
//         | 10h   04h |                   5F - SEG9   5G - SEG9   5E- SEG9   5D - SEG9   PIN10
//   ______|___________|__________         6A - SEG10  6B - SEG10  6C- SEG10  DP6- SEG10  PIN11
//   COM2  |    08h    |                   6F - SEG11  6G - SEG11  6E- SEG11  6D - SEG11  PIN12
//         +-----d-----+                   
//                                         7A - SEG12  7B - SEG12  7C- SEG12  DP7- SEG12  PIN1
//                                         7F - SEG13  7G - SEG13  7E- SEG13  7D - SEG13  PIN3
//                                         8A - SEG14  8B - SEG14  8C- SEG14  DP8- SEG14  PIN1
//                                         8F - SEG15  8G - SEG15  8E- SEG15  8D - SEG15  PIN3
//                                          - SEG16   - SEG16      Ua - SEG16  - SEG16     PIN38
//                                         7F - SEG17  7E - SEG17  -- - SEG17  PIN39
//                                         8A - SEG18  8G - SEG18  8D - SEG18  PIN40
//                                         8F - SEG19  8E - SEG19  -- - SEG19  PIN41
//                                          - SEG28   - SEG28   - SEG28  PIN23
//                                          - SEG29   - SEG29  -- - SEG29  PIN24
//                                         2A - SEG30  2G - SEG30  2D - SEG30  PIN25
//                                         2F - SEG31  2E - SEG31  -- - SEG31  PIN26
//                                         -- - SEG32  -- - SEG32  2L - SEG32  PIN27
//                                         -- - SEG33  -- - SEG33  4L - SEG33  PIN32           
//                                         -- - SEG34  -- - SEG34  6L - SEG34  PIN37           
// Segments.    
// 56    [1-8][a-g] (Digits).   
//  7    [1-7]DP    (Decimal points).
//  3    [2,4,6]L   (Colons ':')
// ----------
// 66    Total



//Private Defines
#define COM0	0xC0
#define COM1	0x80
#define COM2	0x40
#define COM3	0x00


enum SEGS                 // List all segments of display in comm/segment order.
{   
    //COM0 SEGMENTS
    SEG_1A = COM0 | 1, SEG_1F = COM0 | 2, SEG_2A = COM0 | 3, SEG_2F = COM0 | 4,
    SEG_3A = COM0 | 5, SEG_3F = COM0 | 19, SEG_4A = COM0 | 6, SEG_4F = COM0 | 7,
    SEG_5A = COM0 | 8, SEG_5F = COM0 | 9, SEG_6A = COM0 |10, SEG_6F = COM0 |11,
    SEG_7A = COM0 |12,SEG_7F = COM0 |13, SEG_8A = COM0 |26, SEG_8F = COM0 |27,
    SEG_PHONE = COM0 |0, SEG_WARN = COM0 |14, SEG_PCNT = COM0 |15, SEG_VO = COM0 |16,
    SEG_PF = COM0 |17, SEG_CT = COM0 |18, SEG_T9 = COM0 |20, SEG_T1 = COM0 |21, 
    SEG_T8 = COM0 |22, SEG_UPOWER = COM0 |23, SEG_0A = COM0 |24, SEG_0F = COM0 |25,
    SEG_I4 = COM0 |28, SEG_H3 = COM0 |29,
    
    //COM1 SEGMENTS
    SEG_1B = COM1 | 1, SEG_1G = COM1 | 2, SEG_2B = COM1 | 3, SEG_2G = COM1 | 4,
    SEG_3B = COM1 | 5, SEG_3G = COM1 | 19, SEG_4B = COM1 | 6, SEG_4G = COM1 | 7,
    SEG_5B = COM1 | 8, SEG_5G = COM1 | 9, SEG_6B = COM1 |10, SEG_6G = COM1 |11,
    SEG_7B = COM1 |12, SEG_7G = COM1 |13, SEG_8B = COM1 |26, SEG_8G = COM1 |27,
    SEG_STATUS = COM1 |0, SEG_USB = COM1 |14, SEG_v = COM1 |15, SEG_CU = COM1 |16,
    SEG_LASTMONTH = COM1 |17, SEG_LASTDAY = COM1 |18, SEG_T10 = COM1 |20, SEG_T2 = COM1 |21, 
    SEG_T7 = COM1 |22, SEG_POWER = COM1 |23, SEG_0B = COM1 |24, SEG_0G = COM1 |25,
    SEG_I3 = COM1 |28, SEG_H4 = COM1 |29,
   
    //COM2 SEGMENTS
    SEG_1C = COM2 | 1, SEG_1E = COM2 | 2, SEG_2C = COM2 | 3, SEG_2E = COM2 | 4,
    SEG_3C = COM2 | 5, SEG_3E = COM2 | 19, SEG_4C = COM2 | 6, SEG_4E = COM2 | 7,
    SEG_5C = COM2 | 8, SEG_5E = COM2 | 9, SEG_6C = COM2 |10, SEG_6E = COM2 |11,
    SEG_7C = COM2 |12, SEG_7E = COM2 |13, SEG_8C = COM2 |26, SEG_8E = COM2 |27,
    SEG_kvar = COM2 |0, SEG_C = COM2 |14, SEG_i = COM2 |15, SEG_ADDR = COM2 |16,
    SEG_TODAY = COM2 |17, SEG_SUM = COM2 |18, SEG_T11 = COM2 |20, SEG_T3 = COM2 |21, 
    SEG_T6 = COM2 |22, SEG_time = COM2 |23, SEG_0C = COM2 |24, SEG_0E = COM2 |25,
    SEG_I2 = COM2 |28, SEG_H2 = COM2 |29,

	//COM3 SEGMENTS
    SEG_DP1= COM3 | 1, SEG_1D = COM3 | 2, SEG_DP2= COM3 | 3, SEG_2D = COM3 | 4,
    SEG_DP3= COM3 | 5, SEG_3D = COM3 | 19, SEG_DP4= COM3 | 6, SEG_4D = COM3 | 7,
    SEG_DP5= COM3 | 8, SEG_5D = COM3 | 9, SEG_DP6= COM3 |10, SEG_6D = COM3 |11,
    SEG_DP7= COM3 |12, SEG_7D = COM3 |13, SEG_DP8= COM3 |26, SEG_8D = COM3 |27,
    SEG_KW = COM3 |0, SEG_B = COM3 |14, SEG_T13 = COM3 |15, SEG_AREA = COM3 |16,
    SEG_MONTH = COM3 |17, SEG_A = COM3 |18, SEG_T12 = COM3 |20, SEG_T4 = COM3 |21, 
    SEG_T5 = COM3 |22, SEG_day = COM3 |23, SEG_0D = COM3 |25,
    SEG_I1 = COM3 |28, SEG_H1 = COM3 |29
};


// List all segments of all LCD icons.
enum SEGS Digit_0[] = 
    { SEG_0A, SEG_0B, SEG_0C, SEG_0D, SEG_0E, SEG_0F, SEG_0G };    // 0 digit. 
 
enum SEGS Digit_1[] = 
    { SEG_1A, SEG_1B, SEG_1C, SEG_1D, SEG_1E, SEG_1F, SEG_1G };    // 1st digit.                                   
    

enum SEGS Digit_2[] =                                     
    { SEG_2A, SEG_2B, SEG_2C, SEG_2D, SEG_2E, SEG_2F, SEG_2G };   // 2nd digit.

enum SEGS Digit_3[] =                                      
    { SEG_3A, SEG_3B, SEG_3C, SEG_3D, SEG_3E, SEG_3F, SEG_3G };   // 3rd digit.

enum SEGS Digit_4[] =                                      
    { SEG_4A, SEG_4B, SEG_4C, SEG_4D, SEG_4E, SEG_4F, SEG_4G };  // 4th digit.

enum SEGS Digit_5[] =                                     
    { SEG_5A, SEG_5B, SEG_5C, SEG_5D, SEG_5E, SEG_5F, SEG_5G };  // 5th digit.

enum SEGS Digit_6[] =                                     
    { SEG_6A, SEG_6B, SEG_6C, SEG_6D, SEG_6E, SEG_6F, SEG_6G };  // 6th digit.

enum SEGS Digit_7[] =                                   
    { SEG_7A, SEG_7B, SEG_7C, SEG_7D, SEG_7E, SEG_7F, SEG_7G };   // 7th digit.

enum SEGS Digit_8[] =                                     
    { SEG_8A, SEG_8B, SEG_8C, SEG_8D, SEG_8E, SEG_8F, SEG_8G };  // 8th digit.

enum SEGS Icon_DP[]  =                                  
    { SEG_DP1, SEG_DP2, SEG_DP3, SEG_DP4, SEG_DP5, SEG_DP6, SEG_DP7, SEG_DP8 };   // Decimal points.

enum SEGS Icon_List[]  =
    {SEG_VO, SEG_CU, SEG_POWER, SEG_UPOWER , SEG_PF ,SEG_CT};

enum SEGS Icon_Check[]  =
    {SEG_AREA, SEG_ADDR, SEG_time, SEG_day,SEG_MONTH, SEG_TODAY,SEG_LASTMONTH, SEG_LASTDAY};

enum SEGS Icon_Unit[]  = 
	 {SEG_PCNT, SEG_v, SEG_i, SEG_KW, SEG_kvar,SEG_WARN, SEG_STATUS};	   //

enum SEGS Icon_Phase[]  =	 
	 { SEG_A, SEG_B, SEG_C,SEG_SUM,SEG_T12};     //

enum SEGS Icon_Tri[] =
     { SEG_T1, SEG_T2, SEG_T3, SEG_T4, SEG_T5, SEG_T6, SEG_T7, SEG_T8, SEG_T9, SEG_T10, SEG_T11, SEG_T12,SEG_T13 };	   //三角符号
	 
enum SEGS Icon_Cap[] =
     { SEG_H1, SEG_H4, SEG_H3, SEG_H2 };  //电池容量

enum SEGS Icon_Gprs[] = 
     { SEG_I1, SEG_I2, SEG_I3, SEG_I4 };  //GPRS

enum SEGS Icon_Online[] = 
     { SEG_PHONE };  //GPRS

enum SEGS Icon_Usb[] = 
     { SEG_USB };

const uint8_t * icons[]= 
{
    Digit_1,    Digit_2,    Digit_3,    Digit_4,	Digit_5,    
    Digit_6,    Digit_7,    Digit_8,	Icon_DP,	Icon_List,  
    Icon_Check, Icon_Unit,  Icon_Phase, Digit_0,	Icon_Tri,
	Icon_Cap,   Icon_Gprs, Icon_Online, Icon_Usb,
};

// Number of segments in each icon.
const uint8_t num_segs[] =
{
    sizeof (Digit_1), sizeof (Digit_2), sizeof (Digit_3), sizeof (Digit_4),sizeof (Digit_5), 
    sizeof (Digit_6), sizeof (Digit_7), sizeof (Digit_8),sizeof (Icon_DP), sizeof (Icon_List), sizeof (Icon_Check), 
    sizeof (Icon_Unit),sizeof (Icon_Phase),sizeof (Digit_0),sizeof (Icon_Tri),sizeof (Icon_Cap),sizeof (Icon_Gprs),
    sizeof (Icon_Online), sizeof(Icon_Usb),
};

/***************************************************************************
 * History:
 * $Log: LCD_VIM808.c,v $
 * Revision 1.3  2005/08/03 18:34:51  tvander
 * Changed copyright to "Teridian Semiconductor Co."
 * Put in alt-mux change in MPU parameters in api_struct.h
 * Revised validity date of cal.c
 *
 * Revision 1.2  2005/02/17 18:32:25  tvander
 * Added automatic check-in logging to all source code.
 *
 * 2003 OCTOBER 30; First Version. 
 * Copyright (C) 2005 Teridian Semiconductor Corp. All Rights Reserved.	   *
 * this program is fully protected by the United States copyright          *
 * laws and is the property of Teridian Semiconductor Corporation.         *
 ***************************************************************************/


#ifndef HT1621_Ful_addr
#define HT1621_Ful_addr (0x80)
#endif

/*********************************************************/
//写d_count位数据d;从低位开始送
/*********************************************************/
static void ht1621_SendBits(unsigned char wr_lcd_buf,unsigned char d_count)
{
 	unsigned char j;
	HT1621_DATA_OUTPUT;
	HT1621_CS_L;
 	for (j=0; j<d_count; ++j)
	{
        
        HT1621_WR_L;					   //3v供电时，write mode大于3.34小于125us，read mode 大于6.67us（5v供电，write mode大于1.67小于125us，read mode 大于3.34us）
     	Ctl_Delay(10);
		if ((wr_lcd_buf & 0x01)==1)				    //置数据位;
			HT1621_DATA_H;
		else
			HT1621_DATA_L;
		HT1621_WR_H;
		Ctl_Delay(10);					  //10us
		wr_lcd_buf =(wr_lcd_buf>>1);  //左移位
	}
	
}

/*********************************************************/
//从data线上读取d_count个数据
/*********************************************************/
void HT1621_Read_Bits(unsigned char * rd_lcd_buf , unsigned char d_count)
{
 	unsigned char j;
	unsigned long read_data;
	HT1621_DATA_INPUT;
	DATA_INPUT_CONFIG;
	HT1621_CS_L;
	HT1621_WR_H;
	
 	for (j=0; j<d_count; ++j)
	{
		HT1621_RD_L;
		Ctl_Delay(10);
		(*rd_lcd_buf)<<=1; 					//准备一个位(读满8个数据时，是否需要处理，看调用是否需要)
		HT1621_RD_H;
		read_data= GPIOPinRead(GPIO_LCD_PORT_BASE, HT1621_DATA);  //WEI:读取的long型数据HT1621_DATA是否在相应位还是在最低位	     
		(*rd_lcd_buf) |= ((read_data&0xff)>>3);	
	}
	HT1621_CS_H;
}

/*********************************************************/
//将4位数据写入到HT1621地址为addr处
/*********************************************************/
void HT1621_WR_Byte(unsigned char addr,unsigned char wr_byte_buf)//将数据data写入HT1621中地址为addr处
{
	if (addr<=(HT1621_Ful_addr-1)) //判断地址是否有效
 	{
		ht1621_SendBits(0x05,3);
		ht1621_SendBits(addr,6);
		ht1621_SendBits(wr_byte_buf,4);
	}
}

/*********************************************************/
//将HT1621地址为addr处的4位数据读出
/*********************************************************/
void HT1621_RD_Byte(unsigned char addr,unsigned char * rd_byte_buf)//读addr地址处的数据，读出的数据存至data，
{
	if (addr<=(HT1621_Ful_addr-1)) //判断地址是否有效
 	{
		ht1621_SendBits(0x03,3);
		ht1621_SendBits(addr,6);
		HT1621_Read_Bits(rd_byte_buf,4);
	}
		(* rd_byte_buf)=(((* rd_byte_buf)&0x0a)>>1)|(((* rd_byte_buf)&0x05)<<1);
		(* rd_byte_buf)=(((* rd_byte_buf)&0x0c)>>2)|(((* rd_byte_buf)&0x03)<<2);
	HT1621_CS_H;
}

/*********************************************************/
//连续写数据至HT1621起始地址为addr，写n个数据，地址递增，注意对地址是否有效以及是否写满判断
//输入需要写入数据的首地址,需要写入的起始地址，考虑是否将返回值输出，以供判断？在此未还用到
/*********************************************************/
void HT1621_WR_Block(unsigned char addr,unsigned char * wr_block_buf,unsigned char n)
{
		if(HT1621_Ful_addr<=(addr+n))
		{
			ht1621_SendBits(0x5,3);
			ht1621_SendBits(addr,6);
			while(n)
			{
				 ht1621_SendBits((*wr_block_buf),4);
				 wr_block_buf++;
				 n--;
			}
		}
		HT1621_CS_H;
}

/*********************************************************/
//连续从HT1621读出数据，起始地址为addr，读n个数据，注意注意对地址是否有效以及是否写满判断
//返回数据存储地址的指针,在此未还用到
/*********************************************************/
void HT1621_RD_Block(unsigned char addr,unsigned char * rd_block_buf,unsigned char n)
{
		if(HT1621_Ful_addr<=(addr+n))
		{
			ht1621_SendBits(0x3,3);
			ht1621_SendBits(addr,6);
			while(n)
			{
				 HT1621_Read_Bits(rd_block_buf,4);
				 rd_block_buf++;
				 n--;
			}
		}
}


/*********************************************************/
//rom全置零，即无显示
/*********************************************************/
void HT1621_NoPrint()
{
	unsigned char i;
	ht1621_SendBits(0x5,3);
	ht1621_SendBits(0x0,6);
	for (i=0x00;i<0x1E;++i)
	{
		ht1621_SendBits(0x00,4);
	}
	HT1621_CS_H;
}

/*********************************************************/
//省电模式，关闭系统振荡器和偏压发生器，LCD显示空白
/*********************************************************/
void HT1621_Sleep()
{
	ht1621_SendBits(0x01,3);
	ht1621_SendBits(0x0,9);
	HT1621_CS_H;
}

/*********************************************************/
//唤醒省电模式，打开系统振荡器和偏压发生器，LCD显示
/*********************************************************/
void HT1621_Wakeup()
{
////	ht1621_SendBits(0x01,3);
////	ht1621_SendBits(0x80,9);
////	HT1621_CS_H;

	HT1621_CS_L;
	ht1621_SendBits(0x01,3);            //送3位命令模式码100 0x80=0b10000000	（0b100）
	ht1621_SendBits(0x18,9);            //系统时钟选用片内RC	（0b000110000）rc 256k
	ht1621_SendBits(0x80,9);            //打开系统振荡器  （0b000000010）sys_en
	ht1621_SendBits(0x94,9);            //（0b001010010）(1/3偏置，4个公共口)
    ht1621_SendBits(0xc0,9);            //打开LCD偏置发生器 （0b000000110）LCD_EN	
	HT1621_CS_H;						 //使用HT1621_Send_Bits函数之后需将片选置高，若在HT1621_Send_Bits函数内加入片选置高语句则不能连续发送数据

}

/*********************************************************/
//rom全置位，即全显
/*********************************************************/
void HT1621_FulPrint()
{
	unsigned char i;
	ht1621_SendBits(0x5,3);
	ht1621_SendBits(0x0,6);
	for (i=0x00;i<0x1E;++i)
	{
		ht1621_SendBits(0x0f,4);
	}
	HT1621_CS_H;
}

/*********************************************************/
//显示一段（输入是一个指向八位数据的指针，其中高两位表示com（00：com3,01：com2,10：com1，11：com0），低六位表示地址（a5a4a3a2a1a0））
/*********************************************************/
void HT1621_Dis_Section(const uint8_t *section)
{
	unsigned char addr_section=(*section)&0x3f;
	unsigned char	data_section=0x01<<(((*section)&0xc0)>>6);//(两位变四位表示com口d0d1d2d3：com0com1com2com3)
	unsigned char data_config;//缓存需要写入数据的地址的当前数据，用于比较判断
	addr_section=((addr_section&0x38)>>3)|((addr_section&0x07)<<3);	//用于反转数据从a5a4a3a2a1a0变为a0a1a2a3a4a5（因为HT1621_Send_Bits()函数定义的是从低位开始输出）
	addr_section=((addr_section&0x24)>>2)|((addr_section&0x09)<<2)|(addr_section&0x12);
	HT1621_RD_Byte(addr_section,(unsigned char *)&data_config);//回读配置，不改变原输出
	data_section|=data_config; 
	HT1621_WR_Byte(addr_section,data_section);
	HT1621_CS_H;	
}

/*********************************************************/
//取消显示某段
/*********************************************************/
void HT1621_UnDis_Section(const uint8_t *section)
{
	unsigned char addr_section=(*section)&0x3f;
	unsigned char data_section=0x01<<(((*section)&0xc0)>>6);//(两位变四位表示com口d0d1d2d3：com0com1com2com3)
	unsigned char data_config_un;//缓存需要写入数据的地址的当前数据，用于比较判断
	addr_section=((addr_section&0x38)>>3)|((addr_section&0x07)<<3);	//用于反转数据
	addr_section=((addr_section&0x24)>>2)|((addr_section&0x09)<<2)|(addr_section&0x12);
	HT1621_RD_Byte(addr_section,(unsigned char *)&data_config_un);
	data_section=(~data_section)&data_config_un;
	HT1621_WR_Byte(addr_section,data_section);
	HT1621_CS_H;	
}

/*********************************************************/
//HT1621初始化
/*********************************************************/
void ht1621_Init()
{

	SysCtlPeripheralEnable(GPIO_LCD_SYSCTL_PERIPH);		//使能HT1621所在端口，然后配置为输出
	HT1621_OUTPUT(HT1621_CS | HT1621_RD | HT1621_WR);		//HT1621.h处的宏定义
    HT1621_CS_L;
	ht1621_SendBits(0x01, 3);            //送3位命令模式码100 0x80=0b10000000	（0b100）
	ht1621_SendBits(0x18, 9);            //系统时钟选用片内RC	（0b000110000）rc 256k
	ht1621_SendBits(0x80, 9);            //打开系统振荡器  （0b000000010）sys_en
	ht1621_SendBits(0x94, 9);            //（0b001010010）(1/3偏置，4个公共口)
    ht1621_SendBits(0xc0, 9);            //打开LCD偏置发生器 （0b000000110）LCD_EN	
	HT1621_CS_H;						 //片选
	ht1621_Write(0x00, 0x00);		
    HT1621_NoPrint();                   //液晶不显示
}  

/*********************************************************/
//显示数字
/*********************************************************/
void ht1621_Write(uint_t nIcon, uint_t nMask)
{
	const uint8_t *seg;
	uint_t i;

	seg = icons[nIcon];	//要显示的第seg个数字
	for (i = 0; i < num_segs[nIcon]; i++)		//笔画数
	{
		if (nMask & 1)//wei:取mask的最低位，为1表示显示该段
		{
			HT1621_Dis_Section(seg);    // Activate segment.
		}
		else			//为0时表示该段不显示，故清显示
		{
			HT1621_UnDis_Section(seg);   // DeActivate segment.
		}
		seg++;
		nMask >>= 1;//循环输出
	}
}

#endif


