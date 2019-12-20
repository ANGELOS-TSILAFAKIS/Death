/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anselme <anselme@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/20 22:43:34 by anselme           #+#    #+#             */
/*   Updated: 2019/12/20 23:17:21 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOG_H
# define LOG_H

# ifdef DEBUG

void	log_try_infecting(const char *file);
void	log_success(void);
void	log_old_father_seeds(const uint64_t father_seed[2]);
void	log_all_seeds(const uint64_t father_seed[2], const uint64_t son_seed[2], uint64_t client_id, uint64_t unique_seed);

# else
#  define log_try_infecting(...)
#  define log_success(...)
#  define log_old_father_seeds(...)
#  define log_all_seeds(...)
# endif

#endif
