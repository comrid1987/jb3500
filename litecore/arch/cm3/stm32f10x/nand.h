#ifndef __ARCH_NAND_H__
#define __ARCH_NAND_H__


#ifdef __cplusplus
extern "C" {
#endif












//A16 = CLE high
//A17 = ALE high

#if NAND_DATA_WIDTH == 16
#define __raw_nand_cmd(v)		__raw_writew(v, NAND_BASE_ADR | BITMASK(16))
#define __raw_nand_addr(v)		__raw_writew(v, NAND_BASE_ADR | BITMASK(17))
#else
#define __raw_nand_cmd(v)		__raw_writeb(v, NAND_BASE_ADR | BITMASK(16))
#define __raw_nand_addr(v)		__raw_writeb(v, NAND_BASE_ADR | BITMASK(17))
#endif


#if NAND_DATA_WIDTH == 16
#define __raw_nand_read			__raw_readw(NAND_BASE_ADR)
#else
#define __raw_nand_read			__raw_readb(NAND_BASE_ADR)
#endif





#ifdef __cplusplus
}
#endif

#endif

