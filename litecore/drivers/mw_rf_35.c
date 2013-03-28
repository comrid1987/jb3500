#include <stdio.h>
#include <string.h>

#include <litecore.h>
#include <drivers/mw_rf_35.h>

//--------------------------------------------------------------------------------------
//初始化485_3
//--------------------------------------------------------------------------------------
p_dev_uart init_rs485_3(void)
{
    p_dev_uart rs485_3_dev;
//  StructReaderParam srReaderParam;
    
//  sfs_Read(sfsParam,SFS_READER_PARAM,(uint8 *)&srReaderParam);
    if((rs485_3_dev = uart_Get(2, 1000)) != NULL)
    {
        uart_Config(rs485_3_dev, 9600, UART_PARI_NO, UART_DATA_8D, UART_STOP_1D);
    }
    return rs485_3_dev;
}
//--------------------------------------------------------------------------------------
//发送起始字节－－发送流程
//--------------------------------------------------------------------------------------
void send_35lt_start_byte(p_dev_uart rs485_3_dev)
{
    uint8_t mw_stx = MW_STX;
    
    uart_Send(rs485_3_dev, &mw_stx, 1);
}
//--------------------------------------------------------------------------------------
//接收设备就绪字节－－发送流程
//--------------------------------------------------------------------------------------
uint8_t receive_35lt_idl_byte(p_dev_uart rs485_3_dev)
{
    buf rx_buf = {0};
    uint8_t i;
    
    while (rx_buf->len < 1)
    {
        if(uart_RecData(rs485_3_dev, rx_buf, 500) == SYS_R_TMO)
            break;
    }
    for(i = 0; i < rx_buf->len; i++)
    {
        if(rx_buf->p[i] == MW_DLE)
        buf_Release(rx_buf);
        return 1;            
    }
    buf_Release(rx_buf);
    return 0;
}
//--------------------------------------------------------------------------------------
//发送数据－－发送流程
//--------------------------------------------------------------------------------------
void send_35lt_data(p_dev_uart rs485_3_dev, pStructMWTxData psrTxData)
{
    uint8_t tx_bcc = 0;
    uint8_t i;
    buf tx_buf = {0};
    
    buf_Push(tx_buf,&psrTxData->cTxSeq,1);
    if(psrTxData->cTxSeq == MW_DLE)
    {
        buf_Push(tx_buf,&psrTxData->cTxSeq,1);
    }
    tx_bcc = psrTxData->cTxSeq;
    buf_Push(tx_buf,&psrTxData->cCommand,1);
    if(psrTxData->cCommand == MW_DLE)
    {
        buf_Push(tx_buf,&psrTxData->cCommand,1);
    }    
    tx_bcc ^= psrTxData->cCommand;
    if(psrTxData->cLength == MW_DLE)
    {
        buf_Push(tx_buf,&psrTxData->cLength,1);
    }        
    buf_Push(tx_buf,&psrTxData->cLength,1);
    tx_bcc ^= psrTxData->cLength;

    for(i = 0; i < psrTxData->cLength; i++)
    {
        if(psrTxData->buf->p[i] == MW_DLE)
        {
              buf_Push(tx_buf,&psrTxData->buf->p[i],1);
              buf_Push(tx_buf,&psrTxData->buf->p[i],1);
        }
        else
        {
            buf_Push(tx_buf,&psrTxData->buf->p[i],1);
        }
        tx_bcc ^= psrTxData->buf->p[i];
    }

    buf_Push(tx_buf,&tx_bcc,1);

    tx_bcc = MW_DLE;
    buf_Push(tx_buf,&tx_bcc,1);

    tx_bcc = MW_ETX;
    buf_Push(tx_buf,&tx_bcc,1);
    uart_Send(rs485_3_dev, tx_buf->p, tx_buf->len);
    
    buf_Release(tx_buf);
}
//--------------------------------------------------------------------------------------
//接收起始字节－－接收流程
//--------------------------------------------------------------------------------------
uint8_t receive_35lt_start_byte(p_dev_uart rs485_3_dev)
{
    buf rx_buf = {0};
    uint8_t i;
    
    while (rx_buf->len < 2)
    {
        if(uart_RecData(rs485_3_dev, rx_buf, 6000) == SYS_R_TMO)
            break;
    }
    for(i = 0; i < rx_buf->len; i++)
    {
        if((rx_buf->p[i] == MW_DLE) && (rx_buf->p[i + 1] == MW_STX))
        {
            buf_Release(rx_buf);
            return 1;  
        }
    }
    buf_Release(rx_buf);
    return 0;


}
//--------------------------------------------------------------------------------------
//发送设备就绪字节－－接收流程
//--------------------------------------------------------------------------------------
void send_35lt_dle_byte(p_dev_uart rs485_3_dev)
{
    uint8_t mw_dle = MW_DLE;
    
    uart_Send(rs485_3_dev, &mw_dle, 1);
}
//--------------------------------------------------------------------------------------
//接收数据－－接收流程
//--------------------------------------------------------------------------------------
uint8_t receive_35lt_data(p_dev_uart rs485_3_dev, pStructMWRxData psrRxData)
{
    uint8_t rx_status;
    uint16_t tmo;
    uint8_t rx_bcc = 0;
    uint8_t i;
    
    tmo = 4000 / OS_TICK_MS;
    rx_status = 0;
    while ((tmo--) && (rx_status != 4))
    {
        uart_RecData(rs485_3_dev, psrRxData->buf, 50);
        switch (rx_status)
        {
            case 0:     //接收rxseq,command,len
                if(psrRxData->buf->len > 3)
                {
                    psrRxData->cRxSeq = psrRxData->buf->p[0];
                    if((psrRxData->buf->p[0] == MW_DLE) && (psrRxData->buf->p[1] == MW_DLE))
                    {
                        buf_Cut(psrRxData->buf,0,1);
                    }                    
                    psrRxData->cStatus = psrRxData->buf->p[1];
                    if((psrRxData->buf->p[1] == MW_DLE) && (psrRxData->buf->p[2] == MW_DLE))
                    {
                        buf_Cut(psrRxData->buf,1,1);
                    }
                    psrRxData->cLength = psrRxData->buf->p[2];
                    if((psrRxData->buf->p[2] == MW_DLE) && (psrRxData->buf->p[3] == MW_DLE))
                    {
                        buf_Cut(psrRxData->buf,2,1);
                    }
                    rx_bcc = psrRxData->buf->p[0] ^ psrRxData->buf->p[1] ^ psrRxData->buf->p[2];
                    buf_Remove(psrRxData->buf, 3);
                    rx_status = 1;
                }
                break;
            case 1:     //接收data
                if(psrRxData->buf->len >= psrRxData->cLength)    
                {
                    for(i = 0; i < psrRxData->cLength; i++)
                    {
                        if((psrRxData->buf->p[i] == MW_DLE) && (psrRxData->buf->p[i + 1] == MW_DLE))
                        {
                            buf_Cut(psrRxData->buf,i,1);
                            i--;
                        }
                    }                    
                    rx_status = 2;
                }                
                break;
           case 2:      //接收bcc,接收idl和EXT
                if(psrRxData->buf->len > psrRxData->cLength + 2)
                {
                    for(i = 0; i < psrRxData->cLength; i++)
                    {
                        rx_bcc ^= psrRxData->buf->p[i];
                    }
                    if((psrRxData->buf->p[i] == MW_DLE) && (psrRxData->buf->p[i+1] == MW_DLE))
                    {
                        buf_Cut(psrRxData->buf,i,1);
                    }                    
                    if(( rx_bcc == psrRxData->buf->p[psrRxData->cLength]) &&
                       ( MW_DLE == psrRxData->buf->p[psrRxData->cLength + 1]) &&
                       ( MW_ETX == psrRxData->buf->p[psrRxData->cLength + 2]))
                    {
                        rx_status = 4;
                    }
                    else
                    {
                        rx_status = 0;
                    }         
                }
                break;
           default:
                break;
        }
    }
    return rx_status;
}

