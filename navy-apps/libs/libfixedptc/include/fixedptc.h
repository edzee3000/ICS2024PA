#ifndef _FIXEDPTC_H_
#define _FIXEDPTC_H_

/*
 * fixedptc.h is a 32-bit or 64-bit fixed point numeric library.
 *
 * The symbol FIXEDPT_BITS, if defined before this library header file
 * is included, determines the number of bits in the data type (its "width").
 * The default width is 32-bit (FIXEDPT_BITS=32) and it can be used
 * on any recent C99 compiler. The 64-bit precision (FIXEDPT_BITS=64) is
 * available on compilers which implement 128-bit "long long" types. This
 * precision has been tested on GCC 4.2+.
 *
 * The FIXEDPT_WBITS symbols governs how many bits are dedicated to the
 * "whole" part of the number (to the left of the decimal point). The larger
 * this width is, the larger the numbers which can be stored in the fixedpt
 * number. The rest of the bits (available in the FIXEDPT_FBITS symbol) are
 * dedicated to the fraction part of the number (to the right of the decimal
 * point).
 *
 * Since the number of bits in both cases is relatively low, many complex
 * functions (more complex than div & mul) take a large hit on the precision
 * of the end result because errors in precision accumulate.
 * This loss of precision can be lessened by increasing the number of
 * bits dedicated to the fraction part, but at the loss of range.
 *
 * Adventurous users might utilize this library to build two data types:
 * one which has the range, and one which has the precision, and carefully
 * convert between them (including adding two number of each type to produce
 * a simulated type with a larger range and precision).
 *
 * The ideas and algorithms have been cherry-picked from a large number
 * of previous implementations available on the Internet.
 * Tim Hartrick has contributed cleanup and 64-bit support patches.
 *
 * == Special notes for the 32-bit precision ==
 * Signed 32-bit fixed point numeric library for the 24.8 format.
 * The specific limits are -8388608.999... to 8388607.999... and the
 * most precise number is 0.00390625. In practice, you should not count
 * on working with numbers larger than a million or to the precision
 * of more than 2 decimal places. Make peace with the fact that PI
 * is 3.14 here. :)
 */

/*-
 * Copyright (c) 2010-2012 Ivan Voras <ivoras@freebsd.org>
 * Copyright (c) 2012 Tim Hartrick <tim@edgecast.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef FIXEDPT_BITS
#define FIXEDPT_BITS	32
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if FIXEDPT_BITS == 32
typedef int32_t fixedpt;
typedef	int64_t	fixedptd;
typedef	uint32_t fixedptu;
typedef	uint64_t fixedptud;
#elif FIXEDPT_BITS == 64
typedef int64_t fixedpt;
typedef	__int128_t fixedptd;
typedef	uint64_t fixedptu;
typedef	__uint128_t fixedptud;
#else
#error "FIXEDPT_BITS must be equal to 32 or 64"
#endif

#ifndef FIXEDPT_WBITS
#define FIXEDPT_WBITS	24
#endif

#if FIXEDPT_WBITS >= FIXEDPT_BITS
#error "FIXEDPT_WBITS must be less than or equal to FIXEDPT_BITS"
#endif

#define FIXEDPT_VCSID "$Id$"

#define FIXEDPT_FBITS	(FIXEDPT_BITS - FIXEDPT_WBITS)  //32 - 24 = 8
#define FIXEDPT_FMASK	(((fixedpt)1 << FIXEDPT_FBITS) - 1) //0x000000ff

#define fixedpt_rconst(R) ((fixedpt)((R) * FIXEDPT_ONE + ((R) >= 0 ? 0.5 : -0.5)))
#define fixedpt_fromint(I) ((fixedptd)(I) << FIXEDPT_FBITS)
#define fixedpt_toint(F) ((F) >> FIXEDPT_FBITS)
#define fixedpt_add(A,B) ((A) + (B))
#define fixedpt_sub(A,B) ((A) - (B))
#define fixedpt_fracpart(A) ((fixedpt)(A) & FIXEDPT_FMASK)

#define FIXEDPT_ONE	((fixedpt)((fixedpt)1 << FIXEDPT_FBITS))
#define FIXEDPT_ONE_HALF (FIXEDPT_ONE >> 1)
#define FIXEDPT_TWO	(FIXEDPT_ONE + FIXEDPT_ONE)
#define FIXEDPT_PI	fixedpt_rconst(3.14159265358979323846)
#define FIXEDPT_TWO_PI	fixedpt_rconst(2 * 3.14159265358979323846)
#define FIXEDPT_HALF_PI	fixedpt_rconst(3.14159265358979323846 / 2)
#define FIXEDPT_E	fixedpt_rconst(2.7182818284590452354)

/* fixedpt is meant to be usable in environments without floating point support
 * (e.g. microcontrollers, kernels), so we can't use floating point types directly.
 * Putting them only in macros will effectively make them optional. */
#define fixedpt_tofloat(T) ((float) ((T)*((float)(1)/(float)(1L << FIXEDPT_FBITS))))



// 为了让大家更好地理解定点数的表示, 我们在fixedptc.h中去掉了一些API的实现, 你需要实现它们. 
/* Multiplies a fixedpt number with an integer, returns the result. */
static inline fixedpt fixedpt_muli(fixedpt A, int B) {
	return A*B;
}

/* Divides a fixedpt number with an integer, returns the result. */
static inline fixedpt fixedpt_divi(fixedpt A, int B) {
	return A/B;
}

