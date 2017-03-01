all: build

build: tema3apd.c
	mpicc tema3apd.c -o filtru

run:
	mpirun -np ${arg} ./filtru ${topologie} ${imagini} ${statistica}

clean:
	rm -f filtru
