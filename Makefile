all: generate.exe display.exe

generate.exe: generate.c
	gcc -o generate.exe generate.c

display.exe: display.c
	gcc -o display.exe display.c -lglut32 -lopengl32 -lglu32
