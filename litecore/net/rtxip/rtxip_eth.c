/*----------------------------------------------------------------------------
 *      ENET Ethernet Driver Functions
 *----------------------------------------------------------------------------
 *  Required functions for Ethernet driver module:
 *  a. Polling mode: - void init_ethernet ()
 *                   - void send_frame (OS_FRAME *frame)
 *                   - void poll_ethernet (void)
 *  b. Interrupt mode: - void init_ethernet ()
 *                     - void send_frame (OS_FRAME *frame)
 *                     - void int_enable_eth ()
 *                     - void int_disable_eth ()
 *                     - interrupt function 
 *---------------------------------------------------------------------------*/

//Header Files
#include <drivers/dm9000.h>
#include <drivers/enc28j60.h>


//In Net_Config.c
extern uint8_t own_hw_adr[];

void init_ethernet()
{

#if LM3S_ETH_ENABLE
	arch_EmacInit();
	arch_EmacAddr(own_hw_adr);
#endif
#if LPC176X_ETH_ENABLE
	arch_EmacInit();
	arch_EmacAddr(own_hw_adr);
#endif
}
 
void send_frame(OS_FRAME *frame)
{

	/* Send frame to ENET ethernet controller */
#if DM9000_ENABLE
	dm9000_PacketSend(frame->data, frame->length);
#endif
#if ENC28J60_ENABLE
	enc28j60_PacketSend(frame->data, frame->length);
#endif
#if LM3S_ETH_ENABLE
	arch_EmacPacketTx(frame->data, frame->length);
#endif
#if LPC176X_ETH_ENABLE
	arch_EmacPacketTx(frame->data, frame->length);
#endif
}

#if 0
void int_enable_eth()
{

#if DM9000_ENABLE
	dm9000_Control(1, NULL);
#endif
#if ENC28J60_ENABLE
	enc28j60_WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE);
#endif
#if LM3S_ETH_ENABLE
	arch_EmacIntEnable();
#endif
#if LPC176X_ETH_ENABLE
	arch_EmacIntEnable();
#endif
}

void int_disable_eth()
{

#if DM9000_ENABLE
	dm9000_Control(0, NULL);
#endif
#if ENC28J60_ENABLE
	enc28j60_WriteOp(ENC28J60_BIT_FIELD_CLR, EIE, EIE_INTIE);
#endif
#if LM3S_ETH_ENABLE
	arch_EmacIntDisable();
#endif
#if LPC176X_ETH_ENABLE
	arch_EmacIntDisable();
#endif
}

#else

void poll_ethernet()
{

#if DM9000_ENABLE
	dm9000_PacketReceive();
#endif
#if ENC28J60_ENABLE
	enc28j60_IntHandler();
#endif
#if LM3S_ETH_ENABLE
	arch_EmacIsr();
#endif
#if LPC176X_ETH_ENABLE
	arch_EmacIsr();
#endif
}

#endif


