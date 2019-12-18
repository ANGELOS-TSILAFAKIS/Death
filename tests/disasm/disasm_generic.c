
#include "disasm.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
/*
** TODO:
** - remove most unknowns as possible
*/

static char	*_debug_get_register(uint32_t reg)
{
	#define M_RAX	(RAX | MEMORY)
	#define M_RBX	(RBX | MEMORY)
	#define M_RCX	(RCX | MEMORY)
	#define M_RDX	(RDX | MEMORY)
	#define M_RDI	(RDI | MEMORY)
	#define M_RSI	(RSI | MEMORY)
	#define M_R8	(R8  | MEMORY)
	#define M_R9	(R9  | MEMORY)
	#define M_R10	(R10 | MEMORY)
	#define M_R11	(R11 | MEMORY)
	#define M_R12	(R12 | MEMORY)
	#define M_R13	(R13 | MEMORY)
	#define M_R14	(R14 | MEMORY)
	#define M_R15	(R15 | MEMORY)
	#define M_RBP	(RBP | MEMORY)
	#define M_RSP	(RSP | MEMORY)

	#define COLOR_NONE	"\e[0m"
	#define COLOR_RED	"\e[31m"
	#define COLOR_GREEN	"\e[32m"
	#define COLOR_YELLOW	"\e[33m"

	switch (reg)
	{
		case NONE:    return COLOR_NONE"error"COLOR_NONE;
		case RAX:     return COLOR_GREEN"rax"COLOR_NONE;
		case RBX:     return COLOR_GREEN"rbx"COLOR_NONE;
		case RCX:     return COLOR_GREEN"rcx"COLOR_NONE;
		case RDX:     return COLOR_GREEN"rdx"COLOR_NONE;
		case RDI:     return COLOR_GREEN"rdi"COLOR_NONE;
		case RSI:     return COLOR_GREEN"rsi"COLOR_NONE;
		case R8:      return COLOR_GREEN"r8"COLOR_NONE;
		case R9:      return COLOR_GREEN"r9"COLOR_NONE;
		case R10:     return COLOR_GREEN"r10"COLOR_NONE;
		case R11:     return COLOR_GREEN"r11"COLOR_NONE;
		case R12:     return COLOR_GREEN"r12"COLOR_NONE;
		case R13:     return COLOR_GREEN"r13"COLOR_NONE;
		case R14:     return COLOR_GREEN"r14"COLOR_NONE;
		case R15:     return COLOR_GREEN"r15"COLOR_NONE;
		case RBP:     return COLOR_GREEN"rbp"COLOR_NONE;
		case RSP:     return COLOR_GREEN"rsp"COLOR_NONE;
		case RIP:     return COLOR_GREEN"rip"COLOR_NONE;
		case MEMORY:  return COLOR_RED"error"COLOR_NONE;
		case M_RAX:   return COLOR_GREEN"[rax]"COLOR_NONE;
		case M_RBX:   return COLOR_GREEN"[rbx]"COLOR_NONE;
		case M_RCX:   return COLOR_GREEN"[rcx]"COLOR_NONE;
		case M_RDX:   return COLOR_GREEN"[rdx]"COLOR_NONE;
		case M_RDI:   return COLOR_GREEN"[rdi]"COLOR_NONE;
		case M_RSI:   return COLOR_GREEN"[rsi]"COLOR_NONE;
		case M_R8:    return COLOR_GREEN"[r8]"COLOR_NONE;
		case M_R9:    return COLOR_GREEN"[r9]"COLOR_NONE;
		case M_R10:   return COLOR_GREEN"[r10]"COLOR_NONE;
		case M_R11:   return COLOR_GREEN"[r11]"COLOR_NONE;
		case M_R12:   return COLOR_GREEN"[r12]"COLOR_NONE;
		case M_R13:   return COLOR_GREEN"[r13]"COLOR_NONE;
		case M_R14:   return COLOR_GREEN"[r14]"COLOR_NONE;
		case M_R15:   return COLOR_GREEN"[r15]"COLOR_NONE;
		case M_RBP:   return COLOR_GREEN"[rbp]"COLOR_NONE;
		case M_RSP:   return COLOR_GREEN"[rsp]"COLOR_NONE;
		case UNKNOWN: return COLOR_YELLOW"unknown"COLOR_NONE;
		default:      return COLOR_GREEN"SIB"COLOR_NONE;
	}
}

