#ifndef _FIL_BAD_C_
#define _FIL_BAD_C_

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

/****************************************************************************
 *
 * Bad Block handling
 *
 ***************************************************************************/

#include <fs/nftl/llayer.h>
#include <fs/nftl/mlayer.h>
#include <fs/nftl/fil_wear.h>
#include <fs/nftl/fil_bad.h>

/****************************************************************************
 *
 * variables
 *
 ***************************************************************************/

ST_BADBLOCK *gl_badblock; //pointer where information is stored, situated in 1 page with FLTable

static t_ba *gl_bad_table;			   //bad block table having bad block and reserved
	
static unsigned short gl_bad_maxentry; //currently used maximum number of entries
static unsigned short gl_bad_init_index; //used at formatting initialization

static unsigned char gl_badpagebuffer [ MAX_PAGE_SIZE ]; //buffer for storing/reading
static unsigned char gl_bbmodified;

/****************************************************************************
 *
 * bb_getmaxreserved
 *
 * retreives the number of reserved block, valid only after bb_init
 *
 * RETURNS
 *
 * number of used reserved entry
 *
 ***************************************************************************/

unsigned short bb_getmaxreserved() {
	return gl_bad_maxentry;
}

/****************************************************************************
 *
 * bb_init
 *
 * initializing bad block handler, this must be called at power on after
 * flash type are determined, after map page loaded
 *
 * INPUTS
 *
 * maxentry - maximum number of entries
 *
 ***************************************************************************/

unsigned char bb_init(unsigned short maxentry) {
ST_SPARE *sptr=GET_SPARE_AREA(gl_badpagebuffer);
unsigned short a;
unsigned char ret;
	if (maxentry > 256) maxentry=256;	  //maximum allowed in one page!

	gl_bad_table=(t_ba*)gl_badpagebuffer; //set up pointer

	gl_bad_maxentry=maxentry;
	gl_bad_init_index=0;

	if (gl_badblock->pba==BLK_NA) ret=0xf0; //not exist
	else if (gl_badblock->ppo>=MAX_PAGE_PER_BLOCK) ret=0xf1; //invalid page
    else ret=ll_read(gl_badblock->pba,gl_badblock->ppo,gl_badpagebuffer);

	if (!ret) {
		if (sptr->block_type!=BLK_TYPE_BAD) ret=0xf2; //it is correct page?
	}
	else {
		for (a=0; a<maxentry; a++) gl_bad_table[a]=BLK_NA; //reset all entry
	}

	sptr->bad_tag=0xff;				//signal good
	sptr->block_type=BLK_TYPE_BAD;	//set type

	gl_bbmodified=0;				//signal no modified

	return ret;
}

/****************************************************************************
 *
 * _bb_alloc
 *
 * internal function
 * allocate new bad block, it initiate all the variable in gl_badblock
 *
 * RETURNS
 *
 * 0 - if successfully
 * other if any error
 *
 ***************************************************************************/

static unsigned char _bb_alloc() {
ST_SPARE *sptr=GET_SPARE_AREA(gl_badpagebuffer);
unsigned char index;
t_ba pba;

	for (;;) {						   //main loop for allocation from FLTable
		wear_alloc();				   //try to allocate a new bad block storage

		index=gl_wear_allocstruct.free_index;

		if (index==INDEX_NA) return 1; //FATAL!	No more free block available

		pba = gl_freetable[index];
		sptr->wear = gl_wear_allocstruct.free_wear; //store its wear info

		gl_badblock->ppo = 0;		//reset page
		gl_badblock->index=index;   //set FLBTable index

		for (;;) {					//internal loop for swapping pba on index in FLTable
 
			gl_badblock->pba =(t_ba)(pba & FT_ADDRMASK);	//set bad block storage	pba

			gl_freetable[index] = (t_ba)(pba | FT_BAD);     //write back pba and signal it as locked or BAD
			wear_updatedynamicinfo(index,sptr->wear);//write back wear info
	
			if (!ll_erase(pba)) return 0;		//success, so return
	
			pba=bb_setasbb(pba);				//put pba into bb table, and get a new pba
			sptr->wear=0;						//reserved block wear is always 0

			if (pba==BLK_NA) break;				//check if this is exist, other case jump back
		}
	}
}

