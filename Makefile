bc: bc.c
	gcc bc.c `pkg-config hildon-1 --cflags --libs` -lm -Wall -o bc -O6
	strip bc

clean:
	rm -f *.o bc core
