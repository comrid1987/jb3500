#if	REGISTER_ENABLE



//Private Defines
#define REGLOADER_DEBUG_ENABLE			0

#define REGLOADER_FILEDIR				"/sf0/"
#define REGLOADER_FILEOFFSET			5

#define REGLOADER_S_IDLE				0
#define REGLOADER_S_NEED_SYNC			1


#define REG_STATE_FLAG_READ				0x00
#define REG_STATE_FLAG_WRITE			0x01
#define REG_STATE_FLAG_SIZE				0x02
#define REG_STATE_FLAG_TYPE				0x03

#if REGLOADER_DEBUG_ENABLE
#define regloader_trace					rt_kprintf
#else
#define regloader_trace(...)
#endif



//Private Typedefs
struct t_reg_loader {
	uint8_t		ste;
	uint8_t		grp;
	uint16_t	size;
	uint16_t	da;
	uint16_t	hit;
	void *		data;
};
typedef struct t_reg_loader *p_reg_loader;

//Private Variables
static struct t_reg_loader reg_loader[REGISTER_LOADER_QTY];

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
extern p_reg_grptbl tbl_RegGroup[];
static p_reg_grptbl regtbl_GetGroup(uint_t nGrp)
{
	p_reg_grptbl p;
	
	for (p = tbl_RegGroup[0]; p < tbl_RegGroup[1]; p++)
		if (p->id == nGrp)
			return p;
	return NULL;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static p_register regtbl_GetReg(p_reg_grptbl pG, uint_t nID, uint_t *pOffset)
{
	p_register pReg, pEnd;
	uint_t nOffset = 0;
	
	nID &= 0xFF;
	pReg = pG->tblreg;
	pEnd = pReg + pG->qty;
	for (; pReg < pEnd; pReg++) {
		if (pReg->id == nID) {
			*pOffset = nOffset;
			return pReg;
		}
		nOffset += pReg->size;
	}
	return NULL;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void regloader_Init()
{
	p_reg_loader p;

	for (p = &reg_loader[0]; p < ARR_ENDADR(reg_loader); p++) {
		p->ste = REGLOADER_S_IDLE;
		p->size = 0;
		p->hit = 0;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static __inline void regloader_Lock()
{

	os_thd_Lock();
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static __inline void regloader_Unlock()
{

	os_thd_Unlock();
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void regloader_Filename(p_reg_loader p, char *filename)
{

	memcpy(filename, REGLOADER_FILEDIR, REGLOADER_FILEOFFSET);
	bcd2str16(p->da, &filename[REGLOADER_FILEOFFSET]);
	bcd2str8(p->grp, &filename[REGLOADER_FILEOFFSET + 4]);
	filename[REGLOADER_FILEOFFSET + 6] = '\0';
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void regloader_Save(p_reg_loader p, uint_t nType)
{
#if FS_ENABLE
	int fd;
	char filename[REGLOADER_FILEOFFSET + 8];
#endif

	switch (nType) {
#if SFS_ENABLE
	case 1:
		sfs_Write(&reg_SfsDev, (p->grp << 16) | p->da, p->data, p->size);
		p->ste = REGLOADER_S_IDLE;
		break;
#endif
#if FS_ENABLE
	case 2:
		regloader_Filename(p, filename);
		fd = fs_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
			return;
		fs_write(fd, p->data, p->size);
		fs_close(fd);
		p->ste = REGLOADER_S_IDLE;
		regloader_trace("%s\n", filename);
		break;
#endif
	default:
		break;
	}
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void regloader_Close(p_reg_loader p, uint_t nType)
{

	if (p->ste == REGLOADER_S_NEED_SYNC)
		regloader_Save(p, nType);
 	mem_Free(p->data);
	p->size = 0;
	p->hit = 0;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static void regloader_Default(p_reg_loader p, uint_t nType)
{
#if FS_ENABLE
	int fd;
	char filename[REGLOADER_FILEOFFSET + 8];
#endif

	switch (nType) {
#if SFS_ENABLE
	case 1:
		break;
#endif
#if FS_ENABLE
	case 2:
		memcpy(filename, REGLOADER_FILEDIR, REGLOADER_FILEOFFSET);
		filename[REGLOADER_FILEOFFSET] = 'D';
		bcd2str16(p->da, &filename[REGLOADER_FILEOFFSET + 1]);
		bcd2str8(p->grp, &filename[REGLOADER_FILEOFFSET + 5]);
		filename[REGLOADER_FILEOFFSET + 7] = '\0';
		fd = fs_open(filename, O_RDONLY, 0);
		if (fd < 0) {
			bzero(p->data, p->size);
		} else {
			fs_read(fd, p->data, p->size);
			fs_close(fd);
		}
		p->ste = REGLOADER_S_NEED_SYNC;
		break;
#endif
	default:
		break;
	}
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
static p_reg_loader regloader_Open(uint_t nDa, p_reg_grptbl pG, int flag)
{
	p_reg_loader p, pHit;
#if FS_ENABLE
	int fd;
	char filename[REGLOADER_FILEOFFSET + 8];
#endif

	for (p = &reg_loader[0]; p < ARR_ENDADR(reg_loader); p++)
		if ((p->da == nDa) && (p->grp == pG->id)) {
			p->hit += 1;
			return p;
		}

	p = &reg_loader[0];
	pHit = p;
	for (; p < ARR_ENDADR(reg_loader); p++) {
		if (p->size == 0)
			break;
		if (p->hit < pHit->hit)
			pHit = p;
	}
	if (p >= ARR_ENDADR(reg_loader)) {
		p = pHit;
		regloader_Close(p, pG->type);
	}

	p->data = mem_Malloc(pG->size);
	if (p->data != NULL) {
		p->size = pG->size;
		p->da = nDa;
		p->grp = pG->id;
		p->ste = REGLOADER_S_IDLE;
		if (flag) {
			switch (pG->type) {
#if SFS_ENABLE
			case 1:
				if (sfs_Read(&reg_SfsDev, (p->grp << 16) | p->da, p->data) != SYS_R_OK)
					regloader_Default(p, pG->type);
				break;
#endif
#if FS_ENABLE
			case 2:
				regloader_Filename(p, filename);
				fd = fs_open(filename, O_RDONLY, 0);
				if (fd < 0) {
					regloader_Default(p, pG->type);
				} else {
					fs_read(fd, p->data, p->size);
					fs_close(fd);
				}
				break;
#endif
			default:
				break;
			}
		}
		return p;
	}
	p->size = 0;
	return NULL;
}






//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
#define TER_FAT_CODE	"SZHG"
#define TER_MODEL		"GWJ8000"
#define TER_SOFT_VER	"1000"
#define TER_CONFIG_CODE	" "
#define TER_COM_VER		"3761"
#define TER_HW_VER		"100b"

const char TER_SOFT_Data[3] = {0x21, 0x07, 0x10};
const char TER_HW_Data[3] = {0x01, 0x07, 0x10};

static int reg_State(uint_t nDa, uint_t nID, void *pData, uint_t nFlag)
{
	uint_t nLen = 0;
	uint32_t nData;

	switch (nID & 0xFF) {
	case 0x01:
		//终端时间
		nLen = 4;
		switch (nFlag) {
		case REG_STATE_FLAG_READ:
			nData = rtc_GetTimet();
			memcpy(pData, &nData, nLen);
			break;
		case REG_STATE_FLAG_WRITE:
			memcpy(&nData, pData, nLen);
			rtc_SetTimet(nData);
			break;
		case REG_STATE_FLAG_TYPE:
			nLen = REG_TYPE_TIME;
			break;
		default:
			break;
		}
		break;
	case 0x02:
		nLen = 41;
		switch (nFlag) {
		case REG_STATE_FLAG_READ:
			memset(pData, 0x00, 41);
			memcpy(pData, TER_FAT_CODE, sizeof(TER_FAT_CODE));
			memcpy((char *)pData + 4, TER_MODEL, sizeof(TER_MODEL));
			memcpy((char *)pData + 12, TER_SOFT_VER, sizeof(TER_SOFT_VER));
			memcpy((char *)pData + 16, TER_SOFT_Data, 3);
			memcpy((char *)pData + 19, TER_CONFIG_CODE, sizeof(TER_CONFIG_CODE));
			memcpy((char *)pData + 30, TER_COM_VER, sizeof(TER_COM_VER));
			memcpy((char *)pData + 34, TER_HW_VER, sizeof(TER_HW_VER));
			memcpy((char *)pData + 38, TER_HW_Data, 3);
			break;
		case REG_STATE_FLAG_WRITE:
			break;
		case REG_STATE_FLAG_TYPE:
			nLen = REG_TYPE_BIN;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return nLen;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int reg_Get(uint_t nDa, uint_t nID, void *pData)
{
	p_reg_loader p;
	p_reg_grptbl pG;
	p_register pReg;
	uint_t nGrp, nOffset;

	nGrp = nID >> 8;
	if (nGrp == 0xFF)
		return reg_State(nDa, nID, pData, REG_STATE_FLAG_READ);
	pG = regtbl_GetGroup(nGrp);
	if (pG == NULL)
		return 0;
	pReg = regtbl_GetReg(pG, nID, &nOffset);
	if (pReg == NULL)
		return 0;
	regloader_Lock();
	p = regloader_Open(nDa, pG, 1);
	if (p == NULL) {
		regloader_Unlock();
		return 0;
	}
	memcpy(pData, (char *)p->data + nOffset, pReg->size);
	regloader_Unlock();
	return pReg->size;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
uint32_t reg_GetValue(uint_t nDa, uint_t nID)
{
	uint32_t nData = 0;

	reg_Get(nDa, nID, &nData);
	return nData; 
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int reg_GetBit(uint_t nDa, uint_t nID, uint_t nBit)
{
	uint64_t nData;

	reg_Get(nDa, nID, &nData);
	if (nData & BITMASK(nBit))
		return 1;
	return 0;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int reg_Set(uint_t nDa, uint_t nID, const void *pData, uint_t nSave)
{
	p_reg_loader p;
	p_reg_grptbl pG;
	p_register pReg;
	uint_t nGrp, nOffset;

	nGrp = nID >> 8;
	if (nGrp == 0xFF)
		return reg_State(nDa, nID, (void *)pData, REG_STATE_FLAG_WRITE);
	pG = regtbl_GetGroup(nGrp);
	if (pG == NULL)
		return 0;
	pReg = regtbl_GetReg(pG, nID, &nOffset);
	if (pReg == NULL)
		return 0;
	regloader_Lock();
	p = regloader_Open(nDa, pG, 1);
	if (p == NULL) {
		regloader_Unlock();
		return 0;
	}
	//待优化,memcmp和memcpy可以同时进行
	if (memcmp((char *)p->data + nOffset, pData, pReg->size)) {
		memcpy((char *)p->data + nOffset, pData, pReg->size);
		if (nSave)
			regloader_Save(p, pG->type);
		else
			p->ste = REGLOADER_S_NEED_SYNC;
	}
	regloader_Unlock();
	return pReg->size;
}

void reg_SetValue(uint_t nDa, uint_t nID, uint32_t nData, uint_t nSave)
{

	reg_Set(nDa, nID, &nData, nSave);
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int reg_GetRegSize(uint_t nID)
{
	p_reg_grptbl pG;
	p_register pReg;
	uint_t nGrp, nOffset;

	nGrp = nID >> 8;
	if (nGrp == 0xFF)
		return reg_State(TERMINAL, nID, NULL, REG_STATE_FLAG_SIZE);
	if ((pG = regtbl_GetGroup(nGrp)) != NULL)
		if ((pReg = regtbl_GetReg(pG, nID, &nOffset)) != NULL)
			return pReg->size;
	return -1;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int reg_GetRegType(uint_t nID)
{
	p_reg_grptbl pG;
	p_register pReg;
	uint_t nGrp, nOffset;

	nGrp = nID >> 8;
	if (nGrp == 0xFF)
		return reg_State(TERMINAL, nID, NULL, REG_STATE_FLAG_TYPE);
	if ((pG = regtbl_GetGroup(nGrp)) != NULL)
		if ((pReg = regtbl_GetReg(pG, nID, &nOffset)) != NULL)
			return pReg->type;
	return -1;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int reg_ArithmeticCopy(uint_t nDestDa, uint_t nDestReg, uint_t nSrcDa, uint_t nSrcReg, uint_t nType, uint32_t nParam, uint_t nSave)
{
	int nLen;
	uint64_t nTemp;
	void *pBuf;
	
	nLen = reg_GetRegSize(nSrcReg);
	if (nLen < 0)
		return 0;
	if (nType == REG_O_COPY) {
		pBuf = mem_Malloc(nLen);
		if (pBuf == NULL)
			return 0;
		reg_Get(nSrcDa, nSrcReg, pBuf);
		reg_Set(nDestDa, nDestReg, pBuf, nSave);
		mem_Free(pBuf);
	} else {
		if (nLen > 8)
			return 0;
		nTemp = 0;
		reg_Get(nSrcDa, nSrcReg, &nTemp);
		switch (nType) {
		case REG_O_ADD:
			nTemp += nParam;
			break;
		case REG_O_SUB:
			nTemp -= nParam;
			break;
		case REG_O_MUL:
			nTemp *= nParam;
			break;
		case REG_O_DIV:
			nTemp /= nParam;
			break;
		case REG_O_OR:
			//或
			nTemp |= nParam;
			break;
		case REG_O_AND:
			//与
			nTemp &= nParam;
			break;
		case REG_O_SETBIT:
			//置位
			SETBIT(nTemp, nParam);
			break;
		case REG_O_CLRBIT:
			//清位
			CLRBIT(nTemp, nParam);
			break;
		default:
			break; 
		}
		reg_Set(nDestDa, nDestReg, &nTemp, nSave);
	}
	return nLen; 
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int reg_OperateAdd(uint_t nDa, uint_t nReg, uint32_t nParam, uint_t nSave)
{

	return reg_ArithmeticCopy(nDa, nReg, nDa, nReg, REG_O_ADD, nParam, nSave);
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res reg_GroupSave(uint_t nDa, uint_t nGrp)
{
	sys_res res = SYS_R_ERR;
	p_reg_loader p;
	p_reg_grptbl pG;

	pG = regtbl_GetGroup(nGrp);
	if (pG != NULL) {
		regloader_Lock();
		p = regloader_Open(nDa, pG, 1);
		if (p != NULL) {
			regloader_Save(p, pG->type);
			res = SYS_R_OK;
		}
		regloader_Unlock();
	}
	return res;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
sys_res reg_GroupCopy(uint_t nDestDa, uint_t nDestGrp, uint_t nSrcDa, uint_t nSrcGrp, uint_t nSave)
{
	sys_res res = SYS_R_ERR;
	p_reg_loader pDest, pSrc;
	p_reg_grptbl pDestG, pSrcG;

	pSrcG = regtbl_GetGroup(nSrcGrp);
	if (pSrcG != NULL) {
		pDestG = regtbl_GetGroup(nDestGrp);
		if (pDestG != NULL) {
			regloader_Lock();
			pSrc = regloader_Open(nSrcDa, pSrcG, 1);
			if (pSrc != NULL) {
				pDest = regloader_Open(nDestDa, pDestG, 0);
				if (pDest != NULL) {
					//待优化,memcmp和memcpy可以同时进行
					//if (memcmp(pDest->data, pSrc->data, pDest->size)) {
						memcpy(pDest->data, pSrc->data, pDest->size);
						if (nSave)
							regloader_Save(pDest, pDestG->type);
						else
							pDest->ste = REGLOADER_S_NEED_SYNC;
						res = SYS_R_OK;
					//}
				}
			}
			regloader_Unlock();
		}
	}
	return res;
}




const uint8_t arrZero[120] = {
	0, 0, 0, 0, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
const uint8_t arrFF[8] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
uint8_t *reg_DefaultValue(uint_t nID, uint8_t *pBuf)
{
	uint32_t *pInt = (uint32_t *)pBuf;

	switch (nID) {
#if 0
	case 0x0103://终端等待从动站响应的超时时间
	case 0x0104://终端作为启动站允许发送重发次数	
	case 0x1211://轮显时间
	case 0x1241://文件系统碎片整理阀值
		*pInt = 10;
		break;
	case 0x0125://公网通讯模块工作方式 
	case 0x0152://终端地址
	case 0x0170://是否允许终端与主站通话
	case 0x0171://是否允许终端主动上报		
	case 0x01A3://RS232停止位
	case 0x01B3://红外停止位
	case 0x0414://终端停电标志
	case 0x2217://电表通信停止位
	case 0x2761://CT
	case 0x2762://PT
		*pInt = 1;
		break;
	case 0x0102://终端作为启动站允许发送传输延时时间
	case 0x0105://心跳周期 
	case 0x0132://被动激活模式连续无通讯自动断线时间
		*pInt = 300; 
		break;
	case 0x0222://日数据冻结点数
	case 0x0127://永久在线模式重拨间隔
	case nID_MaxVWindow://滑差时间
		*pInt = 60; 
		break;
	case 0x0151://行政区划码
		*pInt = 0x2081;
		break;
	case 0x01A0://"RS232波特率"
	case 0x01B0://"红外波特率"
		*pInt = 9600;
		break;
	case 0x0116://APN
		pBuf = "CMNET";
		break; 
	case 0x01D1://User
	case 0x01D2://Password
		pBuf = "CARD";
		break; 
	case 0x01C1://DNS
		pBuf = "gtpower.vicp.net";
		break;
	case 0x01E1://终端名称
		pBuf = "PDK6000配变监测终端";
		break;
		
	case 0x0221://曲线冻结点数
		*pInt = 5760;
		break;
	case 0x0223://月数据冻结点数
		*pInt = 6;
		break;
	case 0x0410:
	case 0x0411:
		pBuf = (uint8_t *)arrFF;
		break;
	case 0x0861: {//电压谐波限值
			uint16_t *pThd = (uint16_t *)pBuf;
			for (i = 0; i < 19; ++i) {
				pThd[i] = 0x0050;
			}
		}
		break;
	case 0x0862: {//电流谐波限值
			uint16_t *pThd = (uint16_t *)pBuf;
			for (i = 0; i < 19; ++i) {
				pThd[i] = 0x0500;
			}
		}
		break;
	case nID_MaxVCycle://最大需量周期
		*pInt = 900;
		break;
	case 0x0A10:
		pBuf = "JNF";
		break;
	case 0x0A11:
		pBuf = "00000001";
		break;
	case 0x0A12:
		*pInt = VER_SOFT;
		break;
	case 0x0A17:
		*pInt = VER_HARD;
		break;

	case 0x1212:
		pBuf = (uint8_t *)arrFF;
		break;

	case 0x2763://"额定电压"
		*pInt = FLOAT2FIX(220);
		break;
	case 0x2764://"额定电流"
		*pInt = FLOAT2FIX(5);
		break;
	case 0x2767://接线方式
		*pInt = 2;	//3相4线
		break;
	case 0x2771://"电压合格上限", 			"V",
		*pInt = FLOAT2FIX(220 * 1.07);
		break;
	case 0x2772://"电压合格下限", 			"V",
		*pInt = FLOAT2FIX(220 * 0.9);
		break;
	case 0x2773://"电压断相门限", 			"V",
		*pInt = FLOAT2FIX(220 * 0.7);
		break;
	case 0x2774://"过压门限", 				"V",
		*pInt = FLOAT2FIX(220 * 1.3);
		break;
	case 0x2775://"欠压门限", 				"V",
		*pInt = FLOAT2FIX(220 * 0.85);
		break;
	case 0x2776://"过流门限", 				"A",
		*pInt = FLOAT2FIX(5 * 1.3);
		break;
	case 0x2777://"额定电流门限", 			"A",
		*pInt = FLOAT2FIX(5);
		break;
	case 0x2781://"零序电流上限", 			"A",
		*pInt = FLOAT2FIX(5 * 0.25);
		break;
	case 0x2782://"视在功率上上限",			"KVA",
		*pInt = FLOAT2FIX(3 * 1.2);
		break;
	case 0x2783://"视在功率上限", 			"KVA",
		*pInt = FLOAT2FIX(3);
		break;
	case 0x2784://"三相电压不平衡上限",		"",
	case 0x2785://"三相电流不平衡上限", 	"",
		*pInt = FLOAT2FIX(0.5);
		break;
	case 0x2788://"功率因数分段限值1",		"",
		*pInt = FLOAT2FIX(0.95);
		break;
	case 0x2789://"功率因数分段限值2",		"",
		*pInt = FLOAT2FIX(0.8);
		break;
	case 0x2791://"连续失压时间限值", 		"Min",
		*pInt = 5;			
		break;
	case 0x8541://最小电压
	case 0x8542:
	case 0x8543:
	case 0x8841:
	case 0x8842:
	case 0x8843:
		*pInt = 0x7FFFFFFF;
		break;
#endif
	default:
		pBuf = (uint8_t *)arrZero;
		break; 
	}
	return pBuf;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void reg_DefaultReg(uint_t nDa, uint_t nReg, uint_t nSave)
{
	uint8_t arrValue[40], *pData;

	pData = reg_DefaultValue(nReg, arrValue);
	reg_Set(nDa, nReg, pData, nSave);
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void reg_DefaultGroup(uint_t nDa, uint_t nGrp, uint_t nSave)
{
	uint_t nDaStart, nDaEnd;
	p_reg_loader p;
	p_reg_grptbl pG;

	pG = regtbl_GetGroup(nGrp);
	if (pG == NULL)
		return;
	switch (nDa) {
	case REG_DA_ALL:
		switch (pG->type) {
		case 0:
			nDaStart = TERMINAL;
			nDaEnd = TERMINAL;
			break;
		default:
			nDaStart = 0;
////////////////////////////////////Unfinished////////////////////////////			
			//nDaEnd = REGISTER_SIZE_TN;
			nDaEnd = 3;
////////////////////////////////////////////////////////////////////////
			break;
		}
		break;
	default:
		nDaStart = nDa;
		nDaEnd = nDa;
		break;
	}
	regloader_Lock();
	for (nDa = nDaStart; nDa <= nDaEnd; nDa++) {
		p = regloader_Open(nDa, pG, 0);
		if (p != NULL) {
			regloader_Default(p, pG->type);
			if (nSave)
				regloader_Save(p, pG->type);
		}
	}
	regloader_Unlock();
}


sys_res reg_Init()
{

	//Init the register system loader
	regloader_Init();

	return SYS_R_OK;
}

sys_res reg_Maintain()
{
	p_reg_loader p;

	regloader_Lock();
	for (p = &reg_loader[0]; p < ARR_ENDADR(reg_loader); p++) {
		p->hit >>= 1;
		if (p->ste == REGLOADER_S_NEED_SYNC)
			regloader_Save(p, 0);
	}
	regloader_Unlock();

 	return SYS_R_OK;
}


#endif

