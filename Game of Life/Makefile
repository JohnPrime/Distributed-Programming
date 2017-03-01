serial:
	g++ -c g_serial.cpp
	g++ -o g_serial g_serial.o -static-libstdc++

parallel:
	g++ -c g_omp.cpp -fopenmp
	g++ -o g_omp g_omp.o -fopenmp -static-libstdc++

clean:
	rm -rf g_serial g_serial.o g_omp g_omp.o
