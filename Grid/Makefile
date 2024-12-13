all: mk_build compile link run

mk_build:
	@if [ ! -d "build" ]; then mkdir build; fi
	# @if not exist "build" mkdir build
	
# compile:
# 	@echo "* Compiling"
# 	@g++ -c src/main.cpp -I./src -o build/Grid.o

compile:
	@echo "* Compiling"
	@g++ -c src/main.cpp -I./src -DUNICODE -D_UNICODE -o build/Grid.o

link:
	@echo "* Linking"
	@g++ build/Grid.o -o build/Grid -luser32 -lgdi32 -lshcore

dbg:
	@g++ -g -c src/main.cpp -I./src -DUNICODE -D_UNICODE -o build/Grid.o
	@g++ build/Grid.o -o build/Grid -luser32 -lgdi32 -lshcore
	@gdb ./build/Grid.exe

# link:
#     @echo "* Linking"
#     @g++ build/Grid.o -o build/Grid.exe -luser32 -lgdi32 -mwindows

run:
	@echo "* Running Keydows"
	@build/Grid.exe