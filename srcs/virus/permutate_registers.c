
#include <stddef.h>

#include "compiler_utils.h"
#include "disasm.h"
#include "disasm_utils.h"
#include "utils.h"
#include "errors.h"

/*
** Flags used to specify operand or status.
** Although GP registers can have a 8-bit, 16-bit or 32-bit size we consider
** for this disassembler that the whole 64-bit register is affected.
*/
# define NONE		(0)       /* nothing                            */
# define RAX		(1 << 0)  /* al   || ax   || eax  || rax        */
# define RCX		(1 << 1)  /* cl   || cx   || ecx  || rcx        */
# define RDX		(1 << 2)  /* dl   || dx   || edx  || rdx        */
# define RBX		(1 << 3)  /* bl   || bx   || ebx  || rbx        */
# define RSP		(1 << 4)  /* ah   || spl  || sp   || esp || rsp */
# define RBP		(1 << 5)  /* ch   || bpl  || bp   || ebp || rbp */
# define RSI		(1 << 6)  /* dh   || sil  || si   || esi || rsi */
# define RDI		(1 << 7)  /* bh   || dil  || di   || edi || rdi */
# define R8		(1 << 8)  /* r8l  || r8w  || r8d  || r8         */
# define R9		(1 << 9)  /* r9l  || r9w  || r9d  || r9         */
# define R10		(1 << 10) /* r10l || r10w || r10d || r10        */
# define R11		(1 << 11) /* r11l || r11w || r11d || r11        */
# define R12		(1 << 12) /* r12l || r12w || r12d || r12        */
# define R13		(1 << 13) /* r13l || r13w || r13d || r13        */
# define R14		(1 << 14) /* r14l || r14w || r14d || r14        */
# define R15		(1 << 15) /* r15l || r15w || r15d || r15        */

# define RIP		(1 << 16) /* eip  || rip                        */

/*
** Flags used to specify how the intruction should be interpreted
*/
# define MODRM		(1 << 0) /* MODRM byte with register usage       */
# define EXT		(1 << 1) /* MODRM byte with opcode extension     */
# define IMPLICIT_SRC	(1 << 2) /* source register is implicit          */
# define IMPLICIT_DST	(1 << 3) /* destination register is implicit     */

struct reg_match
{
	uint32_t	reg;
	bool		is_init;
};

static void	swap_match(struct reg_match *a, struct reg_match *b)
{
	uint32_t	tmp;

	tmp = a->reg;
	a->reg = b->reg;
	b->reg = tmp;
}

static void	shuffle_registers(struct reg_match *match, uint64_t seed,
			size_t reg_size)
{
	/* registers allowed for shuffling */
	uint32_t	regs_allowed = RAX|RBX|R11|R14|R15;

	struct reg_match	*reg          = match;
	struct reg_match	*reg_ext      = match + 8;
	uint32_t		reg_avail     = regs_allowed & 0xff;
	uint32_t		reg_avail_ext = regs_allowed & 0xff00;
	/* rax to rdi shuffle */
	for (int i = reg_size - 1; i > 0; i--)
	{
		uint64_t	random_seed = random(&seed);
		uint64_t	j = random_inrange(&random_seed, 0, reg_size - 1);

		j = j % (i + 1);

		if ((reg[i].reg & reg_avail) && (reg[j].reg & reg_avail))
			swap_match(&reg[i], &reg[j]);
	}
	/* r8 to r15 shuffle */
	for (int i = reg_size - 1; i > 0; i--)
	{
		uint64_t	random_seed = random(&seed);
		uint64_t	j = random_inrange(&random_seed, 0, reg_size - 1);

		j = j % (i + 1);

		if ((reg_ext[i].reg & reg_avail_ext) && (reg_ext[j].reg & reg_avail_ext))
			swap_match(&reg_ext[i], &reg_ext[j]);
	}
}

static uint32_t	mask_to_index(uint32_t m)
{
	uint32_t	i = 0;
	while (m) {m >>= 1; if (m) i++;}
	return i;
}

static bool	is_init(struct reg_match *match, uint32_t op)
{
	if (!match[op & 0b1111].is_init)
	{
		match[op & 0b1111].is_init = true;
		return false;
	}
	return true;
}

