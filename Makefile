all: signalkcat

signalkcat:
	gcc signalkcat.cpp -O3 -lwebsockets -o signalkcat

static:
	gcc signalkcat.cpp -pthread -static -static-libgcc -O3 -lwebsockets -lssl -lcrypto -lz -ldl -o signalkcat

install: signalkcat
	cp signalkcat /usr/local/bin

clean:
	rm signalkcat