uint8_t mw_send_command(p_dev_uart rs485_3_dev,pStructMWRxData psrRxData,pStructMWTxData psrTxData)
{
    uint8_t status = 0;
    
    send_35lt_start_byte(rs485_3_dev);
    if(receive_35lt_idl_byte(rs485_3_dev) == 1)
    {
        send_35lt_data(rs485_3_dev,psrTxData);
    }
	else
	{
		status = 0x11;			 //收不到设备就绪状态
	}

    if(receive_35lt_start_byte(rs485_3_dev) == 1)
    {
        send_35lt_dle_byte(rs485_3_dev);
        if(receive_35lt_data(rs485_3_dev,psrRxData) == 4)
        {
            send_35lt_dle_byte(rs485_3_dev);
            status = psrRxData->cStatus;
        }
		else
		{
			status = 0x33;	 //收不到读卡器发的数据包
		}
    }
	else
	{
		status = 0x22;		  //收不到读卡器发的起始位
	}
    return status;
    
}
//--------------------------------------------------------------------------------------
//发送蜂鸣器响命令
//--------------------------------------------------------------------------------------
uint8_t mw_35lt_beep(p_dev_uart rs485_3_dev,uint8_t *cTxS,uint16_t beep_time)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};
    uint8_t status = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MW_COMMAND_BUZZER;
    psrTxData->cLength = 2;
    buf_Push(psrTxData->buf, (uint8_t *)&beep_time,2);