static bool	apply_match(void *code, size_t codelen, struct reg_match *match)
{
	uint8_t		*p     = (uint8_t*)code;
	uint8_t		*rex   = NULL; /* REX position */
	uint8_t		prefix = 0;    /* opcode prefix */
	uint8_t		*opcode;

	/* set <codelen> to INSTRUCTION_MAXLEN if it exceeds it */
	if (codelen > INSTRUCTION_MAXLEN) codelen = INSTRUCTION_MAXLEN;

	/* prefix loop */
	next_opcode:
	if (!codelen--) return false; /* error if instruction is too long */
	opcode = p++;

	/* unused legacy / REX prefixes for register disassembler */
	if (*opcode == 0x26 || *opcode == 0x2e
	||  *opcode == 0x36 || *opcode == 0x3e
	||  *opcode == 0x64 || *opcode == 0x65 || *opcode == 0x66 || *opcode == 0x67
	||  *opcode == 0xf0) {goto next_opcode;}
	/* mandatory prefix(es) */
	else if (*opcode == 0x0f) {prefix |= OP_PREFIX_0F; goto next_opcode;}
	else if (*opcode == 0xf2) {prefix |= OP_PREFIX_F2; goto next_opcode;}
	else if (*opcode == 0xf3) {prefix |= OP_PREFIX_F3; goto next_opcode;}
	else if (*opcode == 0x9b) {prefix |= OP_PREFIX_9B; goto next_opcode;}
	/* REX prefix(es) */
	else if (*opcode >= 0x40 && *opcode <= 0x4f) {rex = opcode; goto next_opcode;}

	if (prefix) return true; /* dirty fix for non supported mappings */

	/* table of supported instructions */
	uint32_t	table_supported_opcode[TABLESIZE];
					   /* 0 1 2 3 4 5 6 7  8 9 a b c d e f */
	table_supported_opcode[0] = BITMASK32(1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* 0 */
					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* 1 */
	table_supported_opcode[1] = BITMASK32(0,0,0,0,0,0,0,0, 0,1,0,0,0,0,0,0,  /* 2 */
					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* 3 */
	table_supported_opcode[2] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* 4 */
					      1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1); /* 5 */
	table_supported_opcode[3] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* 6 */
					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* 7 */
	table_supported_opcode[4] = BITMASK32(0,0,0,1,0,1,0,1, 0,1,0,1,0,0,0,0,  /* 8 */
					      1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* 9 */
	table_supported_opcode[5] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* a */
					      0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1); /* b */
	table_supported_opcode[6] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* c */
					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* d */
	table_supported_opcode[7] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* e */
					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* f */

	if (!CHECK_TABLE(table_supported_opcode, *opcode)) return false;

	/*
	** Pack the register value to the following format:
	** - reg: register number
	** - xreg: register extension; promote to extended registers (r8-r15)
	*/
	# define REG_PACK(reg, xreg)	(((xreg << 3) | (reg)) & 0b1111)
	/* structure describing how the instruction should be disassembled */
	struct x86_set
	{
		uint32_t	status;        /* flags about instruction behaviour */
		uint32_t	implicit_base; /* base value for implicit registers */
	} instructions[255];

	instructions[0x00] = (struct x86_set){MODRM       ,    0}; /* add rm8 reg8                 */
	instructions[0x01] = (struct x86_set){MODRM       ,    0}; /* add rm16/32/64 reg16/32/64   */
	instructions[0x29] = (struct x86_set){MODRM       ,    0}; /* sub rm16/32/64 reg16/32/64   */
	instructions[0x50] = (struct x86_set){IMPLICIT_SRC, 0x50}; /* push rAX/r8                  */
	instructions[0x51] = (struct x86_set){IMPLICIT_SRC, 0x50}; /* push rCX/r9                  */
	instructions[0x52] = (struct x86_set){IMPLICIT_SRC, 0x50}; /* push rDX/r10                 */
	instructions[0x53] = (struct x86_set){IMPLICIT_SRC, 0x50}; /* push rBX/r11                 */
	instructions[0x54] = (struct x86_set){IMPLICIT_SRC, 0x50}; /* push rSP/r12                 */
	instructions[0x55] = (struct x86_set){IMPLICIT_SRC, 0x50}; /* push rBP/r13                 */
	instructions[0x56] = (struct x86_set){IMPLICIT_SRC, 0x50}; /* push rSI/r14                 */
	instructions[0x57] = (struct x86_set){IMPLICIT_SRC, 0x50}; /* push rDI/r15                 */
	instructions[0x58] = (struct x86_set){IMPLICIT_DST, 0x58}; /* pop rAX/r8                   */
	instructions[0x59] = (struct x86_set){IMPLICIT_DST, 0x58}; /* pop rCX/r9                   */
	instructions[0x5a] = (struct x86_set){IMPLICIT_DST, 0x58}; /* pop rDX/r10                  */
	instructions[0x5b] = (struct x86_set){IMPLICIT_DST, 0x58}; /* pop rBX/r11                  */
	instructions[0x5c] = (struct x86_set){IMPLICIT_DST, 0x58}; /* pop rSP/r12                  */
	instructions[0x5d] = (struct x86_set){IMPLICIT_DST, 0x58}; /* pop rBP/r13                  */
	instructions[0x5e] = (struct x86_set){IMPLICIT_DST, 0x58}; /* pop rSI/r14                  */
	instructions[0x5f] = (struct x86_set){IMPLICIT_DST, 0x58}; /* pop rDI/r15                  */
	instructions[0x83] = (struct x86_set){EXT         ,    0}; /* ? */
	instructions[0x85] = (struct x86_set){MODRM       ,    0}; /* test r/m16/32/64 reg16/32/64 */
	instructions[0x87] = (struct x86_set){MODRM       ,    0}; /* xchg reg16/32/64 r/m16/32/64 */
	instructions[0x89] = (struct x86_set){MODRM       ,    0}; /* mov r/m16/32/64 reg16/32/64  */
	instructions[0x8b] = (struct x86_set){MODRM       ,    0}; /* mov reg16/32/64 r/m16/32/64  */
	instructions[0xb8] = (struct x86_set){IMPLICIT_DST, 0xb8}; /* mov reAX imm16/32/64         */
	instructions[0xb9] = (struct x86_set){IMPLICIT_DST, 0xb8}; /* mov reCX imm16/32/64         */
	instructions[0xba] = (struct x86_set){IMPLICIT_DST, 0xb8}; /* mov reDX imm16/32/64         */
	instructions[0xbb] = (struct x86_set){IMPLICIT_DST, 0xb8}; /* mov reBX imm16/32/64         */
	instructions[0xbc] = (struct x86_set){IMPLICIT_DST, 0xb8}; /* mov reSP imm16/32/64         */
	instructions[0xbd] = (struct x86_set){IMPLICIT_DST, 0xb8}; /* mov reBP imm16/32/64         */
	instructions[0xbe] = (struct x86_set){IMPLICIT_DST, 0xb8}; /* mov reSI imm16/32/64         */
	instructions[0xbf] = (struct x86_set){IMPLICIT_DST, 0xb8}; /* mov reDI imm16/32/64         */
	instructions[0x90] = (struct x86_set){           0,    0}; /* nop                          */

	struct x86_set	i = instructions[*opcode];
	uint8_t		rex_rxb  = 0;
	uint8_t		reg_mode = 0;
	uint8_t		rm_mode  = 0;
	if (rex)
	{
		rex_rxb  = *rex & 0b00000111;   /* retrieve rex modes      */
		reg_mode = !!(rex_rxb & 0b100); /* get reg extension field */
		rm_mode  = !!(rex_rxb & 0b001); /* get r/m extension field */
	}

	/* macros to define new registers */
	#define NEW_MODRM(reg, rm)	(((reg) << 3) | (rm))
	#define NEW_SIB(index, base)	(((index) << 3) | (base))

	if (i.status & IMPLICIT_SRC)
	{
		uint8_t	base        = i.implicit_base;
		uint8_t	current_reg = *opcode - base;

		uint32_t	op = match[REG_PACK(current_reg, !!(rex))].reg;
		if (!is_init(match, op)) return true;

		op &= 0b111;
		*opcode = base;
		*opcode += op & 0b0111;
	}
	if (i.status & IMPLICIT_DST)
	{
		uint8_t	base        = i.implicit_base;
		uint8_t	current_reg = *opcode - base;

		uint32_t	op = match[REG_PACK(current_reg, !!(rex))].reg;
		match[REG_PACK(current_reg, !!(rex))].is_init = true;

		op &= 0b111;
		*opcode = base;
		*opcode += op & 0b0111;
	}
	if (i.status & EXT)
	{
		if (!codelen--) return false; /* error if instruction is too long */
		uint8_t		*modrm = p++;
		uint8_t		reg    = (*modrm & 0b00111000) >> 3;
		uint8_t		rm     = *modrm & 0b00000111;

		uint8_t	new_rm         = match[REG_PACK(rm, rm_mode)].reg;
		if (!is_init(match, new_rm)) return true;

		new_rm &= 0b111;
		*modrm &= ~(0b00111111);
		*modrm |= NEW_MODRM(reg, new_rm);
	}
	/* MODRM byte management */
	if (i.status & MODRM)
	{
		if (!codelen--) return false; /* error if instruction is too long */
		uint8_t	*modrm  = p++;
		uint8_t	mod     = (*modrm & 0b11000000) >> 6;
		uint8_t	reg     = (*modrm & 0b00111000) >> 3;
		uint8_t	rm      =  *modrm & 0b00000111;
		uint8_t	new_reg = match[REG_PACK(reg, reg_mode)].reg;
		uint8_t	new_rm  = match[REG_PACK(rm, rm_mode)].reg;

		uint8_t		direction = *opcode & 0b10; /* d=1 memory to register (reg dest), d=0 register to memory (reg source) */

		if (mod == 0b11) /* direct register addressing */
		{
			if (direction) {if (!is_init(match, new_rm))  return true;}
			else           {if (!is_init(match, new_reg)) return true;}
			if (direction) match[REG_PACK(new_reg, reg_mode)].is_init = true;
			else           match[REG_PACK(new_rm, rm_mode)].is_init = true;

			new_rm  &= 0b111;
			new_reg &= 0b111;
			*modrm &= ~(0b00111111);
			*modrm |= NEW_MODRM(new_reg, new_rm);
		}
		else if (mod == 0b00) /* indirect register addressing */
		{
			if (rm == 0b100) /* SIB */
			{
				if (!codelen--) return false; /* error if instruction is too long */
				uint8_t	*sib      = p++;
				uint8_t	index     = (*sib & 0b00111000) >> 3;
				uint8_t	base      =  *sib & 0b00000111;
				uint8_t	new_index = match[REG_PACK(index, rm_mode)].reg;
				uint8_t	new_base  = match[REG_PACK(base, rm_mode)].reg;

				/* reg is the destination */
				if (!is_init(match, index)) return true;
				if (!is_init(match, base))  return true;
				match[REG_PACK(new_reg, reg_mode)].is_init = true;

				new_reg &= 0b111;
				*modrm &= ~(0b00111000);
				*modrm |= NEW_MODRM(new_reg, rm);

				new_index &= 0b111;
				new_base &= 0b111;
				*sib &= ~(0b00111111);
				*sib |= NEW_SIB(new_index, new_base);
			}
			else if (rm == 0b101) /* displacement only addressing [rip + disp] */
			{
				/* reg is the destination */
				match[REG_PACK(new_reg, reg_mode)].is_init = true;

				new_reg &= 0b111;
				*modrm &= ~(0b00111000);
				*modrm |= NEW_MODRM(new_reg, rm);
			}
			else /* indirect register addressing */
			{
				/* r/m is dereferenced */
				if (direction) {if (!is_init(match, new_rm))  return true;}
				else           {if (!is_init(match, new_reg)) return true;}
				if (direction) match[REG_PACK(new_reg, reg_mode)].is_init = true;
				else           match[REG_PACK(new_rm, rm_mode)].is_init = true;

				new_reg &= 0b111;
				new_rm &= 0b111;
				*modrm &= ~(0b00111111);
				*modrm |= NEW_MODRM(new_reg, new_rm);
			}
		}
		else if (mod == 0b01 || mod == 0b10) /* indirect register addressing with 1 or 4 byte signed displacement */
		{
			if (rm == 0b100) /* SIB with displacement */
			{
				if (!codelen--) return false; /* error if instruction is too long */
				uint8_t	*sib      = p++;
				uint8_t	index     = (*sib & 0b00111000) >> 3;
				uint8_t	base      =  *sib & 0b00000111;
				uint8_t	new_index = match[REG_PACK(index, rm_mode)].reg;
				uint8_t	new_base  = match[REG_PACK(base, rm_mode)].reg;

				/* reg is the destination */
				if (!is_init(match, index)) return true;
				if (!is_init(match, base))  return true;
				match[REG_PACK(new_reg, reg_mode)].is_init = true;

				new_reg &= 0b111;
				*modrm &= ~(0b00111000);
				*modrm |= NEW_MODRM(new_reg, rm);

				new_index &= 0b111;
				new_base &= 0b111;
				*sib &= ~(0b00111111);
				*sib |= NEW_SIB(new_index, new_base);
			}
			else /* indirect register with displacement */
			{
				/* r/m is dereferenced */
				if (direction) {if (!is_init(match, new_rm))  return true;}
				else           {if (!is_init(match, new_reg)) return true;}
				if (direction) match[REG_PACK(new_reg, reg_mode)].is_init = true;
				else           match[REG_PACK(new_rm, rm_mode)].is_init = true;

				new_reg &= 0b111;
				new_rm &= 0b111;
				*modrm &= ~(0b00111111);
				*modrm |= NEW_MODRM(new_reg, new_rm);
			}
		}
	}
	return true;
}

bool		permutate_registers(void *buffer, uint64_t seed, size_t size)
{
	/* init to masks so unallowed registers can easely be ignored */
	struct reg_match	match[16];
	match[0b0000] = (struct reg_match){RAX, false};
	match[0b0001] = (struct reg_match){RCX, false};
	match[0b0010] = (struct reg_match){RDX, false};
	match[0b0011] = (struct reg_match){RBX, false};
	match[0b0100] = (struct reg_match){RSP, false};
	match[0b0101] = (struct reg_match){RBP, false};
	match[0b0110] = (struct reg_match){RSI, false};
	match[0b0111] = (struct reg_match){RDI, false};
	match[0b1000] = (struct reg_match){R8,  false};
	match[0b1001] = (struct reg_match){R9,  false};
	match[0b1010] = (struct reg_match){R10, false};
	match[0b1011] = (struct reg_match){R11, false};
	match[0b1100] = (struct reg_match){R12, false};
	match[0b1101] = (struct reg_match){R13, false};
	match[0b1110] = (struct reg_match){R14, false};
	match[0b1111] = (struct reg_match){R15, false};

	shuffle_registers(match, seed, 0b111);

	/* convert masks to equivalent value */
	match[0b0000].reg = mask_to_index(match[0b0000].reg);
	match[0b0001].reg = mask_to_index(match[0b0001].reg);
	match[0b0010].reg = mask_to_index(match[0b0010].reg);
	match[0b0011].reg = mask_to_index(match[0b0011].reg);
	match[0b0100].reg = mask_to_index(match[0b0100].reg);
	match[0b0101].reg = mask_to_index(match[0b0101].reg);
	match[0b0110].reg = mask_to_index(match[0b0110].reg);
	match[0b0111].reg = mask_to_index(match[0b0111].reg);
	match[0b1000].reg = mask_to_index(match[0b1000].reg);
	match[0b1001].reg = mask_to_index(match[0b1001].reg);
	match[0b1010].reg = mask_to_index(match[0b1010].reg);
	match[0b1011].reg = mask_to_index(match[0b1011].reg);
	match[0b1100].reg = mask_to_index(match[0b1100].reg);
	match[0b1101].reg = mask_to_index(match[0b1101].reg);
	match[0b1110].reg = mask_to_index(match[0b1110].reg);
	match[0b1111].reg = mask_to_index(match[0b1111].reg);

	size_t		instruction_length = 0;

	while (size)
	{
		instruction_length = disasm_length(buffer, size);
		if (instruction_length == 0) break ;

		if (!apply_match(buffer, instruction_length, match))
			return errors(ERR_VIRUS, _ERR_PERMUTATE_REGISTERS);

		buffer += instruction_length;
		size   -= instruction_length;
	}
	return true;
}
