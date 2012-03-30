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
#define SFS_DEBUG_METHOD		0

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

typedef t_flash_blk				t_sfs_blk;
typedef t_flash_blk *			p_sfs_blk;









//Internal Functions
static sys_res _sfs_Erase(sfs_dev pDev, adr_t nAdr)
{

	return flash_nolockErase(pDev->dev, nAdr);
}

static sys_res _sfs_Program(sfs_dev pDev, adr_t nAdr, const void *pData, uint_t nLen)
{

	return flash_nolockProgram(pDev->dev, nAdr, pData, nLen);
}

static adr_t _sfs_Find(sfs_dev pDev, uint32_t nAnd, uint32_t nRecord, p_sfs_idx pIdx)
{
	adr_t nIdx, nEnd;
	const t_sfs_blk *pBlk, *pEnd;
	t_sfs_ste blk;

	nRecord &= nAnd;
	for (pBlk = pDev->tbl, pEnd = &pDev->tbl[pDev->blk]; pBlk < pEnd; pBlk++) {
		memcpy(&blk, (uint8_t *)pBlk->start, sizeof(blk));
		if (blk.ste != SFS_BLK_IDLE) {
			nIdx = pBlk->start + sizeof(blk);
			nEnd = pBlk->start + pBlk->size;
			for (; nIdx < nEnd; nIdx = ALIGN4(nIdx + sizeof(t_sfs_idx) + pIdx->len)) {
				memcpy(pIdx, (uint8_t *)nIdx, sizeof(t_sfs_idx));
				if (((pIdx->id & nAnd) == nRecord) && (pIdx->ste == SFS_S_VALID))
					return nIdx;
			}
		}
	}
	return 0;
}

static int _sfs_Free(sfs_dev pDev, const t_sfs_blk *pBlk, adr_t *pAdrIdx)
{
	adr_t nEnd;
	t_sfs_idx xIdx;

	*pAdrIdx = pBlk->start + sizeof(t_sfs_ste);
	nEnd = pBlk->start + pBlk->size;
	for (; *pAdrIdx < nEnd; *pAdrIdx = ALIGN4(*pAdrIdx + sizeof(t_sfs_idx) + xIdx.len)) {
		memcpy(&xIdx, (uint8_t *)*pAdrIdx, sizeof(t_sfs_idx));
		if (xIdx.ste == SFS_S_IDLE)
			return (nEnd - (*pAdrIdx + sizeof(t_sfs_idx)));
	}
	return 0;
}

