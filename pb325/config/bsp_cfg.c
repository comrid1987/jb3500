

//-------------------------------------------------------------------------
//GPIO Functions
//-------------------------------------------------------------------------
#if GPIO_ENABLE
	t_gpio_def _tbl_bspGpio[] = {
		//LED
		GPIO_T_HC595,	GPIO_P1,	0,		GPIO_M_OUT_PP,		GPIO_INIT_NULL,		//run LED
		GPIO_T_HC595,	GPIO_P1,	1,		GPIO_M_OUT_PP,		GPIO_INIT_NULL,		//Udisk LED
		GPIO_T_HC595,	GPIO_P1,	6,		GPIO_M_OUT_PP,		GPIO_INIT_NULL,		//LCD_BL
		GPIO_T_HC595,	GPIO_P1,	7,		GPIO_M_OUT_PP,		GPIO_INIT_NULL,		//BEEP
	};
	tbl_gpio_def tbl_bspGpio[] = {_tbl_bspGpio, ARR_ENDADR(_tbl_bspGpio)};
#endif

//-------------------------------------------------------------------------
//Serial Parallel Interface Functions
//-------------------------------------------------------------------------
#if HC138_ENABLE
	t_gpio_def _tbl_bspHC138[] = {
		GPIO_T_INT,		GPIO_P1,	23,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,
		GPIO_T_INT,		GPIO_P1,	24,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,
		GPIO_T_INT,		GPIO_P1,	25,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,
		GPIO_T_INT,		GPIO_P0,	21,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,
	};
	tbl_gpio_def tbl_bspHC138[] = {_tbl_bspHC138, ARR_ENDADR(_tbl_bspHC138)};
#endif

/************************************************************************** 
H165_LE      -> PB2
H165_CLK    -> PB4
H165_DIN    -> PC2      //PD2
by arnold
**************************************************************************/
#if HC165_ENABLE
	t_gpio_def _tbl_bspHC165[] = {
		GPIO_T_INT, 	GPIO_P1,	2,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,
		GPIO_T_INT,		GPIO_P1,	4,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,
		GPIO_T_INT, 	GPIO_P3,	2,		GPIO_M_IN_FLOAT,	GPIO_INIT_NULL,
	};
	tbl_gpio_def tbl_bspHC165[] = {_tbl_bspHC165, ARR_ENDADR(_tbl_bspHC165)};
#endif

/************************************************************************** 
H595_OE     -> PB6
H595_CLK    -> PB4
H595_RCK    -> PB5
H595_SER    -> PB7      //PB0
0:          LED_RUN
1:          LED_UDISK
2:          BAT_CTRL
3:          G_OFF
4:          G_RST   
5:          G_IGT
6:          LCD_BL
7:          BEEP
by arnold
**************************************************************************/
#if HC595_ENABLE
    t_gpio_def _tbl_bspHC595[] = {
        GPIO_T_INT,		GPIO_P1,	6,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,
        GPIO_T_INT,		GPIO_P1,	4,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,
        GPIO_T_INT,		GPIO_P1,	5,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,
        GPIO_T_INT,		GPIO_P1,	0,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,
    };
    tbl_gpio_def tbl_bspHC595[] = {_tbl_bspHC595, ARR_ENDADR(_tbl_bspHC595)};
#endif
//-------------------------------------------------------------------------

#if HT1621_ENABLE
    t_gpio_def _tbl_bspHT1621[] = {
        GPIO_T_INT,		GPIO_P1,	6,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,		//CS
        GPIO_T_INT,		GPIO_P1,	4,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,		//RD
        GPIO_T_INT,		GPIO_P1,	5,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,		//WR
        GPIO_T_INT,		GPIO_P1,	0,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,		//DATA
    };
    tbl_gpio_def tbl_bspHT1621[] = {_tbl_bspHT1621, ARR_ENDADR(_tbl_bspHT1621)};
#endif

