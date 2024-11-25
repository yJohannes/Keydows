all: mk_build compile link run

mk_build:
	@mkdir -p build

compile:
	@echo "* Compiling"
	@g++ -c src/main.cpp -I./src -o build/Keydows.o


link:
	@echo "* Linking"
	@g++ build/Keydows.o -o build/Keydows -luser32

run:
	@echo "* Running Keydows.exe"
	@build/Keydows.exe