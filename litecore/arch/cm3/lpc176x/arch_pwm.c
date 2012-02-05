

static uint_t arch_PwmId(uint_t nPort, uint_t nPin)
{
	uint_t nPwm = 0;

	switch (nPort) {
	case GPIO_P1:
		switch (nPin) {
		case 18:
			nPwm = 1;
			break;
		case 20:
			nPwm = 2;
			break;
		case 21:
			nPwm = 3;
			break;
		case 23:
			nPwm = 4;
			break;
		case 24:
			nPwm = 5;
			break;
		case 26:
			nPwm = 6;
			break;
		default:
			break;
		}
		break;
	case GPIO_P2:
		nPwm = nPin + 1;
		break;
	case GPIO_P3:
		if (nPin == 25)
			nPwm = 2;
		else
			nPwm = 3;
		break;
	default:
		break;
	}
	return nPwm;
}








void arch_PwmConf(uint_t nPort, uint_t nPin, uint_t nMode, uint_t nFreq)
{
	uint_t nTemp;

	switch (nPort) {
	case GPIO_P1:
		nTemp = 2;
		break;
	case GPIO_P2:
		nTemp = 1;
		break;
	case GPIO_P3:
		nTemp = 3;
		break;
	default:
		return;
	}
	//Set IO function
	arch_GpioConf(nPort, nPin, nMode, GPIO_INIT_HIGH);
	//Set as PWM function
	arch_GpioSel(nPort, nPin, nTemp);

	//Set PWM Register
	nFreq = arch_TimerClockGet() / nFreq / 2 - 1;
	SETBIT(LPC_SC->PCONP, 6);
	LPC_PWM1->TCR = BITMASK(1);
	LPC_PWM1->TC = 0;
	LPC_PWM1->IR = 0;
	LPC_PWM1->CTCR = 0;
	LPC_PWM1->PR = nFreq;
	LPC_PWM1->MR0 = 1;
	LPC_PWM1->MCR = BITMASK(1);
	LPC_PWM1->TCR = BITMASK(0);
}


void arch_PwmStart(uint_t nPort, uint_t nPin)
{

	SETBIT(LPC_PWM1->PCR, 8 + arch_PwmId(nPort, nPin));
}

void arch_PwmStop(uint_t nPort, uint_t nPin)
{

	CLRBIT(LPC_PWM1->PCR, 8 + arch_PwmId(nPort, nPin));
}

