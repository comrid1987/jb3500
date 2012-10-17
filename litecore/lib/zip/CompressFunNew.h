

#ifndef _COMPRESS_NEW_
#define _COMPRESS_NEW_


#define N       4096    /* buffer size */
#define F       60  /* lookahead buffer size */
#define THRESHOLD   2
#define NIL     N   /* leaf of tree */

#define N_CHAR      (256 - THRESHOLD + F)
                /* kinds of characters (character code = 0..N_CHAR-1) */
#define T       (N_CHAR * 2 - 1)    /* size of table */
#define R       (T - 1)         /* position of root */
#define MAX_FREQ    0x8000      /* updates tree when the */



#include "CrypFun.h"

	int Expand(DATA * buffer);
	int Compress(DATA * buffer);
	
/*
	DATA inbuffer, outbuffer;
	unsigned long int  textsize, codesize, printcount;
	
	int Encode();
	int Decode();
	void InitVar();
	unsigned char text_buf[N + F - 1];
	int match_position, match_length, lson[N + 1], rson[N + 257], dad[N + 1];
	unsigned freq[T + 1]; //frequency table 

	int prnt[T + N_CHAR]; // pointers to parent nodes, except for the 
            // elements [T..T + N_CHAR - 1] which are used to get 
            //the positions of leaves corresponding to the codes. 

	int son[T];   // pointers to child nodes (son[], son[] + 1) 

	unsigned getbuf;
	unsigned char getlen;
	unsigned long incount, outcount;
	unsigned putbuf;
	unsigned char putlen;
	unsigned code, len;

	void Error(char *message);
	void freememory();
	void InitTree();
	void InsertNode(int r);
	void DeleteNode(int p);
	int GetBit();
	int GetByte();
	void Putcode(int l, unsigned c);
	void StartHuff();
	void reconst();
	void update(int c);
	void EncodeChar(unsigned c);
	void EncodePosition(unsigned c);
	void EncodeEnd();
	int DecodeChar();
	int DecodePosition();
	
*/

#endif
