

//Private Variables
static smartcard gw3761_sc;





sys_res gw3761_AuthorityInit()
{
	sys_res res;

	sc_Init(gw3761_sc);
	res = sc_Get(gw3761_sc, GW3761_ESAM_UARTID, OS_TICK_MS);
	return res;
}


int gw3761_ResponseAuthority(p_gw3761 p, buf b, u_word2 *pDu, uint8_t **ppData)
{
	int res = 0;
	uint_t j, nFn;

	if (gw3761_sc->ste == SC_S_ACTIVE) {
		for (j = 0; j < 8; j++) {
			if ((pDu->word[1] & BITMASK(j)) == 0)
				continue;
			nFn = gw3761_ConvertDt2Fn((pDu->word[1] & 0xFF00) | BITMASK(j));
			switch (nFn) {
			case 5:
				if (sc_GetRandom(gw3761_sc, 8) == SYS_R_OK) {
					buf_Push(b, gw3761_sc->resp, 8);
					buf_Push(b, gw3761_sc->sn, 8);
					res += 1;
				}
				break;
			default:
				break;
			}
		}
	}
	return res;
}



