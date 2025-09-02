ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME = libft_malloc_$(HOSTTYPE).so
SYMLINK = libft_malloc.so

CC = gcc
CFLAGS = -Wall -Wextra -Werror -fPIC
INCLUDES = -I./includes

SRCDIR = src
OBJDIR = obj

SOURCES = malloc.c \
		  free.c \
		  realloc.c \
		  show_alloc_mem.c

SRCS = $(addprefix $(SRCDIR)/, $(SOURCES))
OBJS = $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))

all: $(NAME) static

$(NAME): $(OBJS)
	$(CC) -shared -o $(NAME) $(OBJS)
	ln -sf $(NAME) $(SYMLINK)

static: libft_malloc.a

libft_malloc.a: $(OBJS)
	ar rcs libft_malloc.a $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)
	rm -f $(SYMLINK)

re: fclean all

test: $(NAME)
	$(CC) -L. -lft_malloc test.c -o test_malloc

.PHONY: all clean fclean re test