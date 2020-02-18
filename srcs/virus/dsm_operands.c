
#include "compiler_utils.h"
#include "accessors.h"
#include "utils.h"
#include "dsm_utils.h"

#define REX_R(rex)	(!!(rex & 0b100)) /* MODRM (R)eg field                         */
#define REX_X(rex)	(!!(rex & 0b010)) /* SIB inde(X) field                         */
#define REX_B(rex)	(!!(rex & 0b001)) /* MODRM r/m, SIB (B)ase or OPCODE reg field */

#define ENCODE_FLAG(reg, mode)	(1 << (reg + (mode << 3)))

/*
** Flags used to specify how the intruction should be interpreted
*/
#define MODRM		(1 << 0)  /* MODRM byte with register usage          */
#define EXTENSION	(1 << 1)  /* MODRM byte with opcode extension        */
#define EXT_SRC		(1 << 2) /* extension assigned to source             */
#define EXT_DST		(1 << 3) /* extension assigned to destination        */
#define KEEP_SRC	(1 << 4)  /* source operand is also a destination    */
#define KEEP_DST	(1 << 5)  /* destination operand is also a source    */
#define IMPLICIT_SRC	(1 << 6)  /* source register is implicit             */
#define IMPLICIT_DST	(1 << 7)  /* destination register is implicit        */
#define NONE_SRC	(1 << 8)  /* no final source operand                 */
#define NONE_DST	(1 << 9)  /* no final destination operand            */
/*
** Opcode fields
*/
#define FLD_R 		(1 << 10) /* register code added to primary opcode   */
#define FLD_D		(1 << 11) /* direction of data operation             */

struct x64_set
{
	uint16_t	status;    /* directives          */
	uint32_t	dst_flags; /* destination operand */
	uint32_t	src_flags; /* source operand      */
};

struct dsm
{
	uint32_t	*src;      /* source              */
	uint32_t	*dst;      /* destination         */
	uint8_t		rex;       /* rex byte            */
	bool		direction; /* operation direction */
};

static struct x64_set	map_single(uint8_t *p, uint8_t opcode, size_t codelen)
{
	struct x64_set	i;
	struct x64_set	instructions[0xff+1];

	bzero(instructions, sizeof(instructions));

