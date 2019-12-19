
#include "disasm.h"

static void	disasm_instruction(const void *code, size_t codelen, uint32_t *src, uint32_t *dst)
{
	uint8_t		*p     = (uint8_t*)code;
	uint8_t		prefix = 0;
	uint8_t		rex    = 0;
	uint8_t		opcode;

	*src = UNKNOWN;
	*dst = UNKNOWN;

	codelen = codelen > INSTRUCTION_MAXLEN ? INSTRUCTION_MAXLEN : codelen;

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

	struct x86_set
	{
		uint8_t		status;
		uint32_t	src;
		uint32_t	dst;
	} instructions[255];

	// struct x86_set		instructions[255];
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
			*src |= direction ? gp_registers[REG_PACK(rm, rm_mode)] : gp_registers[REG_PACK(reg, reg_mode)];
			*dst |= direction ? gp_registers[REG_PACK(reg, reg_mode)] : gp_registers[REG_PACK(rm, rm_mode)];
		}
		else if (mod == 0b00) /* Indirect register addressing */
		{
			if (rm == 0b100) /* SIB */
			{
				/* reg is the destination */
				*src |= gp_registers[REG_PACK(index, reg_mode)];
				*src |= gp_registers[REG_PACK(base, reg_mode)];
				*dst |= gp_registers[REG_PACK(reg, reg_mode)];
			}
			else if (rm == 0b101) /* Displacement only addressing [rip + disp] */
			{
				/* reg is the destination */
				*src |= RIP;
				*dst |= gp_registers[REG_PACK(reg, reg_mode)];
			}
			else /* Indirect register addressing */
			{
				/* r/m is where dereferencement occurs */
				*src |= direction ? gp_registers[REG_PACK(rm, rm_mode)] : gp_registers[REG_PACK(reg, reg_mode)];
				*dst |= direction ? gp_registers[REG_PACK(reg, reg_mode)] : gp_registers[REG_PACK(rm, rm_mode)];
				*src |= direction ? MEMORY : 0;
				*dst |= direction ? 0 : MEMORY;
			}
		}
		else if (mod == 0b01 || mod == 0b10) /* Indirect register addressing with 1 or 4 byte signed displacement */
		{
			if (rm == 0b100) /* SIB with displacement */
			{
				/* reg is the destination */
				*src |= gp_registers[REG_PACK(index, reg_mode)];
				*src |= gp_registers[REG_PACK(base, reg_mode)];
				*dst |= gp_registers[REG_PACK(reg, reg_mode)];
			}
			else /* Indirect register with displacement */
			{
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
