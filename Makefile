CPP = c++
COMPILE_FLAG = -std=c++98
CPP_FLAGS = -Wall -Wextra -Werror #-fsanitize=address

NAME = webserv
SRC = ./main.cpp ./infrastructure_init/socket_init/init_socket_engine.cpp ./infrastructure_init/socket_init/init_client_side.cpp \
		./infrastructure_init/socket_init/init_server_side.cpp ./infrastructure_init/multiplexeur/multiplexeur.cpp \
		./infrastructure_init/multiplexeur/utils.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(COMPILE_FLAG) $(POLY) $(CPP_FLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp
	$(CPP) $(COMPILE_FLAG) $(POLY) $(CPP_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
