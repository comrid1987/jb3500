#ifndef __ARCH_TYPEDEF_H__
#define __ARCH_TYPEDEF_H__


#ifdef __cplusplus
extern "C" {
#endif










//Public TypeDefs
typedef unsigned int			uint_t;
typedef signed char				sint8_t;
typedef unsigned char			uint8_t;
typedef short					sint16_t;
typedef unsigned short			uint16_t;
typedef int						sint32_t;
typedef unsigned int			uint32_t;
typedef long long				sint64_t;
typedef unsigned long long		uint64_t;
typedef unsigned char			bit_t;

typedef unsigned int			adr_t;



//InputOuput Defines
#define __raw_writeb(v,a)		(*(volatile uint8_t *)(a) = (v))
#define __raw_writew(v,a)		(*(volatile uint16_t *)(a) = (v))
#define __raw_writel(v,a)		(*(volatile uint32_t *)(a) = (v))

#define __raw_readb(a)			(*(volatile uint8_t *)(a))
#define __raw_readw(a)			(*(volatile uint16_t *)(a))
#define __raw_readl(a)			(*(volatile uint32_t *)(a))




#ifdef __cplusplus
}
#endif

#endif

