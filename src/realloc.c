/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <ebouther@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 00:46:09 by ebouther          #+#    #+#             */
/*   Updated: 2017/06/12 15:07:22 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

static int	resize_blk(size_t size_available,
						size_t new_size,
						t_block *block,
						t_block *tmp)
{
	block->size = new_size;
	if (size_available > new_size)
	{
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

static int	try_resize(t_block *block, size_t new_size, size_t zone_remaining)
{
	t_block	*tmp;
	size_t	size_available;

	tmp = block;
	size_available = block->size;
	while (tmp->next)
	{
		if (tmp->next->freed)
			size_available += tmp->next->size;
		else
			return (0);
		if (size_available >= new_size)
			return (resize_blk(size_available, new_size, block, tmp) || 1);
		tmp = tmp->next;
	}
	if (zone_remaining + size_available >= new_size)
	{
		block->size = new_size;
		block->next = NULL;
		return (1);
	}
	return (0);
}

static void	*parse_blocks(void *ptr,
		t_block **blocks, size_t new_size, size_t zone_remaining)
{
	t_block	*block;
	t_block	*tmp;
	void	*new_ptr;

	block = *blocks;
	tmp = NULL;
	while (block)
	{
		if (block->addr == ptr)
		{
			if (try_resize(block, new_size, zone_remaining))
				return (block->addr);
			else
			{
				new_ptr = malloc(new_size);
				memcpy(new_ptr, ptr, block->size);
				free(ptr);
				return (new_ptr);
			}
		}
		tmp = block;
		block = block->next;
	}
	return (NULL);
}

static void	*parse_zone(void *ptr,
		t_zone *zone, size_t zone_size, size_t new_size)
{
	void	*ret;

	while (zone)
	{
		if (((unsigned int)zone->memory <= (unsigned int)ptr)
			&& ((unsigned int)ptr < (unsigned int)zone->memory + zone_size))
		{
			if ((ret = parse_blocks(ptr,
							&zone->blocks, new_size, zone->remaining)) != NULL)
				return (ret);
		}
		zone = zone->next;
	}
	return (NULL);
}

void		*realloc(void *ptr, size_t size)
{
	static pthread_mutex_t	mutex;
	int						page_size;
	void					*ret;

	mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&mutex);
	if (ptr == NULL)
		return (malloc(size));
	page_size = getpagesize();
	if ((ret = parse_zone(ptr, g_zones.tiny,
			(size_t)(MAX_TINY * MAX_PER_ZONE * page_size), size)) != NULL)
		return (ret);
	if ((ret = parse_zone(ptr, g_zones.small,
			(size_t)(MAX_SMALL * MAX_PER_ZONE * page_size), size)) != NULL)
		return (ret);
	if ((ret = parse_blocks(ptr, &g_zones.large, size, 0)) != NULL)
		return (ret);
	pthread_mutex_unlock(&mutex);
	return (NULL);
}
