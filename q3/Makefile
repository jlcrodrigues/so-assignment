CC = gcc
CFLAGS  = -Wall
TARGET = cypher
SRCS = cypher.c 

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	$(RM) $(TARGET)