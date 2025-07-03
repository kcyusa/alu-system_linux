CC       = gcc
CFLAGS   = -g3 -Wall -Werror -Wextra -pedantic
FILENAME = strace

0_SRC = 0-$(FILENAME).c
$(FILENAME)_0: $(0_SRC:.c=.o)
	$(CC) $(CFLAGS) $(0_SRC) -o $(FILENAME)_0

1_SRC = 1-$(FILENAME).c
$(FILENAME)_1: $(1_SRC:.c=.o)
	$(CC) $(CFLAGS) $(1_SRC) -o $(FILENAME)_1

2_SRC = 2-$(FILENAME).c
$(FILENAME)_2: $(2_SRC:.c=.o)
	$(CC) $(CFLAGS) $(2_SRC) -o $(FILENAME)_2

3_SRC = 3-$(FILENAME).c
$(FILENAME)_3: $(3_SRC:.c=.o)
	$(CC) $(CFLAGS) $(3_SRC) -o $(FILENAME)_3