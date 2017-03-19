/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/08/20 18:10:42 by ebouther          #+#    #+#             */
/*   Updated: 2017/03/19 20:11:03 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"


int check_if_unmapable(t_block **blocks)
{
	t_block	*block;

	block = *blocks;
	while (block)
	{
		if (block->freed == FALSE)
			return (0);
	}
	return (1);
}

/*
**	Check if zone blocks contain the pointer.
**  Returns the size of the block freed
*/

int	parse_blocks(void *ptr, t_block **blocks)
{
	t_block	*block;
	t_block	*tmp;

	block = *blocks;
	tmp = NULL;
	//if (block && block->addr == ptr)
	//{
	//	printf("YUP 1 \n");
	//	//if (munmap(block->addr, block->size) == -1)
	//	//	return (-1);
	//	block->freed = TRUE;
	//	tmp = block->next;
	//	//if (munmap(block, sizeof(t_block)) == -1)
	//	//	return (-1);
	//	*blocks = tmp;
	//	return (1);
	//}
	while (block)
	{
		printf("ADDR : '%d' | PTR: '%d' \n", (unsigned int)block->addr, (unsigned int)ptr);
		if (block->addr == ptr)
		{
			printf("PTR FOUND \n");
			//if (munmap(block->addr, block->size) == -1)
			//	return (1);
			block->freed = TRUE;
			tmp->next = block->next;
			//if (munmap(block, sizeof(t_block)) == -1)
			//	return (-1);
			return ((int)block->size);
		}
		tmp = block;
		block = block->next;
	}
	return (0);
}

/*
** For each zone search ptr in its blocks.
** Unmap zone its blocks were all freed.
*/

int	parse_zone(void *ptr, t_zone *zone, size_t zone_size)
{
	int ret;

	while (zone)
	{
		printf("ZONE\n");
		if ((unsigned int)zone->memory <= (unsigned int)ptr
			&& (unsigned int)ptr < (unsigned int)zone->memory + zone_size)
		{
			printf("PARSE_BLK\n");
			if ((ret = parse_blocks(ptr, &zone->blocks)) == -1)
				return (-1);
			else if (ret > 0)
			{
				if (zone->remaining <= 0)
					check_if_unmapable(&zone->blocks);
				//zone->remaining -= ret;
				return (1);
			}
		}
		zone = zone->next;
	}
	return (0);
}

void	free(void *ptr)
{
	int	page_size;
	int	ret;

	page_size = getpagesize();
	printf("TINY: '%p' | SMALL : '%p' \n", g_zones.tiny, g_zones.small);
	printf("PARSE TINY\n");
	if ((ret = parse_zone(ptr, g_zones.tiny,
			(size_t)(MAX_TINY * MAX_PER_ZONE * page_size))) == -1
		|| ret == 1)
		return ;

	printf("PARSE SMALL\n");
	if ((ret = parse_zone(ptr, g_zones.small,
			(size_t)(MAX_SMALL * MAX_PER_ZONE * page_size))) == -1
		|| ret == 1)
		return ;

	printf("PARSE LARGE\n");
	if (parse_blocks(ptr, &g_zones.large) == -1)
		return ;
}