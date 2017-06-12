/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <ebouther@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/05/01 17:46:41 by ebouther          #+#    #+#             */
/*   Updated: 2017/06/12 12:37:40 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

t_malloc_zones g_zones = (t_malloc_zones){NULL, NULL, NULL, {NULL, 0, 0}};

/*
**  Return the correct zone type (enum e_zones) for the allocation @size.
*/

static enum	e_zones	zone_size(size_t size)
{
	if (size >= TINY && size < SMALL)
		return (TINY);
	else if (size >= SMALL && size < LARGE)
		return (SMALL);
	else if (size > LARGE)
		return (LARGE);
	else
		return ((enum e_zones)(-1));
}

static void			*allocate_block(t_zone **z,
									size_t max_size,
									size_t alloc_size)
{
	t_zone	*zone;
	void	*addr;

	zone = *z;
	if (*z == NULL)
	{
		if (new_zone(z, max_size, alloc_size) == -1)
			return (NULL);
		return ((*z)->memory);
	}
	else
	{
		if ((addr = check_for_blocks(zone, alloc_size)) != NULL)
			return (addr);
		while (zone->next != NULL)
		{
			if ((addr = check_for_blocks(zone->next, alloc_size)) != NULL)
				return (addr);
			zone = zone->next;
		}
		if (new_zone(&zone->next, max_size, alloc_size) == -1)
			return (NULL);
		return (zone->next->memory);
	}
	return (NULL);
}

static void			*alloc_large(t_block **large, size_t size)
{
	t_block	*tmp;

	tmp = *large;
	if (*large == NULL)
	{
		*large = new_list(sizeof(t_block));
		tmp = *large;
	}
	else
	{
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = new_list(sizeof(t_block));
		tmp = tmp->next;
	}
	if ((tmp->addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
					MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		return (NULL);
	tmp->size = size;
	tmp->freed = FALSE;
	tmp->next = NULL;
	return (tmp->addr);
}

static void			*get_address(enum e_zones zone, size_t size)
{
	void	*addr;

	addr = NULL;
	if (zone == TINY)
	{
		addr = allocate_block(&g_zones.tiny,
			S_TINY % getpagesize() != 0 ?
			(size_t)(S_TINY + (getpagesize()
				- (S_TINY % getpagesize()))) : S_TINY, size);
	}
	else if (zone == SMALL)
	{
		addr = allocate_block(&g_zones.small,
			S_SMALL % getpagesize() != 0 ?
			(size_t)(S_SMALL + (getpagesize()
				- (S_SMALL % getpagesize()))) : S_SMALL, size);
	}
	else if (zone == LARGE)
	{
		addr = alloc_large(&g_zones.large, size % getpagesize() != 0 ?
			size + (getpagesize() - (size % getpagesize())) : size);
	}
	return (addr);
}

void				*malloc(size_t size)
{
	if (g_zones.lst_mem.ptr == NULL)
	{
		g_zones.lst_mem.len = getpagesize();
		g_zones.lst_mem.offset = 0;
		if ((g_zones.lst_mem.ptr = mmap(NULL, g_zones.lst_mem.len,
						PROT_READ | PROT_WRITE,
						MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return (NULL);
	}
	return (get_address(zone_size(size), size));
}
