compile:
	gcc -Wall -g3 -fsanitize=address -pthread -lm server.c -o server
	gcc -Wall -g3 -fsanitize=address -pthread -lm client.c -o client
server:
	gcc -Wall -g3 -fsanitize=address -pthread -lm server.c -o server
client:
	gcc -Wall -g3 -fsanitize=address -pthread -lm client.c -o client
backup:
	gcc -Wall -g3 -fsanitize=address -pthread -lm server2.c -o server2
	gcc -Wall -g3 -fsanitize=address -pthread -lm client2.c -o client2
clean: 
	rm -f server client server2 client2
	rm -r *dSYM
	rm -r .vscode