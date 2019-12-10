CC = gcc
CXX = g++
CFLAGS = -g -c -Wall

OBJECTS = rtree.o  test.o
SRCS = rtree.c test.c
TARGET = rtree

$(TARGET) : $(OBJECTS)
		$(CXX) -o $(TARGET) $(OBJECTS) -lm

rtree.o : rtree.h rtree.c
		$(CC) $(CFLAGS) -c rtree.c -lm
test.o :  test.c rtree.h
		$(CXX) $(CFLAGS) -c test.c rtree.h -lm

clean :
		rm -rf $(OBJECTS) $(TARGET)
