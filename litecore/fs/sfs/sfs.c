//File: fs/sfs/sfs.c 
//Simple File System
//
//
//History:
//V1.0	First Release
//		likazhou 2009-02-06
//V1.1	Length 8 supported
//		likazhou 2012-03-29

//Include Header Files



//Private Defines
#define SFS_LOCK_ENABLE			1

#define SFS_RECORD_MASK			0xFFFFFFFF

#if SFS_LOCK_ENABLE
#define sfs_Lock()				os_thd_Lock()
#define sfs_Unlock()			os_thd_Unlock()
#else
#define sfs_Lock()
#define sfs_Unlock()
#endif


#define SFS_BLK_IDLE			0xFFFFFFFF
#define SFS_BLK_ACTIVE			0xFFFFFF00
#define SFS_BLK_FULL			0xFFFF0000


#define SFS_S_IDLE				0xFFFF
#define SFS_S_VALID				0xFF00
#define SFS_S_DUMPING			0x0000
#define SFS_S_INVALID			0x0000


//Private Typedefs
typedef struct {
	uint32_t ste;
}t_sfs_ste, *p_sfs_ste;

typedef struct {
	uint16_t ste;
	uint16_t len;
	t_sfs_id id;
}t_sfs_idx, *p_sfs_idx;










//Internal Functions
static adr_t _sfs_Find(sfs_dev p, uint32_t nAnd, uint32_t nRecord, p_sfs_idx pIdx)
{
	adr_t nBlk, nBEnd, nIdx, nEnd, nAdr = 0;
	t_sfs_ste blk;
	uint_t nSize = flash_BlkSize(p->dev);

	nRecord &= nAnd;
	nBlk = p->start;
	nBEnd = nBlk + nSize * p->blk;
	for (; nBlk < nBEnd; nBlk += nSize) {
		memcpy(&blk, (uint8_t *)nBlk, sizeof(blk));
		if (blk.ste != SFS_BLK_IDLE) {
			nIdx = nBlk + sizeof(blk);
			nEnd = nBlk + nSize;
			for (; nIdx < nEnd; nIdx = ALIGN4(nIdx + sizeof(t_sfs_idx) + pIdx->len)) {
				memcpy(pIdx, (uint8_t *)nIdx, sizeof(t_sfs_idx));
				if (pIdx->ste == SFS_S_VALID) {
					if ((pIdx->id & nAnd) == nRecord) {
						nAdr = nIdx;
						break;
					}
				}
			}
			if (nAdr)
				break;
		}
	}
	return nAdr;
}

static int _sfs_Free(const adr_t nBlk, uint_t nSize, adr_t *pAdr)
{
	adr_t nEnd;
	t_sfs_idx xIdx;

	*pAdr = nBlk + sizeof(t_sfs_ste);
	nEnd = nBlk + nSize;
	for (; *pAdr < nEnd; *pAdr = ALIGN4(*pAdr + sizeof(t_sfs_idx) + xIdx.len)) {
		memcpy(&xIdx, (uint8_t *)*pAdr, sizeof(t_sfs_idx));
		if (xIdx.ste == SFS_S_IDLE)
			return (nEnd - (*pAdr + sizeof(t_sfs_idx)));
	}
	return 0;
}

