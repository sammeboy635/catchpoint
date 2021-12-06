main: core.o agent.o
	g++ -g src/main.cpp lib/core.o lib/agent.o -lcurl -o main

agent.o:
	g++ -o lib/agent.o -Wall -g -c src/agent.cpp

core.o:
	g++ -o lib/core.o -Wall -g -c src/core.cpp

run:
	./main 1 &
	./main 2 &
	./main 3 