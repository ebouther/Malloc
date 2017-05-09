/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <ebouther@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/05/01 17:46:41 by ebouther          #+#    #+#             */
/*   Updated: 2017/05/09 17:17:28 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

t_malloc_zones g_zones = (t_malloc_zones){NULL, NULL, NULL};

/*
**  Return the correct zone type (enum e_zones) for the allocation @size.
*/

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

/*
**  If there's enough memory (@alloc_size) in this @zone
**  check if it can be saved in an existing or a new block.
*/

static void	*check_for_blocks(t_zone *zone, size_t alloc_size)
{
	t_block	*blk;

	//printf(" CHECK_FOR_BLOCKS REMAINING : %zu | SIZE : %zu | zone memory : %x\n", zone->remaining, alloc_size, zone->memory);
	if (zone->remaining >= alloc_size)
	{
		if ((blk = zone->blocks) == NULL)
			return (NULL);
		printf(" CHECK_FOR_BLOCKS 0\n");
		while (blk->next != NULL)
			blk = blk->next;
		printf(" NEW BLOCK\n");
		if ((blk->next = mmap(NULL, sizeof(t_block), PROT_READ | PROT_WRITE,
				MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return (NULL);
		*(blk->next) = (t_block){.next = NULL,
								 .size = alloc_size,
								 .freed = 0,
								 .addr = blk->addr + blk->size + 1};
		zone->remaining -= alloc_size;
		printf(" RETURN BLOCK = %x \n", blk->next->addr);
		return (blk->next->addr);
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
	*(zone->blocks) = (t_block){.next = NULL,
								 .size = alloc_size,
								 .freed = 0,
								 .addr = zone->memory};
	//printf(" 1 RETURN ZONE FIRST BLOCK = %x \n", zone->blocks->addr);
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
		printf("ALLOC NEW ZONE\n");
		if (new_zone(z, max_size, alloc_size) == -1)
			return (NULL);
		return ((*z)->memory);
	}
	else
	{
		if ((addr = check_for_blocks(zone, alloc_size)) != NULL)
			return (addr);
		printf(" CHECKED_BLOCKS \n");
		while (zone->next != NULL)
		{
			if ((addr = check_for_blocks(zone->next, alloc_size)) != NULL)
				return (addr);
			zone = zone->next;
		}
		printf("!!!!!!! NEW ZONE !!!!!!!!\n");
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
	printf(" GET_ADDRESS \n");
	if (zone == TINY)
	{
		addr = allocate_block(&g_zones.tiny,
			(size_t)((MAX_TINY * MAX_PER_ZONE) - (MAX_TINY * MAX_PER_ZONE % getpagesize())), size);
	}
	else if (zone == SMALL)
	{
		addr = allocate_block(&g_zones.small,
			(size_t)((MAX_SMALL * MAX_PER_ZONE) - (MAX_TINY * MAX_PER_ZONE % getpagesize())), size);
	}
	else if (zone == LARGE)
	{
		addr = alloc_large(&g_zones.large, size);
	}
	return (addr);
}

void	*malloc(size_t size)
{
	printf("=======================\n");
	return (get_address(zone_size(size), size));
}

int main()
{
	int alloc_length = 1000;
	char **alloc_array = NULL;

	if ((alloc_array = (char **)malloc(sizeof(char *) * alloc_length)) == NULL)
		printf("Error malloc\n");
	
	int i = 0;
	while (i < alloc_length) {
		if ((alloc_array[i] = (char *)malloc(sizeof(char) * 500)) == NULL)
			printf("Error malloc\n");
		alloc_array[i][0] = 'a' + (i % 26);
		alloc_array[i][499] = 'a' + (i % 26);
		i++;
	}

	i = 0;
	while (i < alloc_length) {
		printf("FIRST : %c", alloc_array[i][0]);
		printf(" | LAST : %c", alloc_array[i][499]);
		printf("\n");
		free(alloc_array[i]);
		i++;
	}
	free(alloc_array);
	return (0);
}


//int main()
//{
//	int	*nb[4] = {NULL, NULL, NULL, NULL};
//
//	printf("Page size: '%d'\n", getpagesize());
//	if ((nb[0] = (int *)malloc(sizeof(int))) == NULL)
//		printf("Error malloc\n");
//	if ((nb[1] = (int *)malloc(sizeof(int))) == NULL)
//		printf("Error malloc\n");
//	if ((nb[2] = (int *)malloc(sizeof(int))) == NULL)
//		printf("Error malloc\n");
//	if ((nb[3] = (int *)malloc(sizeof(int))) == NULL)
//		printf("Error malloc\n");
//	printf(">>>>>>>>>>>>>>>>>>>>\n");
//	//printf("nb : '%p'\n", nb[0]);
//	*(nb[0]) = 4;
//	*(nb[1]) = 12;
//	*(nb[2]) = 16;
//	*(nb[3]) = 28;
//	printf("Nb: '%d'\n", *(nb[0]));
//	printf("Nb1: '%d'\n", *(nb[1]));
//	printf("Nb2: '%d'\n", *(nb[2]));
//	printf("Nb3: '%d'\n", *(nb[3]));
//	printf("1) TINY: '%p' | SMALL : '%p' \n", g_zones.tiny, g_zones.small);
//
//	free(nb[0]);
//	free(nb[1]);
//	free(nb[2]);
//	free(nb[3]);
//
//	return (0);
//}

