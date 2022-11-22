NAME=ft_ping

SRCS= ${wildcard *.c}

OBJS= ${SRCS:.c=.o}

CC= gcc

CFLAGS= -Wall -Wextra -Werror

RM= rm -f

${NAME}: ${OBJS}
	${CC} ${CFLAGS} -o ${NAME} ${OBJS} -lm

all: ${NAME}

clean:
	${RM} ${OBJS}

fclean: clean
	${RM} ${NAME}

re: fclean all

.PHONY: all clean fclean re