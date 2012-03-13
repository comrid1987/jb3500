#include <litecore.h>
#include "system.h"
#include "para.h"
#include "acm.h"
#include "display.h"


//Private Consts
const uint8_t disp_tblHex[] = {
	0x3F, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7C, 0x39, 0x5e, 0x79, 0x71,
};


void Display_Number (signed long d, unsigned char c, unsigned char p)			//wei:p是表示可显示数字0的位数
{
    unsigned char non_zero = FALSE;             // Suppress leading zeros.
    unsigned short i, digit[ MAX_DIGITS ];
	unsigned char d_temp = 0;

//    if (beat)
//    ht1621_Write (iDPS, LCD_Data_Read (iDPS) | DP_7); // LCD XFER Beat.

//    beat ^= 1;

    if (d < 0)
    {
       ht1621_Write (c, MINUS);     // Minus sign, make room for sign. //wei:显示负号
       d_temp=1;
	   d = -d;
    }
//	c--;
    for (i =1; i <=c; i++)			// Display upto 'c' digits.
    {                                   
       digit[ c - i ] = d % 10;			//取d的个位赋值到digit数组
       d /= 10;							//d以十进制格式右移一位
    }

    for(i=(c-d_temp);i<=8;i++)
	{  
	   ht1621_Write ((c-d_temp) , BLANK);		//不显示位
	}
	if (0 == d)
    {
//		p = c - p - 1;
		p = c - p ;
		for (i = 0; i < c; i++)
		{
			if (digit[ i ] || non_zero || (i >= p))	   //p是用来控制显示后面的0（可用来确定精确度），从首位不为0开始显示
			{
				ht1621_Write (c - i - 1, disp_tblHex[digit[i]]);
				non_zero = TRUE;
			}
			else
			{
				ht1621_Write (c - i - 1, BLANK);
			}
		}
	}
	else  									// Number overflows display.
    {
		for (i = 0; i < c; i++)
		{
	      ht1621_Write (i, OVERFLOW);
    	}
	}
}

static void Display_Vrms (unsigned long Vrms)
{
	ht1621_Write (iConCheck, BLANK);
	ht1621_Write (iConList, VO);			
	ht1621_Write (iConUnit, IconV);
//	unsigned short p=*a;
//	ht1621_Write (iCons, SEG_V);    			// No Colons.
    ht1621_Write (iDPS, DP_4);        		// Three decimal places.
    Display_Number (Vrms, 6, 4);
}

static void Display_Irms (unsigned long Irms)
{
//	ht1621_Write (iCons, SEG_A);    		// No Colons.
	ht1621_Write (iConList, CU);
	ht1621_Write (iConUnit, IconA);
    ht1621_Write (iDPS, DP_4);        	// Three decimal places.
    Display_Number (Irms, 6, 4);     
}


static void Display_Power_P (unsigned long Power)
{
	ht1621_Write (iConList, PPOWER);
	ht1621_Write (iConUnit, IconKW);
	ht1621_Write (iDPS, DP_4);
	Display_Number (Power, 6, 4);
//	if(*d<0) *d=-(*d);
//	{
//	ht1621_Write(iDPS, DP_4);        				// Three decimal places.	 4
//	}
//	Display_Number (Power ((U08x *) d)/10, 6, 3);  	// Display upto six digits.修改
//	Display_Number (Power ((U08 *) d)*36/10, 6, 3);		//test
}    

static void Display_Power_Q (unsigned long Power)
{
	ht1621_Write (iConList, PUPOWER);
	ht1621_Write (iConUnit, Iconkvar);
	ht1621_Write (iDPS, DP_4);
	Display_Number (Power, 6, 4);
//	if(*d<0) *d=-(*d);
//	{
//	ht1621_Write(iDPS, DP_4);        				// Three decimal places.	 4
//	}
//	Display_Number (Power ((U08x *) d)/10, 6, 3);  	// Display upto six digits.修改
//	Display_Number (Power ((U08 *) d)*36/10, 6, 3);		//test
} 

static void Display_Time (struct tm *DisTime)
{

    ht1621_Write (iDIGIT6, disp_tblHex[DisTime->tm_hour/10]);
	ht1621_Write (iDIGIT5, disp_tblHex[DisTime->tm_hour%10]);
	ht1621_Write (iDIGIT4, disp_tblHex[DisTime->tm_min/10]);
	ht1621_Write (iDIGIT3, disp_tblHex[DisTime->tm_min%10]);
	ht1621_Write (iDIGIT2, disp_tblHex[DisTime->tm_sec/10]);
	ht1621_Write (iDIGIT1, disp_tblHex[DisTime->tm_sec%10]);
}

