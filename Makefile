NAME = irc 
SRC = $(shell find src -name "*.cpp")
OBJ = $(SRC:.cpp=.o)

CC = c++
DEV_FLAGS = -Wall -Wextra -std=c++98 -g
FLAGS = $(DEV_FLAGS) -Werror -g
LINKS =

all: $(NAME)

$(NAME): $(OBJ)
	echo "SRC $(SRC)"
	echo "OBJ $(OBJ)"
	$(CC) $(DEV_FLAGS) $(OBJ) $(LINKS) -o $(NAME)

.cpp.o: $(SRC)
	$(CC) $(DEV_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
