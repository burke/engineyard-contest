CC=mpicc
CFLAGS=-Wall -O3
LINK=-Wl -Bstatic -lcrypto

OBJ=hamming_distance.o generate.o
HDR=hamming_distance.h
TARGET=generate

$(TARGET):$(OBJ) $(HDR)
	$(CC) $(CFLAGS) $(OBJ) $(LINK) -o $(TARGET) 

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f $(TARGET)
