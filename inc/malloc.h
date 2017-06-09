/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <ebouther@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/05/01 17:50:35 by ebouther          #+#    #+#             */
/*   Updated: 2017/06/09 18:08:54 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MALLOC_H
# define MALLOC_H

# include <stdlib.h>
# include <string.h>
# include <sys/mman.h>
# include <sys/resource.h>
# include <unistd.h>

# define DEBUG 1
# define INFO 1

# define HEX "0123456789ABCDEF"
# define DEC "0123456789"

# define NO_COLOR "\033[0m"
# define DEBUG_COLOR "\033[32m"
# define INFO_COLOR "\033[36m"

# define MAX_TINY 992
# define MAX_SMALL 126999
# define MAX_PER_ZONE 100

# define S_TINY MAX_TINY * MAX_PER_ZONE
# define S_SMALL MAX_SMALL * MAX_PER_ZONE

# define TRUE 1
# define FALSE 0

void					free(void *ptr);
void					*malloc(size_t size);
void					*realloc(void *ptr, size_t size);
void					show_alloc_mem();

void					*new_list(size_t size);
char					*ft_lltoa_base(long long n, char *base);
size_t					ft_strlen(const char *str);

typedef char			t_bool;

enum					e_zones
{
	TINY = 1,
	SMALL = MAX_TINY + 1,
	LARGE = MAX_SMALL + 1
};

/*
** @size could be removed for memory optimization
*/
typedef struct			s_block
{
	void				*addr;
	size_t				size;
	t_bool				freed;
	struct s_block		*next;
}						t_block;

typedef struct			s_zone
{
	void				*memory;
	size_t				remaining;
	size_t				freed_blks_nb;
	t_block				*blocks;
	struct s_zone		*next;
}						t_zone;

typedef struct			s_lst_mem
{
	void				*ptr;
	size_t				len;
	size_t				offset;
}						t_lst_mem;

typedef struct			s_malloc_zones
{
	t_zone				*tiny;
	t_zone				*small;
	t_block				*large;
	t_lst_mem			lst_mem;
}						t_malloc_zones;

extern t_malloc_zones	g_zones;

#endif
