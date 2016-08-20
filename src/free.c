/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/08/20 18:10:42 by ebouther          #+#    #+#             */
/*   Updated: 2016/08/20 20:00:20 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

int	parse_blocks(void *ptr, t_block **blocks)
{
	t_block	*block;
	t_block	*tmp;

	block = *blocks;
	tmp = NULL;
	if (block && block->addr == ptr)
	{
		if (munmap(block->addr, block->size) == -1)
			return (-1);
		tmp = block->next;
		if (munmap(block, sizeof(t_block)) == -1)
			return (-1);
		*blocks = tmp;
		return (1);
	}
	while (block)
	{
		printf("ADDR : '%d' | PTR: '%d' \n", (unsigned int)block->addr, (unsigned int)ptr);
		if (block->addr == ptr)
		{
			if (munmap(block->addr, block->size) == -1)
				return (-1);
			tmp->next = block->next;
			if (munmap(block, sizeof(t_block)) == -1)
				return (-1);
			return (1);
		}
		tmp = block;
		block = block->next;
	}
	return (0);
}

int	parse_zone(void *ptr, t_zone *zone, size_t zone_size)
{
	while (zone)
	{
		printf("ZONE\n");
		if ((unsigned int)zone->memory <= (unsigned int)ptr
			&& (unsigned int)ptr < (unsigned int)zone->memory + zone_size)
		{
			printf("PARSE_BLK\n");
			if (parse_blocks(ptr, &zone->blocks) == -1)
				return (-1);
		}
		zone = zone->next;
	}
	return (0);
}

void	free(void *ptr)
{
	int	page_size;

	page_size = getpagesize();
	if (parse_zone(ptr, g_zones.tiny,
		(size_t)(MAX_TINY * MAX_PER_ZONE * page_size)) == -1)
		return ;
	if (parse_zone(ptr, g_zones.small,
		(size_t)(MAX_SMALL * MAX_PER_ZONE * page_size)) == -1)
		return ;
	if (parse_blocks(ptr, &g_zones.large) == -1)
		return ;
}