//  psrTxData->buf->p[0] = (uint8)beep_time;
//  psrTxData->buf->p[1] = (uint8)(beep_time >> 8);

    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);

    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);
    return status;
}
//-------------------------------------------------------------------------------------
//设置读卡器时间
//-------------------------------------------------------------------------------------
uint8_t mw_35lt_set_time(p_dev_uart rs485_3_dev,uint8_t *cTxS)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};
    uint8_t status = 0;
    uint8_t cTemp;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MW_COMMAND_S_TM;
    psrTxData->cLength = 7;
    cTemp = bin2bcd8(rtc_pTm()->tm_year);
    buf_Push(psrTxData->buf, (uint8_t *)&cTemp, 1);
    cTemp = bin2bcd8(rtc_pTm()->tm_wday + 1);
    buf_Push(psrTxData->buf, (uint8_t *)&cTemp, 1);
    cTemp = bin2bcd8(rtc_pTm()->tm_mon + 1);
    buf_Push(psrTxData->buf, (uint8_t *)&cTemp, 1);
    cTemp = bin2bcd8(rtc_pTm()->tm_mday);
    buf_Push(psrTxData->buf, (uint8_t *)&cTemp, 1);
    cTemp = bin2bcd8(rtc_pTm()->tm_hour);
    buf_Push(psrTxData->buf, (uint8_t *)&cTemp, 1);
    cTemp = bin2bcd8(rtc_pTm()->tm_min);
    buf_Push(psrTxData->buf, (uint8_t *)&cTemp, 1);
    cTemp = bin2bcd8(rtc_pTm()->tm_sec);
    buf_Push(psrTxData->buf, (uint8_t *)&cTemp, 1);

    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);

    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);

    return status;
}

//-------------------------------------------------------------------------------------
//选择读卡类别
//-------------------------------------------------------------------------------------
uint8_t mw_35lt_select_card_type( p_dev_uart rs485_3_dev, uint8_t *cTxS, uint8_t card_type )
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint8_t status = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MW_COMMAND_S_CD;
    psrTxData->cLength = 1;
    buf_Push(psrTxData->buf, (uint8_t *)&card_type,1);

    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);

    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return status;
}