	instructions[0x00] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* add    r/m8        r8                 */
	instructions[0x01] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* add    r/m16/32/64 r16/32/64          */
	instructions[0x03] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* add    r16/32/64   r/m16/32/64        */
	instructions[0x05] = (struct x64_set){IMPLICIT_DST|KEEP_DST                   ,FLAGS|RAX ,         0}; /* add    rAX         imm16/32           */
	instructions[0x09] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* or     r/m16/32/64 r16/32/64          */
	instructions[0x0b] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* or     r16/32/64   r/m16/32/64        */
	instructions[0x21] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* and    r16/32/64   r/m16/32/64        */
	instructions[0x23] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* and    r16/32/64   r/m16/32/64        */
	instructions[0x24] = (struct x64_set){IMPLICIT_DST|KEEP_DST                   ,FLAGS|RAX ,         0}; /* and    al          imm8               */
	instructions[0x25] = (struct x64_set){IMPLICIT_DST|KEEP_DST                   ,FLAGS|RAX ,         0}; /* and    rAX         imm16/32           */
	instructions[0x29] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* sub    r/m16/32/64 r16/32/64          */
	instructions[0x2b] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* sub    r16/32/64   r/m16/32/64        */
	instructions[0x31] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* xor    r/m16/32/64 r16/32/64          */
	instructions[0x33] = (struct x64_set){MODRM|FLD_D|KEEP_DST                    ,FLAGS     ,         0}; /* xor    r16/32/64   r/m16/32/64        */
	instructions[0x39] = (struct x64_set){MODRM|FLD_D|KEEP_DST|NONE_DST|KEEP_DST  ,FLAGS     ,         0}; /* cmp    r/m16/32/64 r16/32/64          */
	instructions[0x3b] = (struct x64_set){MODRM|FLD_D|KEEP_DST|NONE_DST|KEEP_DST  ,FLAGS     ,         0}; /* cmp    r16/32/64   r/m16/32/64        */
	instructions[0x3d] = (struct x64_set){IMPLICIT_SRC|KEEP_DST|NONE_DST          ,FLAGS     ,RAX       }; /* cmp    rAX         imm16/32           */
	instructions[0x50] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_DST,MEMORY|RSP,RAX       }; /* push   rAX/r8                         */
	instructions[0x51] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_DST,MEMORY|RSP,RCX       }; /* push   rCX/r9                         */
	instructions[0x52] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_DST,MEMORY|RSP,RDX       }; /* push   rDX/r10                        */
	instructions[0x53] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_DST,MEMORY|RSP,RBX       }; /* push   rBX/r11                        */
	instructions[0x54] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_DST,MEMORY|RSP,RSP       }; /* push   rSP/r12                        */
	instructions[0x55] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_DST,MEMORY|RSP,RBP       }; /* push   rBP/r13                        */
	instructions[0x56] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_DST,MEMORY|RSP,RSI       }; /* push   rSI/r14                        */
	instructions[0x57] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_DST,MEMORY|RSP,RDI       }; /* push   rDI/r15                        */
	instructions[0x58] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_SRC,RAX       ,MEMORY|RSP}; /* pop    rAX/r8                         */
	instructions[0x59] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_SRC,RCX       ,MEMORY|RSP}; /* pop    rCX/r9                         */
	instructions[0x5a] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_SRC,RDX       ,MEMORY|RSP}; /* pop    rDX/r10                        */
	instructions[0x5b] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_SRC,RBX       ,MEMORY|RSP}; /* pop    rBX/r11                        */
	instructions[0x5c] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_SRC,RSP       ,MEMORY|RSP}; /* pop    rSP/r12                        */
	instructions[0x5d] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_SRC,RBP       ,MEMORY|RSP}; /* pop    rBP/r13                        */
	instructions[0x5e] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_SRC,RSI       ,MEMORY|RSP}; /* pop    rSI/r14                        */
	instructions[0x5f] = (struct x64_set){FLD_R|IMPLICIT_DST|IMPLICIT_SRC|KEEP_SRC,RDI       ,MEMORY|RSP}; /* pop    rDI/r15                        */
	instructions[0x63] = (struct x64_set){MODRM|FLD_D                             ,         0,         0}; /* movsxd r32/64      r/m32              */
	instructions[0x6b] = (struct x64_set){MODRM                                   ,FLAGS     ,         0}; /* imul   r16/32/64   r/m16/32/64   imm8 */
	instructions[0x80] = (struct x64_set){EXTENSION                               ,         0,         0}; /*                                       */
	instructions[0x81] = (struct x64_set){EXTENSION                               ,         0,         0}; /*                                       */
	instructions[0x83] = (struct x64_set){EXTENSION                               ,         0,         0}; /*                                       */
	instructions[0x85] = (struct x64_set){MODRM|KEEP_SRC|NONE_DST                 ,FLAGS     ,         0}; /* test   r/m16/32/64 reg16/32/64        */
	instructions[0x87] = (struct x64_set){MODRM|KEEP_SRC|KEEP_DST                 ,         0,         0}; /* xchg   reg16/32/64 r/m16/32/64        */
	instructions[0x88] = (struct x64_set){MODRM|FLD_D                             ,         0,         0}; /* mov    r/m8        r8                 */
	instructions[0x89] = (struct x64_set){MODRM|FLD_D                             ,         0,         0}; /* mov    r/m16/32/64 r16/32/64          */
	instructions[0x8a] = (struct x64_set){MODRM|FLD_D                             ,         0,         0}; /* mov    r8          r/m8               */
	instructions[0x8b] = (struct x64_set){MODRM|FLD_D                             ,         0,         0}; /* mov    r16/32/64   r/m16/32/64        */
	instructions[0x8d] = (struct x64_set){MODRM                                   ,         0,MEMORY    }; /* lea    r16/32/64   m                  */
	instructions[0x99] = (struct x64_set){IMPLICIT_DST|IMPLICIT_SRC               ,RDX       ,RAX       }; /* cqo                                   */
	instructions[0xa8] = (struct x64_set){IMPLICIT_SRC|NONE_DST                   ,FLAGS     ,RAX       }; /* test   al          imm8               */
	instructions[0xb0] = (struct x64_set){FLD_R|IMPLICIT_DST                      ,RAX       ,         0}; /* mov    rAX         imm8               */
	instructions[0xb8] = (struct x64_set){FLD_R|IMPLICIT_DST                      ,RAX       ,         0}; /* mov    reAX        imm16/32/64        */
	instructions[0xb9] = (struct x64_set){FLD_R|IMPLICIT_DST                      ,RCX       ,         0}; /* mov    reCX        imm16/32/64        */
	instructions[0xba] = (struct x64_set){FLD_R|IMPLICIT_DST                      ,RDX       ,         0}; /* mov    reDX        imm16/32/64        */
	instructions[0xbb] = (struct x64_set){IMPLICIT_DST|NONE_SRC                   ,RBX       ,         0}; /* mov    reBX        imm16/32/64        */
	instructions[0xbc] = (struct x64_set){IMPLICIT_DST|NONE_SRC                   ,RSP       ,         0}; /* mov    reSP        imm16/32/64        */
	instructions[0xbd] = (struct x64_set){IMPLICIT_DST|NONE_SRC                   ,RBP       ,         0}; /* mov    reBP        imm16/32/64        */
	instructions[0xbe] = (struct x64_set){FLD_R|IMPLICIT_DST                      ,RSI       ,         0}; /* mov    RSI         imm16/32/64        */
	instructions[0xbf] = (struct x64_set){FLD_R|IMPLICIT_DST                      ,RDI       ,         0}; /* mov    RDI         imm16/32/64        */
	instructions[0xc1] = (struct x64_set){EXTENSION                               ,         0,         0}; /*                                       */
	instructions[0xc6] = (struct x64_set){EXTENSION                               ,         0,         0}; /*                                       */
	instructions[0xc7] = (struct x64_set){EXTENSION                               ,         0,         0}; /*                                       */
	instructions[0xd3] = (struct x64_set){EXTENSION                               ,         0,         0}; /*                                       */
	instructions[0xf6] = (struct x64_set){EXTENSION                               ,         0,         0}; /*                                       */
	instructions[0xf7] = (struct x64_set){EXTENSION                               ,         0,         0}; /*                                       */

	i = instructions[opcode];

	if (i.status & EXTENSION)
	{
		if (!codelen--) return (struct x64_set){0,0,0}; /* error if instruction is too long */
		uint8_t		modrm = *p;
		uint8_t		reg   = (modrm & 0b00111000) >> 3;

		struct x64_set	extensions[8];

		if (opcode == 0x80)
		{
			extensions[0] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* add r/m8 imm8 */
			extensions[1] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* or  r/m8 imm8 */
			extensions[2] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,FLAGS}; /* adc r/m8 imm8 */
			extensions[3] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,FLAGS}; /* sbb r/m8 imm8 */
			extensions[4] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* and r/m8 imm8 */
			extensions[5] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* sub r/m8 imm8 */
			extensions[6] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* xor r/m8 imm8 */
			extensions[7] = (struct x64_set){EXTENSION|KEEP_DST|NONE_DST,FLAGS,    0}; /* cmp r/m8 imm8 */
		}
		else if (opcode == 0x81)
		{
			extensions[0] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* add r/m16/32/64 imm16/32 */
			extensions[1] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* or  r/m16/32/64 imm16/32 */
			extensions[2] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,FLAGS}; /* adc r/m16/32/64 imm16/32 */
			extensions[3] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,FLAGS}; /* sbb r/m16/32/64 imm16/32 */
			extensions[4] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* and r/m16/32/64 imm16/32 */
			extensions[5] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* sub r/m16/32/64 imm16/32 */
			extensions[6] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* xor r/m16/32/64 imm16/32 */
			extensions[7] = (struct x64_set){EXTENSION|KEEP_DST|NONE_DST,FLAGS,    0}; /* cmp r/m16/32/64 imm16/32 */
		}
		else if (opcode == 0x83)
		{
			extensions[0] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* add r/m16/32/64 imm8 */
			extensions[1] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* or  r/m16/32/64 imm8 */
			extensions[2] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,FLAGS}; /* adc r/m16/32/64 imm8 */
			extensions[3] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,FLAGS}; /* sbb r/m16/32/64 imm8 */
			extensions[4] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* and r/m16/32/64 imm8 */
			extensions[5] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* sub r/m16/32/64 imm8 */
			extensions[6] = (struct x64_set){EXTENSION|KEEP_DST         ,FLAGS,    0}; /* xor r/m16/32/64 imm8 */
			extensions[7] = (struct x64_set){EXTENSION|KEEP_DST|NONE_DST,FLAGS,    0}; /* cmp r/m16/32/64 imm8 */
		}
		else if (opcode == 0xc1)
		{
			extensions[0] = (struct x64_set){EXTENSION|KEEP_DST,FLAGS,    0}; /* rol     r/m16/32/64 imm8 */
			extensions[1] = (struct x64_set){EXTENSION|KEEP_DST,FLAGS,    0}; /* ror     r/m16/32/64 imm8 */
			extensions[2] = (struct x64_set){EXTENSION|KEEP_DST,FLAGS,FLAGS}; /* rcl     r/m16/32/64 imm8 */
			extensions[3] = (struct x64_set){EXTENSION|KEEP_DST,FLAGS,FLAGS}; /* rcr     r/m16/32/64 imm8 */
			extensions[4] = (struct x64_set){EXTENSION|KEEP_DST,FLAGS,    0}; /* shl/sal r/m16/32/64 imm8 */
			extensions[5] = (struct x64_set){EXTENSION|KEEP_DST,FLAGS,    0}; /* shr     r/m16/32/64 imm8 */
			extensions[6] = (struct x64_set){EXTENSION|KEEP_DST,FLAGS,    0}; /* shl/sal r/m16/32/64 imm8 */
			extensions[7] = (struct x64_set){EXTENSION|KEEP_DST,FLAGS,    0}; /* sar     r/m16/32/64 imm8 */
		}
		else if (opcode == 0xc6)
		{
			extensions[0] = (struct x64_set){EXTENSION,0,0}; /* mov r/m8 imm8 */
			extensions[1] = (struct x64_set){        0,0,0}; /* undefined     */
			extensions[2] = (struct x64_set){        0,0,0}; /* undefined     */
			extensions[3] = (struct x64_set){        0,0,0}; /* undefined     */
			extensions[4] = (struct x64_set){        0,0,0}; /* undefined     */
			extensions[5] = (struct x64_set){        0,0,0}; /* undefined     */
			extensions[6] = (struct x64_set){        0,0,0}; /* undefined     */
			extensions[7] = (struct x64_set){        0,0,0}; /* undefined     */
		}
		else if (opcode == 0xc7)
		{
			extensions[0] = (struct x64_set){EXTENSION,0,0}; /* mov r/m16/32/64 imm16/32 */
			extensions[1] = (struct x64_set){        0,0,0}; /* undefined                */
			extensions[2] = (struct x64_set){        0,0,0}; /* undefined                */
			extensions[3] = (struct x64_set){        0,0,0}; /* undefined                */
			extensions[4] = (struct x64_set){        0,0,0}; /* undefined                */
			extensions[5] = (struct x64_set){        0,0,0}; /* undefined                */
			extensions[6] = (struct x64_set){        0,0,0}; /* undefined                */
			extensions[7] = (struct x64_set){        0,0,0}; /* undefined                */
		}
		else if (opcode == 0xd3)
		{
			extensions[0] = (struct x64_set){EXTENSION|IMPLICIT_SRC|KEEP_DST,FLAGS,RCX      }; /* rol     r/m16/32/64 cl */
			extensions[1] = (struct x64_set){EXTENSION|IMPLICIT_SRC|KEEP_DST,FLAGS,RCX      }; /* ror     r/m16/32/64 cl */
			extensions[2] = (struct x64_set){EXTENSION|IMPLICIT_SRC|KEEP_DST,FLAGS,FLAGS|RCX}; /* rcl     r/m16/32/64 cl */
			extensions[3] = (struct x64_set){EXTENSION|IMPLICIT_SRC|KEEP_DST,FLAGS,FLAGS|RCX}; /* rcr     r/m16/32/64 cl */
			extensions[4] = (struct x64_set){EXTENSION|IMPLICIT_SRC|KEEP_DST,FLAGS,RCX      }; /* shl/sal r/m16/32/64 cl */
			extensions[5] = (struct x64_set){EXTENSION|IMPLICIT_SRC|KEEP_DST,FLAGS,RCX      }; /* shr     r/m16/32/64 cl */
			extensions[6] = (struct x64_set){EXTENSION|IMPLICIT_SRC|KEEP_DST,FLAGS,RCX      }; /* sal/shl r/m16/32/64 cl */
			extensions[7] = (struct x64_set){EXTENSION|IMPLICIT_SRC|KEEP_DST,FLAGS,RCX      }; /* sar     r/m16/32/64 cl */
		}
		else if (opcode == 0xf6)
		{
			extensions[0] = (struct x64_set){EXTENSION|NONE_DST                 ,FLAGS    ,  0}; /* test r/m8 imm8           */
			extensions[1] = (struct x64_set){EXTENSION|NONE_DST                 ,FLAGS    ,  0}; /* test r/m8 imm8           */
			extensions[2] = (struct x64_set){EXTENSION                          ,        0,  0}; /* not  r/m8                */
			extensions[3] = (struct x64_set){EXTENSION                          ,FLAGS    ,  0}; /* neg  r/m8                */
			extensions[4] = (struct x64_set){EXTENSION|IMPLICIT_DST|IMPLICIT_SRC,RAX|FLAGS,RAX}; /* mul  ax   al   r/m8      */
			extensions[5] = (struct x64_set){EXTENSION|IMPLICIT_DST|IMPLICIT_SRC,RAX|FLAGS,RAX}; /* imul ax   al   r/m8      */
			extensions[6] = (struct x64_set){EXTENSION|IMPLICIT_DST|IMPLICIT_SRC,RAX|FLAGS,RAX}; /* div  al   ah   ax   r/m8 */
			extensions[7] = (struct x64_set){EXTENSION|IMPLICIT_DST|IMPLICIT_SRC,RAX|FLAGS,RAX}; /* idiv al   ah   ax   r/m8 */
		}
		else if (opcode == 0xf7)
		{
			extensions[0] = (struct x64_set){EXTENSION                                  ,FLAGS        ,      0}; /* test r/m16/32/64 imm16/32             */
			extensions[1] = (struct x64_set){EXTENSION                                  ,FLAGS        ,      0}; /* test r/m16/32/64 imm16/32             */
			extensions[2] = (struct x64_set){EXTENSION                                  ,            0,      0}; /* not  r/m16/32/64                      */
			extensions[3] = (struct x64_set){EXTENSION                                  ,FLAGS        ,      0}; /* neg  r/m16/32/64                      */
			extensions[4] = (struct x64_set){EXTENSION|IMPLICIT_DST|IMPLICIT_SRC        ,RAX|RDX|FLAGS,RAX|RDX}; /* mul  rDX         rAX      r/m16/32/64 */
			extensions[5] = (struct x64_set){EXTENSION|IMPLICIT_DST|IMPLICIT_SRC        ,RAX|RDX|FLAGS,RAX|RDX}; /* imul rDX         rAX      r/m16/32/64 */
			extensions[6] = (struct x64_set){EXTENSION|IMPLICIT_DST|EXT_SRC|IMPLICIT_SRC,RAX|RDX|FLAGS,RAX|RDX}; /* div  rDX         rAX      r/m16/32/64 */
			extensions[7] = (struct x64_set){EXTENSION|IMPLICIT_DST|EXT_SRC             ,RAX|RDX|FLAGS,RAX    }; /* idiv rDX         rAX      r/m16/32/64 */
		}
		i = extensions[reg];
	}

	return i;
}

