/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Luis Bandarra <luis.bandarra@homestudio.pt>
 * All rights reserved.
 */

/* study help on IEEE 754:
 * https://en.wikipedia.org/wiki/Single-precision_floating-point_format
 * https://www.h-schmidt.net/FloatConverter/IEEE754.html
 */

#include <stdio.h>


#define NUMBER_DEC 4

typedef struct pscfloat {
	unsigned int sign: 1;		/* 0 => positive, 1 => negative */
	unsigned int exponent: 8;	/* 2^exponent  */
	unsigned int mantissa: 23;
} PSCFloat;

typedef union {
	float nf;
	PSCFloat pscfloat;
	unsigned int ni;
} NumberFloat;

void
string_to_float(char *string, PSCFloat *pscf)
{
	long number = 0;
	int fdec = 0;
	int dec = 0;

	pscf->sign = 0;
	pscf->exponent = 0;
	pscf->mantissa = 0;

	// convert string to number (int)
	int i = 0;
	char c;
	while ((c = string[i++]) != 0) {
		/* set signal */
		if (c == '-' && i == 1) {
			pscf->sign = 1;
			continue;
		}

		if (c == ',' || c == '.') {
			fdec = 1;
			continue;
		}

		number = (number * 10) + (c - '0');

		if (fdec) {
			dec++;
		}
	}
	//printf("int: %ld\n", number);

	// shift int to binary (n << y) / 10^x
	number = number << NUMBER_DEC;
	for (i = 0; i < dec; i++) {
		number /= 10;
	}
	//printf("int shift: %ld\n", number);

	/* calculate exponent */
	int exp = 0;
	int num_exp = number >> NUMBER_DEC;
	while (num_exp > 1) {
		exp++;
		num_exp >>= 1;
	}
	pscf->exponent = (exp + 127); /* 2^exp from [b30, b23] - b22*/

	/* calculate mantissa */
	int num_man = number << (23 - (NUMBER_DEC + exp));
	pscf->mantissa = num_man;
	//printf("%d %d %d\n", pscf->sign, pscf->exponent, pscf->mantissa);
}

float
conversor(PSCFloat *pscf)
{
	NumberFloat num;
	num.nf = 0;
	num.ni = 0;
	num.pscfloat = *pscf;

	unsigned int number = 0;
	number = (pscf->sign << 31);
	number = number | (pscf->exponent << 23);
	number = number | (pscf->mantissa);

	num.ni = number;

	return num.nf;
}

int
main(int argc, char *argv[])
{
    	PSCFloat pscf1;
	char string1[] = "33.625";
	string_to_float(string1, &pscf1);
	float f1 = conversor(&pscf1);
	int r1 = (f1 == 33.625f);
	printf("%s: = %f\tFloat work? %d\n", string1, f1, r1);

	PSCFloat pscf2;
	char string2[] = "-32.125";
	string_to_float(string2, &pscf2);
	float f2 = conversor(&pscf2);
	int r2 = (f2 == -32.125f);
	printf("%s: = %f\tFloat work? %d\n", string2, f2, r2);

	PSCFloat pscf3;
	char string3[] = "323232.635";
	string_to_float(string3, &pscf3);
	float f3 = conversor(&pscf3);
	int r3 = (f3 == 323232.635f);
	printf("%s: = %f\tFloat work? %d\n", string3, f3, r3);

	PSCFloat pscf4;
	char string4[] = "0.1";
	string_to_float(string4, &pscf4);
	float f4 = conversor(&pscf4);
	int r4 = (f4 == 0.1f);
	printf("%s: = %f\tFloat work? %d\n", string4, f4, r4);

	PSCFloat pscf5;
	char string5[] = "1.0";
	string_to_float(string5, &pscf5);
	float f5 = conversor(&pscf5);
	int r5 = (f5 == 1.0f);
	printf("%s: = %f\tFloat work? %d\n", string5, f5, r5);

	return 0;
}


