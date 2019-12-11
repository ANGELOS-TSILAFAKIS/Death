/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   metamorph_self.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anselme <anselme@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/11 01:29:20 by anselme           #+#    #+#             */
/*   Updated: 2019/12/11 21:45:37 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "polymorphism.h"

bool		metamorph_self(uint64_t seed)
{
	size_t function_size = (size_t)cypher_end - (size_t)cypher;

	return generate_cypher((void *)cypher, seed, function_size) &&
		generate_decypher((void *)decypher, seed, function_size);
}
