/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   loader.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/04 03:38:38 by agrumbac          #+#    #+#             */
/*   Updated: 2019/12/22 20:40:21 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FAMINE_H // TODO
# define FAMINE_H

# include <limits.h> // TODO
# include <linux/elf.h>
# include <stdbool.h>
# include <stdint.h>
# include <stddef.h>
# include <sys/types.h>

struct virus_header
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

/*
** Cypher and decypher
*/

void		cypher(char *data, size_t size);
void		decypher(char *data, size_t size);

void		cypher_end(void);
void		decypher_end(void);

#endif
