server:
	g++ src/server.cpp src/tcp/Epoll.cpp src/tcp/InetAddress.cpp src/tcp/Socket.cpp src/tcp/Channel.cpp src/tcp/Server.cpp \
	src/tcp/EventLoop.cpp src/tcp/Acceptor.cpp src/tcp/Connection.cpp -o server
client:
	g++ src/client.cpp src/tcp/InetAddress.cpp src/tcp/Socket.cpp -o client
clean:
	rm server && rm client