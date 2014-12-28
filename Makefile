CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LFLAGS = -lrt -lpthread
NAME = rivercrossing

.PHONY : all $(NAME) clean
all : $(NAME)

debug : CFLAGS += -g
debug : all

$(NAME) : $(NAME).o
	gcc $(CFLAGS) -o $@ $< $(LFLAGS)

clean :
	rm -f $(NAME) $(NAME).o
	
%.o : %.c
	gcc $(CFLAGS) -o $@ -c $<
