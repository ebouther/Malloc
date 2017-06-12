/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/12 15:08:02 by ebouther          #+#    #+#             */
/*   Updated: 2017/06/12 15:08:04 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/08/20 18:10:42 by ebouther          #+#    #+#             */
/*   Updated: 2017/06/12 14:51:18 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

/*
**	Check if zone blocks contain the pointer.
**  Returns the size of the block freed
*/

static int	parse_blocks(void *ptr, t_block **blocks)
{
	t_block	*block;
	t_block	*tmp;

	block = *blocks;
	tmp = NULL;
	while (block)
	{
		if (block->addr == ptr)
		{
			block->freed = TRUE;
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

static int	parse_zone(void *ptr, t_zone *zone, size_t zone_size)
{
	int ret;

	while (zone)
	{
		if (((unsigned int)zone->memory <= (unsigned int)ptr)
				&& ((unsigned int)ptr < (unsigned int)zone->memory + zone_size))
		{
			if ((ret = parse_blocks(ptr, &zone->blocks)) == -1)
				return (-1);
			else if (ret > 0)
			{
				zone->freed_blks_nb += 1;
				return (1);
			}
		}
		zone = zone->next;
	}
	return (0);
}

void		free(void *ptr)
{
	static pthread_mutex_t	mutex;
	int						page_size;
	int						ret;

	mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&mutex);
	page_size = getpagesize();
	if ((ret = parse_zone(ptr, g_zones.tiny,
					(size_t)(MAX_TINY * MAX_PER_ZONE * page_size))) == -1
			|| ret == 1)
		return ;
	if ((ret = parse_zone(ptr, g_zones.small,
					(size_t)(MAX_SMALL * MAX_PER_ZONE * page_size))) == -1
			|| ret == 1)
		return ;
	if (parse_blocks(ptr, &g_zones.large) == -1)
		return ;
	pthread_mutex_unlock(&mutex);
}