//-------------------------------------------------------------------------------------
//标准请求
//-------------------------------------------------------------------------------------
uint8_t mw_35lt_requst_card( p_dev_uart rs485_3_dev, uint8_t *cTxS, uint8_t command )
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint8_t status = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_REQUEST2;    
    psrTxData->cLength = 1;
    buf_Push(psrTxData->buf, (uint8_t *)&command,1);    

    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);

    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return status;
}

//-------------------------------------------------------------------------------------
//防冲突
//-------------------------------------------------------------------------------------
uint8_t mw_35lt_anti_card( p_dev_uart rs485_3_dev, uint8_t *cTxS, uint32_t *cardnumber)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint8_t status = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_ANITI;
    psrTxData->cLength = 5;
    buf_PushData(psrTxData->buf, 0, 1);
	buf_PushData(psrTxData->buf, 0, 1);
	buf_PushData(psrTxData->buf, 0, 1);
	buf_PushData(psrTxData->buf, 0, 1);
	buf_PushData(psrTxData->buf, 0, 1);

    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);

    rt_memcpy(cardnumber, &psrRxData->buf->p[0],4);
   
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return status;
}
//-------------------------------------------------------------------------------------
//选卡
//-------------------------------------------------------------------------------------
uint8_t mw_35lt_select_card( p_dev_uart rs485_3_dev, uint8_t *cTxS, uint32_t cardnumber)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint8_t status = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_GET_CAP;
    psrTxData->cLength = 4;
    buf_Push(psrTxData->buf, (uint8_t *)&cardnumber, 4);

    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);

    return status;
}
//-------------------------------------------------------------------------------------
//复位－－仅对非接触CPU卡有效
//-------------------------------------------------------------------------------------
uint8_t mw_35lt_reset( p_dev_uart rs485_3_dev, uint8_t *cTxS)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint8_t status = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_RESET;
    psrTxData->cLength = 2;  
    buf_PushData(psrTxData->buf, 5,1);
    buf_PushData(psrTxData->buf, 0,1);

    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);

    return status;
}


//-------------------------------------------------------------------------------------
//复位卡－－仅对非接触CPU卡有效
//-------------------------------------------------------------------------------------
uint8_t mw_35lt_reset_cpu( p_dev_uart rs485_3_dev, uint8_t *cTxS)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint8_t status = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_CPU_RESET;
    psrTxData->cLength = 0;  

    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);

    return status;
}

//-------------------------------------------------------------------------------------
//取随机数－－仅对非接触CPU卡有效
//-------------------------------------------------------------------------------------
uint16_t mw_cpu_get_ramd( p_dev_uart rs485_3_dev, uint8_t *cTxS, void *data)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

    uint16_t err_code = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_CPU_APDU;
    psrTxData->cLength = 9;

    buf_PushData(psrTxData->buf, 0,1);      //NAD
    buf_PushData(psrTxData->buf, 0,1);      //CID
    buf_PushData(psrTxData->buf, 0,1);      //PCB
    buf_PushData(psrTxData->buf, 5,1);      //command len

    buf_PushData(psrTxData->buf, 0,1);      //CLA
    buf_PushData(psrTxData->buf, 0x84,1);   //INS
    buf_PushData(psrTxData->buf, 0,1);      //P1
    buf_PushData(psrTxData->buf, 0,1);      //P2
    buf_PushData(psrTxData->buf, 8,1);      //Len
    
    
    if(mw_send_command(rs485_3_dev,psrRxData,psrTxData) == 0)
    {
        err_code = (uint16_t)(psrRxData->buf->p[12] << 8) + (uint16_t)(psrRxData->buf->p[13]);
        memcpy(data, &psrRxData->buf->p[4],8);
    }
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);

 
 
 return err_code;

}

