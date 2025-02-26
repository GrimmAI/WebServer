server:
	g++ src/server.cpp -o server && g++ src/client.cpp -o client
clean:
	rm server && rm client