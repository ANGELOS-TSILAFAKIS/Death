




- remove all uneeded includes from .h move those required by .c in .c
- remove safe macro and replace with safe()
- disasm_utils clean and renames
- errors.h align
- CALL_INSTR_SIZE replace with label arithmetic
- remove ft_ prefixes remove unused functions, wrap in ifdef DEBUG those used only in debug
- Protect all syscalls CORRECTLY! WE'RE not libC wrappers!! in Linux (0 = success, negative = failure)
