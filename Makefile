# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ebouther <ebouther@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2016/05/21 00:09:33 by ebouther          #+#    #+#              #
#    Updated: 2017/06/12 12:16:14 by ebouther         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME = libft_malloc_$(HOSTTYPE).so

CC = gcc
CFLAGS =  -Wall -Wextra -Werror
DYLDFLAGS = -shared -fPIC
IFLAGS = -I $(INC_DIR)

INC_DIR = ./inc/
INC_FILES = malloc.h

SRC_DIR = ./src/
SRC_FILES = malloc.c \
			free.c \
			realloc.c \
			disp.c \
			blocks.c \
			show_alloc_mem.c \
			ft_ll_itoa_base.c \
			utils.c

OBJ_DIR = ./obj/
OBJ_FILES = $(SRC_FILES:.c=.o)

SRC = $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJ = $(addprefix $(OBJ_DIR), $(OBJ_FILES))
INC = $(addprefix $(INC_DIR), $(INC_FILES))

all: $(OBJ_DIR) $(NAME) auteur

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c $(INC)
	$(CC) $(CFLAGS) $(IFLAGS) -o $@ -c $<

$(NAME): $(OBJ)
	$(CC) $(DYLDFLAGS) $^ -o $(NAME)
	rm -f libft_malloc.so
	ln -s libft_malloc_$(HOSTTYPE).so libft_malloc.so 

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME)
	rm -f libft_malloc.so

re: fclean all

.PHONY: all re fclean clean