static sys_res _sfs_Write(sfs_dev pDev, uint32_t nRecord, const void *pData, uint_t nLen)
{
	sys_res res;
	uint_t i, nIsFull = 1;
	adr_t nAdrOld = 0, nIdx, nEnd, nIdxNext;
	p_sfs_blk pBlk, pEnd, pAct = NULL;
	t_sfs_ste xBlk;
	t_sfs_idx xIdx;

	//恢复拷贝原有有效数据时意外中断
	pBlk = &pDev->tbl[0];
	pEnd = &pDev->tbl[pDev->blk - 1];
	memcpy(&xBlk, (uint8_t *)pBlk->start, sizeof(t_sfs_ste));
	if (xBlk.ste == SFS_BLK_ACTIVE) {
		memcpy(&xBlk, (uint8_t *)pEnd->start, sizeof(t_sfs_ste));
		if (xBlk.ste == SFS_BLK_ACTIVE)
			pAct = pEnd;
	}
	//查找原记录和激活的块
	for (; pBlk <= pEnd; pBlk++) {
		memcpy(&xBlk, (uint8_t *)pBlk->start, sizeof(t_sfs_ste));
		if ((pAct == NULL) && (xBlk.ste == SFS_BLK_ACTIVE))
			pAct = pBlk;
		if ((nAdrOld == 0) && (xBlk.ste != SFS_BLK_IDLE)) {
			nIdx = pBlk->start + sizeof(t_sfs_ste);
			nEnd = pBlk->start + pBlk->size;
			for (; nIdx < nEnd; nIdx = ALIGN4(nIdx + sizeof(t_sfs_idx) + xIdx.len)) {
				memcpy(&xIdx, (uint8_t *)nIdx, sizeof(t_sfs_idx));
				if ((xIdx.id == nRecord) && (xIdx.ste == SFS_S_VALID)) {
					//找到原记录
					nAdrOld = nIdx;
					break;
				}
				//已到末尾
				if (xIdx.ste == SFS_S_IDLE) break;
			}
		}
		if (nAdrOld && (pAct != NULL)) break;
	}
	if (pAct == NULL) {
		//未找到激活的块
		pAct = &pDev->tbl[0];
 		if ((res = _sfs_Erase(pDev, pAct->start))!= SYS_R_OK)
			return res;
		//置第一块Block为激活状态
		xBlk.ste = SFS_BLK_ACTIVE;
 		if ((res = _sfs_Program(pDev, pAct->start, (uint8_t *)&xBlk, sizeof(t_sfs_ste)))!= SYS_R_OK)
			return res;
	}
	//查找空间
	if (_sfs_Free(pDev, pAct, &nIdx) >= (int)(nLen + sizeof(t_sfs_idx)))
		//空间足,直接写入数据
		nIsFull = 0;
	//当前块ID
	i = pAct - pDev->tbl;
	while (nIsFull) {
		//该块空间不足,需要分配新块
		pAct = &pDev->tbl[i];
		i = cycle(i, 0, pDev->blk - 1, 1);
		pEnd = &pDev->tbl[i];
		pBlk = &pDev->tbl[cycle(i, 0, pDev->blk - 1, 1)];
		memcpy(&xBlk, (uint8_t *)pEnd->start, sizeof(t_sfs_ste));
		if (xBlk.ste != SFS_BLK_IDLE) {
			//擦除NextBlk
 			if ((res = _sfs_Erase(pDev, pEnd->start))!= SYS_R_OK)
				return res;
		}
		//置NextBlk为新Act
		xBlk.ste = SFS_BLK_ACTIVE;
 		if ((res = _sfs_Program(pDev, pEnd->start, (uint8_t *)&xBlk, sizeof(t_sfs_ste)))!= SYS_R_OK)
			return res;
		//拷贝原有的有效记录
		memcpy(&xBlk, (uint8_t *)pBlk->start, sizeof(t_sfs_ste));
		if (xBlk.ste != SFS_BLK_IDLE) {
			nIdx = pBlk->start + sizeof(t_sfs_ste);
			nEnd = pBlk->start + pBlk->size;
			nIdxNext = pEnd->start + sizeof(t_sfs_ste);
			for (; nIdx < nEnd; nIdx = ALIGN4(nIdx + sizeof(t_sfs_idx) + xIdx.len)) {
				memcpy(&xIdx, (uint8_t *)nIdx, sizeof(t_sfs_idx));
				if (xIdx.ste == SFS_S_VALID) {
					//找到有效记录
					if (nAdrOld != nIdx) {
 						if ((res = _sfs_Program(pDev, nIdxNext, (uint8_t *)nIdx, sizeof(t_sfs_idx) + xIdx.len))!= SYS_R_OK)
							return res;
						nIdxNext += ALIGN4(sizeof(t_sfs_idx) + xIdx.len);
					} else
						nAdrOld = 0;
				}
			}
		}
		//置原ActBlk为Full
		xBlk.ste = SFS_BLK_FULL;
 		if ((res = _sfs_Program(pDev, pAct->start, (uint8_t *)&xBlk, sizeof(t_sfs_ste)))!= SYS_R_OK)
			return res;
		//擦除OldBlk
 		if ((res = _sfs_Erase(pDev, pBlk->start))!= SYS_R_OK)
			return res;
		if (_sfs_Free(pDev, pEnd, &nIdx) >= nLen)
			//空间足,写入数据
			nIsFull = 0;
	}
	//置原纪录为待删除状态
	//if (nAdrOld) {
	//	_sfs_Read(pDev, nAdrOld, (uint8 *)&xIdx, sizeof(t_sfs_idx));
	//	xIdx.ste = SFS_S_DUMPING;
	//	_sfs_Program(pDev, nAdrOld, (uint8 *)&xIdx, sizeof(t_sfs_idx));
	//}
	//写新记录
	xIdx.ste = SFS_S_VALID;
	xIdx.id = nRecord;
	xIdx.len = nLen;
 	if ((res = _sfs_Program(pDev, nIdx, (uint8_t *)&xIdx, sizeof(t_sfs_idx)))!= SYS_R_OK)
		return res;
 	if ((res = _sfs_Program(pDev, nIdx + sizeof(t_sfs_idx), (uint8_t *)pData, nLen))!= SYS_R_OK)
		return res;
	//删除原记录
	if (nAdrOld) {
		memcpy(&xIdx, (uint8_t *)nAdrOld, sizeof(t_sfs_idx));
		xIdx.ste = SFS_S_INVALID;
 		if ((res = _sfs_Program(pDev, nAdrOld, (uint8_t *)&xIdx, sizeof(t_sfs_idx)))!= SYS_R_OK)
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
sys_res sfs_Init(sfs_dev pDev)
{
	sys_res res;
	uint_t i;
	t_sfs_ste blk;

	sfs_Lock();
	//擦除Flash,清空
	blk.ste = SFS_BLK_IDLE;
	for (i = 0; i < pDev->blk; i++) {
		res = _sfs_Erase(pDev, pDev->tbl[i].start);
		if (res != SYS_R_OK) {
			sfs_Unlock();
			return res;
		}
	}
	//置第一块Block为激活状态
	blk.ste = SFS_BLK_ACTIVE;
	res = _sfs_Program(pDev, pDev->tbl[0].start, (uint8_t *)&blk, sizeof(blk));
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
sys_res sfs_Write(sfs_dev pDev, t_sfs_id nRecord, const void *pData, uint_t nLen)
{
	sys_res res;

	sfs_Lock();
	res = _sfs_Write(pDev, nRecord, pData, nLen);
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
sys_res sfs_Read(sfs_dev pDev, t_sfs_id nRecord, void *pData)
{
	sys_res res = SYS_R_ERR;
	adr_t nIdx;
	t_sfs_idx xIdx;

	sfs_Lock();
	if ((nIdx = _sfs_Find(pDev, SFS_RECORD_MASK, nRecord, &xIdx)) != 0) {
		//找到记录,读取
		memcpy(pData, (uint8_t *)(nIdx + sizeof(t_sfs_idx)), xIdx.len);
		res = SYS_R_OK;
	}
	sfs_Unlock();
	return res;
}

sys_res sfs_ReadRandom(sfs_dev pDev, t_sfs_id nRecord, void *pData, uint_t nOffset, uint_t nLen)
{
	sys_res res = SYS_R_ERR;
	adr_t nIdx;
	t_sfs_idx xIdx;

	sfs_Lock();
	if ((nIdx = _sfs_Find(pDev, SFS_RECORD_MASK, nRecord, &xIdx)) != 0) {
		//找到记录,读取
		if (nOffset < xIdx.len) {
			memcpy(pData, (uint8_t *)(nIdx + sizeof(t_sfs_idx) + nOffset), MIN(nLen, xIdx.len - nOffset));
			res = SYS_R_OK;
		}
	}
	sfs_Unlock();
	return res;
}

sys_res sfs_Read2Buf(sfs_dev pDev, t_sfs_id nRecord, buf b)
{
	sys_res res = SYS_R_ERR;
	adr_t nIdx;
	t_sfs_idx xIdx;

	sfs_Lock();
	if ((nIdx = _sfs_Find(pDev, SFS_RECORD_MASK, nRecord, &xIdx)) != 0) {
		//找到记录,读取
		if (buf_Push(b, (uint8_t *)(nIdx + sizeof(t_sfs_idx)), xIdx.len) == SYS_R_OK)
			res = SYS_R_OK;
		else
			res = SYS_R_EMEM;
	}
	sfs_Unlock();
	return res;
}

sys_res sfs_Find(sfs_dev pDev, t_sfs_id nRecord, buf b, uint_t nLen)
{
	sys_res res = SYS_R_ERR;
	adr_t nIdx, nEnd;
	p_sfs_blk pBlk, pEnd;
	t_sfs_ste blk;
	t_sfs_idx xIdx;

	sfs_Lock();
	for (pBlk = pDev->tbl, pEnd = &pDev->tbl[pDev->blk]; nLen && (pBlk < pEnd); pBlk++) {
		memcpy(&blk, (uint8_t *)pBlk->start, sizeof(blk));
		if (blk.ste != SFS_BLK_IDLE) {
			nIdx = pBlk->start + sizeof(blk);
			nEnd = pBlk->start + pBlk->size;
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
sys_res sfs_Info(sfs_dev pDev, t_sfs_id nRecord, sfs_info info)
{
	sys_res res = SYS_R_ERR;
	t_sfs_idx xIdx;

	sfs_Lock();
	if (_sfs_Find(pDev, SFS_RECORD_MASK, nRecord, &xIdx) != 0) {
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
sys_res sfs_Delete(sfs_dev pDev, t_sfs_id nRecord)
{
	sys_res res = SYS_R_ERR;
	adr_t nIdx;
	t_sfs_idx xIdx;

	sfs_Lock();
	if ((nIdx = _sfs_Find(pDev, SFS_RECORD_MASK, nRecord, &xIdx)) != 0) {
		//找到记录,标记为无效
		xIdx.ste = SFS_S_INVALID;
		res = _sfs_Program(pDev, nIdx, (uint8_t *)&xIdx, sizeof(t_sfs_idx));
	}
	sfs_Unlock();
	return res;
}

