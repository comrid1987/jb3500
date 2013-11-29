#ifndef __NRSEC3000_H__
#define __NRSEC3000_H__


#ifdef __cplusplus
extern "C" {
#endif

//Public Typedefs
typedef struct {
	uint_t		ste;
	p_dev_spi	spi;
}t_nrsec3000, *p_nrsec3000;

typedef struct {
	uint8_t cla;
	uint8_t ins;
	uint8_t p1;
	uint8_t p2;
    uint8_t p3;
}nrsec3000_cmd[1], *p_nrsec3000_cmd;



#ifdef __cplusplus
}
#endif

#endif