static sys_res _sfs_Write(sfs_dev p, uint32_t nRecord, const void *pData, uint_t nLen)
{
	sys_res res;
	adr_t nAdrOld = 0, nIdx, nEnd, nIdxNext, nBlk, nBEnd, nAct = NULL;
	t_sfs_ste xBlk;
	t_sfs_idx xIdx;
	uint_t i, nSize, nQty, nIsFull = 1;

	nSize = flash_BlkSize(p->dev);
	//恢复拷贝原有有效数据时意外中断
	nBlk = p->start;
	nBEnd = nBlk + nSize * (p->blk - 1);
	memcpy(&xBlk, (uint8_t *)nBlk, sizeof(t_sfs_ste));
	if (xBlk.ste == SFS_BLK_ACTIVE) {
		memcpy(&xBlk, (uint8_t *)nBEnd, sizeof(t_sfs_ste));
		if (xBlk.ste == SFS_BLK_ACTIVE)
			nAct = nBEnd;
	}
	//查找原记录和激活的块
	for (; nBlk <= nBEnd; nBlk += nSize) {
		memcpy(&xBlk, (uint8_t *)nBlk, sizeof(t_sfs_ste));
		if ((nAct == NULL) && (xBlk.ste == SFS_BLK_ACTIVE))
			nAct = nBlk;
		if ((nAdrOld == 0) && (xBlk.ste != SFS_BLK_IDLE)) {
			nIdx = nBlk + sizeof(t_sfs_ste);
			nEnd = nBlk + nSize;
			for (; nIdx < nEnd; nIdx = ALIGN4(nIdx + sizeof(t_sfs_idx) + xIdx.len)) {
				memcpy(&xIdx, (uint8_t *)nIdx, sizeof(t_sfs_idx));
				if ((xIdx.id == nRecord) && (xIdx.ste == SFS_S_VALID)) {
					//找到原记录
					nAdrOld = nIdx;
					break;
				}
				//已到末尾
				if (xIdx.ste == SFS_S_IDLE)
					break;
			}
		}
		if (nAdrOld && (nAct != NULL))
			break;
	}
	if (nAct == NULL) {
		//未找到激活的块
		nAct = p->start;
 		if ((res = flash_nolockErase(p->dev, nAct))!= SYS_R_OK)
			return res;
		//置第一块Block为激活状态
		xBlk.ste = SFS_BLK_ACTIVE;
 		if ((res = flash_nolockProgram(p->dev, nAct, (uint8_t *)&xBlk, sizeof(t_sfs_ste))) != SYS_R_OK)
			return res;
	}
	//查找空间
	if (_sfs_Free(nAct, nSize, &nIdx) >= (int)(nLen + sizeof(t_sfs_idx)))
		//空间足,直接写入数据
		nIsFull = 0;
	//当前块ID
	i = (nAct - p->start) / nSize;
	for (nQty = p->blk - 1; nIsFull && nQty; nQty--) {
		//该块空间不足,需要分配新块
		nAct = p->start + nSize * i;
		i = cycle(i, 0, p->blk - 1, 1);
		nBEnd = p->start + nSize * i;
		nBlk = p->start + nSize * cycle(i, 0, p->blk - 1, 1);
		memcpy(&xBlk, (uint8_t *)nBEnd, sizeof(t_sfs_ste));
		if (xBlk.ste != SFS_BLK_IDLE) {
			//擦除NextBlk
 			if ((res = flash_nolockErase(p->dev, nBEnd)) != SYS_R_OK)
				return res;
		}
		//置NextBlk为新Act
		xBlk.ste = SFS_BLK_ACTIVE;
 		if ((res = flash_nolockProgram(p->dev, nBEnd, (uint8_t *)&xBlk, sizeof(t_sfs_ste))) != SYS_R_OK)
			return res;
		//拷贝原有的有效记录
		memcpy(&xBlk, (uint8_t *)nBlk, sizeof(t_sfs_ste));
		if (xBlk.ste != SFS_BLK_IDLE) {
			nIdx = nBlk + sizeof(t_sfs_ste);
			nEnd = nBlk + nSize;
			nIdxNext = nBEnd + sizeof(t_sfs_ste);
			for (; nIdx < nEnd; nIdx = ALIGN4(nIdx + sizeof(t_sfs_idx) + xIdx.len)) {
				memcpy(&xIdx, (uint8_t *)nIdx, sizeof(t_sfs_idx));
				if (xIdx.ste == SFS_S_VALID) {
					//找到有效记录
					if (nAdrOld != nIdx) {
 						if ((res = flash_nolockProgram(p->dev, nIdxNext, (uint8_t *)nIdx, sizeof(t_sfs_idx) + xIdx.len)) != SYS_R_OK)
							return res;
						nIdxNext += ALIGN4(sizeof(t_sfs_idx) + xIdx.len);
					} else
						nAdrOld = 0;
				}
			}
		}
		//置原ActBlk为Full
		xBlk.ste = SFS_BLK_FULL;
 		if ((res = flash_nolockProgram(p->dev, nAct, (uint8_t *)&xBlk, sizeof(t_sfs_ste))) != SYS_R_OK)
			return res;
		//擦除OldBlk
 		if ((res = flash_nolockErase(p->dev, nBlk)) != SYS_R_OK)
			return res;
		if (_sfs_Free(nBEnd, nSize, &nIdx) >= nLen)
			//空间足,写入数据
			nIsFull = 0;
	}
	if (nQty == 0)
		return SYS_R_FULL;
	//置原纪录为待删除状态
	//if (nAdrOld) {
	//	_sfs_Read(p, nAdrOld, (uint8 *)&xIdx, sizeof(t_sfs_idx));
	//	xIdx.ste = SFS_S_DUMPING;
	//	flash_nolockProgram(p, nAdrOld, (uint8 *)&xIdx, sizeof(t_sfs_idx));
	//}
	//写新记录
	xIdx.ste = SFS_S_VALID;
	xIdx.id = nRecord;
	xIdx.len = nLen;
	if ((res = flash_nolockProgram(p->dev, nIdx, (uint8_t *)&xIdx, sizeof(t_sfs_idx))) != SYS_R_OK)
		return res;
	if ((res = flash_nolockProgram(p->dev, nIdx + sizeof(t_sfs_idx), (uint8_t *)pData, nLen)) != SYS_R_OK)
		return res;
	//删除原记录
	if (nAdrOld) {
		memcpy(&xIdx, (uint8_t *)nAdrOld, sizeof(t_sfs_idx));
		xIdx.ste = SFS_S_INVALID;
 		if ((res = flash_nolockProgram(p->dev, nAdrOld, (uint8_t *)&xIdx, sizeof(t_sfs_idx))) != SYS_R_OK)
			return res;
	}
	return SYS_R_OK;
}




