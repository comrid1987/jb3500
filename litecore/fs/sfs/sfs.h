#ifndef __SFS_H__
#define __SFS_H__


//Public Defines





//Public Typedefs
typedef t_flash_dev		sfs_dev[1];

typedef struct {
	uint16_t len;
}sfs_info[1];



//Externel Functions
sys_res sfs_Init(sfs_dev pDev);
sys_res sfs_Write(sfs_dev pDev, uint32_t nRecord, const void *pData, uint_t nLen);
sys_res sfs_Read(sfs_dev pDev, uint32_t nRecord, void *pData);
sys_res sfs_Read2Buf(sfs_dev pDev, uint32_t nRecord, buf b);
sys_res sfs_Find(sfs_dev pDev, uint32_t nPar, buf b, uint_t nLen);
sys_res sfs_Info(sfs_dev pDev, uint32_t nRecord, sfs_info info);
sys_res sfs_Delete(sfs_dev pDev, uint32_t nRecord);




#endif


