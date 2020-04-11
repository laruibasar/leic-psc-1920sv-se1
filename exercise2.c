/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Luis Bandarra <luis@bandarra.pt>
 * All rights reserved.
 */
#include <stdio.h>

#define BUFFER_SIZE 100

/* 
 * Le linha de texto do ficheiro passado como referencia.
 * O texto e depositado num buffer com maximo de buffer_size
 */
int	read_line(FILE*, char*, int);

int
main(void)
{
	FILE *fp = fopen("teste.s", "r");
	
	char buffer[BUFFER_SIZE];
	int s;
	while ((s = read_line(fp, &buffer, BUFFER_SIZE)) != -1) {
		printf("Line (%d): %s\n", s, buffer);
	}

	fclose(fp);

	return 0;
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
