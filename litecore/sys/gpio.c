#if HC165_ENABLE
#include <drivers/hc165.h>
#endif
#if HC595_ENABLE
#include <drivers/hc595.h>
#endif
#if PCA955X_ENABLE
#include <drivers/pca955x.h>
#endif
#if SC16IS7X_ENABLE
#include <drivers/sc16is7x.h>
#endif


void sys_GpioConf(tbl_gpio_def p)
{

	switch (p->type) {
	case GPIO_T_INT:
		arch_GpioConf(p->port, p->pin, p->mode, p->init);
		break;
	default:
		break;
	}
}

int sys_GpioRead(tbl_gpio_def p)
{
	int nR = 0;

	switch (p->type) {
	default:
		nR = arch_GpioRead(p->port, p->pin);
		break;
#if HC165_ENABLE
	case GPIO_T_HC165:
		nR = GETBIT(hc165_Read(), p->pin);
		break;
#endif
#if PCA955X_ENABLE
	case GPIO_T_PCA955X: {
		p_dev_i2c pI2c;
		if ((pI2c = i2c_Get(PCA955X_COMID, OS_TMO_FOREVER)) != NULL) {
			nR = pca955x_GpioRead(pI2c, p->pin);
			i2c_Release(pI2c);
		}
		}break;
#endif
#if SC16IS7X_ENABLE
	case GPIO_T_SC16IS7X:
		nR = sc16is7x_GpioRead(p->pin);
		break;
#endif
	}
	return nR;
}

void sys_GpioSet(tbl_gpio_def p, uint_t nHL)
{

	switch (p->type) {
	default:
		arch_GpioSet(p->port, p->pin, nHL);
		break;	
#if HC595_ENABLE
	case GPIO_T_HC595:
		hc595_Write(p->pin, nHL);
	    break;
#endif
#if PCA955X_ENABLE
	case GPIO_T_PCA955X: {
		p_dev_i2c pI2c;
		if ((pI2c = i2c_Get(PCA955X_COMID, OS_TMO_FOREVER)) != NULL) {
			pca955x_GpioSet(pI2c, p->pin, nHL);
			i2c_Release(pI2c);
		}
		}break;
#endif
#if SC16IS7X_ENABLE
	case GPIO_T_SC16IS7X:
		sc16is7x_GpioSet(p->pin, nHL);
		break;
#endif

	}
}								  
