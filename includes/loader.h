/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   loader.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/04 03:38:38 by agrumbac          #+#    #+#             */
/*   Updated: 2019/12/27 00:13:49 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOADER_H
# define LOADER_H

# include <stdbool.h>
# include <stdint.h>
# include <stddef.h>
# include <sys/types.h>

#define CALL_INSTR_SIZE		0x05

struct			virus_header
{
	uint64_t	seed[2];
	uint64_t	relative_pt_load_address;
	uint64_t	pt_load_size;
	uint64_t	relative_virus_address;
	uint64_t	relative_entry_address;
	uint64_t	virus_size;
}__attribute__((packed));

/*
** Anti debug
*/

bool		detect_spy(void);

/*
** Loader
*/

void		loader_entry(void);
void		mark_below(void);
void		return_to_client(void);

/*
** Cypher and decypher
*/

void		cypher(char *data, size_t size);
void		decypher(char *data, size_t size);

void		cypher_end(void);
void		decypher_end(void);

#endif
