NAME=ft_ping

SRCS= ${wildcard *.c}

OBJS= ${SRCS:.c=.o}

CC=gcc

CFLAGS=-Wall -Wextra -Werror

RM=rm -f

.c.o:
	${CC} ${CFLAGS} -c $< -o ${<:.c=.o}

${NAME}: ${OBJS}
	${CC} ${CFLAGS} -o ${NAME} ${OBJS}

all: ${NAME}

clean:
	${RM} ${OBJS}

fclean: clean
	${RM} ${NAME}

re: fclean all

.PHONY: all clean fclean re