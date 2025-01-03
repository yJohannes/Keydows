all: mk_build compile link run

mk_build:
	@if [ ! -d "build" ]; then mkdir build; fi
# @if not exist "build" mkdir build
	
compile:
	@echo "* Compiling"
	@g++ -c src/main.cpp -I./src -DUNICODE -D_UNICODE -o build/keydows.o

link:
	@echo "* Linking"
	@g++ build/keydows.o -o build/keydows -luser32 -lgdi32 -lshcore -lgdiplus

run:
	@echo "* Running Keydows"
	@build/keydows.exe