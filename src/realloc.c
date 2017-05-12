/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <ebouther@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 00:46:09 by ebouther          #+#    #+#             */
/*   Updated: 2017/05/12 01:20:28 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

/*
**	Check if zone blocks contain the pointer.
**  Returns the size of the block freed
*/

//static size_t	parse_blocks(void *ptr, t_zone *zone)
//{
//	t_block	*block;
//	t_block	*tmp;
//
//	block = zone->blocks;
//	tmp = NULL;
//	
//	while (block)
//	{
//		if (block->addr == ptr)
//		{
//			printf("REALLOC PTR FOUND");
//		}
//		tmp = block;
//		block = block->next;
//	}
//	return (0);
//}
//
///*
//** For each zone search ptr in its blocks.
//** Unmap zone its blocks were all freed.
//*/
//
//int	parse_zone(void *ptr, size_t new_size, t_zone *zone, size_t zone_size)
//{
//	int ret;
//
//	while (zone)
//	{
//		if (((unsigned int)zone->memory <= (unsigned int)ptr)
//			&& ((unsigned int)ptr < (unsigned int)zone->memory + zone_size))
//		{
//			if ((ret = parse_blocks(ptr, zone)) == -1)
//				return (-1);
//		}
//		zone = zone->next;
//	}
//	return (0);
//}

void	*realloc(void *ptr, size_t size)
{
	free(ptr);
	return (malloc(size));
	//int ret;

	//if ((ret = parse_zone(ptr, g_zones.tiny, size)) != NULL)
	//	return (ret);
	//if ((ret = parse_zone(ptr, g_zones.small, size)) != NULL)
	//	return (ret);
	//// LARGE MISSING
	//return (NULL);
}
