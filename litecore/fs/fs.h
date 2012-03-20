#ifndef __FS_H__
#define __FS_H__

#ifdef __cplusplus
extern "C" {
#endif

//Include Headers
#include <fs/bkp/bkp.h>
#include <fs/sfs/sfs.h>
#include <fs/fs_posix.h>

//Public Defines




//External Functions
sys_res fs_init(void);
sys_res fs_usb_Mount(void);
sys_res fs_usb_Unmount(void);
sys_res fs_usb_IsReady(void);




#ifdef __cplusplus
}
#endif

#endif