#if TDK6515_ENABLE
	t_gpio_def _tbl_bspTdk6515[] = {
		GPIO_T_INT,		GPIO_P1,	3,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH, 	//SAMP_RST
        GPIO_T_INT,		GPIO_P2,	5,		GPIO_M_IN_FLOAT,    GPIO_INIT_NULL, 	//SAMP_JL
		GPIO_T_INT,		GPIO_P1,	1,		GPIO_M_IN_FLOAT,	GPIO_INIT_NULL, 	//SAMP_XB
	};
	tbl_gpio_def tbl_bspTdk6515[] = {_tbl_bspTdk6515, ARR_ENDADR(_tbl_bspTdk6515)};
#endif

#if BATTERY_ENABLE
	t_gpio_def _tbl_bspBattery[] = {
		//Battery switch
		GPIO_T_HC595,   GPIO_P0,	2,		GPIO_M_OUT_PP,		GPIO_INIT_LOW,
		//Power Probe
		GPIO_T_HC165,	GPIO_P0,	2,		GPIO_M_IN_PU,		GPIO_INIT_NULL,	    
		//Battery Probe
		GPIO_T_HC165,	GPIO_P0,	3,		GPIO_M_IN_PU,		GPIO_INIT_NULL,	    
		//Battery voltage
		GPIO_T_INT,	    GPIO_P3,	3,	 	GPIO_M_IN_ANALOG,	GPIO_INIT_NULL,     //AD4
	};
	tbl_gpio_def tbl_bspBattery[] = {_tbl_bspBattery, ARR_ENDADR(_tbl_bspBattery)};
#endif

/**************************************************************************
KEY_PAGE -> 74HC165_1_0
KEY_SET/WK ->74HC165_1_1
by arnold
**************************************************************************/

#if KEY_ENABLE
	t_gpio_def _tbl_bspKeypad[] = {
		//KEYBOARD
		GPIO_T_HC165,	GPIO_P0,	0,		GPIO_M_IN_PU,		GPIO_INIT_NULL,			//74HC165_1 第一脚KEY_PAGE
		GPIO_T_HC165,	GPIO_P0,	1,		GPIO_M_IN_PU,		GPIO_INIT_NULL,			//74HC165_1 第二脚KEY_SET/WK
	};
	tbl_gpio_def tbl_bspKeypad[] = {_tbl_bspKeypad, ARR_ENDADR(_tbl_bspKeypad)};
#endif

#if PULSE_COL_ENABLE
	t_gpio_def _tbl_bspPulse[] = {
		GPIO_T_HC165,	GPIO_P0,	7,		GPIO_M_IN_PU,		GPIO_INIT_NULL, 		//0 pulse1
		GPIO_T_HC165,	GPIO_P0,	6,		GPIO_M_IN_PU,		GPIO_INIT_NULL, 		//1 pulse2
		GPIO_T_HC165,	GPIO_P0,	5,		GPIO_M_IN_PU,		GPIO_INIT_NULL, 		//2 pulse3 
	};
	tbl_gpio_def tbl_bspPulse[] = {_tbl_bspPulse, ARR_ENDADR(_tbl_bspPulse)};
#endif



//-------------------------------------------------------------------------
//External Parallel Interface Functions
//-------------------------------------------------------------------------
#if EPI_SOFTWARE
	t_gpio_def _tbl_bspEpiData[] = {
	};
	tbl_gpio_def tbl_bspEpiData[] = {_tbl_bspEpiData, ARR_ENDADR(_tbl_bspEpiData)};
#endif

#if NORFLASH_ENABLE
#endif

#if EXTSRAM_ENABLE
#endif

#if NANDFLASH_ENABLE
#endif

#if GUI_ENABLE
	t_gpio_def _tbl_bspLcdCtrl[] = {
		GPIO_T_INT,	GPIO_P1, 	6,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,		//reset
		GPIO_T_INT,	GPIO_P1, 	5,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,		//backlight
	};
	tbl_gpio_def tbl_bspLcdCtrl[] = {_tbl_bspLcdCtrl, ARR_ENDADR(_tbl_bspLcdCtrl)};