/****************************************************************************
 *
 * bb_flush
 *
 * this will write a new bad block table into flash if necessary
 * function must be called before writing FLTable
 *
 * RETURNS
 *
 * 0 - if successfully
 * other if any error
 *
 ***************************************************************************/

t_bit bb_flush() {
unsigned char index;

	if (!gl_bbmodified) return 0; //no save needed

	for (;;) { //avoid recursion and solving BB handling locally

		if (gl_badblock->ppo==INDEX_NA) { //check if allocation is needed
			if (_bb_alloc()) return 1;	   //FATAL
		}
		else {
			gl_badblock->ppo++;					//go to next position

			if (gl_badblock->ppo==MAX_PAGE_PER_BLOCK) {   //check if finished

				index=gl_badblock->index;
				wear_updatedynamicinfo(index,WEAR_NA); //lock it in allocation
				gl_freetable[index] &= ~FT_BAD;  	   //remove signal

				if (_bb_alloc()) return 1;	   //FATAL
			}
		}

		if (!ll_write(gl_badblock->pba,gl_badblock->ppo,gl_badpagebuffer)) {
			gl_bbmodified=0; //clear modified flag
			return 0; //if write success we can return
		}
		//other case we just try to write into the next page or allocate new block
	}
}

/****************************************************************************
 *
 * bb_addbb
 *
 * this function is used at formatting for creating initialized bad-block
 * when maxentry is reached then Bad-Block Map automatically flushed
 *
 * INPUTS
 *
 * pba - physical block address of a bad block
 *
 * RETURNS
 *
 * 0 - if successfully
 * other if any error
 *
 ***************************************************************************/

t_bit bb_addbb(t_ba pba) {
	if (gl_bad_init_index < gl_bad_maxentry) {
		pba|=FT_BAD;					//signal as originally bad block
		gl_bad_table[gl_bad_init_index]=pba;
		gl_bad_init_index++;
		gl_bbmodified=1;			   //set modification needed
		return 0;	//ok
	}

	return 1; //no space to store
}

/****************************************************************************
 *
 * bb_addreserved
 *
 * this function is used at formatting for creating reserved blocks
 * when maxentry is reached then Bad-Block Map automatically flushed
 *
 * INPUTS
 *
 * pba - physical block address of a reserved block
 *
 * RETURNS
 *
 * 0 - if successfully
 * other if any error
 *
 ***************************************************************************/

t_bit bb_addreserved(t_ba pba) {
	if (gl_bad_init_index < gl_bad_maxentry) {
		gl_bad_table[gl_bad_init_index]=pba;
		gl_bad_init_index++;
		gl_bbmodified=1;			   //set modification needed
		return 0;	//ok
	}

	return 1; //no space to store
}

/****************************************************************************
 *
 * bb_setasbb
 *
 * Runtime found bad block handler, this function needed to be called
 * whenewer a runtime bad block is found, this function will check if there 
 * is available reserved block, if it is then will returns with that PBA.
 * if it is return with BLK_NA, then program should call wear_alloc to
 * get a free block directy from FLBTable. Other case program should put
 * back this return value into FLBTable as "from now" useable pba.
 *
 * INPUTS
 *
 * pba - physical block address of a bad block
 *
 * RETURNS
 *
 * physical block address of a reserved block or BLK_NA if there are no
 * more reserved block.
 *
 ***************************************************************************/

t_ba bb_setasbb(t_ba pba) {   //只在擦除出错时才添加坏块(即调用此函数)
unsigned short a;
	for (a=0; a<gl_bad_maxentry; a++) {
		t_ba newpba=gl_bad_table[a];

		if (!(newpba & FT_BAD)) {
			gl_bad_table[a]=(t_ba)(pba | FT_BAD); //store BB as BB
			gl_bbmodified=1;			   //set modification needed
			return newpba;				   //return with new physical
		}
	}

	gl_bad_maxentry=0; //avoiding next searching if there is no more reserved

	return BLK_NA; //no more reserved block
}

/****************************************************************************
 *
 * end of fil_bad.c
 *
 ***************************************************************************/

#endif	//_FIL_BAD_C_

