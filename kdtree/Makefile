CC = gcc
CXX = g++
CFLAGS = -g -c -Wall

OBJECTS = KDTree.o
SRCS = KDTree.c 
TARGET = kdtree

$(TARGET) : $(OBJECTS)
		$(CXX) -o $(TARGET) $(OBJECTS) -lm

KDTree.o : KDTree.c
		$(CXX) $(CFLAGS) -c KDTree.c -lm

clean :
		rm -rf $(OBJECTS) $(TARGET)
