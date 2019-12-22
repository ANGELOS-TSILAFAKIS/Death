/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   accessors.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agrumbac <agrumbac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/05 06:32:25 by agrumbac          #+#    #+#             */
/*   Updated: 2019/12/22 23:36:30 by anselme          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/mman.h>
#include <sys/stat.h>

#include "accessors.h"
#include "syscall.h"
#include "errors.h"
#include "compiler_utils.h"

/*
** safe()
** returns a safe pointer
** returns NULL if requested memory is out of range
*/

#define VIRUS_SIZE	((uint64_t)start - (uint64_t)loader_entry)

__warn_unused_result
void			*safe(size_t offset, size_t size, struct safe_ptr ref)
{
	if (offset + size > ref.size || offset + size < offset)
		return (NULL);
	return (ref.ptr + offset);
}

bool			free_accessor(struct safe_ptr *ref)
{
	if (ref->ptr)
	{
		if (sys_munmap(ref->ptr, ref->size))
			return errors(ERR_SYS, _ERR_MUNMAP_FAILED);
	}
	return true;
}

__warn_unused_result
bool			init_original_safe(struct safe_ptr *accessor, const char *filename)
{
	void		*ptr;
	struct stat	buf;
	int		fd = sys_open(filename, O_RDONLY);

	if (fd < 0)
		return errors(ERR_SYS, _ERR_OPEN_FAILED);
	if (sys_fstat(fd, &buf) < 0)
		{sys_close(fd); return errors(ERR_SYS, _ERR_FSTAT_FAILED);}
	if (buf.st_mode & S_IFDIR)
		{sys_close(fd); return errors(ERR_USAGE, _ERR_NOT_A_FILE);}
	if ((ptr = sys_mmap(0, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		{sys_close(fd); errors(ERR_SYS, _ERR_MMAP_FAILED);}
	if (sys_close(fd))
		return errors(ERR_SYS, _ERR_CLOSE_FAILED);

	accessor->ptr  = ptr;
	accessor->size = buf.st_size;
	return true;
}

__warn_unused_result
bool			init_clone_safe(struct safe_ptr *accessor, const size_t original_filesize)
{
	// TODO check if enough!
	// magic number was: 131072??????
	accessor->size = original_filesize + VIRUS_SIZE + PAGE_ALIGNMENT;
	accessor->ptr  = sys_mmap(0, accessor->size, \
		PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (accessor->ptr == MAP_FAILED)
		return errors(ERR_SYS, _ERR_MMAP_FAILED);

	return true;
}

__warn_unused_result
bool			write_file(const struct safe_ptr accessor, \
				const char *filename)
{
	int	fd = sys_open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);

	if (fd < 0) return errors(ERR_SYS, _ERR_OPEN_FAILED);

	if (sys_write(fd, accessor.ptr, accessor.size) == -1)
	{
		sys_close(fd);
		return errors(ERR_SYS, _ERR_CLOSE_FAILED);
	}

	sys_close(fd);
	return true;
}
