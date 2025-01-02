all: mk_build compile link run

mk_build:
	@if [ ! -d "build" ]; then mkdir build; fi
# @if not exist "build" mkdir build
	
# compile:
# 	@echo "* Compiling"
# 	@g++ -c src/main.cpp -I./src -o build/keydows.o

compile:
	@echo "* Compiling"
	@g++ -c src/main.cpp -I./src -DUNICODE -D_UNICODE -o build/keydows.o

link:
	@echo "* Linking"
	@g++ build/keydows.o -o build/keydows -luser32 -lgdi32 -lshcore -lgdiplus

dbg:
	@g++ -g -c src/main.cpp -I./src -DUNICODE -D_UNICODE -o build/keydows.o
	@g++ build/keydows.o -o build/keydows -luser32 -lgdi32 -lshcore -lgdiplus
	@gdb ./build/keydows.exe

# link:
#     @echo "* Linking"
#     @g++ build/Grid.o -o build/Grid.exe -luser32 -lgdi32 -mwindows

run:
	@echo "* Running Keydows"
	@build/keydows.exe