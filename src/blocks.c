/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   blocks.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/12 12:05:10 by ebouther          #+#    #+#             */
/*   Updated: 2017/06/12 12:27:13 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

void		*new_list(size_t size)
{
	if (g_zones.lst_mem.offset + size >= g_zones.lst_mem.len)
	{
		g_zones.lst_mem.len = getpagesize();
		g_zones.lst_mem.offset = size;
		if ((g_zones.lst_mem.ptr = mmap(NULL, g_zones.lst_mem.len,
						PROT_READ | PROT_WRITE,
						MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return (NULL);
		return (g_zones.lst_mem.ptr);
	}
	else
	{
		g_zones.lst_mem.offset += size;
		return (g_zones.lst_mem.ptr + g_zones.lst_mem.offset);
	}
}

static void	*use_freed_block(t_zone *zone, t_block *blk, size_t alloc_size)
{
	t_block *tmp;

	blk->freed = FALSE;
	if (blk->size == alloc_size)
	{
		zone->freed_blks_nb -= 1;
		if (DEBUG)
			disp_use_old_blk(blk->addr, zone->freed_blks_nb, 0);
		return (blk->addr);
	}
	else
	{
		if (DEBUG)
			disp_use_old_blk(blk->addr, zone->freed_blks_nb, 1);
		tmp = blk->next;
		blk->next = new_list(sizeof(t_block));
		*(blk->next) = (t_block){.next = tmp,
			.size = blk->size - alloc_size,
			.freed = TRUE,
			.addr = blk->addr + alloc_size};
		blk->size = alloc_size;
		return (blk->addr);
	}
}

/*
**  If there's enough memory (@alloc_size) in this @zone
**  check if it can be saved in an existing or a new block.
*/

void		*check_for_blocks(t_zone *zone, size_t alloc_size)
{
	t_block	*blk;

	if (zone->remaining >= alloc_size || zone->freed_blks_nb > 0)
	{
		if ((blk = zone->blocks) == NULL)
			return (ft_putstr("ERROR in check_for_blocks\n") ? NULL : NULL);
		while (blk->next != NULL)
		{
			if (zone->freed_blks_nb > 0
					&& blk->freed == TRUE
					&& blk->size >= alloc_size)
				return (use_freed_block(zone, blk, alloc_size));
			blk = blk->next;
		}
		blk->next = new_list(sizeof(t_block));
		*(blk->next) = (t_block){.next = NULL,
			.size = alloc_size,
			.freed = FALSE,
			.addr = blk->addr + blk->size};
		zone->remaining -= alloc_size;
		return (blk->next->addr);
	}
	return (NULL);
}

int			new_zone(t_zone **z, size_t max_size, size_t alloc_size)
{
	t_zone	*zone;

	zone = new_list(sizeof(t_zone));
	*zone = (t_zone){.remaining = max_size - alloc_size,
		.freed_blks_nb = 0,
		.next = NULL};
	if ((zone->memory = mmap(NULL, max_size * getpagesize(),
					PROT_READ | PROT_WRITE,
					MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		return (-1);
	zone->blocks = new_list(sizeof(t_zone));
	*(zone->blocks) = (t_block){.next = NULL,
		.size = alloc_size,
		.freed = FALSE,
		.addr = zone->memory};
	*z = zone;
	return (0);
}
