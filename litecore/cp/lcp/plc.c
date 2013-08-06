
static sys_res ecl_Plc_AddrSet(t_gw3762 *p, uint_t nTmo)
{
	t_afn04_f85 xF85;
	uint8_t aAdr[6];
	uint32_t nAdr;

	icp_ParaRead(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
	nAdr = bin2bcd32(xF85.addr);
#if 1
	memset(aAdr, 0, sizeof(aAdr));
	memcpy(&aAdr[0], &nAdr, 2);
#else
	memcpy(&aAdr[0], (void *)&xF85.area, 2);
	memcpy(&aAdr[2], &nAdr, 4);
#endif
	return gw3762_ModAdrSet(p, aAdr, nTmo);
}































