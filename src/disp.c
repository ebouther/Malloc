/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   disp.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/09 18:44:23 by ebouther          #+#    #+#             */
/*   Updated: 2017/06/10 18:05:06 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

void	disp_large_zone(void *blk_addr)
{
	ft_putstr("LARGE : 0x");
	ft_putstr(ft_lltoa_base((long long)blk_addr, HEX));
	ft_putstr("\n");
}

void	disp_blk(void *blk_addr, size_t blk_size, char freed)
{
	if (freed)
		ft_putstr(DEBUG_COLOR);
	ft_putstr("0x");
	ft_putstr(ft_lltoa_base((long long)blk_addr, HEX));
	ft_putstr(" - 0x");
	ft_putstr(ft_lltoa_base((long long)blk_addr + blk_size, HEX));
	ft_putstr(" : ");
	ft_putstr(ft_lltoa_base(blk_size, DEC));
	if (freed)
	{
		ft_putstr(" octets [FREED]\n");
		ft_putstr(NO_COLOR);
	}
	else
		ft_putstr(" octets\n");
}

void	disp_use_old_blk(void *blk_addr, size_t freed_blks_nb, char split)
{
	ft_putstr(DEBUG_COLOR);
	if (split)
		ft_putstr("DIVIDE AND USE OLD BLOCK AT ADDR : ");
	else
		ft_putstr("USE OLD BLOCK AT ADDR : ");
	ft_putstr(ft_lltoa_base((long long)blk_addr, HEX));
	ft_putstr("\nFreed blks left in zone :");
	ft_putstr(ft_lltoa_base((long long)freed_blks_nb, DEC));
	ft_putstr(NO_COLOR);
	ft_putstr("\n");
}