//-------------------------------------------------------------------------------------
//结束CPU卡操作
//-------------------------------------------------------------------------------------
uint8_t mw_35lt_end_cpu( p_dev_uart rs485_3_dev, uint8_t *cTxS)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint8_t status = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_CPU_END;
    psrTxData->cLength = 0;  

    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);

    return status;
}
//-------------------------------------------------------------------------------------
//从cpu外部认证
//-------------------------------------------------------------------------------------
uint16_t mw_cpu_ext_auth( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t p2,uint8_t * pdata)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint16_t err_code = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_CPU_APDU;
    psrTxData->cLength = 17;
 
    buf_PushData(psrTxData->buf, 0,1);      //NAD
    buf_PushData(psrTxData->buf, 0,1);      //CID
    buf_PushData(psrTxData->buf, 0,1);      //PCB
    buf_PushData(psrTxData->buf, 0x0D,1);      //command len

    buf_PushData(psrTxData->buf, 0,1);      //CLA
    buf_PushData(psrTxData->buf, 0x82,1);   //INS
    buf_PushData(psrTxData->buf, 0,1);      //P1
    buf_PushData(psrTxData->buf, p2,1);      //P2
    buf_PushData(psrTxData->buf, 8,1);      //Lc
    buf_Push(psrTxData->buf,pdata,8);
    
    if(mw_send_command(rs485_3_dev,psrRxData,psrTxData) == 0)
    {
        err_code = (uint16_t)(psrRxData->buf->p[4])<<8 + (uint16_t)(psrRxData->buf->p[5]);
    }
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return err_code;
}
//-------------------------------------------------------------------------------------
//CPU内部认证
//-------------------------------------------------------------------------------------
uint16_t mw_cpu_int_auth( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t p2, uint8_t *pData, uint8_t *pOutData)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint16_t err_code = 0;

     (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_CPU_APDU;
    psrTxData->cLength = 0x11;
 
    buf_PushData(psrTxData->buf, 0,1);      //NAD
    buf_PushData(psrTxData->buf, 0,1);      //CID    
    buf_PushData(psrTxData->buf, 0,1);      //PCB
    buf_PushData(psrTxData->buf, 0x0D,1);      //command len

    buf_PushData(psrTxData->buf, 0,1);      //CLA
    buf_PushData(psrTxData->buf, 0x88,1);   //INS
    buf_PushData(psrTxData->buf, 0,1);      //P1
    buf_PushData(psrTxData->buf, p2,1);     //P2
    buf_PushData(psrTxData->buf, 8,1);      //Len  

    buf_Push(psrTxData->buf, pData,8);
    
    if(mw_send_command(rs485_3_dev,psrRxData,psrTxData) == 0)
    {
        err_code = (uint16_t)(psrRxData->buf->p[12])<<8 + (uint16_t)(psrRxData->buf->p[13]);
        memcpy(pOutData, &psrRxData->buf->p[4],8);        
    }
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return err_code;
}

//-------------------------------------------------------------------------------------
//CPU内选择二进制文件
//-------------------------------------------------------------------------------------
uint16_t mw_cpu_select_bin( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t p1,uint8_t p2,uint8_t * pdata, uint8_t len)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint16_t err_code = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_CPU_APDU;
    psrTxData->cLength = 9+len;
 
    buf_PushData(psrTxData->buf, 0,1);      //NAD
    buf_PushData(psrTxData->buf, 0,1);      //CID 
    buf_PushData(psrTxData->buf, 0,1);      //PCB
    buf_PushData(psrTxData->buf, 5 + len,1);//command len

    buf_PushData(psrTxData->buf, 0,1);      //CLA   
    buf_PushData(psrTxData->buf, 0xA4,1);   //INS
    buf_PushData(psrTxData->buf, p1,1);      //P1
    buf_PushData(psrTxData->buf, p2,1);     //P2
    buf_PushData(psrTxData->buf, len,1);     //Len
    buf_Push(psrTxData->buf,pdata,len);    
    
    if(mw_send_command(rs485_3_dev,psrRxData,psrTxData) == 0)
    {
        err_code = (uint16_t)(psrRxData->buf->p[psrRxData->cLength])<<8 + (uint16_t)(psrRxData->buf->p[psrRxData->cLength + 1]);
    }
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return err_code;
}