//-------------------------------------------------------------------------
//sfs_Init - 初始化一个SFS设备
//
//@sfsDev:操作的设备句柄
//
//Note:
//
//Return: SYS_R_OK on success, errno otherwise
//-------------------------------------------------------------------------
sys_res sfs_Init(sfs_dev p)
{
	sys_res res;
	adr_t nAdr, nEnd;
	t_sfs_ste blk;
	uint_t nSize = flash_BlkSize(p->dev);

	sfs_Lock();
	//擦除Flash,清空
	blk.ste = SFS_BLK_IDLE;
	nAdr = p->start;
	nEnd = nAdr + nSize * p->blk;
	for (; nAdr < nEnd; nAdr += nSize) {
		res = flash_nolockErase(p->dev, nAdr);
		if (res != SYS_R_OK) {
			sfs_Unlock();
			return res;
		}
	}
	//置第一块Block为激活状态
	blk.ste = SFS_BLK_ACTIVE;
	res = flash_nolockProgram(p->dev, p->start, (uint8_t *)&blk, sizeof(blk));
	sfs_Unlock();
	return res;
}

//-------------------------------------------------------------------------
//sfs_Write - 写入一条记录
//
//@sfsDev: 操作的设备句柄
//@nParam: 记录标识号
//@buf: 数据指针
//@nDataLen: 数据长度
//
//Note: 数据长度范围0 ~ 655535
//
//Return: SYS_R_OK on success, errno otherwise
//-------------------------------------------------------------------------
sys_res sfs_Write(sfs_dev p, t_sfs_id nRecord, const void *pData, uint_t nLen)
{
	sys_res res;

	sfs_Lock();
	res = _sfs_Write(p, nRecord, pData, nLen);
	sfs_Unlock();
	return res;
}

//-------------------------------------------------------------------------
//_sfs_Read - 读出一条记录
//
//@sfsDev: 操作的设备句柄
//@nParam: 记录标识号
//@pData: 数据指针
//
//Note:
//
//Return: 成功读取的数据长度, errno otherwise
//-------------------------------------------------------------------------
sys_res sfs_Read(sfs_dev p, t_sfs_id nRecord, void *pData, uint_t nLen)
{
	sys_res res = SYS_R_ERR;
	adr_t nIdx;
	t_sfs_idx xIdx;

	sfs_Lock();
	if ((nIdx = _sfs_Find(p, SFS_RECORD_MASK, nRecord, &xIdx)) != 0) {
		//找到记录,读取
		memcpy(pData, (uint8_t *)(nIdx + sizeof(t_sfs_idx)), MIN(nLen, xIdx.len));
		res = SYS_R_OK;
	}
	sfs_Unlock();
	return res;
}

