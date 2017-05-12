/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouther <ebouther@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/05/01 17:46:41 by ebouther          #+#    #+#             */
/*   Updated: 2017/05/12 14:41:03 by ebouther         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

t_malloc_zones g_zones = (t_malloc_zones)
	{
	 NULL,
	 NULL,
	 NULL,
	 {NULL, 0, 0}
	};

static void parse_blocks(t_block **blocks)
{
	t_block	*block;

	block = *blocks;
	while (block)
	{
		if (block->freed == FALSE)
			printf("%x - %x : %zu octets\n",
					(unsigned int)block->addr,
					(unsigned int)(block->addr + block->size),
					(unsigned long)block->size);
		else if (DEBUG)
		{
			printf("%s%x - %x : %zu octets [FREED]%s\n",
				DEBUG_COLOR,
				(unsigned int)block->addr,
				(unsigned int)(block->addr + block->size),
				(unsigned long)block->size,
				NO_COLOR);

		}
		block = block->next;
	}
}

static void parse_zone(const char *zone_type, t_zone *zone)
{
	while (zone)
	{
		printf("%s%x \n", zone_type, (unsigned int)zone->memory);
		parse_blocks(&zone->blocks);
		zone = zone->next;
	}

}

void	show_alloc_mem() {
    parse_zone("TINY : ", g_zones.tiny);
    parse_zone("SMALL : ", g_zones.small);
	printf("LARGE :\n");
    parse_blocks(&g_zones.large);
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------

void *new_list(size_t size)
{
	//printf("OFFSET + SIZE = %zu | LEN = %zu\n", g_zones.lst_mem.offset + size,
	//	g_zones.lst_mem.len);
	if (g_zones.lst_mem.offset + size >= g_zones.lst_mem.len)
	{
		//printf("NEW LST_MEM (SIZE : %zu)\n", size);
		g_zones.lst_mem.len = getpagesize();
		g_zones.lst_mem.offset = size;
		if ((g_zones.lst_mem.ptr = mmap(NULL, g_zones.lst_mem.len, PROT_READ | PROT_WRITE,
				MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return (NULL);
		return (g_zones.lst_mem.ptr);
	}
	else
	{
		//printf("FILL CURR LST_MEM (SIZE : %zu)\n", size);
		g_zones.lst_mem.offset += size;
		return (g_zones.lst_mem.ptr + g_zones.lst_mem.offset);
	}
}


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

static void	*use_freed_block(t_zone *zone, t_block *blk, size_t alloc_size)
{
	blk->freed = FALSE;
	if (blk->size == alloc_size)
	{
		zone->freed_blks_nb -= 1;
		if (DEBUG)
		{
			printf("%sUSE OLD BLOCK MEMORY AT ADDR : %x\nFreed blks left in zone : %zu%s\n",
				DEBUG_COLOR,
				(unsigned int)blk->addr,
				zone->freed_blks_nb,
				NO_COLOR);
		}
		return (blk->addr);
	}
	else
	{
		if (DEBUG)
			printf("%sDIVIDE AND USE OLD BLOCK AT ADDR : %x%s\n", DEBUG_COLOR, (unsigned int)blk->addr, NO_COLOR);

		t_block *tmp = blk->next;
		
		//if ((blk->next = mmap(NULL, sizeof(t_block), PROT_READ | PROT_WRITE,
		//		MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		//	return (NULL);
		blk->next = new_list(sizeof(t_block));
		*(blk->next) = (t_block){.next = tmp,
								 .size = blk->size - alloc_size,
								 .freed = TRUE,
								 .addr = blk->addr + alloc_size};
		blk->size = alloc_size;
		return (blk->addr);
	}

}

/*
**  If there's enough memory (@alloc_size) in this @zone
**  check if it can be saved in an existing or a new block.
*/

