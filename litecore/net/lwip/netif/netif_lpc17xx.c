
#if TCPPS_TYPE == TCPPS_T_KEILTCP
#include <net/rtxip/Net_Config.h>


void arch_EmacIsr()
{
	uint32_t status, size, Index;
	OS_FRAME *frame;

	status = LPC_EMAC->IntStatus & LPC_EMAC->IntEnable;

	/* Clear the interrupt. */ 
	LPC_EMAC->IntClear = status; 
 
	if (status & INT_RX_DONE)
	{

		/* Disable EMAC RxDone interrupts. */
		LPC_EMAC->IntEnable = 0;

		/* a frame has been received */
		Index = LPC_EMAC->RxConsumeIndex;
		if (Index != LPC_EMAC->RxProduceIndex)
		{
			size = (RX_STAT_INFO(Index) & 0x7ff) + 1;
			if (size > ETH_FRAG_SIZE) size = ETH_FRAG_SIZE;

			/* Receive packet */
			/* Flag 0x80000000 to skip sys_error() call when out of memory. */
			frame = alloc_mem(ALIGN4(size) | 0x80000000);
			/* if 'alloc_mem()' has failed, ignore this packet. */
			if (frame != NULL) {
				/* copy the packet from the receive buffer */
				memcpy(frame->data, (void *)RX_BUF(Index), size);
				frame->length = size;
				put_in_queue(frame);
			}
			
			/* move Index to the next */
			if(++Index > LPC_EMAC->RxDescriptorNumber)
				Index = 0;

			/* set consume index */
			LPC_EMAC->RxConsumeIndex = Index;
		}

		/* Enable RxDone interrupt */
		LPC_EMAC->IntEnable = INT_RX_DONE;
	}
}



void lpcethif_Init(void)
{

}

 
#endif


