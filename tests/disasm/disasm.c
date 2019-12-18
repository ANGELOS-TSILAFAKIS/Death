
#include "disasm.h"

struct x86_set
{
	uint8_t		opcode;
	uint8_t		dst;
	uint8_t		src;
	uint8_t		flags;
};

/*    RFLAGS status    */
#define CF	0b000001	/* carry flag    */
#define PF	0b000010	/* parity flag   */
#define AF	0b000100	/* adjust flag   */
#define ZF	0b001000	/* zero flag     */
#define SF	0b010000	/* sign flag     */
#define OF	0b100000	/* overflow flag */

#define REG	0b01
#define RM	0b10

/* x86 instruction set */
#define X86_00	{0x00, RM, REG,           OF | SZ | ZF | AF | PF | CF}		/* _ADD */
#define X86_01	{0x01, RM, REG,           OF | SZ | ZF | AF | PF | CF}		/* _ADD */
#define X86_02	{0x02, REG, RM,           OF | SZ | ZF | AF | PF | CF}		/* _ADD */
#define X86_03	{0x03, REG, RM,           OF | SZ | ZF | AF | PF | CF}		/* _ADD */
#define X86_04	{0x04, RAX, BYTE,         OF | SZ | ZF | AF | PF | CF}		/* _ADD */
#define X86_05	{0x05, RAX, WORD | DWORD, OF | SZ | ZF | AF | PF | CF}		/* _ADD */
#define X86_08	{0x08, RM, REG,           OF | SZ | ZF | AF | PF | CF}		/* _OR */
#define X86_09	{0x09, RM, REG,           OF | SZ | ZF | AF | PF | CF}		/* _OR */
#define X86_0A	{0x0A, REG, RM,           OF | SZ | ZF | AF | PF | CF}		/* _OR */
#define X86_0B	{0x0B, REG, RM,           OF | SZ | ZF | AF | PF | CF}		/* _OR */
#define X86_0C	{0x0C, RAX, BYTE,         OF | SZ | ZF | AF | PF | CF}		/* _OR */
#define X86_0D	{0x0D, RAX, WORD | DWORD, OF | SZ | ZF | AF | PF | CF}		/* _OR */
// #define X86_ADC
// #define X86_SBB
#define X86_20	{0x20, RM, REG,           OF | SZ | ZF | AF | PF | CF}	/* _AND */
#define X86_21	{0x21, RM, REG,           OF | SZ | ZF | AF | PF | CF}	/* _AND */
#define X86_22	{0x22, REG, RM,           OF | SZ | ZF | AF | PF | CF}	/* _AND */
#define X86_23	{0x23, REG, RM,           OF | SZ | ZF | AF | PF | CF}	/* _AND */
#define X86_24	{0x24, RAX, BYTE,         OF | SZ | ZF | AF | PF | CF}	/* _AND */
#define X86_25	{0x25, RAX, WORD | DWORD, OF | SZ | ZF | AF | PF | CF}	/* _AND */
#define X86_28	{0x28, RM, REG,           OF | SZ | ZF | AF | PF | CF}	/* _SUB */
#define X86_29	{0x29, RM, REG,           OF | SZ | ZF | AF | PF | CF}	/* _SUB */
#define X86_2A	{0x2A, REG, RM,           OF | SZ | ZF | AF | PF | CF}	/* _SUB */
#define X86_2B	{0x2B, REG, RM,           OF | SZ | ZF | AF | PF | CF}	/* _SUB */
#define X86_2C	{0x2C, RAX, BYTE,         OF | SZ | ZF | AF | PF | CF}	/* _SUB */
#define X86_2D	{0x2D, RAX, WORD | DWORD, OF | SZ | ZF | AF | PF | CF}	/* _SUB */
#define X86_30	{0x30, RM, REG,           OF | SZ | ZF | AF | PF | CF}	/* _XOR */
#define X86_31	{0x31, RM, REG,           OF | SZ | ZF | AF | PF | CF}	/* _XOR */
#define X86_32	{0x32, REG, RM,           OF | SZ | ZF | AF | PF | CF}	/* _XOR */
#define X86_33	{0x33, REG, RM,           OF | SZ | ZF | AF | PF | CF}	/* _XOR */
#define X86_34	{0x34, RAX, BYTE,         OF | SZ | ZF | AF | PF | CF}	/* _XOR */
#define X86_35	{0x35, RAX, WORD | DWORD, OF | SZ | ZF | AF | PF | CF}	/* _XOR */
#define X86_38	{0x38, RM, REG,           OF | SZ | ZF | AF | PF | CF}	/* _CMP */
#define X86_39	{0x39, RM, REG,           OF | SZ | ZF | AF | PF | CF}	/* _CMP */
#define X86_3A	{0x3A, REG, RM,           OF | SZ | ZF | AF | PF | CF}	/* _CMP */
#define X86_3B	{0x3B, REG, RM,           OF | SZ | ZF | AF | PF | CF}	/* _CMP */
#define X86_3C	{0x3C, RAX, BYTE,         OF | SZ | ZF | AF | PF | CF}	/* _CMP */
#define X86_3D	{0x3D, RAX, WORD | DWORD, OF | SZ | ZF | AF | PF | CF}	/* _CMP */
#define X86_50	{0x50, RSP | MEMORY, RAX, NONE}				/* _PUSH */
#define X86_51	{0x51, RSP | MEMORY, RCX, NONE}				/* _PUSH */
#define X86_52	{0x52, RSP | MEMORY, RDX, NONE}				/* _PUSH */
#define X86_53	{0x53, RSP | MEMORY, RBX, NONE}				/* _PUSH */
#define X86_54	{0x54, RSP | MEMORY, RSP, NONE}				/* _PUSH */
#define X86_55	{0x55, RSP | MEMORY, RBP, NONE}				/* _PUSH */
#define X86_56	{0x56, RSP | MEMORY, RSI, NONE}				/* _PUSH */
#define X86_57	{0x57, RSP | MEMORY, RDI, NONE}				/* _PUSH */
#define X86_58	{0x50, RAX, RSP | MEMORY, NONE}				/* _POP */
#define X86_59	{0x51, RCX, RSP | MEMORY, NONE}				/* _POP */
#define X86_5A	{0x52, RDX, RSP | MEMORY, NONE}				/* _POP */
#define X86_5B	{0x53, RBX, RSP | MEMORY, NONE}				/* _POP */
#define X86_5C	{0x54, RSP, RSP | MEMORY, NONE}				/* _POP */
#define X86_5D	{0x55, RBP, RSP | MEMORY, NONE}				/* _POP */
#define X86_5E	{0x56, RSI, RSP | MEMORY, NONE}				/* _POP */
#define X86_5F	{0x57, RDI, RSP | MEMORY, NONE}				/* _POP */
// #define X86_MOVSDX
// #define X86_IMUL
// #define X86_INS
// #define X86_OUTS
// #define X86_JO
// #define X86_JNO
// #define X86_JB_JNAE_JC
// #define X86_JNB_JAE_JNC
// #define X86_JZ_JE
// #define X86_JNE_JNE
// #define X86_JBE_JNA
// #define X86_JS
// #define X86_JNS
// #define X86_JP_JPE
// #define X86_JNP_JPO
// #define X86_JL_JNGE
// #define X86_JNL_JGE
// #define X86_JLE_JNG
// #define X86_JNLE_JG
// #define X86_TEST
// #define X86_XCHG
#define X86_89	{0x89, REG, RM, NONE}						/* _MOV */
#define X86_8A	{0x8A, REG, RM, NONE}						/* _MOV */
#define X86_8B	{0x8B, RM, REG, NONE}						/* _MOV */
#define X86_8C	{0x8C, RM, REG, NONE}						/* _MOV */
// #define X86_LEA
// #define X86_NOP
// #define X86_PAUSE
// #define X86_CBW
// #define X86_CWD
// #define X86_MOVS
// #define X86_CMPS
// #define X86_STOS
// #define X86_LODS
// #define X86_SCAS
// #define X86_ROL
// #define X86_ROR
// #define X86_RCL
// #define X86_RCR
// #define X86_SHL
// #define X86_SHR
// #define X86_SAR
// #define X86_RETN
// #define X86_ENTER
// #define X86_LEAVE
// #define X86_INT
// #define X86_REPNZ
// #define X86_REP
// #define X86_REPZ
// #define X86_REPE
// #define X86_NOT
// #define X86_NEG
// #define X86_MUL
// #define X86_DIV
// #define X86_CLD
// #define X86_INC
// #define X86_DEC
// #define X86_CALL
// #define X86_JMP