static void	*check_for_blocks(t_zone *zone, size_t alloc_size)
{
	t_block	*blk;

	////printf(" CHECK_FOR_BLOCKS REMAINING : %zu | SIZE : %zu | zone memory : %x\n", zone->remaining, alloc_size, zone->memory);

	if (zone->remaining >= alloc_size || zone->freed_blks_nb > 0)
	{
		if ((blk = zone->blocks) == NULL)
		{
			printf("ERROR in check_for_blocks\n");
			return (NULL);
		}
		while (blk->next != NULL)
		{
			if (zone->freed_blks_nb > 0
					&& blk->freed == TRUE
					&& blk->size >= alloc_size)
				return (use_freed_block(zone, blk, alloc_size));
			blk = blk->next;
		}
		//if ((blk->next = mmap(NULL, sizeof(t_block), PROT_READ | PROT_WRITE,
		//		MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		//	return (NULL);
		blk->next = new_list(sizeof(t_block));
		*(blk->next) = (t_block){.next = NULL,
								 .size = alloc_size,
								 .freed = FALSE,
								 .addr = blk->addr + blk->size};

		zone->remaining -= alloc_size;
		return (blk->next->addr);
	}
	return (NULL);
}

static int	new_zone(t_zone **z, size_t max_size, size_t alloc_size)
{
	t_zone	*zone;

	//if ((zone = mmap(NULL, sizeof(t_zone), PROT_READ | PROT_WRITE,
	//		MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
	//	return (-1);
	zone = new_list(sizeof(t_zone));

	*zone = (t_zone){.remaining = max_size - alloc_size,
					 .freed_blks_nb = 0,
					 .next = NULL};

	if ((zone->memory = mmap(NULL, max_size * getpagesize(), PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		return (-1);
	//if ((zone->blocks = (t_block *)mmap(NULL, sizeof(t_block), PROT_READ | PROT_WRITE,
	//		MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED) 
	//	return (-1);
	zone->blocks = new_list(sizeof(t_zone));
	*(zone->blocks) = (t_block){.next = NULL,
								 .size = alloc_size,
								 .freed = FALSE,
								 .addr = zone->memory};

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
		//printf("ALLOC NEW ZONE\n");
		if (new_zone(z, max_size, alloc_size) == -1)
			return (NULL);
		return ((*z)->memory);
	}
	else
	{
		if ((addr = check_for_blocks(zone, alloc_size)) != NULL)
			return (addr);
		//printf(" CHECKED_BLOCKS \n");
		while (zone->next != NULL)
		{
			//printf("2 CHECK_FOR_BLK\n");
			if ((addr = check_for_blocks(zone->next, alloc_size)) != NULL)
				return (addr);
			zone = zone->next;
		}
		//printf("!!!!!!! NEW ZONE !!!!!!!!\n");
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
	printf("ALLOC LARGE\n");
	if (*large == NULL)	
	{
		//if ((*large = mmap(NULL, sizeof(t_block), PROT_READ | PROT_WRITE,
		//	MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		//	return (NULL);
		*large = new_list(sizeof(t_block));
		tmp = *large;
	}
	else
	{
		while (tmp->next != NULL)
			tmp = tmp->next;
		//if ((tmp->next = mmap(NULL, sizeof(t_block), PROT_READ | PROT_WRITE,
		//	MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
		//	return (NULL);
		tmp->next = new_list(sizeof(t_block));
		tmp = tmp->next;
	}
	if ((tmp->addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return (NULL);
	tmp->size = size;
	tmp->freed = FALSE;
	tmp->next = NULL;
	return (tmp->addr);
}

static void	*get_address(enum e_zones zone, size_t size)
{
	void	*addr;

	addr = NULL;
	//printf(" GET_ADDRESS \n");
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
			printf("%s MALLOC %s\n", DEBUG_COLOR, NO_COLOR);
	if (g_zones.lst_mem.ptr == NULL)
	{
		g_zones.lst_mem.len = getpagesize();
		g_zones.lst_mem.offset = 0;
		if ((g_zones.lst_mem.ptr = mmap(NULL, g_zones.lst_mem.len, PROT_READ | PROT_WRITE,
				MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
			return (NULL);
	}
	return (get_address(zone_size(size), size));
}

//int main()
//{
//	int alloc_length = 60;
//	char **alloc_array = NULL;
//	int len_1 = 200;
//	int len_2 = 200;
//
//	void *tmp;
//
//	if ((alloc_array = (char **)malloc(sizeof(char *) * alloc_length)) == NULL)
//		printf("Error malloc\n");
//	
//	int i = 0;
//	while (i < alloc_length) {
//		if (i % 2 == 0) {
//			if ((alloc_array[i] = (char *)malloc(sizeof(char) * len_1)) == NULL)
//				printf("Error malloc\n");
//			alloc_array[i][0] = 'c';
//			alloc_array[i][len_1 - 1] = 'c';
//		} else {
//			if ((alloc_array[i] = (char *)malloc(sizeof(char) * len_2)) == NULL) // 10 - 11 - 12 sgft
//				printf("Error malloc\n");
//			//printf("MAIN WRITE 'a' at %x\n", alloc_array[i]);
//			alloc_array[i][0] = 'a';
//			alloc_array[i][len_2 - 1] = 'a';
//		}
//		if (i == 6)
//		{
//			free (alloc_array[i]);
//			tmp = malloc(10);
//		}
//		i++;
//	}
//
//
//	show_alloc_mem();
//
//
//
//	i = 0;
//	while (i < alloc_length) {
//		free(alloc_array[i]);
//		i++;
//	}
//	free(alloc_array);
//	return (0);
//}

// USE OLD BLOCK MAIN
//int main()
//{
//	char *str[4] = {NULL, NULL, NULL, NULL};
//
//	if ((str[0] = malloc(10)) == NULL)
//		printf("Error malloc\n");
//	if ((str[1] = malloc(10)) == NULL)
//		printf("Error malloc\n");
//	if ((str[2] = malloc(10)) == NULL)
//		printf("Error malloc\n");
//	
//	show_alloc_mem();
//
//	printf("--------------------\n");
//	free(str[1]);
//	show_alloc_mem();
//
//	printf("--------------------\n");
//	if ((str[1] = malloc(10)) == NULL)
//		printf("Error malloc\n");
//	show_alloc_mem();
//
//	printf("--------------------\n");
//	free(str[1]);
//	show_alloc_mem();
//
//	printf("--------------------\n");
//	if ((str[1] = malloc(4)) == NULL)
//		printf("Error malloc\n");
//	show_alloc_mem();
//
//	printf("--------------------\n");
//	if ((str[1] = malloc(4)) == NULL)
//		printf("Error malloc\n");
//	show_alloc_mem();
//	printf("--------------------\n");
//	if ((str[1] = malloc(4)) == NULL)
//		printf("Error malloc\n");
//	show_alloc_mem();
//
//	free(str[0]);
//	free(str[2]);
//	printf("--------------------\n");
//	show_alloc_mem();
//
//	return (0);
//}

//int main ()
//{
//	int i = 0;
//	while (i < 400)
//	{
//		char *str;
//		str = malloc(2);
//		memset(str, 0, 2);
//		memset(str, 'a', 1);
//		printf("%s\n", str);
//		i++;
//	}
//
//	return (0);
//}

//int      main()
//{
//   char  *addr[4];
//
//   addr[0] = (char*)malloc(1024);
//   addr[0][0] = 42;
//
//   addr[1] = (char*)malloc(1024);
//   addr[1][0] = 42;
//   free(addr[1]);
//
//   addr[2] = (char*)malloc(1024);
//   addr[2][0] = 42;
//   show_alloc_mem();
//   realloc(addr[0], 2048);
//   printf("---------------\n");
//   show_alloc_mem();
//   return (0);
//}


//#include <strings.h>
//#include <stdlib.h>
//#include <unistd.h>
//
//#define M (1024 * 1024)
//
//void print(char *s)
//{
//   write(1, s, strlen(s));
//}
//
//int     main()
//{
//   char *addr1;
//   char *addr3;
//
//   addr1 = (char*)malloc(16*M);
//   strcpy(addr1, "Bonjours\n");
//   print(addr1);
//   addr3 = (char*)realloc(addr1, 128*M);
//   addr3[127*M] = 42;
//   print(addr3);
//   return (0);
//}
#define M (1024 * 1024)

//void print(char *s)
//{
//   write(1, s, strlen(s));
//}

int     main()
{
   char *addr1;
   //char *addr3;

   write(1, "BEGIN", 5);
   write(1, "BEGIN", 5);
   write(1, "BEGIN", 5);
   write(1, "BEGIN", 5);
   write(1, "BEGIN", 5);
   write(1, "BEGIN", 5);
   addr1 = (char*)malloc(16*M);
   strcpy(addr1, "Bonjours\n");
   //print(addr1);
   //addr3 = (char*)realloc(addr1, 128*M);
   //addr3[127*M] = 42;
   //print(addr3);
   return (0);
}
