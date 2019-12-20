
#include "disasm.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

void		famine(void);

static void             hexdump_text(const char *text, uint64_t offset, uint64_t size)
{
	uint64_t	i;
	uint64_t	j;

	printf("--------------------------------------\n");
	i = 0;
	while (i < size)
	{
		// printf("\e[34m%016llx\e[0m\t", offset + i);
		j = 0;
		while (j < 0x10 && i + j < size)
		{
			// if (i_size)
			// {
				printf("\e[36m%02hhx \e[0m", text[i + j++]);
				// i_size--;
			// }
			// else
				// printf("%02hhx ", text[i + j++]);
		}
		printf("\n");
		i += 0x10;
	}
}

static char	*_debug_get_register(uint32_t reg)
{
	#define COLOR_NONE	"\e[0m"
	#define COLOR_RED	"\e[31m"
	#define COLOR_GREEN	"\e[32m"
	#define COLOR_YELLOW	"\e[33m"

	char	s[256] = {0};

	if (reg == UNKNOWN)	{strcat(s, "\e[33m unknown \e[0m"); return strdup(s);}

	if (reg & RAX)		{strcat(s, "\e[32m rax \e[0m");}
	if (reg & RBX)		{strcat(s, "\e[32m rbx \e[0m");}
	if (reg & RCX)		{strcat(s, "\e[32m rcx \e[0m");}
	if (reg & RDX)		{strcat(s, "\e[32m rdx \e[0m");}
	if (reg & RDI)		{strcat(s, "\e[32m rdi \e[0m");}
	if (reg & RSI)		{strcat(s, "\e[32m rsi \e[0m");}
	if (reg & R8)		{strcat(s, "\e[32m r8 \e[0m");}
	if (reg & R9)		{strcat(s, "\e[32m r9 \e[0m");}
	if (reg & R10)		{strcat(s, "\e[32m r10 \e[0m");}
	if (reg & R11)		{strcat(s, "\e[32m r11 \e[0m");}
	if (reg & R12)		{strcat(s, "\e[32m r12 \e[0m");}
	if (reg & R13)		{strcat(s, "\e[32m r13 \e[0m");}
	if (reg & R14)		{strcat(s, "\e[32m r14 \e[0m");}
	if (reg & R15)		{strcat(s, "\e[32m r15 \e[0m");}
	if (reg & RBP)		{strcat(s, "\e[32m rbp \e[0m");}
	if (reg & RSP)		{strcat(s, "\e[32m rsp \e[0m");}
	if (reg & RIP)		{strcat(s, "\e[32m rip \e[0m");}
	if (reg & MEMORY)	{strcat(s, "\e[32m memory \e[0m");}
	if (reg & FLAGS)	{strcat(s, "\e[32m flags \e[0m");}

	if (*s == 0)		{strcat(s, "\e[32m none \e[0m");}

	return strdup(s);
}

int		main(int ac, char **av)
{
	size_t			buflen = 50;
	struct s_instruction	buf[buflen];
	uint64_t		ret;
	void			*code = famine;
	size_t			codelen =  (size_t)disasm_length - (size_t)famine;

	ret = disasm(code, codelen, buf, buflen);
	printf("codelen: %zu\n", codelen);
	printf("Successfully disassembled %llu instructions\n", ret);

	int			fd = 1;
	if (ac > 1)
	{
		fd = open(av[1], O_WRONLY);
		if (fd < 0) return 1;
	}
	for (int i = 0; i < ret; i++)
	{
		// hexdump_text((const char*)buf[i].addr, 0, buf[i].length, buf[i].length);
		char *src = _debug_get_register(buf[i].src);
		char *dst = _debug_get_register(buf[i].dst);
		// hexdump_text((const char*)buf[i].addr, 0, buf[i].length);
		dprintf(fd, "buf[%d]: addr:%p length:%zu src:%s dst:%s\n", i, buf[i].addr, buf[i].length, src, dst);
		free(src);
		free(dst);
	}
	if (fd != 1) close(fd);

	return 0;
}