static void Display_Date (struct tm *DisTime)
{	
	if(DisTime->tm_year >= 100)
	{
		ht1621_Write (iDIGIT6, disp_tblHex[(DisTime->tm_year-(DisTime->tm_year/100)*100)/10]);
	}
	else 
	{
		ht1621_Write (iDIGIT6, disp_tblHex[DisTime->tm_year/10]);
	}
    DisTime->tm_mon += 1;
	ht1621_Write (iDIGIT5, disp_tblHex[DisTime->tm_year%10]);
	ht1621_Write (iDIGIT4, disp_tblHex[DisTime->tm_mon/10]);
	ht1621_Write (iDIGIT3, disp_tblHex[DisTime->tm_mon%10]);
	ht1621_Write (iDIGIT2, disp_tblHex[DisTime->tm_mday/10]);
	ht1621_Write (iDIGIT1, disp_tblHex[DisTime->tm_mday%10]);
}

static void Display_Addr(uint16_t addr_4)
{
	uint8_t  tmp[4];
	uint8_t  i;	
	ht1621_Write(iDPS,BLANK);
	ht1621_Write(iDIGIT7,BLANK);
	ht1621_Write(iDIGIT6,BLANK);
	ht1621_Write(iDIGIT5,BLANK);	
	tmp[3]=(unsigned char)((addr_4&0xF000)>>12);	
	tmp[2]=(unsigned char)((addr_4&0x0F00)>>8);
	tmp[1]=(unsigned char)((addr_4&0x00F0)>>4);
	tmp[0]=(unsigned char)((addr_4&0x000F));
	for(i = 0; i <4; i++)
	{
		ht1621_Write(i,disp_tblHex[tmp[i]]);
			
	}
}

static void Display_BCD_Addr(uint32_t addr_5)
{
	uint8_t tmp[5];
	uint_t i;

	ht1621_Write(iDPS, BLANK);
	ht1621_Write(iDIGIT7, BLANK);
	ht1621_Write(iDIGIT6, BLANK);
    addr_5 = bin2bcd32(addr_5) & 0x000FFFFF;
	tmp[4]=(unsigned char)((addr_5&0xF0000)>>16);	
	tmp[3]=(unsigned char)((addr_5&0x0F000)>>12);	
	tmp[2]=(unsigned char)((addr_5&0x00F00)>>8);
	tmp[1]=(unsigned char)((addr_5&0x000F0)>>4);
	tmp[0]=(unsigned char)((addr_5&0x0000F));
	for (i = 0; i <5; i++)
		ht1621_Write(i, disp_tblHex[tmp[i]]);
}

