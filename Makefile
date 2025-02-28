server:
	g++ src/server.cpp src/tcp/Epoll.cpp src/tcp/InetAddress.cpp src/tcp/Socket.cpp src/tcp/Channel.cpp -o server
client:
	g++ src/client.cpp src/tcp/InetAddress.cpp src/tcp/Socket.cpp -o client
clean:
	rm server && rm client