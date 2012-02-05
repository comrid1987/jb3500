#include "IR.h"
#include "temp.h"


void ir_send_byte(unsigned char byte)		  //发送1字节数据处理
{
	int i;
	TimerEnable(ir_timer_base, TIMER_A);
	ir_send_l;
	while (!(TimerIntStatus(ir_timer_base,0) & TIMER_TIMA_TIMEOUT));
	TimerIntClear(ir_timer_base,TIMER_TIMA_TIMEOUT);
	for(i=0;i<=7;i++)
	{
		if(byte&0x01==1)
		{
		ir_send_h;
		}
		else
		{
		ir_send_l;
		}
		byte>>=1;
	while (!(TimerIntStatus(ir_timer_base,0) & TIMER_TIMA_TIMEOUT));
	TimerIntClear(ir_timer_base,TIMER_TIMA_TIMEOUT);		
	}
	ir_send_h;
	while (!(TimerIntStatus(ir_timer_base,0) & TIMER_TIMA_TIMEOUT));
	TimerIntClear(ir_timer_base,TIMER_TIMA_TIMEOUT);
	TimerDisable(ir_timer_base, TIMER_A);

}

void ir_Send_data(void)	  //发送帧数据处理
{
	unsigned char i;

	if (ir_debug)
	{
		ir_send_length=0x0E;
		ir_send_buf[0]=0x68;  								//起始位
		ir_send_buf[1]=0x86;			//地址码
		ir_send_buf[2]=0x02;			//
		ir_send_buf[3]=0x16;			//
		ir_send_buf[4]=0x16;			//
		ir_send_buf[5]=0x68;			//
		ir_send_buf[6]=0x02;			//
		ir_send_buf[7]=0x85;								//起始位
		ir_send_buf[8]=0x86;			//地址码
		ir_send_buf[9]=0x02;			//
		ir_send_buf[10]=0x16;			//
		ir_send_buf[11]=0x16;			//
		ir_send_buf[12]=0xbf;			//
		ir_send_buf[13]=0x16;			//
	}
	if(ir_send_length)
	{
		for(i=0;i<=(ir_send_length-1);i++ )
		{
			ir_status=0xe0;
			ir_send_byte(ir_send_buf[i]);
//			TimerDisable(ir_pwm_timer_base, TIMER_A);
//			SysCtlDelay(5 * (SysCtlClockGet()/3000));
		}
	}
//	ir_send_h;  //作为延时用
//	TimerDisable(ir_timer_base, TIMER_A);
}

void ir_send(void)
{
	IntDisable(INT_GPIOE);
	SysCtlDelay(100 * (SysCtlClockGet()/3000));		//延时等待接收器进入接收状态（用延时发送方式时需此延时）
	ir_Send_data();		//启动发送程序
	ir_receive_counter=0x00;
	ir_send_flag=0x00;			//关闭发送启动标志位
	ir_status=0x00;				//转为接收状态
//	SysCtlDelay(500 * (SysCtlClockGet()/3000));
	IntEnable(INT_GPIOE);
}

void ir_receive_byte(void)
{
	volatile long  rd_pin=0x00;
	unsigned char i,i2;	
  	TimerEnable(ir_timer_base, TIMER_A);			//启动定时器
	for(i=0;i<=9;i++)
	{
		rd_pin = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4) & GPIO_PIN_4;
		if(rd_pin == 0)
		{	 
			if( (i >= 1) && (i <= 8) )
			{
				ir_temp_byte >>=1;
				ir_temp_byte &= 0x7F;
			}
			ir_temp_data[i]=0x00;
		}
		else 
		{	 
			if( (i >= 1) && (i <= 8) )
			{
				ir_temp_byte >>=1;
				ir_temp_byte |= 0x80;
			}
			i2++;
			ir_temp_data[i]=0x01; 
		}
		while (!(TimerIntStatus(ir_timer_base,0) & TIMER_TIMA_TIMEOUT));
		TimerIntClear(ir_timer_base,TIMER_TIMA_TIMEOUT); 
	}
	TimerDisable(ir_timer_base, TIMER_A);			//启动定时器
	i2%=2;
	i2=ir_temp_data[9];
}


void ir_receive(void)
{
	if(ir_receive_counter<ir_receive_length)
	{
		ir_receive_flag=0x01; 				//重置接收标志位
		ir_temp_byte=0x00;
		ir_receive_byte();
		ir_receive_buf[ir_receive_counter]=ir_temp_byte;
		ir_receive_counter++;
		ir_status=0x01;
	}
}


void ir_receive_int(void)		   //接收中断服务程序
{
	long  rd_pin=0x00;
    unsigned long ul_int_staus=0x00;
    ul_int_staus = GPIOPinIntStatus(GPIO_PORTE_BASE, GPIO_PIN_4);	   	// 读取中断源
	GPIOPinIntClear(GPIO_PORTE_BASE, ul_int_staus);	  					// 清除中断源
	SysCtlDelay(416 * (SysCtlClockGet()/3000000));	 					//判断是否为干扰
	rd_pin = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4) & GPIO_PIN_4;	   	//读取接收引脚电位
	if (rd_pin==0x00)
    {
		IntDisable(INT_GPIOE);
		ir_receive();
		IntEnable(INT_GPIOE);
	}
	
}

void ir_receive_deal(void)
{
		//帧长度控制
		if ((ir_receive_counter==0x07) && (ir_receive_flag != 0x00))
		{
			if(ir_receive_buf[ir_receive_counter]==0x11)
			{
				if(ir_receive_buf[ir_receive_counter-1]==0x01)
				{
					ir_receive_length=0x17;				//设置校表参数时的接收长度
				}
				else
				{
					ir_receive_length=0x0c;				//读校表参数时的接收长度
				}
			}
			else if ((ir_receive_buf[ir_receive_counter]==0x05) && (ir_receive_buf[ir_receive_counter-1]==0x01))
			{
				ir_receive_length=0x0e;				   //设置地址时的接收长度
			}
			else if ((ir_receive_buf[ir_receive_counter]==0x03)&& (ir_receive_buf[ir_receive_counter-1]==0x01))
			{
				ir_receive_length=0x10;				   //设置日期时的接收长度
			}
			else if ((ir_receive_buf[ir_receive_counter]==0x0a)&& (ir_receive_buf[ir_receive_counter-1]==0x01))
			{
				ir_receive_length=0x0c;				   //设置变比时的接收长度
			}

		}
		ir_receive_flag=0x01;
		if ((ir_receive_counter>=ir_receive_length) || (ir_receive_flag==0x00))		//接收错误或者接收完毕
		{
			IntDisable(INT_GPIOE);
			if(ir_receive_flag!=0x00)			//正常接收
			{
				ir_status=0x10;				//置发送控制标志位
			}
			ir_receive_counter=0x00;		   	//清空接收计数器
			ir_receive_length=0x0b;		   	//恢复默认接收帧长度
		}
}

void ir_start()
{

	if (ir_status != 0x00)
	{
		ir_receive_deal();
		if(ir_status >= 0x10)
		{
			ir_protocol();	//协议解析子程序
			if(ir_status >= 0x10)
			{
				ir_send();
			}
		}
		ir_status=0x00;
	}

}