static bool	disasm_instruction(const void *code, size_t codelen, uint32_t *src, uint32_t *dst)
{
	uint32_t	table_opcode_modrm_noext[TABLESIZE];
	table_opcode_modrm_noext[0]      = BITMASK32(1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0);
	table_opcode_modrm_noext[1]      = BITMASK32(1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0);
	table_opcode_modrm_noext[2]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_noext[3]      = BITMASK32(0,0,1,1,0,0,0,0, 0,1,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_noext[4]      = BITMASK32(0,0,0,0,1,1,1,1, 1,1,1,1,1,1,1,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_noext[5]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_noext[6]      = BITMASK32(0,0,0,0,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_noext[7]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_0f_opcode_modrm_noext[TABLESIZE];
	table_0f_opcode_modrm_noext[0]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[1]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[2]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[3]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[4]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[5]   = BITMASK32(0,0,0,1,1,1,0,0, 0,0,0,1,1,1,0,1, 1,1,1,1,1,1,1,1, 0,0,0,1,1,1,1,1);
	table_0f_opcode_modrm_noext[6]   = BITMASK32(1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[7]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_f20f_opcode_modrm_noext[TABLESIZE];
	table_f20f_opcode_modrm_noext[0] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_modrm_noext[1] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,1,0,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_modrm_noext[2] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0, 1,1,1,0,1,1,1,1);
	table_f20f_opcode_modrm_noext[3] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0, 0,0,0,0,1,1,0,0);
	table_f20f_opcode_modrm_noext[4] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_modrm_noext[5] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_modrm_noext[6] = BITMASK32(0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,1,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_modrm_noext[7] = BITMASK32(0,0,0,0,0,0,1,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_f30f_opcode_modrm_noext[TABLESIZE];
	table_f30f_opcode_modrm_noext[0] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,0,0,0,1,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_noext[1] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,1,0,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_noext[2] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,1,1,0,0,0,0, 1,1,1,1,1,1,1,1);
	table_f30f_opcode_modrm_noext[3] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,1,1);
	table_f30f_opcode_modrm_noext[4] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_noext[5] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_noext[6] = BITMASK32(0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,1,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_noext[7] = BITMASK32(0,0,0,0,0,0,1,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_660f_opcode_modrm_noext[TABLESIZE];
	table_660f_opcode_modrm_noext[0] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_noext[1] = BITMASK32(0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_noext[2] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,0,0,1,1,1,1, 1,1,1,1,1,1,1,1);
	table_660f_opcode_modrm_noext[3] = BITMASK32(1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,0,0,0,1,1,1,0, 0,0,0,0,1,1,1,1);
	table_660f_opcode_modrm_noext[4] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_noext[5] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_noext[6] = BITMASK32(0,0,1,0,1,1,1,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1);
	table_660f_opcode_modrm_noext[7] = BITMASK32(1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0);
	uint32_t		table_opcode_modrm_ext[TABLESIZE];
	table_opcode_modrm_ext[0]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[1]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[2]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[3]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[4]        = BITMASK32(1,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[5]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[6]        = BITMASK32(1,1,0,0,0,0,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,0, 1,1,1,1,1,1,1,1);
	table_opcode_modrm_ext[7]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,1,1, 0,0,0,0,0,0,1,1);
	uint32_t		table_0f_opcode_modrm_ext[TABLESIZE];
	table_0f_opcode_modrm_ext[0]     = BITMASK32(1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_ext[1]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_ext[2]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_ext[3]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_ext[4]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1);
	table_0f_opcode_modrm_ext[5]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,1,0,0,0,0,0);
	table_0f_opcode_modrm_ext[6]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_ext[7]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t		table_9b_opcode_modrm_ext[TABLESIZE];
	table_9b_opcode_modrm_ext[0]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[1]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[2]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[3]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[4]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[5]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[6]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,1,0,1,0,0);
	table_9b_opcode_modrm_ext[7]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t		table_f30f_opcode_modrm_ext[TABLESIZE];
	table_f30f_opcode_modrm_ext[0]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[1]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[2]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[3]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[4]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[5]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[6]   = BITMASK32(0,0,0,0,0,0,0,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[7]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t		table_660f_opcode_modrm_ext[TABLESIZE];
	table_660f_opcode_modrm_ext[0]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[1]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[2]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[3]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[4]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[5]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[6]   = BITMASK32(0,0,0,0,0,0,0,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[7]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);


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

	uint8_t		*p     = (uint8_t*)code;
	uint8_t		prefix = 0;
	uint8_t		rex    = 0;
	uint8_t		flags  = 0;
	uint8_t		opcode;

	*src = UNKNOWN;
	*dst = UNKNOWN;

	/* Prefix loop */
	next_opcode:
	if (!codelen--) return EDISASM_INSTRUCTION; /* Error if instruction is too long */
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

	/* Check correct opcode tables for registers usage */
	if (prefix == OP_660F)
	{
		if (CHECK_TABLE(table_660f_opcode_modrm_noext, opcode))
			flags |= MODRM;
	}
	else if (prefix == OP_F20F)
	{
		if (CHECK_TABLE(table_f20f_opcode_modrm_noext, opcode))
			flags |= MODRM;
	}
	else if (prefix == OP_F30F)
	{
		if (CHECK_TABLE(table_f30f_opcode_modrm_noext, opcode))
			flags |= MODRM;
	}
	else if (prefix == OP_0F)
	{
		if (CHECK_TABLE(table_0f_opcode_modrm_noext, opcode))
			flags |= MODRM;
	}
	else
	{
		if (CHECK_TABLE(table_opcode_modrm_noext, opcode))
			flags |= MODRM;
		else if (CHECK_TABLE(table_opcode_modrm_ext, opcode))
			flags |= EXT;
	}

	/* If posseses MOD/RM byte with register usage */
	if (flags & MODRM)
	{
		*src = 0;
		*dst = 0;
		if (!codelen--) return EDISASM_INSTRUCTION; /* Error if instruction is too long */
		uint8_t		modrm = *p++;
		uint8_t		mod   = (modrm & 0b11000000) >> 6;
		uint8_t		reg   = (modrm & 0b00111000) >> 3;
		uint8_t		rm    =  modrm & 0b00000111;

		if (!codelen--) return EDISASM_INSTRUCTION; /* Error if instruction is too long */
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
	else if (flags & EXT)
	{
		*src = 0;
		*dst = 0;
		if (!codelen--) return EDISASM_INSTRUCTION; /* Error if instruction is too long */
		uint8_t		modrm = *p++;
		uint8_t		rm    =  modrm & 0b00000111;

		uint8_t		rex_rxb  = rex & 0b00000111;
		uint8_t		rm_mode  = !!(rex_rxb & 0b001) << 3;

		/* r/m is always the "destination", reg being the opcode extension */
		/* src is often an immediate value */
		*src |= UNKNOWN;
		*dst |= gp_registers[REG_PACK(rm, rm_mode)];
	}
	return true;
}

uint64_t	disasm(const void *code, size_t codelen, struct s_instruction *buf, size_t buflen)
{
	void			*p_code = (void*)code;
	struct s_instruction	*p_buf  = (void*)buf;
	size_t			instruction_length = 0;

	while (codelen && buflen)
	{
		instruction_length = disasm_length(p_code, codelen);
		if (instruction_length == EDISASM_LENGTH)
			break ;
		if (disasm_instruction(p_code, codelen, &p_buf->src, &p_buf->dst) == EDISASM_INSTRUCTION)
			break ;

		p_buf->addr   = p_code;
		p_buf->length = instruction_length;

		p_code  += instruction_length;
		codelen -= instruction_length;
		p_buf   += 1;
		buflen  -= 1;
	}
	/* Now used for debug */
	// while (buflen) /* If exceeds codelen */
	// {
	// 	buf->addr   = NULL;
	// 	buf->length = 0;
	// 	buf->src    = NONE;
	// 	buf->dst    = NONE;
	// 	buf    += 1;
	// 	buflen -= 1;
	// }
	return p_buf - buf; /* Number of instructions successfully disassembled */
}

static void             dummy(void)
{
        asm volatile (".intel_syntax;\n"
	"add rcx, [rip+0x1]\n"
        "leave\n"
        "ret\n"
        );
        __builtin_unreachable();
}

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
	size_t			buflen = 1000;
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
		dprintf(fd, "buf[%d]: addr:%p length:%zu src:%s dst:%s\n", i, buf[i].addr, buf[i].length, _debug_get_register(buf[i].src), _debug_get_register(buf[i].dst));
	}
	if (fd != 1) close(fd);

	return 0;
}
