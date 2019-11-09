SRCS=IT8951.c main.c
CC=gcc
TARGET=IT8951

$(TARGET):$(SRCS)
	$(CC) -O3 -Wall $(SRCS) -o $(TARGET) -lbcm2835 -lpng -lpthread
	
clean:
	rm -f $(TARGET)
