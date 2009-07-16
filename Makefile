CC=gcc
CFLAGS=-Wall -g -O3
LINK=-lcrypto

OBJ=hamming_distance.o generate.o table_generated.o
HDR=hamming_distance.h table_generated.h
TARGET=generate

$(TARGET):$(OBJ) $(HDR)
	$(CC) $(CFLAGS) $(OBJ) $(LINK) -o $(TARGET) 

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f $(TARGET)