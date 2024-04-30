cc = gcc
flags = -Wall -O3 -Iinc -ggdb -D DEBUG
libs = -lX11 -lXrandr -lasound -lm

srcs = $(wildcard src/*.c)
objs = $(patsubst src/%.c,obj/%.o,$(srcs))

bin/kardia: $(objs)
	$(cc) $(flags) $^ -o $@ $(libs)
  
obj/%.o: src/%.c inc/*.h
	$(cc) $(flags) -c $< -o $@

run:
	make -f makefile.nix -B && bin/kardia -v 255 -f sanserif.psf

clean:
	rm -rf obj/*.o bin/kardia

