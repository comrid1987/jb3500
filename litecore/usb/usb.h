#ifndef __SYS_USB_H__
#define __SYS_USB_H__


#ifdef __cplusplus
extern "C" {
#endif



//External Functions
void usb_Init(void);
void usb_HostHandler(void);
void usb_HostIRQ(void);
int usb_HostIsConnected(void *p);

#if USBH_MSC_ENABLE
int usb_HostMscRead(void *p, uint_t nSector, void *pBuf, uint_t nLen);
int usb_HostMscWrite(void *p, uint_t nSector, const void *pBuf, uint_t nLen);
#endif


#if USBH_HID_ENABLE
int usb_Keybrd_GetData(void);
void usb_Mouse_GetData(void);
#endif


#ifdef __cplusplus
}
#endif


#endif
