/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ll_itoa_base.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/02/02 14:56:35 by ebouther          #+#    #+#             */
/*   Updated: 2017/06/09 18:08:27 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

static char	*ft_toa_bis(int *limit, char *res)
{
	res[0] = '-';
	*limit = 1;
	return (res);
}

static int	ft_get_res_length(long long n, int base_length)
{
	int		i;

	if (n == 0)
		return (1);
	i = (n < 0 ? 1 : 0);
	while (n != 0)
	{
		n /= base_length;
		i++;
	}
	return (i);
}

char		*ft_lltoa_base(long long n, char *base)
{
	int		base_length;
	int		res_length;
	char	*res;
	int		i;
	int		limit;

	base_length = ft_strlen(base);
	if (base_length < 2)
		return (NULL);
	res_length = ft_get_res_length(n, base_length);
	if (!(res = (char*)malloc(sizeof(char) * (res_length + 1))))
		return (NULL);
	res[res_length] = '\0';
	limit = 0;
	if (n < 0)
		res = ft_toa_bis(&limit, res);
	i = res_length - 1;
	while (i >= limit)
	{
		res[i] = base[n % base_length];
		n /= base_length;
		i--;
	}
	return (res);
}