#endif

#if DM9000_ENABLE
	t_gpio_def _tbl_bspDm9000[] = {
		GPIO_T_INT, GPIO_P6,	6,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,		//reset
		GPIO_T_INT, GPIO_P6,	7,		GPIO_M_IN_FLOAT,	GPIO_INIT_NULL,		//irq
	};
	tbl_gpio_def tbl_bspDm9000[] = {_tbl_bspDm9000, ARR_ENDADR(_tbl_bspDm9000)};
#endif


/*************************************************************************
HW-IIC using HYM8025
I2C1SCL ->PA6
I2C1SDA ->PA7
by arnold
**************************************************************************/
//-------------------------------------------------------------------------
//I2C Interface Functions
//-------------------------------------------------------------------------
#if I2C_ENABLE
	#if I2C_SOFTWARE
		t_gpio_def _tbl_bspI2cDef0[] = {
			GPIO_T_INT, GPIO_P0,	6,		GPIO_M_OUT_OD,		GPIO_INIT_HIGH, 	//scl
			GPIO_T_INT, GPIO_P0,	7,		GPIO_M_OUT_OD,		GPIO_INIT_HIGH, 	//sda
		};
		tbl_gpio_def tbl_bspI2cDef[BSP_I2C_QTY] = {_tbl_bspI2cDef0};
	#else
		t_i2c_def tbl_bspI2cDef[BSP_I2C_QTY] = {
			GPIO_P0,	6,	GPIO_P0,	7,	I2C_ADR_7BIT,	50000,
		};
	#endif
#endif

#if PCA955X_ENABLE
	t_gpio_def _tbl_bspPca955x[] = {
		GPIO_T_INT,	GPIO_P1,	11, 	GPIO_M_IN_PU,		GPIO_INIT_NULL, 	//irq
	};
	tbl_gpio_def tbl_bspPca955x[] = {_tbl_bspPca955x, ARR_ENDADR(_tbl_bspPca955x)};
#endif

/*************************************************************************
SW-SPI be used by SPI FLASH（MX25L3206EM2I-12G）
SPI_CLK     ->   PA2
SPI_CS0     ->   PA3
SPI_MISO    ->  PA4
SPI_MOSI    ->  PA5
by arnold
**************************************************************************/
//-------------------------------------------------------------------------
//SPI Interface Functions
//-------------------------------------------------------------------------
#if SPI_ENABLE
	#if SPI_SOFTWARE
		t_gpio_def _tbl_bspSpiDef0[] = {
			GPIO_T_INT, GPIO_P0,	2,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH, 		//clk
			GPIO_T_INT, GPIO_P0,	5,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH, 		//mosi
			GPIO_T_INT, GPIO_P0,	4,		GPIO_M_IN_FLOAT,	GPIO_INIT_NULL, 		//miso
		#if SPI_CE_ENABLE
			GPIO_T_INT, GPIO_P0,	3, 	       GPIO_M_OUT_PP,		GPIO_INIT_HIGH,		//ce
		#endif
		};
		tbl_gpio_def tbl_bspSpiDef[BSP_SPI_QTY] = {_tbl_bspSpiDef0};
	#else
		t_spi_def tbl_bspSpiDef[BSP_SPI_QTY] = {
			0,	GPIO_P0,	15,	GPIO_P0,	18,	GPIO_P0,	17,	GPIO_P0,	16,	DEV_PIN_PP,
		};
	#endif
#endif

#if SPIFLASH_ENABLE
	#if SPI_SEL_ENABLE
		const uint8_t tbl_bspSpifCsid[BSP_SPIF_QTY] = {
			0, 1, 2, 3, 8, 9, 10, 11,
		};
	#endif
#endif

