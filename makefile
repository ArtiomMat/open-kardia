cc = gcc
flags = -O3 -Iinc
libs = -lX11

srcs = $(wildcard src/*.c)
objs = $(patsubst src/%.c,obj/%.o,$(srcs))

bin/kardia: $(objs)
	$(cc) $(flags) $^ -o $@ $(libs)
  
obj/%.o: src/%.c inc/%.h
	$(cc) $(flags) -c $< -o $@

clean:
	rm -rf obj/*.o bin/kardia

