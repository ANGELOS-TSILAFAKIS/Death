
#include "disasm.h"

size_t		disasm_length(const void *code, size_t codelen)
{
	uint32_t	table_opcode_modrm_ext[TABLESIZE];
	table_opcode_modrm_ext[0]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[1]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[2]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[3]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[4]        = BITMASK32(1,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[5]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_ext[6]        = BITMASK32(1,1,0,0,0,0,1,1, 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,0, 1,1,1,1,1,1,1,1);
	table_opcode_modrm_ext[7]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,1,1, 0,0,0,0,0,0,1,1);
	uint32_t	table_opcode_modrm_noext[TABLESIZE];
	table_opcode_modrm_noext[0]      = BITMASK32(1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0);
	table_opcode_modrm_noext[1]      = BITMASK32(1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0, 1,1,1,1,0,0,0,0);
	table_opcode_modrm_noext[2]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_noext[3]      = BITMASK32(0,0,1,1,0,0,0,0, 0,1,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_noext[4]      = BITMASK32(0,0,0,0,1,1,1,1, 1,1,1,1,1,1,1,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_noext[5]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_noext[6]      = BITMASK32(0,0,0,0,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_modrm_noext[7]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_opcode_imm8[TABLESIZE];
	table_opcode_imm8[0]             = BITMASK32(0,0,0,0,1,0,0,0, 0,0,0,0,1,0,0,0, 0,0,0,0,1,0,0,0, 0,0,0,0,1,0,0,0);
	table_opcode_imm8[1]             = BITMASK32(0,0,0,0,1,0,0,0, 0,0,0,0,1,0,0,0, 0,0,0,0,1,0,0,0, 0,0,0,0,1,0,0,0);
	table_opcode_imm8[2]             = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm8[3]             = BITMASK32(0,0,0,0,0,0,0,0, 0,0,1,1,0,0,0,0, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1);
	table_opcode_imm8[4]             = BITMASK32(1,0,1,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm8[5]             = BITMASK32(0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0);
	table_opcode_imm8[6]             = BITMASK32(1,1,0,0,0,0,1,0, 1,0,0,0,0,1,0,0, 0,0,0,0,1,1,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm8[7]             = BITMASK32(1,1,1,1,1,1,1,1, 0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_opcode_imm16[TABLESIZE];
	table_opcode_imm16[0]            = BITMASK32(0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0);
	table_opcode_imm16[1]            = BITMASK32(0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0);
	table_opcode_imm16[2]            = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm16[3]            = BITMASK32(0,0,0,0,0,0,0,0, 1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm16[4]            = BITMASK32(0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm16[5]            = BITMASK32(0,0,0,0,0,0,0,0, 0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1);
	table_opcode_imm16[6]            = BITMASK32(0,0,1,0,0,0,0,1, 1,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm16[7]            = BITMASK32(0,0,0,0,0,0,0,0, 1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_opcode_imm32[TABLESIZE];
	table_opcode_imm32[0]            = BITMASK32(0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0);
	table_opcode_imm32[1]            = BITMASK32(0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0, 0,0,0,0,0,1,0,0);
	table_opcode_imm32[2]            = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm32[3]            = BITMASK32(0,0,0,0,0,0,0,0, 1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm32[4]            = BITMASK32(0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm32[5]            = BITMASK32(0,0,0,0,0,0,0,0, 0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1);
	table_opcode_imm32[6]            = BITMASK32(0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_imm32[7]            = BITMASK32(0,0,0,0,0,0,0,0, 1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_opcode_mem16_32[TABLESIZE];
	table_opcode_mem16_32[0]         = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_mem16_32[1]         = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_mem16_32[2]         = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_mem16_32[3]         = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_mem16_32[4]         = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_mem16_32[5]         = BITMASK32(1,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_mem16_32[6]         = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_opcode_mem16_32[7]         = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_0f_opcode_modrm_noext[TABLESIZE];
	table_0f_opcode_modrm_noext[0]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[1]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[2]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[3]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[4]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[5]   = BITMASK32(0,0,0,1,1,1,0,0, 0,0,0,1,1,1,0,1, 1,1,1,1,1,1,1,1, 0,0,0,1,1,1,1,1);
	table_0f_opcode_modrm_noext[6]   = BITMASK32(1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_noext[7]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_0f_opcode_modrm_ext[TABLESIZE];
	table_0f_opcode_modrm_ext[0]     = BITMASK32(1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_ext[1]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_ext[2]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_ext[3]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_ext[4]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1);
	table_0f_opcode_modrm_ext[5]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,1,0,0,0,0,0);
	table_0f_opcode_modrm_ext[6]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_modrm_ext[7]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_0f_opcode_single[TABLESIZE];
	table_0f_opcode_single[0]        = BITMASK32(0,0,0,0,0,0,1,0, 1,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_single[1]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_single[2]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_single[3]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_single[4]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_single[5]        = BITMASK32(1,1,1,0,0,0,0,0, 1,1,1,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_single[6]        = BITMASK32(0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_single[7]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_0f_opcode_imm8[TABLESIZE];
	table_0f_opcode_imm8[0]          = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm8[1]          = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm8[2]          = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm8[3]          = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm8[4]          = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm8[5]          = BITMASK32(0,0,0,0,1,0,0,0, 0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,0, 0,0,1,0,0,0,0,0);
	table_0f_opcode_imm8[6]          = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm8[7]          = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_0f_opcode_imm16_32[TABLESIZE];
	table_0f_opcode_imm16_32[0]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm16_32[1]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm16_32[2]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm16_32[3]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm16_32[4]      = BITMASK32(1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm16_32[5]      = BITMASK32(0,0,0,0,1,0,0,0, 0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,0, 0,0,1,0,0,0,0,0);
	table_0f_opcode_imm16_32[6]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_0f_opcode_imm16_32[7]      = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_9b_opcode_modrm_ext[TABLESIZE];
	table_9b_opcode_modrm_ext[0]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[1]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[2]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[3]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[4]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[5]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_9b_opcode_modrm_ext[6]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,1,0,1,0,0);
	table_9b_opcode_modrm_ext[7]     = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_f20f_opcode_modrm_noext[TABLESIZE];
	table_f20f_opcode_modrm_noext[0] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_modrm_noext[1] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,1,0,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_modrm_noext[2] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0, 1,1,1,0,1,1,1,1);
	table_f20f_opcode_modrm_noext[3] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0, 0,0,0,0,1,1,0,0);
	table_f20f_opcode_modrm_noext[4] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_modrm_noext[5] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_modrm_noext[6] = BITMASK32(0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,1,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_modrm_noext[7] = BITMASK32(0,0,0,0,0,0,1,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_f20f_opcode_imm8[TABLESIZE];
	table_f20f_opcode_imm8[0]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_imm8[1]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_imm8[2]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_imm8[3]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_imm8[4]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_imm8[5]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_imm8[6]        = BITMASK32(0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f20f_opcode_imm8[7]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_f30f_opcode_modrm_noext[TABLESIZE];
	table_f30f_opcode_modrm_noext[0] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,0,0,0,1,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_noext[1] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,1,0,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_noext[2] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,1,1,0,0,0,0, 1,1,1,1,1,1,1,1);
	table_f30f_opcode_modrm_noext[3] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,1,1);
	table_f30f_opcode_modrm_noext[4] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_noext[5] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_noext[6] = BITMASK32(0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,1,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_noext[7] = BITMASK32(0,0,0,0,0,0,1,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_f30f_opcode_modrm_ext[TABLESIZE];
	table_f30f_opcode_modrm_ext[0]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[1]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[2]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[3]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[4]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[5]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[6]   = BITMASK32(0,0,0,0,0,0,0,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_modrm_ext[7]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_f30f_opcode_imm8[TABLESIZE];
	table_f30f_opcode_imm8[0]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_imm8[1]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_imm8[2]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_imm8[3]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_imm8[4]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_imm8[5]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_imm8[6]        = BITMASK32(0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_f30f_opcode_imm8[7]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_660f_opcode_modrm_noext[TABLESIZE];
	table_660f_opcode_modrm_noext[0] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_noext[1] = BITMASK32(0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_noext[2] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,0,0,1,1,1,1, 1,1,1,1,1,1,1,1);
	table_660f_opcode_modrm_noext[3] = BITMASK32(1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,0,0,0,1,1,1,0, 0,0,0,0,1,1,1,1);
	table_660f_opcode_modrm_noext[4] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_noext[5] = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_noext[6] = BITMASK32(0,0,1,0,1,1,1,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1);
	table_660f_opcode_modrm_noext[7] = BITMASK32(1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0);
	uint32_t	table_660f_opcode_modrm_ext[TABLESIZE];
	table_660f_opcode_modrm_ext[0]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[1]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[2]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[3]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[4]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[5]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[6]   = BITMASK32(0,0,0,0,0,0,0,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_modrm_ext[7]   = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	uint32_t	table_660f_opcode_imm8[TABLESIZE];
	table_660f_opcode_imm8[0]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_imm8[1]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_imm8[2]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_imm8[3]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_imm8[4]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_imm8[5]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_imm8[6]        = BITMASK32(0,0,1,0,1,1,1,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);
	table_660f_opcode_imm8[7]        = BITMASK32(0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0);

	int8_t		defmem   = DWORD;
	int8_t		defdata  = DWORD;
	int8_t		datasize = 0;
	int8_t		memsize  = 0;
	int8_t		flags    = 0;
	int8_t		prefix   = 0;

	uint8_t		*p = (uint8_t*)code;
	uint8_t		opcode;

	codelen = codelen > INSTRUCTION_MAXLEN ? INSTRUCTION_MAXLEN : codelen;

	next_opcode:
	if (!codelen--) return EDISASM_LENGTH; /* Error if instruction is too long */
	opcode = *p++;

	/* Legacy / REX prefixes unused for disasm */
	if (opcode == 0x26 || opcode == 0x2e
	||  opcode == 0x36 || opcode == 0x3e
	||  opcode == 0x64 || opcode == 0x65
	||  opcode == 0x40 || opcode == 0x41 || opcode == 0x42 || opcode == 0x43
	||  opcode == 0x44 || opcode == 0x45 || opcode == 0x46 || opcode == 0x47
	||  opcode == 0x49 || opcode == 0x4a || opcode == 0x4b || opcode == 0x4c
	||  opcode == 0x4d || opcode == 0x4e || opcode == 0x4f
	||  opcode == 0xf0)
	{
		goto next_opcode;
	}
	/* Operand size override */
	else if (opcode == 0x66)
	{
		defdata = WORD;
		prefix |= OP_PREFIX_66;
		goto next_opcode;
	}
	/* Address size override */
	else if (opcode == 0x67)
	{
		defmem = DWORD;
		goto next_opcode;
	}
	/* Mandatory prefixes */
	else if (opcode == 0x0f)
	{
		prefix |= OP_PREFIX_0F;
		goto next_opcode;
	}
	else if (opcode == 0xf2)
	{
		prefix |= OP_PREFIX_F2;
		goto next_opcode;
	}
	else if (opcode == 0xf3)
	{
		prefix |= OP_PREFIX_F3;
		goto next_opcode;
	}
	else if (opcode == 0x9b)
	{
		prefix |= OP_PREFIX_9B;
		goto next_opcode;
	}
	/* REX.W prefix */
	else if (opcode == 0x48)
	{
		defdata = DWORD, defmem = DWORD;
		goto next_opcode;
	}


	if (prefix == OP_9B)
	{
		if (CHECK_TABLE(table_9b_opcode_modrm_ext, opcode))
			flags |= MODRM;
	}
	else if (prefix == OP_660F)
	{
		if (CHECK_TABLE(table_660f_opcode_modrm_noext, opcode)
		|| CHECK_TABLE(table_660f_opcode_modrm_ext, opcode))
			flags |= MODRM;
		if (CHECK_TABLE(table_660f_opcode_imm8, opcode))
			datasize += defdata;
	}
	else if (prefix == OP_F20F)
	{
		if (CHECK_TABLE(table_f20f_opcode_modrm_noext, opcode))
			flags |= MODRM;
		if (CHECK_TABLE(table_f20f_opcode_imm8, opcode))
			datasize += defdata;
	}
	else if (prefix == OP_F30F)
	{
		if (CHECK_TABLE(table_f30f_opcode_modrm_noext, opcode)
		|| CHECK_TABLE(table_f30f_opcode_modrm_ext, opcode))
			flags |= MODRM;
		if (CHECK_TABLE(table_f30f_opcode_imm8, opcode))
			datasize += defdata;
	}
	else if (prefix == OP_0F)
	{
		if (CHECK_TABLE(table_0f_opcode_modrm_noext, opcode)
		|| CHECK_TABLE(table_0f_opcode_modrm_ext, opcode))
			flags |= MODRM;
		if (CHECK_TABLE(table_0f_opcode_imm8, opcode))
			datasize += defdata;
		if (CHECK_TABLE(table_0f_opcode_imm16_32, opcode))
			datasize += defdata;
	}
	else
	{
		if (CHECK_TABLE(table_opcode_modrm_noext, opcode)
		||  CHECK_TABLE(table_opcode_modrm_ext, opcode))
			flags |= MODRM;
		if      (opcode == 0xf6) /* Exeption for <test> which has the same opcode as other instructions with different length */
			flags |= TEST_F6;
		else if (opcode == 0xf7)
			flags |= TEST_F7;
		if (CHECK_TABLE(table_opcode_imm8, opcode))
			datasize += BYTE;
		if (CHECK_TABLE(table_opcode_imm16, opcode)
		||  CHECK_TABLE(table_opcode_imm32, opcode))
			datasize += defdata;
		if (CHECK_TABLE(table_opcode_mem16_32, opcode))
			memsize  += defmem;
	}

	if (flags & MODRM)
	{
		if (!codelen--) return EDISASM_LENGTH; /* Error if instruction is too long */
		opcode = *p++;
		uint8_t		mod = opcode & 0b11000000;
		uint8_t		rm  = opcode & 0b00000111;
		if ((flags & TEST) && (rm == 0b00000000 || rm == 0b00000001)) /* Exeption for <test> which has the same opcode as other instructions with different length */
		{
			datasize += flags & TEST_F7 ? defdata : BYTE;
		}
		else if (mod != 0b11000000) /* If addressing mode is not register addressing */
		{
			if      (mod == 0b01000000) /* One byte signed displacement */
			{ memsize += BYTE; }
			else if (mod == 0b10000000) /* Four byte signed displacement */
			{ memsize += defmem; }
			if (defmem == WORD) /* If 16-bit mode (no SIB) */
			{
				if (mod == 0b00000000 && rm == 0b110) memsize += WORD; /* If 16-bit displacement */
			}
			else /* SIB may be present */
			{
				if (!codelen--) return EDISASM_LENGTH; /* Error if instruction is too long */
				if (rm == 0b100) rm = *p++ & 0b111; /* SIB addressing and get base register */
				if (mod == 0b00000000 && rm == 0b101) memsize += DWORD; /* 32-bit displacement mode */
			}
		}
	}

	if ((int64_t)codelen - (datasize + memsize) < 0) return EDISASM_LENGTH; /* Error if instruction is too long */
	p += datasize + memsize;
	return (void*)p - (void*)code;
}