static struct x64_set	map_0f(uint8_t *p, uint8_t opcode, size_t codelen)
{
	struct x64_set	i;
	struct x64_set	instructions[0xff+1];

	bzero(instructions, sizeof(instructions));

	instructions[0x45] = (struct x64_set){MODRM               ,    0,FLAGS}; /* cmovnz/cmovne r16/32/64 r/m16/32/64 */
	instructions[0x95] = (struct x64_set){EXTENSION           ,    0,    0}; /*                                     */
	instructions[0xaf] = (struct x64_set){MODRM|FLD_D|KEEP_DST,FLAGS,    0}; /* imul          r16/32/64 r/m16/32/64 */
	instructions[0xb6] = (struct x64_set){MODRM|FLD_D         ,    0,    0}; /* movzx         r16/32/64 r/m8        */
	instructions[0xb7] = (struct x64_set){MODRM|FLD_D         ,    0,    0}; /* movzx         r16/32/64 r/m16       */
	instructions[0xbe] = (struct x64_set){MODRM|FLD_D         ,    0,    0}; /* movsx         r16/32/64 r/m8        */

	i = instructions[opcode];

	if (i.status & EXTENSION)
	{
		if (!codelen--) return (struct x64_set){0,0,0}; /* error if instruction is too long */
		uint8_t		modrm = *p;
		uint8_t		reg   = (modrm & 0b00111000) >> 3;

		struct x64_set	extensions[8];

		if (opcode == 0x95)
		{
			extensions[0] = (struct x64_set){EXTENSION|KEEP_DST         ,0,FLAGS}; /* setz/sete r/m8 */
			extensions[1] = (struct x64_set){                          0,0,    0}; /* undefined      */
			extensions[2] = (struct x64_set){                          0,0,    0}; /* undefined      */
			extensions[3] = (struct x64_set){                          0,0,    0}; /* undefined      */
			extensions[4] = (struct x64_set){                          0,0,    0}; /* undefined      */
			extensions[5] = (struct x64_set){                          0,0,    0}; /* undefined      */
			extensions[6] = (struct x64_set){                          0,0,    0}; /* undefined      */
			extensions[7] = (struct x64_set){                          0,0,    0}; /* undefined      */
		}

		i = extensions[reg];
	}

