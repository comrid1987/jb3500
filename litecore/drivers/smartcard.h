#ifndef __SMARTCARD_H__
#define __SMARTCARD_H__


#ifdef __cplusplus
extern "C" {
#endif




//Exported constants
#define SC_LE_MAX			20

#define SC_TYPE_PSAM		0x02
#define SC_TYPE_ESAM		0x00

#define SC_S_IDLE			0
#define SC_S_ACTIVE			1
#define SC_S_BUSY			2

/* SC Tree Structure -----------------------------------------------------------
                              MasterFile
                           ________|___________
                          |        |           |
                        System   UserData     Note
------------------------------------------------------------------------------*/

/* SC ADPU Command: Operation Code -------------------------------------------*/
#define SC_CLA_GSM11		0xA0

/*------------------------ Data Area Management Commands ---------------------*/
#define SC_SELECT_FILE		0xA4
#define SC_GET_RESPONCE		0xC0
#define SC_STATUS			0xF2
#define SC_UPDATE_BINARY	0xD6
#define SC_READ_BINARY		0xB0
#define SC_WRITE_BINARY		0xD0
#define SC_UPDATE_RECORD	0xDC
#define SC_READ_RECORD		0xB2

/*-------------------------- Administrative Commands -------------------------*/ 
#define SC_CREATE_FILE		0xE0

/*-------------------------- Safety Management Commands ----------------------*/
#define SC_VERIFY			0x20
#define SC_CHANGE			0x24
#define SC_DISABLE			0x26
#define SC_ENABLE			0x28
#define SC_UNBLOCK			0x2C
#define SC_EXTERNAL_AUTH	0x82
#define SC_GET_CHALLENGE	0x84


/* ATR structure - Answer To Reset -------------------------------------------*/
#define SC_SETUP_SIZE		20
#define SC_HIST_SIZE		20
typedef struct {
	uint8_t TS;               /* Bit Convention */
	uint8_t T0;               /* High nibble = Number of setup byte; low nibble = Number of historical byte */
	uint8_t T[SC_SETUP_SIZE];  /* Setup array */
	uint8_t H[SC_HIST_SIZE];   /* Historical array */
	uint8_t Tlength;          /* Setup array dimension */
	uint8_t Hlength;          /* Historical array dimension */
}sc_atr[1];

typedef struct {
	uint8_t cla;
	uint8_t ins;
	uint8_t p1;
	uint8_t p2;
}sc_cmd[1], *p_sc_cmd;

typedef struct {
	p_dev_uart 	parent;
	uint8_t 	ste;
	uint8_t		ptrl;
	uint8_t		sw1;
	uint8_t		sw2;
	uint8_t		sn[8];
	uint8_t		resp[SC_LE_MAX];
}smartcard[1];



//External Functions
void sc_Init(smartcard sc);
void sc_Release(smartcard sc);
sys_res sc_Get(smartcard sc, uint_t nId, uint_t nTmo);
sys_res sc_GetRandom(smartcard sc, uint_t nLen);
sys_res sc_Authority(smartcard sc, uint_t nIns, uint_t nP1, uint_t nP2, void *pBuf, uint_t nLen);
sys_res sc_EsamIntAuthority(smartcard sc, uint_t nP1, uint_t nP2, void *pBuf);


#ifdef __cplusplus
}
#endif

#endif


