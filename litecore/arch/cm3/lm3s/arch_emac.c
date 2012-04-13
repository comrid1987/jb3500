#if LM3S_ETH_ENABLE


int arch_EmacInit()
{
 

	return 0;
}

void arch_EmacAddr(uint8_t *pAdr)
{
	unsigned long nUser0, nUser1;

	MAP_FlashUserGet(&nUser0, &nUser1);
	if ((nUser0 == 0xffffffff) || (nUser1 == 0xffffffff)) {
		memcpy(&nUser0, &pAdr[0], 4);
		memcpy(&nUser1, &pAdr[4], 2);
		MAP_FlashUserSet(nUser0, nUser1);
	} else {
		memcpy(&pAdr[0], &nUser0, 4);
		memcpy(&pAdr[4], &nUser1, 2);
	}
	/* Program the hardware with it's MAC address (for filtering). */
	MAP_EthernetMACAddrSet(ETH_BASE, pAdr);
}

void arch_EmacIntEnable()
{

}


void arch_EmacIntDisable()
{

}

void arch_EmacPacketTx(const void *pData, uint_t nLen)
{
#if 0
	MAP_EthernetPacketPut(ETH_BASE, pData, nLen);
#else

	/* Send frame to EMAC ethernet controller */
	uint_t len, val;
	uint16_t *sp;

	/* Wait for current packet to complete. */
	while(!MAP_EthernetSpaceAvail(ETH_BASE));

	sp = (uint16_t *)pData;
	val = nLen - 14;
	val |= (*sp++) << 16;
	__raw_writel(val, ETH_BASE + MAC_O_DATA);
	/* Copy frame data to EMAC packet buffers. */
	for (len = (nLen + 1) >> 2; len; sp += 2, len--) {
		__raw_writel(sp[0] | ((uint_t)sp[1] << 16), ETH_BASE + MAC_O_DATA);
	}

	/* Start Transmitting */
    __raw_writel(MAC_TR_NEWTX, ETH_BASE + MAC_O_TR);
#endif
}

void arch_EmacIsr()
{
	OS_FRAME *frame;
	uint_t val, RxLen, len;
	uint16_t *dp;
    uint_t nStatus;

    //
    // Read and Clear the interrupt.
    //
    nStatus = MAP_EthernetIntStatus(ETH_BASE, false);
    MAP_EthernetIntClear(ETH_BASE, nStatus);

    //
    // Check to see if an RX Interrupt has occured.
    //
    if (nStatus & ETH_INT_RX) {

		/* Packet received, get the packet size. */
		val = __raw_readl(ETH_BASE + MAC_O_DATA);
		RxLen = (val & 0xFFFF) - 6;
		/* DMA adds also 4-byte CRC and 2-byte 'size' to packet size. */

		if (RxLen > ETH_MTU) {
			/* In case of error discard the entire RX FIFO. */
			__raw_writel(ETH_BASE + MAC_O_RCTL, __raw_readl(ETH_BASE + MAC_O_RCTL) | MAC_RCTL_RSTFIFO);
		}
		
		/* Flag 0x80000000 to skip sys_error() call when out of memory. */
		frame = alloc_mem (RxLen | 0x80000000);
		if (frame != NULL) {
			dp	= (U16 *)&frame->data[0];
			for (len = (RxLen + 3) >> 2; len; dp += 2, len--) {
				dp[0] = val >> 16;
				val = __raw_readl(ETH_BASE + MAC_O_DATA);
				dp[1] = val & 0xFFFF;
			}
			if ((RxLen - 1) & 0x02) {
				/* Drain the remaining 1 or 2 byte(s) of the CRC. */
				val = __raw_readl(ETH_BASE + MAC_O_DATA);
			}
			put_in_queue (frame);
		} else {
			/* In case of error discard the entire RX FIFO. */
			__raw_writel(ETH_BASE + MAC_O_RCTL, __raw_readl(ETH_BASE + MAC_O_RCTL) | MAC_RCTL_RSTFIFO);
		}
    }
}


#endif