	return i;
}

static void	memory_destination(uint32_t *src, uint32_t *dst)
{
	if (src && dst && (*dst & MEMORY && *dst & 0xffff))
	{
		*src |= *dst & 0xffff;
		*dst &= ~(0xffff);
	}
}

static void	register_code(uint32_t *operand, uint32_t *flags,
			uint32_t reg_flag, uint32_t reg_ext)
{
	*flags   &= ~(reg_flag); /* clear old flag              */
	*operand |= reg_ext;     /* assigned new register flag  */
}

static bool	sib(struct safe_ptr ref, struct dsm dsm, uint8_t reg)
{
	if (!ref.size--) return false; /* error if instruction is too long */

	uint32_t	*dst      = dsm.dst;
	uint32_t	*src      = dsm.src;
	uint8_t		rex       = dsm.rex;

	uint8_t		opcode = *((uint8_t*)ref.ptr++);
	uint8_t		index  = (opcode & 0b00111000) >> 3;
	uint8_t		base   =  opcode & 0b00000111;

	/* MODRM.reg is the destination */
	if (src)
	{
		*src |= ENCODE_FLAG(index, REX_X(rex));
		*src |= ENCODE_FLAG(base, REX_B(rex));
	}
	if (dst)
		*dst |= ENCODE_FLAG(reg, REX_R(rex));

	return true;
}

static bool	direct_register(__unused struct safe_ptr ref, struct dsm dsm,
			uint8_t reg, uint8_t rm)
{
	uint32_t	*dst      = dsm.dst;
	uint32_t	*src      = dsm.src;
	uint8_t		rex       = dsm.rex;
	bool		direction = dsm.direction;

	if (src) *src |= direction ? ENCODE_FLAG(rm, REX_B(rex)) : ENCODE_FLAG(reg, REX_R(rex));
	if (dst) *dst |= direction ? ENCODE_FLAG(reg, REX_R(rex)) : ENCODE_FLAG(rm, REX_B(rex));

	return true;
}

static bool	indirect_register(struct safe_ptr ref, struct dsm dsm,
			uint8_t reg, uint8_t rm)
{
	uint32_t	*dst      = dsm.dst;
	uint32_t	*src      = dsm.src;
	uint8_t		rex       = dsm.rex;
	bool		direction = dsm.direction;

	if (rm == 0b100) /* SIB */
	{
		if (!sib(ref, dsm, reg)) return false;
	}
	else if (rm == 0b101) /* displacement only addressing [rip + disp] */
	{
		/* MODRM.reg is the destination */
		if (src) *src |= RIP;
		if (dst) *dst |= ENCODE_FLAG(reg, REX_R(rex));
	}
	else /* indirect register addressing */
	{
		/* MODRM.rm is dereferenced */
		if (src) *src |= direction ? ENCODE_FLAG(rm, REX_B(rex)) : ENCODE_FLAG(reg, REX_R(rex));
		if (dst) *dst |= direction ? ENCODE_FLAG(reg, REX_R(rex)) : ENCODE_FLAG(rm, REX_B(rex));
	}
	if (src) *src |= direction ? MEMORY : 0;
	if (dst) *dst |= direction ? 0 : MEMORY;
	memory_destination(src, dst);

	return true;
}

