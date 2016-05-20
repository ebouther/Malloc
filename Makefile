ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME = libft_malloc_$(HOSTTYPE).so

CC = gcc
CFLAGS = -Wall -Wextra -Werror
LFLAGS = -L$(LIBFT_PATH) -lft
IFLAGS = -I $(INC_DIR)

LIBFT_PATH = ./libft
LIBFT_BIN = libft.a
LIBFT = $(addprefix $(LIBFT_PATH), $(LIBFT_BIN))

INC_DIR = ./inc/
INCLUDES = malloc.h

SRC_DIR = ./src/
SRC_FILES = malloc.c

OBJ_DIR = ./obj/
OBJ_FILES = $(SRC_FILES:.c=.o)

SRC = $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJ = $(addprefix $(OBJ_DIR), $(OBJ_FILES))
INC = $(addprefix $(INC_DIR), $(INC_FILES))

all: $(OBJ_DIR) $(LIBFT) $(NAME) auteur

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c $(INC)
	$(CC) $(CFLAGS) $(IFLAGS) -o $@ -c $<

$(NAME): $(OBJ)
	ar rcs $(NAME) $(OBJ)
	ln -s libft_malloc_$(HOSTTYPE).so libft_malloc.so 

clean:
	make -C $(LIBFT_PATH) clean
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME) $(LIBFT)
	rm -f libft_malloc.so

re: fclean all

.PHONY: all re fclean clean $(LIBFT)
