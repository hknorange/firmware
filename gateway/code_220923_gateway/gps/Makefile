all: pipegps
pipegps : pipegps.cpp sc16is750.cpp sc16is750.h Makefile
	g++ -o pipegps pipegps.cpp sc16is750.cpp -Wall -lwiringPi
clean:
	rm -f pipegps *.o


