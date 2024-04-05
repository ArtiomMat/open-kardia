cc = gcc
flags = -O3 -Iinc -ggdb -D PSF_X_KARDIA
libs = -lX11 -lasound -lm

srcs = $(wildcard src/*.c)
objs = $(patsubst src/%.c,obj/%.o,$(srcs))

bin/kardia: $(objs)
	$(cc) $(flags) $^ -o $@ $(libs)
  
obj/%.o: src/%.c inc/%.h
	$(cc) $(flags) -c $< -o $@

clean:
	rm -rf obj/*.o bin/kardia