/* Multiplies two fixedpt numbers, returns the result. */
static inline fixedpt fixedpt_mul(fixedpt A, fixedpt B) {
	int64_t A1=A; int64_t B1=B;
	int64_t temp = A1 * B1;
	temp>>=FIXEDPT_FBITS;
	return (fixedpt)temp;
}


/* Divides two fixedpt numbers, returns the result. */
static inline fixedpt fixedpt_div(fixedpt A, fixedpt B) {
	int64_t A1=A; int64_t B1=B;
	int64_t temp=A1<<FIXEDPT_FBITS;
	temp/=B1;
	return (fixedpt)temp;
}

static inline fixedpt fixedpt_abs(fixedpt A) {
	return A < 0 ? 0-A : A ;
}


// 关于fixedpt_floor()和fixedpt_ceil(), 你需要严格按照man中floor()和ceil()的语义来实现它们, 
// 否则在程序中用fixedpt_floor()代替floor()之后行为会产生差异, 在类似仙剑奇侠传这种规模较大的程序中,
// 这种差异导致的现象是非常难以理解的. 因此你也最好自己编写一些测试用例来测试你的实现.
static inline fixedpt fixedpt_floor(fixedpt A) {
	return A>>FIXEDPT_FBITS;
}

static inline fixedpt fixedpt_ceil(fixedpt A) {
	//注意对于ceil来说需要判断小数部分是否为零, 不是的话清零小数部分后加1  但是对于floor可以直接清零小数部分
	return ((A<<FIXEDPT_WBITS)==0)? A>>FIXEDPT_FBITS : (A>>FIXEDPT_FBITS)+1;
}

//如果要用的话需要经过测试  因为这个函数我还没有测试过
static inline fixedpt fixedpt_fromfloat(void *p)
{	
	//假设有一个void *p的指针变量, 它指向了一个32位变量, 这个变量的本质是float类型, 它的真值落在fixedpt类型可表示的范围中. 
	// 如果我们定义一个新的函数fixedpt fixedpt_fromfloat(void *p), 如何在不引入浮点指令的情况下实现它?
	//下面的都是针对于32位机器  根据KISS法则先不要弄得那么复杂
	fixedpt real_value=*(fixedpt*)p;  //取出真值
	fixedpt flag=real_value>>31;//计算正负
	fixedpt order= ((uint32_t)(real_value<<1))>>24 - 127 ; //对阶码进行操作，这里表示应该是2^xx次方
	fixedpt tail= real_value&0x007fffff | 0x00800000;  //这里是1.xxxxxxxx  取尾数  记得最高位要多一个1  一共是24位
	int right_shift_bit=24-9-order;//计算要右移多少位 当然也有可能是左移，取负即可
	tail = right_shift_bit>=0 ? tail>>right_shift_bit : tail<<(0-right_shift_bit); 
	return flag < 0 ? 0-tail : tail;//如果原来的小数为负的话取负，如果为正直接返回
}

/*
 * Note: adding and substracting fixedpt numbers can be done by using
 * the regular integer operators + and -.
 */

/**
 * Convert the given fixedpt number to a decimal string.
 * The max_dec argument specifies how many decimal digits to the right
 * of the decimal point to generate. If set to -1, the "default" number
 * of decimal digits will be used (2 for 32-bit fixedpt width, 10 for
 * 64-bit fixedpt width); If set to -2, "all" of the digits will
 * be returned, meaning there will be invalid, bogus digits outside the
 * specified precisions.
 */
void fixedpt_str(fixedpt A, char *str, int max_dec);

/* Converts the given fixedpt number into a string, using a static
 * (non-threadsafe) string buffer */
static inline char* fixedpt_cstr(const fixedpt A, const int max_dec) {
	static char str[25];

	fixedpt_str(A, str, max_dec);
	return (str);
}


/* Returns the square root of the given number, or -1 in case of error */
fixedpt fixedpt_sqrt(fixedpt A);


/* Returns the sine of the given fixedpt number. 
 * Note: the loss of precision is extraordinary! */
fixedpt fixedpt_sin(fixedpt fp);


/* Returns the cosine of the given fixedpt number */
static inline fixedpt fixedpt_cos(fixedpt A) {
	return (fixedpt_sin(FIXEDPT_HALF_PI - A));
}


/* Returns the tangens of the given fixedpt number */
static inline fixedpt fixedpt_tan(fixedpt A) {
	return fixedpt_div(fixedpt_sin(A), fixedpt_cos(A));
}


/* Returns the value exp(x), i.e. e^x of the given fixedpt number. */
fixedpt fixedpt_exp(fixedpt fp);


/* Returns the natural logarithm of the given fixedpt number. */
fixedpt fixedpt_ln(fixedpt x);


/* Returns the logarithm of the given base of the given fixedpt number */
static inline fixedpt fixedpt_log(fixedpt x, fixedpt base) {
	return (fixedpt_div(fixedpt_ln(x), fixedpt_ln(base)));
}


/* Return the power value (n^exp) of the given fixedpt numbers */
static inline fixedpt fixedpt_pow(fixedpt n, fixedpt exp) {
	if (exp == 0)
		return (FIXEDPT_ONE);
	if (n < 0)
		return 0;
	return (fixedpt_exp(fixedpt_mul(fixedpt_ln(n), exp)));
}

#ifdef __cplusplus
}
#endif

#endif
