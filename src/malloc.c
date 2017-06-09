/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <ebouther@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/05/01 17:46:41 by ebouther          #+#    #+#             */
/*   Updated: 2017/06/09 19:26:39 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

t_malloc_zones g_zones = (t_malloc_zones){NULL, NULL, NULL, {NULL, 0, 0}};

static unsigned int	parse_blocks(t_block **blocks, char is_large)
{
	t_block			*block;
	unsigned int	total;

	block = *blocks;
	total = 0;
	while (block)
	{
		if (block->freed == FALSE)
		{
			if (is_large)
			{
				disp_large_zone(block->addr);
				is_large = 0;
			}
			disp_blk(block->addr, block->size, 0);
			total += block->size;
		}
		else if (DEBUG)
			disp_blk(block->addr, block->size, 1);
		block = block->next;
	}
	return (total);
}

static unsigned int	parse_zone(const char *zone_type, t_zone *zone)
{
	unsigned int	total;

	total = 0;
	while (zone)
	{
		ft_putstr((char *)zone_type);
		ft_putstr("0x");
		ft_putstr(ft_lltoa_base((long long)zone->memory, HEX));
		ft_putstr("\n");
		total += parse_blocks(&zone->blocks, 0);
		zone = zone->next;
	}
	return (total);
}

void		show_alloc_mem(void)
{
	unsigned int	total;

	total = 0;
	total += parse_zone("TINY : ", g_zones.tiny);
	total += parse_zone("SMALL : ", g_zones.small);
	total += parse_blocks(&g_zones.large, 1);
	ft_putstr("Total : ");
	ft_putstr(ft_lltoa_base(total, DEC));
	ft_putstr(" octets\n");
}

void	*new_list(size_t size)
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

/*
**  Return the correct zone type (enum e_zones) for the allocation @size.
*/

static enum e_zones	zone_size(size_t size)
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

static void	*use_freed_block(t_zone *zone, t_block *blk, size_t alloc_size)
{
	t_block *tmp;

	blk->freed = FALSE;
	if (blk->size == alloc_size)
	{
		zone->freed_blks_nb -= 1;
		if (DEBUG)
			disp_use_old_blk(blk->addr, zone->freed_blks_nb, 0);
		//	ft_putstr(DEBUG_COLOR);
		//	ft_putstr("USE OLD BLOCK MEMORY AT ADDR : ");
		//	ft_putstr("\nFreed blks left in zone :");
		//	ft_printf("%sUSE OLD BLOCK MEMORY AT ADDR : %x\n\
		//			Freed blks left in zone : %zu%s\n",
		//			DEBUG_COLOR,
		//			(unsigned int)blk->addr,
		//			zone->freed_blks_nb,
		//			NO_COLOR);
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

static void	*check_for_blocks(t_zone *zone, size_t alloc_size)
{
	t_block	*blk;

	if (zone->remaining >= alloc_size || zone->freed_blks_nb > 0)
	{
		if ((blk = zone->blocks) == NULL)
		{
			ft_putstr("ERROR in check_for_blocks\n");
			return (NULL);
		}
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

#include <stdio.h>
static int	new_zone(t_zone **z, size_t max_size, size_t alloc_size)
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

static void	*allocate_block(t_zone **z, size_t max_size, size_t alloc_size)
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

static void	*alloc_large(t_block **large, size_t size)
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

static void	*get_address(enum e_zones zone, size_t size)
{
	void	*addr;

	addr = NULL;
	if (zone == TINY)
	{
		addr = allocate_block(&g_zones.tiny,
			S_TINY % getpagesize() != 0 ?
				(size_t)(S_TINY + (getpagesize() - (S_TINY % getpagesize()))) : S_TINY, size);
	}
	else if (zone == SMALL)
	{
		addr = allocate_block(&g_zones.small,
			S_SMALL % getpagesize() != 0 ?
			(size_t)(S_SMALL + (getpagesize() - (S_SMALL % getpagesize()))) : S_SMALL, size);
	}
	else if (zone == LARGE)
	{
		addr = alloc_large(&g_zones.large, size % getpagesize() != 0 ?
			size + (getpagesize() - (size % getpagesize())) : size);
	}
	return (addr);
}

void	*malloc(size_t size)
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
