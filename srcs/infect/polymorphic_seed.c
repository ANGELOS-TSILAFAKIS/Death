/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   polymorphic_seed.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anselme <anselme@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/12 18:29:06 by anselme           #+#    #+#             */
/*   Updated: 2019/12/13 22:29:33 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdbool.h>
#include <stdint.h>

/*
** yield_seed_to_heir is the seed inheritance manager.
**   It SHOULD be the only function WITH WRITE ACCESS to the father seeds
**   AND the son seed! If this virus was in Rust...
**
**   uint64_t father_seed[2] MUST be a pointer to the viral code.
**   TODO: It is then changed directly in the file
**
**   This function never fails. If a virus runs out of seeds the son will get
**   already owned seeds.
*/
bool	yield_seed_to_heir(uint64_t father_seed[2], uint64_t son_seed[2])
{
	uint64_t	range_length = father_seed[1] - father_seed[0];

	son_seed[1]    = father_seed[1];
	son_seed[0]    = father_seed[0] + range_length / 2 + 1;
	father_seed[1] = father_seed[0] + range_length / 2;

	return true;
}