//-------------------------------------------------------------------------------------
//CPU内读二进制文件
//-------------------------------------------------------------------------------------
uint16_t mw_cpu_read_bin( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t p1,uint8_t p2, buf pdata, uint8_t len)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint16_t err_code = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_CPU_APDU;
    psrTxData->cLength = 9;
 
    buf_PushData(psrTxData->buf, 0,1);      //NAD
    buf_PushData(psrTxData->buf, 0,1);      //CID 
    buf_PushData(psrTxData->buf, 0,1);      //PCB
    buf_PushData(psrTxData->buf, 5,1);      //command len

    buf_PushData(psrTxData->buf, 0,1);      //CLA    
    buf_PushData(psrTxData->buf, 0xB0,1);   //INS
    buf_PushData(psrTxData->buf, p1,1);      //P1
    buf_PushData(psrTxData->buf, p2,1);     //P2
    buf_PushData(psrTxData->buf, len,1);    //Len  
    
    if(mw_send_command(rs485_3_dev,psrRxData,psrTxData) == 0)
    {
        err_code = (uint16_t)(psrRxData->buf->p[4 + len])<<8 + (uint16_t)(psrRxData->buf->p[5 + len]);
        buf_Push(pdata, &psrRxData->buf->p[4],len);
    }
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return err_code;
}
//-------------------------------------------------------------------------------------
//CPU内修改二进制文件
//-------------------------------------------------------------------------------------
uint16_t mw_cpu_update_bin( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t p1,uint8_t p2,buf pdata, uint8_t len)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint16_t err_code = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_CPU_APDU;
    psrTxData->cLength = 9+len;
 
    buf_PushData(psrTxData->buf, 0,1);      //NAD
    buf_PushData(psrTxData->buf, 0,1);      //CID 
    buf_PushData(psrTxData->buf, 0,1);      //PCB
    buf_PushData(psrTxData->buf, 5 + len,1);//command len

    buf_PushData(psrTxData->buf, 0,1);      //CLA   
    buf_PushData(psrTxData->buf, 0xD6,1);   //INS
    buf_PushData(psrTxData->buf, p1,1);     //P1
    buf_PushData(psrTxData->buf, p2,1);     //P2
    buf_PushData(psrTxData->buf, len,1);    //Len
    buf_Push(psrTxData->buf,pdata->p,len);    
    
    if(mw_send_command(rs485_3_dev,psrRxData,psrTxData) == 0)
    {
        err_code = (uint16_t)(psrRxData->buf->p[4])<<8 + (uint16_t)(psrRxData->buf->p[5]);
    }
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return err_code;
}

//-------------------------------------------------------------------------------------
//装载密码
//-------------------------------------------------------------------------------------
uint8_t mw_35lt_load_password( p_dev_uart rs485_3_dev, uint8_t *cTxS, uint8_t sector, uint8_t * card_password)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint8_t status = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MF_COMMAND_LOAD_PASSWORD;
    psrTxData->cLength = 18;
    buf_PushData(psrTxData->buf, 0,1); 
    buf_Push(psrTxData->buf, (uint8_t *)&sector,1); 
    buf_Push(psrTxData->buf, (uint8_t *)&mf_trans_pass[sector],6);
    buf_Push(psrTxData->buf, card_password, 6);
    
    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return status;
}

//-------------------------------------------------------------------------------------
//复位SAM卡
//-------------------------------------------------------------------------------------
uint8_t mw_sam_reset( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t baud)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint8_t status = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MH_SAM_RESET;
    psrTxData->cLength = 1;
 
    buf_Push(psrTxData->buf, (uint8_t *)&baud,1); 
    
    status = mw_send_command(rs485_3_dev,psrRxData,psrTxData);
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return status;
}

