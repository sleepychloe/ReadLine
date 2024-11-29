# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: yhwang <yhwang@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/11/28 09:51:42 by yhwang            #+#    #+#              #
#    Updated: 2024/11/29 20:35:49 by yhwang           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = readline
CPP = c++

SRC_DIR = ./srcs/
INCS_DIR = ./incs/

CPPFLAGS = -Wall -Wextra -Werror -std=c++17 -g3

RM = rm -f

FILES = $(SRC_DIR)main \
		$(SRC_DIR)ReadLine

SRCS = $(addsuffix .cpp, $(FILES))
OBJS = $(addsuffix .o, $(FILES))

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) $^ -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
