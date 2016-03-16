all: signalkcat

signalkcat:
	gcc signalkcat.cpp -O3 -lwebsockets -o signalkcat

install: signalkcat
	cp signalkcat /usr/local/bin

clean:
	rm signalkcat