static bool	indirect_register_displacement(struct safe_ptr ref, struct dsm dsm,
			uint8_t reg, uint8_t rm)
{
	uint32_t	*src      = dsm.src;
	uint32_t	*dst      = dsm.dst;
	uint8_t		rex       = dsm.rex;
	bool		direction = dsm.direction;

	if (rm == 0b100) /* SIB with displacement */
	{
		if (!sib(ref, dsm, reg)) return false;
	}
	else /* indirect register with displacement */
	{
		/* MODRM.rm is dereferenced */
		if (src) *src |= direction ? ENCODE_FLAG(rm, REX_B(rex)) : ENCODE_FLAG(reg, REX_R(rex));
		if (dst) *dst |= direction ? ENCODE_FLAG(reg, REX_R(rex)) : ENCODE_FLAG(rm, REX_B(rex));
	}
	if (src) *src |= direction ? MEMORY : 0;
	if (dst) *dst |= direction ? 0 : MEMORY;
	memory_destination(src, dst);

	return true;
}

static bool	modrm(struct safe_ptr ref, struct dsm dsm)
{
	if (!ref.size--) return false; /* error if instruction is too long */

	uint8_t		opcode = *((uint8_t*)ref.ptr++);
	uint8_t		mod    = (opcode & 0b11000000) >> 6;
	uint8_t		reg    = (opcode & 0b00111000) >> 3;
	uint8_t		rm     =  opcode & 0b00000111;

	bool		(*addressing_mode[0b100])(struct safe_ptr, struct dsm, uint8_t, uint8_t);

	addressing_mode[0b00] = indirect_register;
	addressing_mode[0b01] = indirect_register_displacement;
	addressing_mode[0b10] = indirect_register_displacement;
	addressing_mode[0b11] = direct_register;

	return addressing_mode[mod](ref, dsm, reg, rm);
}

static void	dsm_instruction(uint8_t *code, size_t codelen,
			uint32_t *src, uint32_t *dst)
{
	uint8_t		prefix = 0;
	uint8_t		rex    = 0;
	uint8_t		opcode;

	*src = UNKNOWN;
	*dst = UNKNOWN;

	/* set <codelen> to INSTRUCTION_MAXLEN if it exceeds it */
	if (codelen > INSTRUCTION_MAXLEN) codelen = INSTRUCTION_MAXLEN;

	/* prefix loop */
	next_opcode:
	if (!codelen--) return ; /* error if instruction is too long */
	opcode = *code++;

	/* skip unused legacy / REX prefixes */
	if ((opcode == 0x26 || opcode == 0x2e
	||   opcode == 0x36 || opcode == 0x3e
	||   opcode == 0x64 || opcode == 0x65 || opcode == 0x66 || opcode == 0x67
	||   opcode == 0xf0)
	&&  !(prefix))
		{goto next_opcode;}
	/* mandatory prefixes */
	else if (opcode == 0x0f) {prefix |= OP_PREFIX_0F; goto next_opcode;}
	else if (opcode == 0xf2) {prefix |= OP_PREFIX_F2; goto next_opcode;}
	else if (opcode == 0xf3) {prefix |= OP_PREFIX_F3; goto next_opcode;}
	else if (opcode == 0x9b) {prefix |= OP_PREFIX_9B; goto next_opcode;}
	/* REX prefix(es) */
	else if (!prefix && (opcode >= 0x40 && opcode <= 0x4f))
		{rex = opcode; goto next_opcode;}

	/*
	** Choose between different tables dependent of the mapping
	** to retrieve informations about the instruction and
	** how it should be interpreted.
	*/
	struct x64_set	i = (struct x64_set){0,0,0};

	if      (prefix == 0)      {i = map_single(code, opcode, codelen);}
	else if (prefix == MAP_0F) {i = map_0f(code, opcode, codelen);}

	if (i.status == 0) return ; /* instruction is not supported */

	*src = 0;
	*dst = 0;

	if (i.status & FLD_R)
	{
		/* register code added to primary opcode */
		uint8_t		reg_code = opcode & 0b111;
		uint32_t	reg_flag = ENCODE_FLAG(reg_code, 0);
		uint32_t	reg_ext  = ENCODE_FLAG(reg_code, REX_B(rex));

		if (i.src_flags & reg_flag)
			register_code(src, &i.src_flags, reg_flag, reg_ext);
		if (i.dst_flags & reg_flag)
			register_code(dst, &i.dst_flags, reg_flag, reg_ext);
	}
	else if (i.status & EXTENSION)
	{
		if (i.status & EXT_SRC)
		{
			if (!modrm((struct safe_ptr){.ptr=code, .size=codelen},
				(struct dsm){.dst=NULL, .src=src, .rex=rex, .direction=1}))
				return ;
		}
		else
		{
			if (!modrm((struct safe_ptr){.ptr=code, .size=codelen},
				(struct dsm){.dst=NULL, .src=dst, .rex=rex, .direction=1}))
				return ;
			memory_destination(src, dst);
		}
	}
	else if (i.status & MODRM)
	{
		/* d=1 memory to register (reg dest)   */
		/* d=0 register to memory (reg source) */
		bool	direction = i.status & FLD_D ? !!(opcode & 0b10) : 1;
		if (!modrm((struct safe_ptr){.ptr=code, .size=codelen},
			(struct dsm){.dst=dst, .src=src, .rex=rex, .direction=direction}))
			return ;
	}

	*src |= i.src_flags; /* get remaining flags */
	*dst |= i.dst_flags; /* get remaining flags */
	if (i.status & KEEP_DST) {*src |= *dst & 0xffff;} /* get register(s) from dst */
	if (i.status & KEEP_SRC) {*dst |= *src & 0xffff;} /* get register(s) from src */
	if (i.status & NONE_SRC) {*src &= ~(0xffff);}
	if (i.status & NONE_DST) {*dst &= ~(0xffff);}
}

size_t		dsm_operands(const void *code, size_t codelen,
			struct operands *buf, size_t buflen)
{
	void		*p_code = (void*)code;
	struct operands	*p_buf  = buf;
	size_t		instruction_length = 0;

	while (codelen && buflen)
	{
		instruction_length = dsm_length(p_code, codelen);
		if (instruction_length == 0) break ;
		dsm_instruction(p_code, codelen, &p_buf->src, &p_buf->dst);
		p_buf->addr   = p_code;
		p_buf->length = instruction_length;

		p_code  += instruction_length;
		codelen -= instruction_length;
		p_buf   += 1;
		buflen  -= 1;
	}
	return (p_buf - buf); /* number of instructions successfully disassembled */
}