#if SC16IS7X_ENABLE
	t_gpio_def _tbl_bspSc16is7x[] = {
		GPIO_T_INT, GPIO_P0,	7,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH, 	//reset
		GPIO_T_INT, GPIO_P0,	12, 	GPIO_M_OUT_PP,		GPIO_INIT_HIGH, 	//nss
		GPIO_T_INT,	GPIO_P2,	8,		GPIO_M_IN_PU,		GPIO_INIT_NULL, 	//irq
	};
	tbl_gpio_def tbl_bspSc16is7x[] = {_tbl_bspSc16is7x, ARR_ENDADR(_tbl_bspSc16is7x)};
#endif

#if ATT7022_ENABLE
	t_gpio_def _tbl_bspAtt7022[] = {
		GPIO_T_INT,	GPIO_P2,	5,		GPIO_M_IN_PU,		GPIO_INIT_NULL, 		//SAMP_JL   //已经更改，勿用
		GPIO_T_INT,	GPIO_P1,	1,		GPIO_M_IN_PU,		GPIO_INIT_NULL, 		//SAMP_XB
		GPIO_T_INT,	GPIO_P1,    3,		GPIO_M_OUT_PP, 	    GPIO_INIT_LOW, 	        //SAMP_RST
	};
	tbl_gpio_def tbl_bspAtt7022[] = {_tbl_bspAtt7022, ARR_ENDADR(_tbl_bspAtt7022)};
#endif

#if VOICE_ENABLE
	t_gpio_def _tbl_bspVoice[] = {
		GPIO_T_INT,	GPIO_P2,	0,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,			//reset
		GPIO_T_INT,	GPIO_P2,	5,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH, 		//nss
		GPIO_T_INT,	GPIO_P2,	1,		GPIO_M_IN_FLOAT,	GPIO_INIT_NULL, 		//busy
	};
	tbl_gpio_def tbl_bspVoice[] = {_tbl_bspVoice, ARR_ENDADR(_tbl_bspVoice)};
#endif

#if MIFARE_ENABLE
	t_gpio_def _tbl_bspMifare[] = {
		GPIO_T_INT,	GPIO_P1,	8,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,			//reset
		GPIO_T_INT,	GPIO_P0,	4,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH,			//nss
		//GPIO_T_INT,	GPIO_P0,	1,		GPIO_M_IN_FLOAT,		GPIO_INIT_NULL, 		//irq
	};
	tbl_gpio_def tbl_bspMifare[] = {_tbl_bspMifare, ARR_ENDADR(_tbl_bspMifare)};
#endif



/****************************************************************************
UART0 :         PA1	TXD
                PA0	RXD
UART1           PC7	SAMP_TXD
                PC6	SAMP_RXD
UART2           PD1	G_TXD
                PD0	G_RXD
*****************************************************************************/
//-------------------------------------------------------------------------
//UART Interface Functions
//-------------------------------------------------------------------------
#if UART_ENABLE
	t_uart_def tbl_bspUartDef[BSP_UART_QTY] = {
		UART_T_INT,		0,	GPIO_P0,	1,	UART_MODE_IRQ,	GPIO_P0,	0,	UART_MODE_IRQ,	NULL,		NULL,	UART_FUN_NORMAL,	DEV_PIN_PP,
		UART_T_INT,		1,	GPIO_P2,	7,	UART_MODE_IRQ,	GPIO_P2,	6,	UART_MODE_IRQ,	NULL,		NULL,	UART_FUN_NORMAL,	DEV_PIN_PP,
		UART_T_INT,		2,	GPIO_P3,	1,	UART_MODE_IRQ,	GPIO_P3,	0,	UART_MODE_IRQ,	NULL,		NULL,	UART_FUN_NORMAL,	DEV_PIN_PP,
		UART_T_TIMER,	0,	GPIO_P5,	4,	UART_MODE_IRQ,	GPIO_P5,	5,	UART_MODE_IRQ,	NULL,		NULL,	UART_FUN_NORMAL,	DEV_PIN_PP,
	};
#endif

#if RS485_ENABLE
	const uint8_t tbl_bspRs485Id[BSP_RS485_QTY] = {5, 6, 7};
#endif

