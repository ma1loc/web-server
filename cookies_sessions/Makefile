CPP = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = cookies_session_handler.cpp Session.cpp SessionManager.cpp \

HEADERS = Session.hpp SessionManager.hpp cookies_session_handler.hpp


OBJ = $(SRC:.cpp=.o)

all: cookies

cookies: $(OBJ) $(HEADERS)
	$(CPP)  $(OBJ) -o cookies

%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -f cookies

re: fclean all

.SECONDARY:

.PHONY: all fclean clean re