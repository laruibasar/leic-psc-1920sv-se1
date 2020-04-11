/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Luis Bandarra <luis@bandarra.pt>
 * All rights reserved.
 */
#include <stdio.h>
#include <string.h>

typedef struct {
	uint8_t byte_count;
	uint16_t address;
	uint8_t type;
	uint8_t data[256];
	uint8_t checksum;
} IntelHex;

/*
 * Extrai informacao contida na linha de texto passada, em Intel HEX,
 * e preenche o struct.
 * Devolve 1 no caso de o texto ser valido, 0 caso contrario
 */
int		decode_record(char *, IntelHex *);

/* char to 2bit hex */
uint16_t	char_to_hex(char, char);

/* Auxiliar, converte 1 hex para int, -1 em caso erro */
int		hex_to_int(char, int);

int
main(void)
{
	IntelHex block;
	int ch = decode_record(":10041a000e24c00c0008b10c11080c5c910c1020c1", &block);
	printf("%d :%02x %04x %02x ", ch, block.byte_count, block.address, block.type);
	for (int i = 0; i < block.byte_count; i++)
		printf("%02x", block.data[i]);
	printf(" %02x\n", block.checksum);

	return 0;
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

