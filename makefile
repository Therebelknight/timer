all: clean timer

clean:
	@rm -rf timer
timer:
	gcc main.c -o timer -lrt
