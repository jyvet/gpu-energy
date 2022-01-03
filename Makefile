gpu-energy:
	gcc -Wall -O3 -o gpu-energy args.c gpu-energy.c -ldcgm

all: gpu-energy

clean:
	-@rm -f gpu-energy
