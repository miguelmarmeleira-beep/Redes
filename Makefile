CC = gcc
CFLAGS = -Wall -Wextra -pedantic

SRC = user.c config.c validation.c udp.c tcp.c login.c logout.c unregister.c publish.c remove.c list.c versions.c

HEADERS = $(wildcard *.h)

.PHONY: all clean

all: user

user: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o user

clean:
	rm -f user
