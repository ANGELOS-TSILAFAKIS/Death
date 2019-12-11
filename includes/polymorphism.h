/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   polymorphism.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anselme <anselme@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/11 20:01:16 by anselme           #+#    #+#             */
/*   Updated: 2019/12/11 20:49:40 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POLYMORPHISM_H
# define POLYMORPHISM_H

# include <stdbool.h>
# include <stdint.h>
# include <stddef.h>

# include "famine.h"

bool		generate_cypher(char *buffer, uint64_t seed, size_t size);
bool		generate_decypher(char *buffer, uint64_t seed, size_t size);

#endif
