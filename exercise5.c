/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Luis Bandarra <luis@bandarra.pt>
 * All rights reserved.
 */
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 100
#define INSTRUCTION_SIZE 20
#define OFFSET_MAX 0x3FF

typedef struct {
	uint8_t byte_count;
	uint16_t address;
	uint8_t type;
	uint8_t data[256];
	uint8_t checksum;
} IntelHex;

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

/* 
 * Le linha de texto do ficheiro passado como referencia.
 * O texto e depositado num buffer com maximo de buffer_size
 */
int	read_line(FILE*, char*, int);

/*
 * Extrai informacao contida na linha de texto passada, em Intel HEX,
 * e preenche o struct.
 * Devolve 1 no caso de o texto ser valido, 0 caso contrario
 */
int	decode_record(char *, IntelHex *);

/* char to 2bit hex */
uint16_t	char_to_hex(char, char);

/* Auxiliar, converte 1 hex para int, -1 em caso erro */
int	hex_to_int(char, int);

/* Obtem a struct IntelHex e apresenta o conteudo 
 * atraves da verificacao das instrucoes P16
 */
int	dissassemble_record(IntelHex *);

/* Lista as instrucoes e retorna o texto da instrucao
 * passando 8bit + 8bit
 */
char *	decode_instruction(char *, uint16_t, uint8_t, uint8_t);

/* Ajuda para uso do programa */
void
helper(void)
{
	printf("Usage: pdas file\n");
}

int
main(int argc, char* argv[])
{
	if (argc < 2) {
		helper();
		return (-1);
	}

	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("Erro abrir ficheiro: %s\n", argv[1]);
		return (-1);
	}
	
	IntelHex block;
	char buffer[BUFFER_SIZE];
	int s, line = 1;
	while ((s = read_line(fp, &buffer, BUFFER_SIZE)) != -1) {
		int record = decode_record(buffer, &block);
		if (record == 0) {
			printf("Erro na codificacao IntelHex na linha %d\n%s\n",
					line,
					buffer);
			return (-1);
		}
		int dissassemble = dissassemble_record(&block); 
		if (dissassemble < 0) {
			printf("Erro na decodificacao da instrucao:\n");
			printf("%02x %04x %02x ",
					block.byte_count,
					block.address,
					block.type);
			for (int i = 0; i < block.byte_count; i++)
				printf("%02x", block.data[i]);
			printf(" %02x\n", block.checksum);
			return (-1);
		}

		if (dissassemble > 0)
			return (0);

		line++;
	}

	return (0);
}

int
dissassemble_record(IntelHex *pblock)
{
	/* end of file */
	if (pblock->type == 0x01)
		return (1);
	
	char instruction[INSTRUCTION_SIZE];
	int counter = pblock->address;
	int idx = 0;
	while (idx < pblock->byte_count) {
		decode_instruction(
				&instruction,
				counter + 2, /* pc adiantado */
				pblock->data[idx],
				pblock->data[idx + 1]);

		printf("%04x %02x%02x\t%s\n",
				counter,
				pblock->data[idx],
				pblock->data[idx + 1],
				instruction); 
		idx += 2;
		counter +=2;
	}
	return (0);
}

