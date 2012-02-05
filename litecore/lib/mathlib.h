#ifndef __MATHLIB_H__
#define __MATHLIB_H__


//-------------------------------------------------------------------------
//Mathematical Definitions
//-------------------------------------------------------------------------
#define PI2				(6.28318530717958647692528676654) //PIµÄ2±¶ 
#define PI				(3.14159265358979323846264338327) //PI

extern const float math_div10[];
extern const uint64_t math_pow10[];
extern const float math_sint[];


//-------------------------------------------------------------------------
//FixPoint
//-------------------------------------------------------------------------
typedef sint32_t fixpoint;
typedef sint64_t fixpoint_l;

typedef struct {
	fixpoint_l real;
	fixpoint_l imag;
}t_complex_fix_l;

typedef struct {
	fixpoint real;
	fixpoint imag;
}t_complex_fix;

#define EXP 			(13)
#define FIX2DOUBLE(x)	(((double)(x)) / (1UL << EXP))
#define DOUBLE2FIX(x)	((fixpoint)(((double)(x)) * (1UL << EXP)))
#define FIX2FLOAT(x)	(((float)(x)) / (1UL << EXP))
#define FLOAT2FIX(x)	((fixpoint)(((float)(x)) * (1UL << EXP)))
#define FIXP_EPSILON	(1.0 / (1UL << EXP))

uint32_t fix_Sqrt(uint64_t d, uint32_t N);
uint_t uintabs(uint_t a, uint_t b);


#endif