sys_res sfs_ReadRandom(sfs_dev p, t_sfs_id nRecord, void *pData, uint_t nOffset, uint_t nLen)
{
	sys_res res = SYS_R_ERR;
	adr_t nIdx;
	t_sfs_idx xIdx;

	sfs_Lock();
	if ((nIdx = _sfs_Find(p, SFS_RECORD_MASK, nRecord, &xIdx)) != 0) {
		//找到记录,读取
		if (nOffset < xIdx.len) {
			memcpy(pData, (uint8_t *)(nIdx + sizeof(t_sfs_idx) + nOffset), MIN(nLen, xIdx.len - nOffset));
			res = SYS_R_OK;
		}
	}
	sfs_Unlock();
	return res;
}

sys_res sfs_Read2Buf(sfs_dev p, t_sfs_id nRecord, buf b)
{
	sys_res res = SYS_R_ERR;
	adr_t nIdx;
	t_sfs_idx xIdx;

	sfs_Lock();
	if ((nIdx = _sfs_Find(p, SFS_RECORD_MASK, nRecord, &xIdx)) != 0) {
		//找到记录,读取
		if (buf_Push(b, (uint8_t *)(nIdx + sizeof(t_sfs_idx)), xIdx.len) == SYS_R_OK)
			res = SYS_R_OK;
		else
			res = SYS_R_EMEM;
	}
	sfs_Unlock();
	return res;
}

sys_res sfs_Find(sfs_dev p, t_sfs_id nRecord, buf b, uint_t nLen)
{
	sys_res res = SYS_R_ERR;
	adr_t nIdx, nEnd, nBlk, nBEnd;
	t_sfs_ste blk;
	t_sfs_idx xIdx;
	uint_t nSize = flash_BlkSize(p->dev);

	sfs_Lock();
	nBlk = p->start;
	nBEnd = nBlk + nSize * p->blk;
	for (; nLen && (nBlk < nBEnd); nBlk += nSize) {
		memcpy(&blk, (uint8_t *)nBlk, sizeof(blk));
		if (blk.ste != SFS_BLK_IDLE) {
			nIdx = nBlk + sizeof(blk);
			nEnd = nBlk + nSize;
			for (; nLen && (nIdx < nEnd); nIdx = ALIGN4(nIdx + sizeof(t_sfs_idx) + xIdx.len)) {
				memcpy(&xIdx, (uint8_t *)nIdx, sizeof(t_sfs_idx));
				if (((xIdx.id & nRecord) == nRecord) && (xIdx.ste == SFS_S_VALID)) {
					buf_Push(b, &xIdx.id, sizeof(xIdx.id));
					nLen -= 1;
					res = SYS_R_OK;
				}
			}
		}
	}
	sfs_Unlock();
	return res;
}


//-------------------------------------------------------------------------
//sfs_Info - 获取一条记录的信息
//
//@sfsDev:操作的设备句柄
//@nParam: 记录标识号
//@info:sfs_info结构体指针
//
//Note:
//
//Return: SYS_R_OK on success, errno otherwise
//-------------------------------------------------------------------------
sys_res sfs_Info(sfs_dev p, t_sfs_id nRecord, sfs_info info)
{
	sys_res res = SYS_R_ERR;
	t_sfs_idx xIdx;

	sfs_Lock();
	if (_sfs_Find(p, SFS_RECORD_MASK, nRecord, &xIdx) != 0) {
		//找到记录
		info->len = xIdx.len;
		res = SYS_R_OK;
	}
	sfs_Unlock();
	return res;
}


//-------------------------------------------------------------------------
//sfs_Delete - 删除一条记录
//
//@sfsDev:操作的设备句柄
//@nParam: 记录标识号
//
//Note:
//
//Return: SYS_R_OK on success, errno otherwise
//-------------------------------------------------------------------------
sys_res sfs_Delete(sfs_dev p, t_sfs_id nRecord)
{
	sys_res res = SYS_R_ERR;
	adr_t nIdx;
	t_sfs_idx xIdx;

	sfs_Lock();
	if ((nIdx = _sfs_Find(p, SFS_RECORD_MASK, nRecord, &xIdx)) != 0) {
		//找到记录,标记为无效
		xIdx.ste = SFS_S_INVALID;
		res = flash_nolockProgram(p->dev, nIdx, (uint8_t *)&xIdx, sizeof(t_sfs_idx));
	}
	sfs_Unlock();
	return res;
}

