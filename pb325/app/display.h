#ifndef __APP_DISPLAY_H__
#define __APP_DISPLAY_H__


#ifdef __cplusplus
extern "C" {
#endif





#define LCD_CLEAR           0x01           // Clears entire LCD.
#define LCD_DISPLAY_ON      0x0C           // Display ON.
#define LCD_DISPLAY_OFF     0x08           // Display OFF.
#define MAX_DIGITS          8

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

enum LCD_BIAS { BIAS_3rd_4_STATES, BIAS_3rd_3_STATES,
                 BIAS_half_2_STATES, BIAS_half_3_STATES, 
                 BIAS_STATIC };

enum LCD_CLK { _75HZ, _150HZ, _300HZ, _600HZ };


enum MASK { ZERO = 0x3F, ONE = 0x06, TWO = 0x5B, THREE = 0x4F, FOUR = 0x66,
 	        FIVE = 0x6D, SIX = 0x7D, SEVEN = 0x07, EIGHT = 0x7F, NINE = 0x6F,
            BLANK= 0x00, MINUS = 0x40, OVERFLOW = 0x49,
            DP_0 = 0x00, DP_1 = 0x01, DP_2  = 0x02, DP_3 = 0x04, DP_4 = 0x08, DP_5 = 0x10, DP_6  = 0x20, DP_7 = 0x40, DP_ALL = 0x7F,
	        VO =0x01, CU=0x02, PPOWER=0x04, PUPOWER=0x08, PPF=0x10, CTtype=0x20,	/*List*/
	        IconArea =0x01, IconAddr =0x02, IconTime =0x04, IconDay =0x08, IconTomonth=0x10,IconToday=0x20,IconLastmonth=0x40,IconLastday=0x80,	 /*Check*/	
	        IconPcnt=0x01, IconV=0x02, IconA=0x04, IconKW=0x08, Iconkvar=0x10, IconWarn=0x20, IconStatus=0x40,		/*Unit*/
	        Icon_A =0x01, Icon_B =0x02, Icon_C =0x04, Icon_Sum =0x08, Icon_Work=0x10, 	/*Phase*/
	        IconPhone=0x01,
	        IconUSB=0x01,
	};

enum ICON
{   
 	iDIGIT1, iDIGIT2, iDIGIT3, iDIGIT4, iDIGIT5, iDIGIT6, iDIGIT7, iDIGIT8, 
    iDPS,
	iConList, iConCheck, iConUnit, iConPhase, iDIGIT0, iConTri, 
	iConCap, iConGprs,
	iConOnline, iUsb,
};

enum Dis
{
	Dis_Va, Dis_Vb, Dis_Vc, 
    Dis_Ia, Dis_Ib, Dis_Ic, Dis_In, 
    Dis_Pa, Dis_Pb, Dis_Pc, Dis_Pn, 
    Dis_Qa, Dis_Qb, Dis_Qc, Dis_Qn, 
    Dis_PQa, Dis_PQb, Dis_PQc, Dis_PQn, 
    Dis_Addr1, Dis_Addr2, 
    Dis_Time, Dis_Date
};

enum Engineering_Mode
{
    Master_IP0, Master_IP1, Master_IP2, Master_IP3,
    Master_PORT, 
    Net_IP0, Net_IP1, Net_IP2, Net_IP3, 
    Net_HB,
    VER
};



//External Functions
void tsk_Display(void *args);



#ifdef __cplusplus
}
#endif

#endif

