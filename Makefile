CC=g++
CFLAGS=-Wall -pg -O3
LINK=-lcrypto

OBJ=hamming_distance.o generate.o sha1.o
HDR=hamming_distance.h sha1.h
TARGET=generate

$(TARGET):$(OBJ) $(HDR)
	$(CC) $(CFLAGS) $(OBJ) $(LINK) -o $(TARGET) 

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f $(TARGET)