static void disp_Handle(uint_t nSel)
{
	int nTemp;
	t_afn04_f85 xF85;
	t_acm_rtdata *pD = &acm_rtd;

	ht1621_Write(iDIGIT8, BLANK);
	ht1621_Write(iDIGIT7, BLANK);
#if MODEM_ENABLE
    nTemp = modem_GetSignal();
    if((nTemp > 0)&&(nTemp != 99)){
        if(nTemp < 2)
            ht1621_Write(iConGprs, 0x01);
        if((nTemp < 5)&&(nTemp >= 2))
            ht1621_Write(iConGprs, 0x03);
        if((nTemp < 10)&&(nTemp >= 5))
             ht1621_Write(iConGprs, 0x07);
        if(nTemp >= 10)
             ht1621_Write(iConGprs, 0x0F);
    }
	else
		ht1621_Write(iConGprs, BLANK);
	if (rcp_IsLogin() == SYS_R_OK)
		ht1621_Write(iConOnline, IconPhone);
	else
		ht1621_Write(iConOnline, BLANK);
	switch(modem_GetState()) {
	case MODEM_S_RESET:
		ht1621_Write(iConTri,BLANK);	  //wei：三角显示，高三位必为0
		break;
	case MODEM_S_POWEROFF:
		ht1621_Write(iConTri,0x01); 	 //wei：三角显示，高三位必为0
		break;
	case MODEM_S_POWERON:
		ht1621_Write(iConTri,0x03); 	 //wei：三角显示，高三位必为0
		break;
	case MODEM_S_INIT:
		ht1621_Write(iConTri,0x07); 	 //wei：三角显示，高三位必为0
		break;
	case MODEM_S_WAITDIAL:
		ht1621_Write(iConTri,0x0F); 	 //wei：三角显示，高三位必为0
		break;
	case MODEM_S_READY:
		ht1621_Write(iConTri,0x1F); 	 //wei：三角显示，高三位必为0
		break;
	case MODEM_S_ONLINE:
		ht1621_Write(iConTri,0x3F); 	 //wei：三角显示，高三位必为0
		break;
	default:
		ht1621_Write(iConTri,BLANK);	  //wei：三角显示，高三位必为0
		break;
	}
#endif
	//ht1621_Write(iConUnit,Usb);		//wei：Usb显示
	switch (nSel) {
    case Dis_Va:		//Vrms_A
    	ht1621_Write(iConPhase, Icon_A);	
		nTemp = pD->vol[0] * 1000;
		Display_Vrms(nTemp);			
		break;
	case Dis_Vb:		//Vrms_B
		ht1621_Write(iConPhase, Icon_B);	
		nTemp = pD->vol[1] * 1000;
		Display_Vrms(nTemp);
		break;
	case Dis_Vc:		//Vrms_C
		ht1621_Write(iConPhase, Icon_C);	
		nTemp = pD->vol[2] * 1000;
		Display_Vrms(nTemp);
		break;
	case Dis_In:		//	Irms_ABC
		ht1621_Write(iConPhase, Icon_Sum);
		nTemp = pD->cur[3] * 1000;
		Display_Irms(nTemp);	
		break;						 
	case Dis_Ia:		// Irms_A
		ht1621_Write(iConPhase, Icon_A);
		nTemp = pD->cur[0] * 1000;
		Display_Irms(nTemp);
		break;
	case Dis_Ib:		// Irms_B
		ht1621_Write(iConPhase, Icon_B);
		nTemp = pD->cur[1] * 1000;
		Display_Irms(nTemp);		
		break;
	case Dis_Ic:		//	Irms_C
		ht1621_Write(iConPhase, Icon_C);	
		nTemp = pD->cur[2] * 1000;
		Display_Irms(nTemp);		
		break;
	case Dis_Pa:		//P_A
        ht1621_Write(iConPhase, Icon_A);
		nTemp = pD->pp[1] * 1000;
		Display_Power_P(nTemp);		
        break;
	case Dis_Pb:		//P_B
        ht1621_Write(iConPhase, Icon_B);
		nTemp = pD->pp[2] * 1000;
		Display_Power_P(nTemp);
        break;
    case Dis_Pc:		//P_C
        ht1621_Write (iConPhase, Icon_C);		   
		nTemp = pD->pp[3] * 1000;
		Display_Power_P(nTemp); 
        break;
    case Dis_Pn:		//P_ABC
        ht1621_Write(iConPhase, Icon_Sum);		   
		nTemp = pD->pp[0] * 1000;
		Display_Power_P(nTemp); 
        break;
	 case Dis_Qa:		//Q_A
		ht1621_Write(iConPhase, Icon_A);
		nTemp = pD->pq[1] * 1000;
		Display_Power_Q(nTemp); 	
        break;
    case Dis_Qb:		//Q_B
        ht1621_Write(iConPhase, Icon_B);	
		nTemp = pD->pq[2] * 1000;
		Display_Power_Q(nTemp);
        break;
    case Dis_Qc:		//Q_C
        ht1621_Write(iConPhase, Icon_C);	
		nTemp = pD->pq[3] * 1000;
		Display_Power_Q(nTemp);
        break;
	 case Dis_Qn:		//Q_ABC
		ht1621_Write(iConPhase, Icon_Sum);
		nTemp = pD->pq[0] * 1000;
		Display_Power_Q(nTemp); 	
        break;
	case Dis_PQa:		//PQ_A
	    ht1621_Write(iConUnit, BLANK);
		ht1621_Write(iDPS, DP_4);			
		nTemp = pD->cos[1] * 1000;
		ht1621_Write(iConList, PPF);
        ht1621_Write(iConPhase, Icon_A);						
		Display_Number(nTemp, 4, 4);  // Display upto six digits.
	    break;
	case Dis_PQb:		//PQ_B
		ht1621_Write(iDPS, DP_4);			
		nTemp = pD->cos[2] * 1000;
		ht1621_Write(iConList, PPF);
        ht1621_Write(iConPhase, Icon_B);						
		Display_Number(nTemp, 4, 4);  // Display upto six digits.
	    break;
	case Dis_PQc:		//PQ_C
		ht1621_Write(iDPS, DP_4);			
		nTemp = pD->cos[3] * 1000;
		ht1621_Write(iConList, PPF);
        ht1621_Write(iConPhase, Icon_C);						
		Display_Number(nTemp, 4, 4);  // Display upto six digits.			
		break;		
	case Dis_PQn:		//PQ_ABC
		ht1621_Write(iDPS, DP_4);			
		nTemp = pD->cos[0] * 1000;
		ht1621_Write(iConList, PPF);
		ht1621_Write(iConCheck, BLANK);
        ht1621_Write(iConPhase, Icon_Sum);						
		Display_Number(nTemp, 4, 4);  // Display upto six digits.			
		break;
	case Dis_Addr1:
		ht1621_Write(iConPhase, BLANK);
		ht1621_Write(iConList, BLANK);
		ht1621_Write(iDPS, BLANK);
		ht1621_Write(iDIGIT6, BLANK);
		ht1621_Write(iDIGIT5, BLANK);
		ht1621_Write(iConCheck, IconArea);
		icp_ParaRead(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
		Display_Addr(xF85.area);
		break;
	case Dis_Addr2:
		ht1621_Write(iConPhase, BLANK);
		ht1621_Write(iConList, BLANK);
		ht1621_Write(iDPS, BLANK);
		ht1621_Write(iDIGIT5, BLANK);
		ht1621_Write(iDIGIT6, BLANK);
		ht1621_Write(iConCheck, IconAddr);	
		icp_ParaRead(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
        Display_BCD_Addr(xF85.addr);
		break;
	case Dis_Time:		//Time
		ht1621_Write(iConPhase, BLANK);
		ht1621_Write(iConList, BLANK);			
		ht1621_Write(iConCheck, IconTime);
		ht1621_Write(iDPS, DP_1 | DP_3 | DP_5 | DP_6);
		Display_Time(rtc_pTm());
        break;
	case Dis_Date:		//Date
	    ht1621_Write(iConUnit, BLANK);
	    ht1621_Write(iConPhase, BLANK);
		ht1621_Write(iConList, BLANK);
	    ht1621_Write(iConCheck, IconDay);
		ht1621_Write(iDPS, DP_5 | DP_3);
		Display_Date(rtc_pTm());
		break;
	default:
		break;
    }
}


void tsk_Display(void *args)
{
	os_que que;
	uint_t nMount = 0, nCnt = 0, nBlCnt = 0, nCycle = 0, nSel = 21, nKey;
	time_t tTime;

    Display_Number(bcd2bin16(VER_SOFT), 8, 4);
	que = os_que_Wait(QUE_EVT_KEYBOARD, NULL, 1000);
	if (que != NULL) {
		if (que->data->val == 1)
			CLRBIT(g_sys_status, 0);
		os_que_Release(que);
	}
    for (; ; ) {
		if (tTime != rtc_GetTimet()) {
			tTime = rtc_GetTimet();
			nBlCnt += 1;
			nCycle += 1;
			nCnt += 1;
			if (nCycle > 5) {
				nCycle = 0;
				nSel = cycle(nSel, 0, 22, 1);
			}
			if (nBlCnt > 30) {
				nBlCnt = 0;
				LCD_BL(0);
			}
		}
 		que = os_que_Wait(QUE_EVT_KEYBOARD, NULL, 1000);
		if (que != NULL) {
			nBlCnt = 0;
			nCycle = 0;
			nKey = que->data->val;
			os_que_Release(que);
			switch (nKey) {
			case 1:
				nSel = cycle(nSel, 0, 22, 1);
				LCD_BL(1);
				break;
			case 2:
				nSel = cycle(nSel, 0, 22, -1);
			    LCD_BL(1);
				break;
			case 3:
				LCD_BL(0);
				if (nMount == 0)
					break;
				if (sys_IsUsbFormat() != SYS_R_OK)
					break;
				icp_UdiskLoad();
				break;
			default:
				break;
			}
		}
 		if ((nCnt & 7) == 0)
			ht1621_Init();
		disp_Handle(nSel);
		if (fs_usb_IsReady() == SYS_R_OK) {
			ht1621_Write(iUsb, IconUSB);
			nMount = 1;
			if (sys_IsUsbFormat() == SYS_R_OK)
				data_Copy2Udisk();
		} else {
			ht1621_Write(iUsb, BLANK);
			if (nMount) {
				fs_usb_Unmount();
				fs_usb_Mount();	
			}
			nMount = 0;
		}
	}
}



