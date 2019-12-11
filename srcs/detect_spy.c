/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   detect_spy.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anselme <anselme@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/10 16:14:06 by anselme           #+#    #+#             */
/*   Updated: 2019/12/10 16:14:08 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdbool.h>
#include <stddef.h>
#include "syscall.h"
#include "utils.h"

bool		detect_spy(void)
{
	return (famine_ptrace(PTRACE_TRACEME, 0, NULL, 0) == -1);
}
