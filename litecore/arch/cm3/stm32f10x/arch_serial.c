

//Private Defines
#define SERIAL_USARTX				UART5


/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int sendchar(int ch)
{

	while ((SERIAL_USARTX->SR & USART_FLAG_TXE) == 0);
	SERIAL_USARTX->DR = ch & 0x1FF;
	return ch;
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int getkey(void)
{

	while ((SERIAL_USARTX->SR & USART_FLAG_RXNE) == 0);
	return ((int)(SERIAL_USARTX->DR & 0x1FF));
}


