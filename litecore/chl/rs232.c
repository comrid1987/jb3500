#if UART_ENABLE


sys_res chl_rs232_Config(chl p, uint_t nBaud, uint_t nPari, uint_t nData, uint_t nStop)
{

	if (uart_Config(p->pIf, nBaud, nPari, nData, nStop) != SYS_R_OK)
		return SYS_R_ERR;
	p->ste = CHL_S_READY;
	return SYS_R_OK;
}


#endif

