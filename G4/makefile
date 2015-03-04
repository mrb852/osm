.phony: leftmost_one clean

leftmost_one: 
	@make -s clean
	@gcc -DUNITTEST_LEFTMOST_ONE -Wall -std=c11 -x c leftmost_one.h -lm
	./a.out

clean:
	@rm -f *~ a.out *.aux *.log 2>/dev/null
