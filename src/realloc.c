/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <ebouther@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 00:46:09 by ebouther          #+#    #+#             */
/*   Updated: 2017/05/12 06:36:10 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

static int try_resize(t_block *block, size_t new_size, size_t zone_remaining)
{
	t_block *tmp;
	size_t size_available;

	tmp = block;
	size_available = block->size;
	while (tmp->next)
	{
		if (tmp->next->freed)
			size_available += tmp->next->size;
		else
			return (0);
		if (size_available >= new_size)
		{
			printf("REALLOC: ENOUGH FREED BLOCKS TO RESIZE\n");
			block->size = new_size;
			if (size_available > new_size)
			{
				printf("REALLOC: RUSTINE BLK SIZE: %zu\n", size_available - new_size);
				tmp = tmp->next->next;
				block->next = new_list(sizeof(t_block));
				*(block->next) = (t_block){.next = tmp,
										 .size = size_available - new_size,
										 .freed = TRUE,
										 .addr = block->addr + new_size};
			}
			else
				block->next = tmp->next->next;
			return (1);
		}
		tmp = tmp->next;
	}
	printf("REALLOC: PTR IS THE LAST BLOCK CREATED IN ZONE OR BLKS ARE FREED TILL THE LAST ONE\n");
	if (zone_remaining + size_available >= new_size)
	{
		printf("REALLOC: RESIZE BLK AS LAST ONE \n");
		block->size = new_size;
		block->next = NULL;
		return (1);
	}
	return (0);
}

static void	*parse_blocks(void *ptr, t_block **blocks, size_t new_size, size_t zone_remaining)
{
	t_block	*block;
	t_block	*tmp;

	block = *blocks;
	tmp = NULL;

	while (block)
	{
		//printf("ADDR : %x\n", block);
		//printf("PTR : %x\n", ptr);
		if (block->addr == ptr)
		{
			if (DEBUG)
				printf("%sREALLOC: BLOCK FOUND.%s\n",
					DEBUG_COLOR, NO_COLOR);
			if (try_resize(block, new_size, zone_remaining))
				return (block->addr);
			else
			{
				if (DEBUG)
					printf("%sREALLOC: CANNOT RESIZE, ALLOC NEW.%s\n",
						DEBUG_COLOR, NO_COLOR);
				free(ptr);
				return (malloc(new_size));
			}
		}
		tmp = block;
		block = block->next;
	}
	return (NULL);
}


static void	*parse_zone(void *ptr, t_zone *zone, size_t zone_size, size_t new_size)
{
	void	*ret;

	while (zone)
	{
		if (((unsigned int)zone->memory <= (unsigned int)ptr)
			&& ((unsigned int)ptr < (unsigned int)zone->memory + zone_size))
		{

			if (zone->remaining == 0 && zone->freed_blks_nb == 0)
			{
				if (DEBUG)
					printf("%sREALLOC: NO SPACE LEFT IN ZONE, ALLOC NEW.%s\n",
						DEBUG_COLOR, NO_COLOR);
				free(ptr);
				return (malloc(new_size));
			}
			if ((ret = parse_blocks(ptr, &zone->blocks, new_size, zone->remaining)) != NULL)
				return (ret);
		}
		zone = zone->next;
	}
	return (NULL);
}

void	*realloc(void *ptr, size_t size)
{
	int		page_size;
	void	*ret;

	page_size = getpagesize();
	//printf("PARSE TINY\n");
	if ((ret = parse_zone(ptr, g_zones.tiny,
			(size_t)(MAX_TINY * MAX_PER_ZONE * page_size), size)) != NULL)
		return (ret);

	//printf("PARSE SMALL\n");
	if ((ret = parse_zone(ptr, g_zones.small,
			(size_t)(MAX_SMALL * MAX_PER_ZONE * page_size), size)) != NULL)
		return (ret);

	//printf("PARSE LARGE\n");
	//if (parse_blocks(ptr, &g_zones.large) == )
	//	return ;
	return (NULL);
}
