#ifndef __SFS_H__
#define __SFS_H__


#ifdef __cplusplus
extern "C" {
#endif



//Public Defines
#if SFS_RECORD_LEN == 4
#define t_sfs_id		uint32_t
#elif SFS_RECORD_LEN == 8
#define t_sfs_id		uint64_t
#else
#error "SFS_RECORD_LEN must be 4 or 8!!!"
#endif



//Public Typedefs
typedef t_flash_dev		sfs_dev[1];

typedef struct {
	uint16_t len;
}sfs_info[1];



//Externel Functions
sys_res sfs_Init(sfs_dev p);
sys_res sfs_Write(sfs_dev p, t_sfs_id nRecord, const void *pData, uint_t nLen);
sys_res sfs_Read(sfs_dev p, t_sfs_id nRecord, void *pData, uint_t nLen);
sys_res sfs_ReadRandom(sfs_dev p, t_sfs_id nRecord, void *pData, uint_t nOffset, uint_t nLen);
sys_res sfs_Read2Buf(sfs_dev p, t_sfs_id nRecord, buf b);
sys_res sfs_Find(sfs_dev p, t_sfs_id nRecord, buf b, uint_t nLen);
sys_res sfs_Info(sfs_dev p, t_sfs_id nRecord, sfs_info info);
sys_res sfs_Delete(sfs_dev p, t_sfs_id nRecord);


#ifdef __cplusplus
}
#endif

#endif


