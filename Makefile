all: compile link run

compile:
	@echo "*" Compiling
	@g++ -c src/main.cpp -I./src -o build/main.o


link:
	@echo "*" Linking
	@g++ build/main.o -o build/main -luser32

run:
	@echo "*" Running main.exe
	@build/main.exe

clean:
	del /f build\main.exe build\main.o