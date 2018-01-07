CC = gcc
CFLAGS = -I./src/include -Wall -Wextra -Werror -pedantic -O2
LDLIBS = -lreadline -lcapstone -lunwind

VPATH = ./src/
OBJS= info_cmd.o breakpoint.o bp_list.o cmd.o prompt.o
BIN= my-gdb

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

clean:
	${RM} ${OBJS} ${BIN}

.PHONY: clean all
