/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <ebouther@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/05/01 17:46:41 by ebouther          #+#    #+#             */
/*   Updated: 2016/08/03 14:11:09 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

static t_malloc_zones	g_zones = (t_malloc_zones){NULL, NULL, NULL};

static enum e_zones	zone_size(size_t size)
{
	if (size >= TINY && size < SMALL)
		return (TINY);
	else if (size >= SMALL && size < LARGE)
		return (SMALL);
	else if (size > LARGE)
		return (LARGE);
	else
		return ((enum e_zones)(-1));
}

static void	*check_for_blocks(t_zone *zone, size_t alloc_size)
{
	t_block	*blk;

	if (zone->remaining >= alloc_size)
	{
		if ((blk = zone->blocks) == NULL)
			return (NULL);
		while (blk->next != NULL)
			blk = blk->next;
		if ((blk->next = mmap(NULL, sizeof(t_block), PROT_READ | PROT_WRITE,
				MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return (NULL);
		blk->next->next = NULL;
		blk->next->size = alloc_size;
		return (blk->next->addr = blk->addr + blk->size + 1);
	}
	return (NULL);
}

static int	new_zone(t_zone **z, size_t max_size, size_t alloc_size)
{
	t_zone	*zone;

	if ((zone = mmap(NULL, sizeof(t_zone), PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		return (-1);
	if ((zone->memory = mmap(NULL, max_size, PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		return (-1);
	if ((zone->blocks = mmap(NULL, sizeof(t_block), PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		return (-1);
	*(zone->blocks) = (t_block){zone->memory, alloc_size, NULL};
	zone->remaining = max_size - alloc_size;
	zone->next = NULL;
	*z = zone;
	return (0);
}

static void	*allocate_block(t_zone **z, size_t max_size, size_t alloc_size)
{
	t_zone	*zone;
	void	*addr;

	zone = *z;
	if (*z == NULL)
	{
		if (new_zone(z, max_size, alloc_size) == -1)
			return (NULL);
		return ((*z)->memory);
	}
	else
	{
		if ((addr = check_for_blocks(zone, alloc_size)) != NULL)
			return (addr);
		while (zone->next != NULL)
		{
			if ((addr = check_for_blocks(zone->next, alloc_size)) != NULL)
				return (addr);
			zone = zone->next;
		}
		if (new_zone(&zone->next, max_size, alloc_size) == -1)
			return (NULL);
		return (zone->next->memory);
	}
	return (NULL);
}

static void	*alloc_large(t_block **large, size_t size)
{
	t_block	*tmp;

	tmp = *large;
	if (*large == NULL)	
	{
		if ((*large = mmap(NULL, sizeof(t_block), PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return (NULL);
		tmp = *large;
	}
	else
	{
		while (tmp->next != NULL)
			tmp = tmp->next;
		if ((tmp->next = mmap(NULL, sizeof(t_block), PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return (NULL);
		tmp = tmp->next;
	}
	if ((tmp->addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return (NULL);
	tmp->size = size;
	tmp->next = NULL;
	return (tmp->addr);
}

static void	*get_address(enum e_zones zone, size_t size)
{
	void	*addr;

	addr = NULL;
	if (zone == TINY)
		addr = allocate_block(&g_zones.tiny,
			(size_t)(MAX_TINY * MAX_PER_ZONE * getpagesize()), size);
	else if (zone == SMALL)
		addr = allocate_block(&g_zones.small,
			(size_t)(MAX_SMALL * MAX_PER_ZONE * getpagesize()), size);
	else if (zone == LARGE)
		addr = alloc_large(&g_zones.large, size);
	return (addr);
}

void	*malloc(size_t size)
{
	return (get_address(zone_size(size), size));
}

#include <stdio.h>
int main()
{
	int	*nb[4] = {NULL, NULL, NULL, NULL};

	printf("Page size: '%d'\n", getpagesize());
	if ((nb[0] = (int *)malloc(sizeof(int))) == NULL)
		printf("Error malloc\n");
	if ((nb[1] = (int *)malloc(sizeof(int))) == NULL)
		printf("Error malloc\n");
	if ((nb[2] = (int *)malloc(sizeof(int))) == NULL)
		printf("Error malloc\n");
	if ((nb[3] = (int *)malloc(sizeof(int))) == NULL)
		printf("Error malloc\n");
	//printf("nb : '%p'\n", nb[0]);
	*(nb[0]) = 4;
	*(nb[1]) = 12;
	*(nb[2]) = 16;
	*(nb[3]) = 28;
	printf("Nb: '%d'\n", *(nb[0]));
	printf("Nb1: '%d'\n", *(nb[1]));
	printf("Nb2: '%d'\n", *(nb[2]));
	printf("Nb3: '%d'\n", *(nb[3]));
	return (0);
}

/*void	free(void *ptr);
{
	munmap(ptr, );
}*/
