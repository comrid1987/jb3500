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

//Local Variables
static uint_t eth_inited = 0;

void init_ethernet()
{

#if STM32_ETH_ENABLE || LM3S_ETH_ENABLE || LPC176X_ETH_ENABLE
	if (arch_EmacInit() == 0) {
		eth_inited = 1;
		arch_EmacAddr(own_hw_adr);
	} else {
		eth_inited = 0;
	}
#endif
}
 
void send_frame(OS_FRAME *frame)
{

	/* Send frame to ENET ethernet controller */
	if (eth_inited) {
#if DM9000_ENABLE
		dm9000_PacketSend(&frame->data[0], frame->length);
#endif
#if ENC28J60_ENABLE
		enc28j60_PacketSend(&frame->data[0], frame->length);
#endif
#if STM32_ETH_ENABLE || LM3S_ETH_ENABLE || LPC176X_ETH_ENABLE
		arch_EmacPacketTx(&frame->data[0], frame->length);
#endif
	}
}

#if ETH_INT_ENABLE
void int_enable_eth()
{

	if (eth_inited) {
#if DM9000_ENABLE
		dm9000_Control(1, NULL);
#endif
#if ENC28J60_ENABLE
		enc28j60_WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE);
#endif
#if STM32_ETH_ENABLE || LM3S_ETH_ENABLE || LPC176X_ETH_ENABLE
		arch_EmacIntEnable();
#endif
	}
}

void int_disable_eth()
{

	if (eth_inited) {
#if DM9000_ENABLE
		dm9000_Control(0, NULL);
#endif
#if ENC28J60_ENABLE
		enc28j60_WriteOp(ENC28J60_BIT_FIELD_CLR, EIE, EIE_INTIE);
#endif
#if STM32_ETH_ENABLE || LM3S_ETH_ENABLE || LPC176X_ETH_ENABLE
		arch_EmacIntDisable();
#endif
}

#else

void poll_ethernet()
{

	if (eth_inited) {
#if DM9000_ENABLE
		dm9000_PacketReceive();
#endif
#if ENC28J60_ENABLE
		enc28j60_IntHandler();
#endif
#if STM32_ETH_ENABLE || LM3S_ETH_ENABLE || LPC176X_ETH_ENABLE
		arch_EmacIsr();
#endif
	}
}

#endif


