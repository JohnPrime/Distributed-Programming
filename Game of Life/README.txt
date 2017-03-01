Nume: Ion Bogdan-Ionut
Grupa: 332CB
Facultatea de Automatica si Calculatoare, UPB


	Tema a fost realizata in C++. Initial, citesc numarul de linii si colane din fisier si creez 
dinamic doua matrici, una pentru iteratia curent si cealalta pentru iteratia urmatoare a automatului. 
Ambele matrici au dimensiunea [rows + 2][columns + 2], pentru a face loc marginilor. Citirea datelor 
din fisier, se va face in matricea de la iteratia curenta, exceptand prima si ultima linie si prima si 
ultima coloana. Matricea din fisier va fi bordata de aceste margini. Dupa citire, bordez marginile matricei 
dupa regulile din enunt.

	Procesul de modificare a starilor celulelor il fac intr-un loop care se executa de numar_iteratii ori. 
La fiecare iteratie, parcurg matricea de la iteratia curenta, calculez numarul de vecini pentru celula curenta 
de la (i, j) si in functie de numarul de vecini, aceasta va fi moarta sau vie la iteratia urmatoare a automatului.
Asignez in matricea de la iteratia urmatoare a automatului la pozitia (i, j) starea celulei.
Dupa parcurgerea matricei, copiez matricea de la iteratia urmatoare in cea de la iteratia curenta si o bordez din 
nou. La final, copiez rezultatul(matricea starii finale a automatului), dar fara cele 4 margini, pentru a se observa 
clar diferenta intre matricea initiala si cea finala. De asemenea, eliberez spatiul ocupat de cele doua matrici.

	Paralelizarea programului o fac prin crearea de thread-uri(#pragma omp parallel) inainte de loop-ul 
care se executa de numar_iteratii ori. Cele doua matrici sunt shared, iar iteratorii si numarul de vecini 
sunt variabile locale fiecarui thread. Am paralelizat cele doua nested loops cu #pragma omp for si am pus 
si collapse(2), pentru a paraleliza atat for-ul din exterior, cat si cel din interior, si nu doar pe cel din 
exterior. Bordarea matricei si incrementarea iteratorului k o fac secvential prin #pragma omp single.

	De asemenea, am adaugat in arhiva fisierul de input pe care am testat si scriptul cu care am rulat. 
Si am adaugat in Makefile la compilarea resurselor si -static-libstdc++, deoarece nu puteam rula pe fep(am folosit streams si getline).
Si am masurat timpul de executie folosind time ./executabil utilizand coada de procesare ibm-nehalem.q. Timpul este afisat in fisierul script.sh.e<job_id>.