/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Luis Bandarra <luis@bandarra.pt>
 * All rights reserved.
 */
#include <stdio.h>

/* 
 * Manipula os bits na representacao binaria de valores inteiros
 * get_bits retorna o valor representado pelos bits situados entre
 * as posicoes lsp (less significate bit) e msp (more significate bit)
 * do value passado, returnado o valor representado.
 */
uint32_t	get_bits(uint32_t, int, int);
/* 
 * set_bits afeta o valor value com o new_value, nos bits situados
 * entre o lsp e o msp, e retorna o valor modificado.
 */
uint32_t	set_bits(uint32_t, int, int, uint32_t);

uint32_t
get_bits(uint32_t value, int msp, int lsp)
{
	/* mascara tudo a 1 */
	int mask = ~(~0 << msp + 1); 
	return (value & mask) >> lsp;
}

uint32_t
set_bits(uint32_t value, int msp, int lsp, uint32_t new_value)
{
	/* mascara com tudo a 1 entre msp - lsp */
	int mask = ~(~0 << (msp - lsp));
	mask <<= lsp;
	/* eliminar valores no local que queremos mudar, atencao negacao mask */
	value &= ~mask;

	/* colocar o novo valor na posicao a mudar e garantir que nao
	 * ultrapassa o gap pretendido */
	new_value <<= lsp;
	new_value &= mask;

	return value | new_value;
}

int
main(void)
{
	/* teste 1, get_bits */
	printf("Teste get_bits\n");
	printf("value=0x2AD555BC\tmsp=6\tlsp=3\n");
	printf("return=0x%x\n", get_bits(0x2AD555BC, 6, 3));

	/* teste2, set_bits */
	printf("Teste set_bits\n");
	printf("value=0x2AD555BC\tmsp=11\tlsp=8\tnew=2\n");
	printf("value=0x%x\n", set_bits(0x2AD555BC, 11, 8, 2));

	return 0;
}

