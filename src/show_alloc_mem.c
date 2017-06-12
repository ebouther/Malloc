/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   show_alloc_mem.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/12 12:06:56 by ebouther          #+#    #+#             */
/*   Updated: 2017/06/12 12:31:27 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

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

void				show_alloc_mem(void)
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