//-------------------------------------------------------------------------------------
//从SAM卡取随机数
//-------------------------------------------------------------------------------------
uint16_t mw_sam_get_random( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t len,uint8_t * data)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint16_t err_code = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MH_SAM_COMMAND;
    psrTxData->cLength = 8;
 
    buf_PushData(psrTxData->buf, 0,1);      //NAD
    buf_PushData(psrTxData->buf, 0,1);      //PCB
    buf_PushData(psrTxData->buf, 5,1);      //command len

    buf_PushData(psrTxData->buf, 0,1);      //CLA
    buf_PushData(psrTxData->buf, 0x84,1);   //INS
    buf_PushData(psrTxData->buf, 0,1);      //P1
    buf_PushData(psrTxData->buf, 0,1);      //P2
    buf_PushData(psrTxData->buf, len,1);    //Len
        
    if(mw_send_command(rs485_3_dev,psrRxData,psrTxData) == 0)
    {
        err_code = (uint16_t)(psrRxData->buf->p[3 + len])<<8 + (uint16_t)(psrRxData->buf->p[4 + len]);
        memcpy(data, &psrRxData->buf->p[3],len);
    }
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return err_code;
}
//-------------------------------------------------------------------------------------
//从SAM外部认证
//-------------------------------------------------------------------------------------
uint16_t mw_sam_ext_auth( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t p2,uint8_t * pdata)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint16_t err_code = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MH_SAM_COMMAND;
    psrTxData->cLength = 16;
 
    buf_PushData(psrTxData->buf, 0,1);      //NAD
    buf_PushData(psrTxData->buf, 0,1);      //PCB
    buf_PushData(psrTxData->buf, 5,1);      //command len

    buf_PushData(psrTxData->buf, 0,1);      //CLA
    buf_PushData(psrTxData->buf, 0x82,1);   //INS
    buf_PushData(psrTxData->buf, 0,1);      //P1
    buf_PushData(psrTxData->buf, p2,1);      //P2
    buf_PushData(psrTxData->buf, 8,1);      //Lc
    buf_Push(psrTxData->buf,pdata,8);
    
    if(mw_send_command(rs485_3_dev,psrRxData,psrTxData) == 0)
    {
        err_code = (uint16_t)(psrRxData->buf->p[3])<<8 + (uint16_t)(psrRxData->buf->p[4]);
    }
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return err_code;
}
//-------------------------------------------------------------------------------------
//从SAM内部认证
//-------------------------------------------------------------------------------------
uint16_t mw_sam_int_auth( p_dev_uart rs485_3_dev, uint8_t *cTxS ,uint8_t p2, uint8_t * pdata, uint8_t *pOutData)
{
    StructMWTxData psrTxData[1] = {0};
    StructMWRxData psrRxData[1] = {0};

 // StructTimeBin srTimeBin;
    uint16_t err_code = 0;

    (*cTxS)++;
    psrTxData->cTxSeq = *cTxS;
    psrTxData->cCommand = MH_SAM_COMMAND;
    psrTxData->cLength = 0x18;
 
    buf_PushData(psrTxData->buf, 0,1);      //NAD
    buf_PushData(psrTxData->buf, 0,1);      //PCB
    buf_PushData(psrTxData->buf, 0x15,1);      //command len

    buf_PushData(psrTxData->buf, 0,1);      //CLA
    buf_PushData(psrTxData->buf, 0x88,1);   //INS
    buf_PushData(psrTxData->buf, 0,1);      //P1
    buf_PushData(psrTxData->buf, p2,1);      //P2
    buf_PushData(psrTxData->buf, 16,1);    //Len
    buf_Push(psrTxData->buf,pdata,16);    
    
    if(mw_send_command(rs485_3_dev,psrRxData,psrTxData) == 0)
    {
        err_code = (uint16_t)(psrRxData->buf->p[11])<<8 + (uint16_t)(psrRxData->buf->p[12]);
        memcpy(pOutData,&psrRxData->buf->p[3],8);
    }
    buf_Release(psrTxData->buf);
    buf_Release(psrRxData->buf);


    return err_code;
}
//
//初始化CPU卡操作－－发送射频能量、寻卡、选卡
//
uint8_t mw_35lt_init_cpu(p_dev_uart rs485_3_dev,uint8_t *cTxS,uint32_t *cardnumber)
{
    if(mw_35lt_reset(rs485_3_dev,cTxS) == 0)
	{
        if(mw_35lt_requst_card(rs485_3_dev,cTxS,1) == 0)
        {
            if(mw_35lt_anti_card(rs485_3_dev,cTxS,cardnumber) == 0)
            {
                if(mw_35lt_select_card(rs485_3_dev,cTxS,*cardnumber) == 0)
                {
                    return 0;
                }
            }
        }
	}
    return 1;
}