static void	disasm_instruction(const void *code, size_t codelen, uint32_t *src, uint32_t *dst)
{
	uint32_t		table_supported_opcode[TABLESIZE];
	table_supported_opcode[0]   = BITMASK32(1,1,1,1,1,1,0,0, 1,1,1,1,1,1,0,0, 1,1,1,1,1,1,0,0, 1,1,1,1,1,1,0,0);
	table_supported_opcode[1]   = BITMASK32(1,1,1,1,1,1,0,0, 1,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[2]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1);
	table_supported_opcode[3]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[4]   = BITMASK32(0,0,0,0,0,0,0,0, 0,1,1,1,1,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[5]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[6]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_supported_opcode[7]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);

	PD_ARRAY(struct x86_set, set,
	{0x00, RM, REG,           OF | SZ | ZF | AF | PF | CF},
	{0x01, RM, REG,           OF | SZ | ZF | AF | PF | CF},
	{0x02, REG, RM,           OF | SZ | ZF | AF | PF | CF},
	{0x03, REG, RM,           OF | SZ | ZF | AF | PF | CF},
	{0x04, RAX, BYTE,         OF | SZ | ZF | AF | PF | CF},
	{0x05, RAX, WORD | DWORD, OF | SZ | ZF | AF | PF | CF},
	{0x06, NONE, NONE},
	{0x07, NONE, NONE},
	{0x08, RM, REG,           OF | SZ | ZF | AF | PF | CF},
	{0x09, RM, REG,           OF | SZ | ZF | AF | PF | CF},
	{0x0A, REG, RM,           OF | SZ | ZF | AF | PF | CF},
	{0x0B, REG, RM,           OF | SZ | ZF | AF | PF | CF},
	{0x0C, RAX, BYTE,         OF | SZ | ZF | AF | PF | CF},
	{0x0D, RAX, WORD | DWORD, OF | SZ | ZF | AF | PF | CF},



	);

	uint8_t		*p = (uint8_t*)code;
	uint8_t		opcode;

	next_opcode:
	if (!codelen--) return ; /* Error if instruction is too long */
	opcode = *p++;

	printf("opcode: %02x\n", opcode);
	struct x86_set	i = GET_OPCODE(opcode);

	printf("opcode:%02x src:%02x dst:%02x flags:%02x\n", i.opcode, i.src, i.dst, i.flags);

	if (!CHECK_TABLE(table_supported_opcode, opcode))
		*src = UNKNOWN; *dst = UNKNOWN; return ;

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

// static void             dummy(void)
// {
//         asm volatile (".intel_syntax;\n"
// 	"add rcx, [rip+0x1]\n"
//         "leave\n"
//         "ret\n"
//         );
//         __builtin_unreachable();
// }

// static void             hexdump_text(const char *text, uint64_t offset, uint64_t size, uint8_t i_size)
// {
//         uint64_t        i;
//         uint64_t        j;
//
//         printf("--------------------------------------\n");
//         i = 0;
//         while (i < size)
//         {
//                 printf("\e[34m%016llx\e[0m\t", offset + i);
//                 j = 0;
//                 while (j < 0x10 && i + j < size)
//                 {
//                         if (i_size)
//                         {
//                                 printf("\e[36m%02hhx \e[0m", text[i + j++]);
//                                 i_size--;
//                         }
//                         else
//                                 printf("%02hhx ", text[i + j++]);
//                 }
//                 printf("\n");
//                 i += 0x10;
//         }
// }

int		main(int ac, char **av)
{
	size_t			buflen = 50;
	struct s_instruction	buf[buflen];
	uint64_t		ret;
	void			*code = disasm_instruction;
	size_t			codelen =  (size_t)disasm_instruction - (size_t)disasm_length;

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
		// dprintf(fd, "buf[%d]: addr:%p length:%zu src:%s dst:%s\n", i, buf[i].addr, buf[i].length, _debug_get_register(buf[i].src), _debug_get_register(buf[i].dst));
	}
	if (fd != 1) close(fd);

	return 0;
}