//
// #include "disasm_utils.h"
//
// /*
// ** Flags used to specify operand or status.
// ** Although GP registers can have a 8-bit, 16-bit or 32-bit size we consider
// ** for this disassembler that the whole 64-bit register is affected.
// */
// # define NONE		(0)       /* nothing                            */
// # define RAX		(1 << 0)  /* al   || ax   || eax  || rax        */
// # define RCX		(1 << 1)  /* cl   || cx   || ecx  || rcx        */
// # define RDX		(1 << 2)  /* dl   || dx   || edx  || rdx        */
// # define RBX		(1 << 3)  /* bl   || bx   || ebx  || rbx        */
// # define RSP		(1 << 4)  /* ah   || spl  || sp   || esp || rsp */
// # define RBP		(1 << 5)  /* ch   || bpl  || bp   || ebp || rbp */
// # define RSI		(1 << 6)  /* dh   || sil  || si   || esi || rsi */
// # define RDI		(1 << 7)  /* bh   || dil  || di   || edi || rdi */
// # define R8		(1 << 8)  /* r8l  || r8w  || r8d  || r8         */
// # define R9		(1 << 9)  /* r9l  || r9w  || r9d  || r9         */
// # define R10		(1 << 10) /* r10l || r10w || r10d || r10        */
// # define R11		(1 << 11) /* r11l || r11w || r11d || r11        */
// # define R12		(1 << 12) /* r12l || r12w || r12d || r12        */
// # define R13		(1 << 13) /* r13l || r13w || r13d || r13        */
// # define R14		(1 << 14) /* r14l || r14w || r14d || r14        */
// # define R15		(1 << 15) /* r15l || r15w || r15d || r15        */
//
// # define RIP		(1 << 16) /* eip  || rip                        */
//
// # define FLAGS		(1 << 30) /* uses or modifies flags             */
// # define MEMORY		(1 << 31) /* references a memory location       */
// # define UNKNOWN	(~0)      /* unknown modification(s)            */
//
// /*
// ** Flags used to specify how the intruction should be interpreted
// */
// # define MODRM		(1 << 0) /* MODRM byte with register usage       */
// # define EXT		(1 << 1) /* MODRM byte with opcode extension     */
// # define KEEP_SRC	(1 << 2) /* source operand is also a destination */
// # define KEEP_DST	(1 << 3) /* destination operand is also a source */
// # define IMPLICIT_SRC	(1 << 4) /* source register is implicit          */
// # define IMPLICIT_DST	(1 << 5) /* destination register is implicit     */
// # define NO_SRC		(1 << 6) /* no source register                   */
// # define NO_DST		(1 << 7) /* no destination register              */
//
// static void	disasm_instruction(const void *code, size_t codelen,
// 			uint32_t *src, uint32_t *dst)
// {
// 	uint8_t		*p     = (uint8_t*)code;
// 	uint8_t		prefix = 0;              /* opcode prefix */
// 	uint8_t		rex    = 0;              /* REX byte value */
// 	uint8_t		opcode;
//
// 	*src = UNKNOWN;
// 	*dst = UNKNOWN;
//
// 	/* set <codelen> to INSTRUCTION_MAXLEN if it exceeds it */
// 	if (codelen > INSTRUCTION_MAXLEN) codelen = INSTRUCTION_MAXLEN;
//
// 	/* prefix loop */
// 	next_opcode:
// 	if (!codelen--) return ; /* error if instruction is too long */
// 	opcode = *p++;
//
// 	/* unused legacy / REX prefixes for register disassembler */
// 	if (opcode == 0x26 || opcode == 0x2e
// 	||  opcode == 0x36 || opcode == 0x3e
// 	||  opcode == 0x64 || opcode == 0x65 || opcode == 0x66 || opcode == 0x67
// 	||  opcode == 0xf0) {goto next_opcode;}
// 	/* mandatory prefix(es) */
// 	else if (opcode == 0x0f) {prefix |= OP_PREFIX_0F; goto next_opcode;}
// 	else if (opcode == 0xf2) {prefix |= OP_PREFIX_F2; goto next_opcode;}
// 	else if (opcode == 0xf3) {prefix |= OP_PREFIX_F3; goto next_opcode;}
// 	else if (opcode == 0x9b) {prefix |= OP_PREFIX_9B; goto next_opcode;}
// 	/* REX prefix(es) */
// 	else if (opcode >= 0x40 && opcode <= 0x4f) {rex = opcode; goto next_opcode;}
//
// 	if (prefix) {return ;} /* dirty fix for non supported mappings */
//
// 	/* table of supported instructions */
// 	uint32_t	table_supported_opcode[TABLESIZE];
// 					   /* 0 1 2 3 4 5 6 7  8 9 a b c d e f */
// 	table_supported_opcode[0] = BITMASK32(1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* 0 */
// 					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* 1 */
// 	table_supported_opcode[1] = BITMASK32(0,0,0,0,0,0,0,0, 0,1,0,0,0,0,0,0,  /* 2 */
// 					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* 3 */
// 	table_supported_opcode[2] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* 4 */
// 					      1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1); /* 5 */
// 	table_supported_opcode[3] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* 6 */
// 					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* 7 */
// 	table_supported_opcode[4] = BITMASK32(0,1,0,1,0,1,0,1, 0,1,0,1,0,0,0,0,  /* 8 */
// 					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* 9 */
// 	table_supported_opcode[5] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* a */
// 					      0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1); /* b */
// 	table_supported_opcode[6] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* c */
// 					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* d */
// 	table_supported_opcode[7] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  /* e */
// 					      0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0); /* f */
//
// 	if (!CHECK_TABLE(table_supported_opcode, opcode)) return ;
//
// 	/*
// 	** Pack the register value to the following format:
// 	** - reg: register number
// 	** - xreg: register extension; promote to extended registers (r8-r15)
// 	*/
// 	# define REG_PACK(reg, xreg)	(((xreg << 3) | (reg)) & 0b1111)
// 	/* GP registers.
// 	** The 3 lower bytes stands for the register number.
// 	** The higher byte stands for the extended version of the register.
// 	*/
// 	uint32_t	gp_registers[16];
// 	gp_registers[0b0000] = RAX;
// 	gp_registers[0b0001] = RCX;
// 	gp_registers[0b0010] = RDX;
// 	gp_registers[0b0011] = RBX;
// 	gp_registers[0b0100] = RSP;
// 	gp_registers[0b0101] = RBP;
// 	gp_registers[0b0110] = RSI;
// 	gp_registers[0b0111] = RDI;
// 	gp_registers[0b1000] = R8;
// 	gp_registers[0b1001] = R9;
// 	gp_registers[0b1010] = R10;
// 	gp_registers[0b1011] = R11;
// 	gp_registers[0b1100] = R12;
// 	gp_registers[0b1101] = R13;
// 	gp_registers[0b1110] = R14;
// 	gp_registers[0b1111] = R15;
//
// 	struct op_pack
// 	{
// 		uint32_t	flags; /* additionnal flags to add to result                  */
// 		uint8_t		reg;   /* register affected; used for implicits modifications */
// 		uint8_t		ext;   /* opcode extension                                    */
// 	};
// 	/* structure describing how the instruction should be disassembled */
// 	struct x86_set
// 	{
// 		uint32_t	status; /* flags about instruction behaviour */
// 		struct op_pack	src;    /* source operand                    */
// 		struct op_pack	dst;    /* destination operand               */
// 	} instructions[255];
//
// 	instructions[0x00] = (struct x86_set){MODRM|KEEP_DST                    , {     0,    0,    0}, { FLAGS,    0,    0}}; /* add rm8 reg8                 */
// 	instructions[0x01] = (struct x86_set){MODRM|KEEP_DST                    , {     0,    0,    0}, { FLAGS,    0,    0}}; /* add rm16/32/64 reg16/32/64   */
// 	instructions[0x29] = (struct x86_set){MODRM|KEEP_DST                    , {     0,    0,    0}, { FLAGS,    0,    0}}; /* sub rm16/32/64 reg16/32/64   */
// 	instructions[0x50] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_DST, {     0,0b000,    0}, {MEMORY,0b100,    0}}; /* push rAX/r8                  */
// 	instructions[0x51] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_DST, {     0,0b001,    0}, {MEMORY,0b100,    0}}; /* push rCX/r9                  */
// 	instructions[0x52] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_DST, {     0,0b010,    0}, {MEMORY,0b100,    0}}; /* push rDX/r10                 */
// 	instructions[0x53] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_DST, {     0,0b011,    0}, {MEMORY,0b100,    0}}; /* push rBX/r11                 */
// 	instructions[0x54] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_DST, {     0,0b100,    0}, {MEMORY,0b100,    0}}; /* push rSP/r12                 */
// 	instructions[0x55] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_DST, {     0,0b101,    0}, {MEMORY,0b100,    0}}; /* push rBP/r13                 */
// 	instructions[0x56] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_DST, {     0,0b110,    0}, {MEMORY,0b100,    0}}; /* push rSI/r14                 */
// 	instructions[0x57] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_DST, {     0,0b111,    0}, {MEMORY,0b100,    0}}; /* push rDI/r15                 */
// 	instructions[0x58] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_SRC, {MEMORY,0b100,    0}, {     0,0b000,    0}}; /* pop rAX/r8                   */
// 	instructions[0x59] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_SRC, {MEMORY,0b100,    0}, {     0,0b001,    0}}; /* pop rCX/r9                   */
// 	instructions[0x5a] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_SRC, {MEMORY,0b100,    0}, {     0,0b010,    0}}; /* pop rDX/r10                  */
// 	instructions[0x5b] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_SRC, {MEMORY,0b100,    0}, {     0,0b011,    0}}; /* pop rBX/r11                  */
// 	instructions[0x5c] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_SRC, {MEMORY,0b100,    0}, {     0,0b100,    0}}; /* pop rSP/r12                  */
// 	instructions[0x5d] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_SRC, {MEMORY,0b100,    0}, {     0,0b101,    0}}; /* pop rBP/r13                  */
// 	instructions[0x5e] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_SRC, {MEMORY,0b100,    0}, {     0,0b110,    0}}; /* pop rSI/r14                  */
// 	instructions[0x5f] = (struct x86_set){IMPLICIT_SRC|IMPLICIT_DST|KEEP_SRC, {MEMORY,0b100,    0}, {     0,0b111,    0}}; /* pop rDI/r15                  */
// 	instructions[0x81] = (struct x86_set){EXT|NO_SRC|KEEP_DST               , {     0,    0,0b101}, { FLAGS,    0,0b101}}; /* add(0b000) && sub(0b101)     */
// 	instructions[0x83] = (struct x86_set){EXT|NO_SRC|KEEP_DST               , {     0,    0,0b000}, { FLAGS,    0,0b000}}; /* add(0b000)                   */
// 	instructions[0x85] = (struct x86_set){MODRM|KEEP_SRC                    , {     0,    0,    0}, { FLAGS,    0,    0}}; /* test r/m16/32/64 reg16/32/64 */
// 	instructions[0x87] = (struct x86_set){MODRM|KEEP_SRC|KEEP_DST           , {     0,    0,    0}, {     0,    0,    0}}; /* xchg reg16/32/64 r/m16/32/64 */
// 	instructions[0x89] = (struct x86_set){MODRM                             , {     0,    0,    0}, {     0,    0,    0}}; /* mov r/m16/32/64 reg16/32/64  */
// 	instructions[0x8b] = (struct x86_set){MODRM|KEEP_SRC                    , {     0,    0,    0}, {     0,    0,    0}}; /* mov reg16/32/64 r/m16/32/64  */
// 	instructions[0xb8] = (struct x86_set){NO_SRC|IMPLICIT_DST               , {     0,    0,    0}, {     0,0b000,    0}}; /* mov reAX imm16/32/64         */
// 	instructions[0xb9] = (struct x86_set){NO_SRC|IMPLICIT_DST               , {     0,    0,    0}, {     0,0b001,    0}}; /* mov reCX imm16/32/64         */
// 	instructions[0xba] = (struct x86_set){NO_SRC|IMPLICIT_DST               , {     0,    0,    0}, {     0,0b010,    0}}; /* mov reDX imm16/32/64         */
// 	instructions[0xbb] = (struct x86_set){NO_SRC|IMPLICIT_DST               , {     0,    0,    0}, {     0,0b011,    0}}; /* mov reBX imm16/32/64         */
// 	instructions[0xbc] = (struct x86_set){NO_SRC|IMPLICIT_DST               , {     0,    0,    0}, {     0,0b100,    0}}; /* mov reSP imm16/32/64         */
// 	instructions[0xbd] = (struct x86_set){NO_SRC|IMPLICIT_DST               , {     0,    0,    0}, {     0,0b101,    0}}; /* mov reBP imm16/32/64         */
// 	instructions[0xbe] = (struct x86_set){NO_SRC|IMPLICIT_DST               , {     0,    0,    0}, {     0,0b110,    0}}; /* mov reSI imm16/32/64         */
// 	instructions[0xbf] = (struct x86_set){NO_SRC|IMPLICIT_DST               , {     0,    0,    0}, {     0,0b111,    0}}; /* mov reDI imm16/32/64         */
//
// 	struct x86_set		i = instructions[opcode];       /* get instruction         */
// 	uint8_t			rex_rxb  = rex & 0b00000111;    /* retrieve rex modes      */
// 	uint8_t			reg_mode = !!(rex_rxb & 0b100); /* get reg extension field */
// 	uint8_t			rm_mode  = !!(rex_rxb & 0b001); /* get r/m extension field */
//
// 	*src = 0;
// 	*dst = 0;
//
// 	/* dirty hack to not mix extended registers with not extended one for 1 byte opcode-based instructions */
// 	if (i.status & IMPLICIT_SRC) {if (i.src.flags & MEMORY) {*src |= gp_registers[REG_PACK(i.src.reg & 0b111, reg_mode)];}
// 				      else 			{*src |= gp_registers[REG_PACK(i.src.reg & 0b111, rm_mode)];}}
// 	if (i.status & IMPLICIT_DST) {if (i.dst.flags & MEMORY) {*dst |= gp_registers[REG_PACK(i.dst.reg & 0b111, reg_mode)];}
// 				      else 			{*dst |= gp_registers[REG_PACK(i.dst.reg & 0b111, rm_mode)];}}
// 	if (i.status & EXT)
// 	{
// 		if (!codelen--) return ; /* error if instruction is too long */
// 		uint8_t		modrm = *p++;
// 		uint8_t		rm    =  modrm & 0b00000111;
//
// 		*dst |= gp_registers[REG_PACK(rm, rm_mode)];
// 	}
// 	/* MODRM byte management */
// 	if (i.status & MODRM)
// 	{
// 		if (!codelen--) return ; /* error if instruction is too long */
// 		uint8_t		modrm = *p++; /* get MODRM byte                                                         */
// 		uint8_t		mod   = (modrm & 0b11000000) >> 6;
// 		uint8_t		reg   = (modrm & 0b00111000) >> 3;
// 		uint8_t		rm    =  modrm & 0b00000111;
//
// 		uint8_t		direction = opcode & 0b10; /* d=1 memory to register (reg dest), d=0 register to memory (reg source) */
//
// 		if (mod == 0b11) /* direct register addressing */
// 		{
// 			*src |= direction ? gp_registers[REG_PACK(rm, rm_mode)] : gp_registers[REG_PACK(reg, reg_mode)];
// 			*dst |= direction ? gp_registers[REG_PACK(reg, reg_mode)] : gp_registers[REG_PACK(rm, rm_mode)];
// 		}
// 		else if (mod == 0b00) /* indirect register addressing */
// 		{
// 			if (rm == 0b100) /* SIB */
// 			{
// 				if (!codelen--) return ; /* error if instruction is too long */
// 				uint8_t		sib   = *p++;
// 				uint8_t		index = (sib & 0b00111000) >> 3;
// 				uint8_t		base  =  sib & 0b00000111;
// 				/* reg is the destination */
// 				*src |= gp_registers[REG_PACK(index, rm_mode)];
// 				*src |= gp_registers[REG_PACK(base, rm_mode)];
// 				*dst |= gp_registers[REG_PACK(reg, reg_mode)];
// 			}
// 			else if (rm == 0b101) /* displacement only addressing [rip + disp] */
// 			{
// 				/* reg is the destination */
// 				*src |= RIP;
// 				*dst |= gp_registers[REG_PACK(reg, reg_mode)];
// 			}
// 			else /* indirect register addressing */
// 			{
// 				/* r/m is dereferenced */
// 				*src |= direction ? gp_registers[REG_PACK(rm, rm_mode)] : gp_registers[REG_PACK(reg, reg_mode)];
// 				*dst |= direction ? gp_registers[REG_PACK(reg, reg_mode)] : gp_registers[REG_PACK(rm, rm_mode)];
// 			}
// 			*src |= direction ? MEMORY : 0;
// 			*dst |= direction ? 0 : MEMORY;
// 		}
// 		else if (mod == 0b01 || mod == 0b10) /* indirect register addressing with 1 or 4 byte signed displacement */
// 		{
// 			if (rm == 0b100) /* SIB with displacement */
// 			{
// 				if (!codelen--) return ; /* error if instruction is too long */
// 				uint8_t		sib   = *p++;
// 				uint8_t		index = (sib & 0b00111000) >> 3;
// 				uint8_t		base  =  sib & 0b00000111;
// 				/* reg is the destination */
// 				*src |= gp_registers[REG_PACK(index, rm_mode)];
// 				*src |= gp_registers[REG_PACK(base, rm_mode)];
// 				*dst |= gp_registers[REG_PACK(reg, reg_mode)];
// 			}
// 			else /* indirect register with displacement */
// 			{
// 				/* r/m is dereferenced */
// 				*src |= direction ? gp_registers[REG_PACK(rm, rm_mode)] : gp_registers[REG_PACK(reg, reg_mode)];
// 				*dst |= direction ? gp_registers[REG_PACK(reg, reg_mode)] : gp_registers[REG_PACK(rm, rm_mode)];
// 			}
// 			*src |= direction ? MEMORY : 0;
// 			*dst |= direction ? 0 : MEMORY;
// 		}
// 	}
// 	if (i.status & KEEP_DST) {*src |= *dst & 0xffff;} /* get register(s) from dst if have to add to permutation check */
// 	if (i.status & KEEP_SRC) {*dst |= *src & 0xffff;} /* get register(s) from src if have to add to permutation check */
// 	*src |= i.src.flags; /* get remaining flags */
// 	*dst |= i.dst.flags; /* get remaining flags */
// }
//
// size_t		disasm(const void *code, size_t codelen,
// 			struct s_instruction *buf, size_t buflen)
// {
// 	void			*p_code = (void*)code;
// 	struct s_instruction	*p_buf  = buf;
// 	size_t			instruction_length = 0;
//
// 	while (codelen && buflen)
// 	{
// 		instruction_length = disasm_length(p_code, codelen);
// 		if (instruction_length == 0) break ;
//
// 		disasm_instruction(p_code, codelen, &p_buf->src, &p_buf->dst);
// 		p_buf->addr   = p_code;
// 		p_buf->length = instruction_length;
//
// 		p_code  += instruction_length;
// 		codelen -= instruction_length;
// 		p_buf   += 1;
// 		buflen  -= 1;
// 	}
// 	return p_buf - buf; /* number of instructions successfully disassembled */
// }
