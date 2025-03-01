server:
	g++ src/server.cpp src/tcp/Epoll.cpp  src/tcp/Channel.cpp src/tcp/Server.cpp \
	src/tcp/EventLoop.cpp src/tcp/Acceptor.cpp src/tcp/Connection.cpp src/tcp/Buffer.cpp src/tcp/ThreadPool.cpp -o server
client:
	g++ src/client.cpp  src/tcp/Buffer.cpp -o client

test:
	g++ src/tcp/ThreadPool.cpp src/test_threadpool.cpp src/tcp/Buffer.cpp -o test

clean:
	rm server && rm client && rm test