//循环检测卡是否还在读卡区
uint8_t mw_35lt_check_card(p_dev_uart rs485_3_dev,uint8_t *cTxseq,uint32_t *cardnumber)
{
    uint32_t wTemp;
    
    if(mw_35lt_reset(rs485_3_dev,cTxseq) == 0)
    {
        if(mw_35lt_requst_card(rs485_3_dev,cTxseq,1) == 0)
        {
            if(mw_35lt_anti_card(rs485_3_dev,cTxseq,&wTemp) == 0)
            {
                if(wTemp == *cardnumber)
                {
                    mw_35lt_end_cpu(rs485_3_dev,cTxseq);
                    return 0;
                }
            }
        }
    }
    
    mw_35lt_end_cpu(rs485_3_dev,cTxseq);
    return 1;
}

//终端内部认证
uint8_t mw_35lt_int_auth(p_dev_uart rs485_3_dev, uint8_t *cTxseq,uint32_t *cardnumber)
{
    uint64_t dwRamdon;
    uint64_t dwD1;
    uint8_t cStr[16];

    if(mw_sam_get_random(rs485_3_dev,cTxseq,8,(uint8_t *)&dwRamdon) == 0x9000)//从SAM取随机数
    {
        if(mw_cpu_int_auth(rs485_3_dev, cTxseq, 0x01, (uint8_t *)&dwRamdon, (uint8_t*)&dwD1) == 0x9000)//cpu卡内部认证
        {
            memcpy(cStr,&dwRamdon,8);
            memcpy(&cStr[8],cardnumber,4);
            reverse(&cStr[8],4);
            rt_memset(&cStr[12],0,4);            
            if(mw_sam_int_auth(rs485_3_dev, cTxseq, 0x01,cStr,(uint8_t*)&dwRamdon) == 0x9000)//sam卡内部认证
            {
                if(dwD1 == dwRamdon)
                    return 0;
            }
        }
    }

    return 1;
}
//cpu卡外部认证－－通过后能读写CPU卡
uint8_t mw_35lt_cpu_ext_auth(p_dev_uart rs485_3_dev,uint8_t *cTxseq,uint32_t *cardnumber)
{
    uint64_t dwRamdon;
    uint64_t dwD1;
    uint8_t cStr[16];

    if(mw_cpu_get_ramd(rs485_3_dev,cTxseq,&dwRamdon) == 0x9000)//从CPU卡取随机数
    {
        memcpy(cStr,&dwRamdon,8);
        memcpy(&cStr[8],cardnumber,4);
        reverse(&cStr[8],4);
        rt_memset(&cStr[12],0,4); 
        if(mw_sam_int_auth(rs485_3_dev, cTxseq, 0x03,cStr,(uint8_t*) &dwD1) == 0x9000)
        {
            if(mw_cpu_ext_auth(rs485_3_dev, cTxseq, 0x03,(uint8_t*)&dwD1) == 0x9000)
            {
                return 0;
            }
        }
    }

    return 1;
}