char *
decode_instruction(char* instruction, uint16_t count, uint8_t lsb, uint8_t msb)
{
	char opcode[5], arg1[10], arg2[10], arg3[10];
	uint32_t code = 0;
	
	code = set_bits(code, 16, 8, msb);
	code = set_bits(code, 8, 0, lsb);
	switch (get_bits(code, 15, 13)) {
		case 0:
			sprintf(arg1, "r%d", get_bits(code, 3, 0));
			sprintf(arg2, ", [r%d", get_bits(code, 6, 4));
			switch (get_bits(code, 12, 10)) {
				case 0:
					sprintf(opcode, "ldr ");
					sprintf(arg3, ", %d]", get_bits(code, 9, 7));
					break;
				case 1:
					sprintf(opcode, "pop ");
					sprintf(arg2, "");
					sprintf(arg3, "");
				break;
				case 2:
					sprintf(opcode, "ldrb ");
					sprintf(arg3, ", %d]", get_bits(code, 9, 7));
					break;
				case 3:
					sprintf(opcode, "ldr ");
					sprintf(arg2, ", [pc");
					sprintf(arg3, ", %d]", get_bits(code, 9, 4));
					break;
				case 4:
				case 5:
					sprintf(opcode, "ldr ");
					sprintf(arg3, ", r%d]", get_bits(code, 10, 7));
					break;
				case 6:
				case 7:
					sprintf(opcode, "ldrb ");
					sprintf(arg3, ", r%d]", get_bits(code, 10, 7));
					break;
			}
			break;
		case 1:
			sprintf(arg1, "r%d", get_bits(code, 3, 0));
			sprintf(arg2, ", [r%d", get_bits(code, 6, 4));
			sprintf(arg3, ", %d]", get_bits(code, 9, 7));
	
			switch (get_bits(code, 12, 10)) {
				case 0:
					sprintf(opcode, "str");
					break;
				case 1:
					sprintf(opcode, "push");
					sprintf(arg1, "r%d", get_bits(code, 3, 0));
					sprintf(arg2, "");
					sprintf(arg3, "");
					break;
				case 2:
					sprintf(opcode, "strb");
					break;
				case 4:
				case 5:
					sprintf(opcode, "str");
					sprintf(arg3, ", r%d]", get_bits(code, 9, 7));
					break;
				case 6:
				case 7:
					sprintf(opcode, "strb");
					sprintf(arg3, ", r%d]", get_bits(code, 9, 7));
					break;
			}
			break;
		case 2:
			sprintf(arg1, "0x%04x", OFFSET_MAX & (count + 2 * get_bits(code, 9, 0)));
			sprintf(arg2, "");
			sprintf(arg3, "");
	
			switch (get_bits(code, 12, 10)) {
				case 0:
					sprintf(opcode, "bzs");
					break;
				case 1:
					sprintf(opcode, "bzc");
					break;
				case 2:
					sprintf(opcode, "bcs");
					break;
				case 3:
					sprintf(opcode, "bcc");
					break;
				case 4:
					sprintf(opcode, "bge");
					break;
				case 5:
					sprintf(opcode, "blt");
					break;
				case 6:
					sprintf(opcode, "b");
					break;
				case 7:
					sprintf(opcode, "bl");
					break;
			}
			break;
		case 3:
			sprintf(arg1, "r%d", get_bits(code, 3, 0));
			sprintf(arg2, ", %d", get_bits(code, 11, 4));
			sprintf(arg3, "");
	
			switch (get_bits(code, 12, 12)) {
				case 0:
					sprintf(opcode, "mov");
					break;
				case 1:
					sprintf(opcode, "movt");
					break;
			}
			break;
		case 4:
			sprintf(arg1, "r%d", get_bits(code, 3, 0));
			sprintf(arg2, ", r%d", get_bits(code, 6, 4));
			sprintf(arg3, ", r%d", get_bits(code, 10, 7));
	
			switch (get_bits(code, 12, 11)) {
				case 0:
					sprintf(opcode, "add");
				break;
				case 1:
					sprintf(opcode, "sub");
					break;
				case 2:
					sprintf(opcode, "adc");
					break;
				case 3:
					sprintf(opcode, "sbc");
					break;
			}
			break;
		case 5:
			sprintf(arg1, "r%d", get_bits(code, 3, 0));
			sprintf(arg2, ", r%d", get_bits(code, 6, 4));
			sprintf(arg3, ", %d", get_bits(code, 10, 7));
			switch (get_bits(code, 12, 11)) {
				case 0:
					sprintf(opcode, "add");
				break;
				case 1:
					sprintf(opcode, "sub");
					break;
				case 2:
					switch (get_bits(code, 6, 4)) {
						case 0:
							sprintf(opcode, "mov");
							if (get_bits(code, 3, 0) != 15) 
								sprintf(arg1, "r%d", get_bits(code, 3, 0));
							else
								sprintf(arg1, "pc");
							
							if (get_bits(code, 10, 7) != 14)
								sprintf(arg2, ", r%d", get_bits(code, 10, 7));
							else 
								sprintf(arg2, ", lr");
							sprintf(arg3, "");
							break;
						case 1:
							sprintf(opcode, "mvn");
							sprintf(arg2, ", r%d", get_bits(code, 10, 7));
							sprintf(arg3, "");
							break;
						case 2:
							sprintf(opcode, "movs");
							sprintf(arg1, "pc, ");
							sprintf(arg2, ", lr");
							sprintf(arg3, "");
							break;
						case 4:
							sprintf(opcode, "msr");
							sprintf(arg1, "cspr");
							sprintf(arg2, ", r%d", get_bits(code, 10, 7));
							sprintf(arg3, "");
							break;
						case 5:
							sprintf(opcode, "msr");
							sprintf(arg1, "spsr");
							sprintf(arg2, ", r%d", get_bits(code, 10, 7));
							sprintf(arg3, "");
							break;
						case 6:
							sprintf(opcode, "mrs");
							sprintf(arg2, ", cpsr");
							sprintf(arg3, "");
							break;
						case 7:
							sprintf(opcode, "mrs");
							sprintf(arg2, ", spsr");
							sprintf(arg3, "");
							break;
						}
					break;
				case 3:
					sprintf(opcode, "cmp");
					sprintf(arg1, "r%d", get_bits(code, 6, 4));
					sprintf(arg2, ", r%d", get_bits(code, 10, 7));
					sprintf(arg3, "");
					break;
			}
			break;
		case 6:
			sprintf(arg1, "r%d", get_bits(code, 3, 0));
			sprintf(arg2, ", r%d", get_bits(code, 6, 4));
			sprintf(arg3, ", r%d", get_bits(code, 10, 7));
	
			switch (get_bits(code, 12, 11)) {
				case 0:
					sprintf(opcode, "and");
					break;
				case 1:
					sprintf(opcode, "orr");
					break;
				case 2:
					sprintf(opcode, "eor");
					break;
				case 3:
					sprintf(opcode, "rrx");
					sprintf(arg3, "");
					break;
			}
			break;
		case 7:
			switch (get_bits(code, 12, 11)) {
				case 0:
					sprintf(opcode, "lsl");
					break;
				case 1:
					sprintf(opcode, "lsr");
					break;
				case 2:
					sprintf(opcode, "asr");
					break;
				case 3:
					sprintf(opcode, "ror");
					break;
			}
			sprintf(arg1, "r%d", get_bits(code, 3, 0));
			sprintf(arg2, ", r%d", get_bits(code, 6, 4));
			sprintf(arg3, ", %d", get_bits(code, 10, 7));
			break;
	}
	
	sprintf(instruction,"%s\t%s%s%s", opcode, arg1, arg2, arg3);

	return instruction;
}

