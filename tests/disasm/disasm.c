
#include "disasm.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define OP_PACK(reg, ext, stat)		(stat | ((ext & 0b111) << 3) | (reg & 0b111))

void		famine(void);

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

struct x86_set
{
	uint8_t		status;
	uint32_t	src;
	uint32_t	dst;
};

static void	disasm_instruction(const void *code, size_t codelen, uint32_t *src, uint32_t *dst)
{
	uint8_t		*p     = (uint8_t*)code;
	uint8_t		prefix = 0;
	uint8_t		rex    = 0;
	uint8_t		flags  = 0;
	uint8_t		opcode;

	*src = UNKNOWN;
	*dst = UNKNOWN;

	/* Prefix loop */
	next_opcode:
	if (!codelen--) return ; /* Error if instruction is too long */
	opcode = *p++;

	/* Unused legacy / REX prefixes for this register disassembler */
	if (opcode == 0x26 || opcode == 0x2e
	||  opcode == 0x36 || opcode == 0x3e
	||  opcode == 0x64 || opcode == 0x65 || opcode == 0x66 || opcode == 0x67
	||  opcode == 0xf0)
		goto next_opcode;
	/* Mandatory prefixes */
	else if (opcode == 0x0f) {prefix |= OP_PREFIX_0F; goto next_opcode;}
	else if (opcode == 0xf2) {prefix |= OP_PREFIX_F2; goto next_opcode;}
	else if (opcode == 0xf3) {prefix |= OP_PREFIX_F3; goto next_opcode;}
	else if (opcode == 0x9b) {prefix |= OP_PREFIX_9B; goto next_opcode;}
	/* REX prefixes */
	else if (opcode >= 0x40 && opcode <= 0x4f) {rex = opcode; goto next_opcode;}

	uint32_t	table_supported_opcode[TABLESIZE];
	table_supported_opcode[0] = BITMASK32(0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[1] = BITMASK32(0,0,0,0,0,0,0,0, 0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[2] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,1,1, 1,0,0,0,0,0,0,1);
	table_supported_opcode[3] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[4] = BITMASK32(0,0,0,1,0,0,0,1, 0,1,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[5] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[6] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[7] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);

	if (!CHECK_TABLE(table_supported_opcode, opcode)) return ;

	uint32_t	gp_registers[16];
	gp_registers[0b0000] = RAX;
	gp_registers[0b0001] = RCX;
	gp_registers[0b0010] = RDX;
	gp_registers[0b0011] = RBX;
	gp_registers[0b0100] = RSP;
	gp_registers[0b0101] = RBP;
	gp_registers[0b0110] = RSI;
	gp_registers[0b0111] = RDI;
	gp_registers[0b1000] = R8;
	gp_registers[0b1001] = R9;
	gp_registers[0b1010] = R10;
	gp_registers[0b1011] = R11;
	gp_registers[0b1100] = R12;
	gp_registers[0b1101] = R13;
	gp_registers[0b1110] = R14;
	gp_registers[0b1111] = R15;

	struct x86_set		instructions[255];
	instructions[0x01] = (struct x86_set){MODRM|KEEP_DST,          OP_PACK(0,0,0),              OP_PACK(0,0,FLAGS)};
	instructions[0x29] = (struct x86_set){MODRM|KEEP_DST,          OP_PACK(0,0,0),              OP_PACK(0,0,FLAGS)};
	instructions[0x50] = (struct x86_set){IMPLICIT|KEEP_DST,       OP_PACK(0b000,0b000,0),      OP_PACK(0b100,0b000,MEMORY)};
	instructions[0x51] = (struct x86_set){IMPLICIT|KEEP_DST,       OP_PACK(0b001,0b000,0),      OP_PACK(0b100,0b000,MEMORY)};
	instructions[0x52] = (struct x86_set){IMPLICIT|KEEP_DST,       OP_PACK(0b010,0b000,0),      OP_PACK(0b100,0b000,MEMORY)};
	instructions[0x53] = (struct x86_set){IMPLICIT|KEEP_DST,       OP_PACK(0b011,0b000,0),      OP_PACK(0b100,0b000,MEMORY)};
	instructions[0x56] = (struct x86_set){IMPLICIT|KEEP_DST,       OP_PACK(0b110,0b000,0),      OP_PACK(0b100,0b000,MEMORY)};
	instructions[0x57] = (struct x86_set){IMPLICIT|KEEP_DST,       OP_PACK(0b111,0b000,0),      OP_PACK(0b100,0b000,MEMORY)};
	instructions[0x58] = (struct x86_set){IMPLICIT|KEEP_SRC,       OP_PACK(0b100,0b000,MEMORY), OP_PACK(0b000,0b000,0)};
	instructions[0x5f] = (struct x86_set){IMPLICIT|KEEP_SRC,       OP_PACK(0b100,0b000,MEMORY), OP_PACK(0b111,0b000,0)};
	instructions[0x83] = (struct x86_set){EXT|KEEP_DST,            OP_PACK(0b000,0b101,0),      OP_PACK(0b000,0b000,FLAGS)};
	instructions[0x87] = (struct x86_set){MODRM|KEEP_SRC|KEEP_DST, OP_PACK(0,0,0),              OP_PACK(0,0,0)};
	instructions[0x89] = (struct x86_set){MODRM,                   OP_PACK(0,0,0),              OP_PACK(0,0,0)};
	instructions[0x8b] = (struct x86_set){MODRM,                   OP_PACK(0,0,0),              OP_PACK(0,0,0)};
	instructions[0x90] = (struct x86_set){NONE,                    OP_PACK(0,0,0),              OP_PACK(0,0,0)};

	struct x86_set		i = instructions[opcode];

	*src = 0;
	*dst = 0;

	if (i.status == NONE)
	{
		*src = NONE;
		*dst = NONE;
		return ;
	}
	else if (i.status & EXT) /* Assume that EXT status takes an immediate as a source, even if that is not always the case .. */
	{
		if (!codelen--) return ; /* Error if instruction is too long */
		uint8_t		modrm = *p++;
		uint8_t		rm    =  modrm & 0b00000111;
		uint8_t		rex_rxb  = rex & 0b00000111;
		uint8_t		rm_mode  = !!(rex_rxb & 0b001) << 3;

		*src = NONE;
		*dst |= gp_registers[REG_PACK(rm, rm_mode)];
	}
	else if (i.status & IMPLICIT)
	{
		uint8_t		rex_rxb  = rex & 0b00000111;
		uint8_t		reg_mode = !!(rex_rxb & 0b100) << 3;
		uint8_t		rm_mode  = !!(rex_rxb & 0b001) << 3;

		/* Dirty hack to not mix extended registers with not extended one for 1 byte opcode-based instructions */
		*src |= i.src & MEMORY ? gp_registers[REG_PACK(i.src & 0b111, reg_mode)] : gp_registers[REG_PACK(i.src & 0b111, rm_mode)];
		*dst |= i.dst & MEMORY ? gp_registers[REG_PACK(i.dst & 0b111, reg_mode)] : gp_registers[REG_PACK(i.dst & 0b111, rm_mode)];
	}
	/* If posseses MOD/RM byte with register usage */
	else if (i.status & MODRM)
	{
		*src = 0;
		*dst = 0;
		if (!codelen--) return ; /* Error if instruction is too long */
		uint8_t		modrm = *p++;
		uint8_t		mod   = (modrm & 0b11000000) >> 6;
		uint8_t		reg   = (modrm & 0b00111000) >> 3;
		uint8_t		rm    =  modrm & 0b00000111;

		if (!codelen--) return ; /* Error if instruction is too long */
		uint8_t		sib   = *p++;
		uint8_t		index = (sib & 0b00111000) >> 3;
		uint8_t		base  =  sib & 0b00000111;

		uint8_t		rex_rxb  = rex & 0b00000111;
		uint8_t		reg_mode = !!(rex_rxb & 0b100) << 3;
		uint8_t		rm_mode  = !!(rex_rxb & 0b001) << 3;

		uint8_t		direction = opcode & 0b10; /* d=1 memory to register (reg dest), d=0 register to memory (reg source) */

		if (mod == 0b11) /* Direct register addressing */
		{
			// printf("<disasm_registers> \e[32mregister addressing\e[0m\n");
			*src |= direction ? gp_registers[REG_PACK(rm, rm_mode)] : gp_registers[REG_PACK(reg, reg_mode)];
			*dst |= direction ? gp_registers[REG_PACK(reg, reg_mode)] : gp_registers[REG_PACK(rm, rm_mode)];
		}
		else if (mod == 0b00) /* Indirect register addressing */
		{
			// printf("<disasm_registers> \e[32mindirect register addressing\e[0m\n");
			if (rm == 0b100) /* SIB */
			{
				// printf("<disasm_registers> \t\e[33mSIB\e[0m\n");
				/* reg is the destination */
				*src |= gp_registers[REG_PACK(index, reg_mode)];
				*src |= gp_registers[REG_PACK(base, reg_mode)];
				*dst |= gp_registers[REG_PACK(reg, reg_mode)];
			}
			else if (rm == 0b101) /* Displacement only addressing [rip + disp] */
			{
				/* reg is the destination */
				// printf("<disasm_registers> \t\e[33mdisplacement only\e[0m\n");
				*src |= RIP;
				*dst |= gp_registers[REG_PACK(reg, reg_mode)];
			}
			else /* Indirect register addressing */
			{
				// printf("<disasm_registers> \t\e[33mindirect register addressing\e[0m\n");
				/* r/m is where dereferencement occurs */
				*src |= direction ? gp_registers[REG_PACK(rm, rm_mode)] : gp_registers[REG_PACK(reg, reg_mode)];
				*dst |= direction ? gp_registers[REG_PACK(reg, reg_mode)] : gp_registers[REG_PACK(rm, rm_mode)];
				*src |= direction ? MEMORY : 0;
				*dst |= direction ? 0 : MEMORY;
			}
		}
		else if (mod == 0b01 || mod == 0b10) /* Indirect register addressing with 1 or 4 byte signed displacement */
		{
			// printf("<disasm_registers> \e[32msigned displacement\e[0m\n");
			if (rm == 0b100) /* SIB with displacement */
			{
				// printf("<disasm_registers> \t\e[33mSIB with displacement\e[0m\n");
				/* reg is the destination */
				*src |= gp_registers[REG_PACK(index, reg_mode)];
				*src |= gp_registers[REG_PACK(base, reg_mode)];
				*dst |= gp_registers[REG_PACK(reg, reg_mode)];
			}
			else /* Indirect register with displacement */
			{
				// printf("<disasm_registers> \t\e[33mindirect register with displacement\e[0m\n");
				/* r/m is where dereferencement occurs */
				*src |= direction ? gp_registers[REG_PACK(rm, rm_mode)] : gp_registers[REG_PACK(reg, reg_mode)];
				*dst |= direction ? gp_registers[REG_PACK(reg, reg_mode)] : gp_registers[REG_PACK(rm, rm_mode)];
				*src |= direction ? MEMORY : 0;
				*dst |= direction ? 0 : MEMORY;
			}
		}
	}
	if (i.status & KEEP_DST) *src |= *dst & 0xffff;
	if (i.status & KEEP_SRC) *dst |= *src & 0xffff;
	*src |= i.src & 0xff000000;
	*dst |= i.dst & 0xff000000;
}

uint64_t	disasm(const void *code, size_t codelen, struct s_instruction *buf, size_t buflen)
{
	void			*p_code = (void*)code;
	struct s_instruction	*p_buf  = (void*)buf;
	size_t			instruction_length = 0;

	while (codelen && buflen)
	{
		instruction_length = disasm_length(p_code, codelen);
		if (instruction_length == EDISASM_LENGTH) break ;

		disasm_instruction(p_code, codelen, &p_buf->src, &p_buf->dst);
		p_buf->addr   = p_code;
		p_buf->length = instruction_length;

		p_code  += instruction_length;
		codelen -= instruction_length;
		p_buf   += 1;
		buflen  -= 1;
	}
	return p_buf - buf; /* Number of instructions successfully disassembled */
}

int		main(int ac, char **av)
{
	size_t			buflen = 50;
	struct s_instruction	buf[buflen];
	uint64_t		ret;
	void			*code = famine;
	size_t			codelen =  (size_t)disasm - (size_t)famine;

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
		dprintf(fd, "buf[%d]: addr:%p length:%zu src:%s dst:%s\n", i, buf[i].addr, buf[i].length, src, dst);
		free(src);
		free(dst);
	}
	if (fd != 1) close(fd);

	return 0;
}
