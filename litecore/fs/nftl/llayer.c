#ifndef _LLAYER_C_
#define _LLAYER_C_

/****************************************************************************
 *
 *            Copyright (c) 2005 by HCC Embedded 
 *
 * This software is copyrighted by and is the sole property of 
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,  
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1132
 * Victor Hugo Utca 11-15
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/
#include <fs/nftl/llayer.h>
#include <fs/nftl/prgmacro.h>
#include <fs/nftl/mdebug.h>




/****************************************************************************
 *
 * ll_init
 *
 * low level init function, this is called from mlayer lowinit function once
 *
 * RETURNS
 *
 * 0 - if successfuly
 * other if any error
 *
 ***************************************************************************/

unsigned char ll_init()
{

	if (nand_Identify() == SYS_R_OK)
		return 0;
	return 1;
}

/****************************************************************************
 *
 * ll_erase
 *
 * erase a block
 *
 * INPUTS
 *
 * pba - physical block address
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * LL_ERROR - if any error
 *
 ***************************************************************************/

unsigned char ll_erase(t_ba pba)
{
	u_byte4 xPage;

	xPage.n = pba * MAX_PAGE_PER_BLOCK;
	if (nand_EraseBlock(&xPage) != SYS_R_OK)
		return LL_ERROR;
	return LL_OK;
}


/****************************************************************************
 *
 * ll_write
 *
 * write a page 
 *
 * INPUTS
 *
 * pba - physical block address
 * ppo - physical page offset
 * buffer - page data to be written (data+spare)
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * LL_ERROR - if any error
 *
 ***************************************************************************/

unsigned char ll_write(t_ba pba, t_po ppo, unsigned char *buffer)
{
	u_byte4 xPage;
	uint32_t nEcc;
	ST_SPARE *sptr = GET_SPARE_AREA(buffer);

	xPage.n = pba * MAX_PAGE_PER_BLOCK + ppo;
	if (nand_ProgData(&xPage, buffer, &nEcc) != SYS_R_OK) {
		DEBPR("WritePage: error 0 pba %d ppo %d\n", pba, ppo);
		return LL_ERROR;
	}
	sptr->ecc = nEcc;
	if (nand_ProgSpare(&xPage, sptr) != SYS_R_OK) {
		DEBPR("WritePage: error 1 pba %d ppo %d\n", pba, ppo);
		return LL_ERROR;
	}
	return LL_OK;
}

/****************************************************************************
 *
 * ll_writedouble
 *
 * write a page from 2 buffers
 *
 * INPUTS
 *
 * pba - physical block address
 * ppo - physical page offset
 * buffer - 1st half of page data to be written
 * buffer - 2nd half of page data + spare data to be written 
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * LL_ERROR - if any error
 *
 ***************************************************************************/

unsigned char ll_writedouble(t_ba pba, t_po ppo, unsigned char *buffer0, unsigned char *buffer1)
{
	u_byte4 xPage;
	uint32_t nEcc;
	ST_SPARE *sptr = (ST_SPARE *)(buffer1 + MAX_DATA_SIZE / 2);

	xPage.n = pba * MAX_PAGE_PER_BLOCK + ppo;
	if (nand_ProgHalfDouble(&xPage, buffer0, buffer1, &nEcc) != SYS_R_OK) {
		DEBPR("WritePage: error 0 pba %d ppo %d\n", pba, ppo);
		return LL_ERROR;
	}
	sptr->ecc = nEcc;
	if (nand_ProgSpare(&xPage, sptr) != SYS_R_OK) {
		DEBPR("WritePage: error 1 pba %d ppo %d\n", pba, ppo);
		return LL_ERROR;
	}
	return LL_OK;
}


/****************************************************************************
 *
 * ll_read
 *
 * read a page 
 *
 * INPUTS
 *
 * pba - physical block address
 * ppo - physical page offset
 * buffer - page data pointer where to store data (data+spare)
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * LL_ERASED - if page is empty
 * LL_ERROR - if any error
 *
 ***************************************************************************/

unsigned char ll_read(t_ba pba, t_po ppo, unsigned char *buffer)
{
	u_byte4 xPage;
	uint_t i, nEcchi, nEcclo;
	uint32_t nEcc;
	ST_SPARE *sptr = GET_SPARE_AREA(buffer);

	xPage.n = pba * MAX_PAGE_PER_BLOCK + ppo;
	if (nand_ReadPage(&xPage, buffer, &nEcc) != SYS_R_OK) {
		DEBPR("ReadPage: error 0 pba %d ppo %d\n", pba, ppo);
		return LL_ERROR; /* read error */
	}
	nEcc ^= sptr->ecc;
	if (nEcc == 0) {
		DEBPR("ReadPage: ok pba %d ppo %d\n", pba, ppo);
		return LL_OK; /* no bit error */
    }
	nEcclo = nEcc & 0xFFFF;
	nEcchi = nEcc >> 16;
	if ((nEcchi + nEcclo) != 0xFFFF) {
		for (i = 0; i < NAND_PAGE_DATA + NAND_PAGE_SPARE - 4; i++) {
		   	if (buffer[i] != 0xFF) {
				DEBPR("ReadPage: error 1 pba %d ppo %d\n", pba, ppo);
				return LL_ERROR; /* nEcc error */
		   	}
		}
		if (sptr->ecc == 0xFFFFFFFF) {
			DEBPR("ReadPage: erased pba %d ppo %d\n", pba, ppo);
			return LL_ERASED; /* erased not written */
		}
		DEBPR("ReadPage: error 2 pba %d ppo %d\n", pba, ppo);
		return LL_ERROR; /* nEcc error */
	}
	buffer[nEcchi >> 3] ^= (1 << (nEcchi & 7)); /* correcting error */
	DEBPR("ReadPage: corrected pba %d ppo %d\n", pba, ppo);
	return LL_OK;
}


/****************************************************************************
 *
 * end of llayer.c
 *
 ***************************************************************************/

#endif //_LLAYER_C_