uint32_t
get_bits(uint32_t value, int msp, int lsp)
{
	/* mascara tudo a 1 */
	int mask = ~(~0 << (msp + 1)); 
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
read_line(FILE *fp, char *buffer, int buffer_size)
{
	int size = buffer_size;
	char c;
	while (size-- > 0) {
		c = getc(fp);
		if (c == '\n')
			break;

		if (c == EOF)
			return (-1);

		*buffer++ = c;
	}

	*buffer = '\0';
	return (buffer_size - size - 1);
}

int
decode_record(char *record, IntelHex *pblock)
{
	int sum = 0;
	uint16_t value;

	if (record[0] != ':')
		return (0);

	/* byte_count - 2digits */
	int i = 1;
	value = char_to_hex(record[i], record[i + 1]);
	if (value > 0xff)
		return (0);
	i += 2;
	sum += value;
	pblock->byte_count = value;

	/* address - 4digits */
	int n = 4;
	value = 0;
	while (n > 0 && record[i] != 0) {
		value += hex_to_int(record[i++], n--);
	}
	sum += char_to_hex(record[3], record[4]);
	sum += char_to_hex(record[5], record[6]);
	pblock->address = value;

	/* type - 2digits */
	value = char_to_hex(record[i], record[i + 1]);
	if (value > 0x05)
		return (0);
	i += 2;
	sum += value;
	pblock->type = value;

	/* data = 2n */
	int idx = 0;
	int repeat = (strlen(record) - 1 - i) / 2;
	while (repeat-- > 0) {
		value = char_to_hex(record[i], record[i + 1]);
		sum += value;
		pblock->data[idx++] = value;
		i += 2;
	}

	/* checksum */
	value = char_to_hex(record[i], record[i + 1]);
	if (value > 0xff)
		return (0);
	pblock->checksum = value;

	if (((sum + value) & 0xff) == 0)
		return (1);

	return (i - strlen(record) == 0 ? 1 : 0);
}

uint16_t
char_to_hex(char msb, char lsb)
{
	uint16_t value = 0;
	int n = 2;
	char c = msb;

	while (n > 0) {
		value += hex_to_int(c, n--);
		c = lsb;
	}
	
	return (value);
}

int
hex_to_int(char c, int d)
{
	uint16_t value = c - '0';
	uint16_t multiplier = 1;

	while (--d > 0)
		multiplier *= 16;

	if (value <= 9)
		return (value * multiplier);

	if ((value = c - 'a') < 6 || (value = c - 'A') < 6)
		return ((value + 10) * multiplier);

	return(-1);
}

