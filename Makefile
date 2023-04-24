transport:
	gcc -std=gnu17 -Wall -Wextra -o transport main.c

clean:
	$(RM) *.o

distclean:
	$(RM) transport *.o