#if VK321X_ENABLE
	t_gpio_def _tbl_bspVk321x[] = {
		GPIO_T_INT, GPIO_P1,	7,		GPIO_M_OUT_PP,		GPIO_INIT_HIGH, 	//reset
		GPIO_T_INT, GPIO_P6,	13,		GPIO_M_IN_PU,		GPIO_INIT_NULL, 	//irq
	};
	tbl_gpio_def tbl_bspVk321x[] = {_tbl_bspVk321x, ARR_ENDADR(_tbl_bspVk321x)};
#endif

#if MODEM_ENABLE
	t_modem_def tbl_bspModem01 = {
		2,
		MODEM_EFFECT_LOW, 
		{GPIO_T_HC595,	GPIO_P0,	5,		GPIO_M_OUT_PP,		GPIO_INIT_NULL},	//Act
		MODEM_EFFECT_HIGH,
		{GPIO_T_HC595,	GPIO_P0,	3,		GPIO_M_OUT_PP,		GPIO_INIT_NULL},	//Power
		MODEM_EFFECT_LOW,
		{GPIO_T_HC595, 	GPIO_P0,	4,		GPIO_M_OUT_PP,		GPIO_INIT_NULL},	//Reset
	};
	t_modem_def * const tbl_bspModem[BSP_MODEM_QTY] = {
		&tbl_bspModem01,
	};
#endif




//-------------------------------------------------------------------------
//File System Functions
//-------------------------------------------------------------------------
#if REGISTER_ENABLE
	t_flash_blk _tbl_reg_SfsDev[] = {
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 3,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 4,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 5,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 6,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 7,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 8,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 9,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 10,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x3C000 + INTFLASH_BLK_SIZE * 11,	INTFLASH_BLK_SIZE,
	};
	t_flash_dev reg_SfsDev = {
		FLASH_DEV_INT, ARR_SIZE(_tbl_reg_SfsDev), _tbl_reg_SfsDev,
	};
#endif




//-------------------------------------------------------------------------
//In Application Programming Functions
//-------------------------------------------------------------------------
#if GDFTS_ENABLE
	t_flash_blk _tbl_upd_DataDev[] = {
		NORFLASH_BASE_ADR + 0x7A0000,	0x10000,
		NORFLASH_BASE_ADR + 0x7B0000,	0x10000,
		NORFLASH_BASE_ADR + 0x7C0000,	0x10000,
		NORFLASH_BASE_ADR + 0x7D0000,	0x10000,
		NORFLASH_BASE_ADR + 0x7E0000,	0x10000,
		NORFLASH_BASE_ADR + 0x7F0000,	0x2000,
		NORFLASH_BASE_ADR + 0x7F2000,	0x2000,
		NORFLASH_BASE_ADR + 0x7F4000,	0x2000,
		NORFLASH_BASE_ADR + 0x7F6000,	0x2000,
		NORFLASH_BASE_ADR + 0x7F8000,	0x2000,
		NORFLASH_BASE_ADR + 0x7FA000,	0x2000,
		NORFLASH_BASE_ADR + 0x7FC000,	0x2000,
		NORFLASH_BASE_ADR + 0x7FE000,	0x2000,
	};
	t_flash_dev upd_DataDev = {
		FLASH_DEV_EXTNOR, ARR_SIZE(_tbl_upd_DataDev), _tbl_upd_DataDev,
	};

	t_sfs_blk _tbl_upd_SfsDev[] = {
		INTFLASH_BASE_ADR + 0x7E000 + INTFLASH_BLK_SIZE * 0,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x7E000 + INTFLASH_BLK_SIZE * 1,	INTFLASH_BLK_SIZE,
		INTFLASH_BASE_ADR + 0x7E000 + INTFLASH_BLK_SIZE * 2,	INTFLASH_BLK_SIZE,
	};
	sfs_dev upd_SfsDev = {
		FLASH_DEV_INT, ARR_SIZE(_tbl_upd_SfsDev), _tbl_upd_SfsDev,
	};
#endif


