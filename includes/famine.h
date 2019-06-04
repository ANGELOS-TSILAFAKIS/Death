/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   famine.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/04 03:38:38 by agrumbac          #+#    #+#             */
/*   Updated: 2019/06/04 04:47:00 by agrumbac         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FAMINE_H
# define FAMINE_H

# include <limits.h>
# include <linux/elf.h>

/*
** famine
*/

void		famine();
bool		elf64_identifier(void);
void		infect_files_in(const char *path);
bool		infect(const char *file);

/*
** endian management
*/

void		endian_big_mode(bool is_big_endian);
uint16_t	endian_2(uint16_t n);
uint32_t	endian_4(uint32_t n);
uint64_t	endian_8(uint64_t n);

#endif
