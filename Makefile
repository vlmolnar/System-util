# Practical solution
myls: myls.c
	gcc -Wall -Wextra myls.c -o myls

# Starter code
starter: starter.c
	gcc -Wall -Wextra starter.c -o starter

clean:
	rm starter myls *.o
