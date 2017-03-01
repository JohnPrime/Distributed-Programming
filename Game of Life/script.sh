#!/bin/bash
export OMP_NUM_THREADS=8
time ./g_serial $1 $2 $3; time ./g_omp $1 $2 $3
