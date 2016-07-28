all : Gravity
	
Gravity: src/gravity.c src/gfx5.c 
	gcc src/gravity.c src/gfx5.c -lm -lX11 -o Gravity

clean:
	rm -rf Gravity src/*.o
