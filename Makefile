all : Gravity
	
Gravity: gravity.c gfx5.c 
	gcc gravity.c gfx5.c -lm -lX11 -o Gravity

clean:
	rm -rf Gravity *.o
