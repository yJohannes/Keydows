all: mk_build compile link run

mk_build:
	@if not exist "build" mkdir build
	
compile:
	@echo "* Compiling"
	@g++ -c src/main.cpp -I./src -o build/Keydows.o


link:
	@echo "* Linking"
	@g++ build/Keydows.o -o build/Keydows -luser32

run:
	@echo "* Running Keydows"
	@build/Keydows.exe