CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
SRCS = main.c csv.c lexer.c parser.c eval.c exec.c
TARGET = csvq

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)