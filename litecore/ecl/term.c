

//Local Variables
static const SFS_BLOCK tbl_sfsSysCore[] = {
	0x60720000, 0x10000,
	0x60730000, 0x10000,
	0x60740000, 0x10000,
};
static const SFS_DEVICE sfs_SysCore = {SFS_DEV_NORFALSH, ARR_SIZE(tbl_sfsSysCore), tbl_sfsSysCore};


//External Variables
CORE_TER_KERNEL core_TerKernel;

int core_ResetTerComParam()
{

	return 0;
}

int core_ResetTerParam()
{
	CORE_TER_PARAM *p;

	if ((p = (CORE_TER_PARAM *)alloca(sizeof(CORE_TER_PARAM))) != NULL) {
		core_GetTerParam(p);
		//串口通讯参数
		p->stuRS232.nBaud = 115200;
		p->stuRS232.nParity = UART_PARITY_EVEN;
		p->stuRS232.nStop = UART_STOPBITS_1;
		p->stuRS232.nMode = UART_MODE_8D;
		//Irda通讯参数
		p->stuIrDA.nBaud = 9600;
		p->stuIrDA.nParity = UART_PARITY_EVEN;
		p->stuIrDA.nStop = UART_STOPBITS_1;
		p->stuIrDA.nMode = UART_MODE_8D;

		return core_SetTerParam(p);
	}
	return 0;
}

int core_GetTerParam(CORE_TER_PARAM *p)
{

	return sfs_Read(sfs_SysCore, CORE_SFS_SYS_TER_PARAM, (uint8_t *)p);
}

int core_SetTerParam(CORE_TER_PARAM *p)
{

	return sfs_Write(sfs_SysCore, CORE_SFS_SYS_TER_PARAM, (uint8_t *)p, sizeof(CORE_TER_PARAM));
}

int core_GetTerInfo(CORE_TER_INFO *p)
{

	return sfs_Read(sfs_SysCore, CORE_SFS_SYS_TER_INFO, (uint8_t *)p);
}

int core_SetTerInfo(CORE_TER_INFO *p)
{

	return sfs_Write(sfs_SysCore, CORE_SFS_SYS_TER_INFO, (uint8_t *)p, sizeof(CORE_TER_INFO));
}

int core_GetTnParam(uint32_t nTn, CORE_TN_PARAM *p)
{

	return sfs_Read(sfs_SysCore, CORE_SFS_SYS_TN_PARAM | nTn, (uint8_t *)p);
}

int core_SetTnParam(uint32_t nTn, CORE_TN_PARAM *p)
{

	return sfs_Write(sfs_SysCore, CORE_SFS_SYS_TN_PARAM | nTn, (uint8_t *)p, sizeof(CORE_TN_PARAM));
}

int core_DeleteTn(uint32_t nTn)
{

	return sfs_Delete(sfs_SysCore, CORE_SFS_SYS_TN_PARAM | nTn);
}

void core_Init()
{
	CORE_TER_PARAM *pTerParam;

	if ((pTerParam = (CORE_TER_PARAM *)alloca(sizeof(CORE_TER_PARAM))) != NULL) {
		core_GetTerParam(pTerParam);
		memcpy(&core_TerKernel, &pTerParam->stuKernel, sizeof(pTerParam->stuKernel));
	}
}



