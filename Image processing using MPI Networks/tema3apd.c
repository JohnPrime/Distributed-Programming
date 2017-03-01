#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FINISH_TAG 3

typedef struct {
	int *parents;
	int *neighbours;
	int nr_neighbours;
	int updated_nr_neighbours;
	bool isLeaf;
} Node;

typedef struct {
	char *magic_number;
	char *comment;
	char *img_size;
	char *max_value;
	int **matrix;
} Gimp;

typedef struct {
	char filter[20];
	char img_name[100];
	char res_img_name[105];
} ImgFilterOp;

typedef struct {
	int index;
	int **recv_matrix;
} ChunkMatrix;

int number_of_digits(int number) {
	int count = 0;

	while(number != 0) {
		number /= 10;
		count++;
	}

	return count;
}

Node get_neighbours(int rank, FILE *input, int nr_processes) {

	// citesc linie cu linie din fisier
	char *line = NULL;
	int first_number, i;
	size_t len = 0;
	ssize_t read;
	Node node;
	memset(&node, 0, sizeof(Node));

	// salvez intr-o structura, numarul de vecini, si structura arborelui(vectorul de parinti - parintele fiecarui nod din arbore)
	node.neighbours = (int *)malloc((nr_processes - 1) * sizeof(int));
	memset(node.neighbours, -1, (nr_processes - 1) * sizeof(*node.neighbours));

	node.parents = (int *)malloc(nr_processes * sizeof(int));
	memset(node.parents, -1, nr_processes * sizeof(*node.parents));

	node.isLeaf = false;

	while((read = getline(&line, &len, input)) != -1) {
		char *ptr;
		first_number = strtol(line, &ptr, 10);

		// daca primul numar de pe linie e rank-ul procesului
		if (first_number == rank) {
			// am gasit linia cu nodul asociat procesului
			int nr_digits = number_of_digits(first_number);
			char *pch;
			pch = strtok(line + nr_digits + 2, " ");
			i = 0;

			while (pch != NULL)
			{
				node.neighbours[i++] = atoi(pch);
				pch = strtok(NULL, " ");
			}

			// asta e numarul de vecini ai nodului
			node.nr_neighbours = i;

			break;
		}
	}

	if (line)
        free(line);

	return node;
}

int* merge(int *response, int *parents, int size) {
	int i;

	// actualizez vectorul de parinti, cu ceea ce am primit
	for (i = 0; i < size; i++) {
		parents[i] = (parents[i] == -1 && response[i] != -1) ? response[i] : parents[i];
	}

	return parents;
}

int* merge_statistics(int *nodes, int *recv_nodes, int size) {
	int i;

	// actualizez vectorul de statistici, cu ceea ce am primit
	for (i = 0; i < size; i++) {
		nodes[i] = (nodes[i] == -1 && recv_nodes[i] != -1) ? recv_nodes[i] : nodes[i];
	}

	return nodes;
}

int** convolution(int **matrix, int **out_matrix, int filter[3][3], int height, int width, int factor) {
	int i, j, ii, jj, sum;

	for (i = 1; i < height - 1; i++) {
		for (j = 1; j < width - 1; j++) {
			sum = 0;
			// aplic filtrul asupra unui pixel din matrice la un moment dat
			for(ii = 0; ii < 3; ii++) {
				for (jj = 0; jj < 3; jj++) {
					// convolutie discreta
					sum += matrix[i - ii + 1][j - jj + 1] * filter[ii][jj];
				}
			}
			// impart rezultatul la factorul filtrului
			int val = sum / factor;
			if (val < 0) val = 0;
			if (val > 255) val = 255;
			out_matrix[i - 1][j - 1] = val;
		}
	}

	return out_matrix;
}

int main(int argc, char *argv[]) {

	int rank, i;
	int nProcesses;
	FILE *input = NULL;
	Node proc_node;
	memset(&proc_node, 0, sizeof(Node));

	// filtrele
	int smooth_filter[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
	int smooth_factor = 9;

	int blur_filter[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
	int blur_factor = 16;

	int sharpen_filter[3][3] = {{0, -2, 0}, {-2, 11, -2}, {0, -2, 0}};
	int sharpen_factor = 3;

	int mean_removal_filter[3][3] = {{-1, -1, -1}, {-1, 9, -1}, {-1, -1, -1}};
	int mean_removal_factor = 1;

	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Request request;


	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

	input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("Error opening file");
		exit(-1);
	}
	proc_node = get_neighbours(rank, input, nProcesses);

	if (rank == 0) {
		for (i = 0; i < proc_node.nr_neighbours; i++) {
			// trimit proba(vectorul de parinti) la toti copii
			MPI_Send(proc_node.parents, nProcesses, MPI_INT, proc_node.neighbours[i], 0, MPI_COMM_WORLD);
		}

		for (i = 0; i < proc_node.nr_neighbours; i++) {
			// primesc raspunsul(vectorul de parinti) de la toti copii
			// unesc raspunsurile in proc_node.parents
			if (proc_node.neighbours[i] != proc_node.parents[rank]) {
				int response[nProcesses];
				MPI_Recv(response, nProcesses, MPI_INT, proc_node.neighbours[i], 0, MPI_COMM_WORLD, &status);
				proc_node.parents = merge(response, proc_node.parents, nProcesses);
			}
		}
	} else {
		MPI_Recv(proc_node.parents, nProcesses, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

		// marchez nodul sursa ca si parinte al acestui nod
		proc_node.parents[rank] = status.MPI_SOURCE;

		// trimit proba(vectorul de parinti) la toti copii
		for (i = 0; i < proc_node.nr_neighbours; i++) {
			// daca nodul nu este parinte, ci un copil
			if (proc_node.neighbours[i] != proc_node.parents[rank]) {
				MPI_Send(proc_node.parents, nProcesses, MPI_INT, proc_node.neighbours[i], 0, MPI_COMM_WORLD);
			}
		}

		for (i = 0; i < proc_node.nr_neighbours; i++) {
			// primesc raspunsul(vectorul de parinti) de la toti copii
			// unesc raspunsurile in proc_node.parents
			if (proc_node.neighbours[i] != proc_node.parents[rank]) {
				int response[nProcesses];
				MPI_Recv(response, nProcesses, MPI_INT, proc_node.neighbours[i], 0, MPI_COMM_WORLD, &status);
				proc_node.parents = merge(response, proc_node.parents, nProcesses);
			}
		}

		// trimit parintelui ce am
		MPI_Send(proc_node.parents, nProcesses, MPI_INT, proc_node.parents[rank], 0, MPI_COMM_WORLD);
	}

	if (rank == 0) {
		printf("Arborele de acoperire: \n");
		for (i = 0; i < nProcesses; i++) {
			printf("%d ", proc_node.parents[i]);
		}
		printf("\n");
	}

	// trimit arborele de acoperire si la celelalte noduri(procese)
	if (rank == 0) {
		for (i = 0; i < proc_node.nr_neighbours; i++) {
			// nodul radacina trimite arborele la vecinii(copii) lui
			MPI_Send(proc_node.parents, nProcesses, MPI_INT, proc_node.neighbours[i], 1, MPI_COMM_WORLD);
		}
	} else {
		MPI_Recv(proc_node.parents, nProcesses, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);

		// nodul curent trimite si el arborele la toti copii, mai putin la parinte
		for (i = 0; i < proc_node.nr_neighbours; i++) {
			if (proc_node.neighbours[i] != proc_node.parents[rank] && proc_node.parents[proc_node.neighbours[i]] == rank) {
				int k = MPI_Send(proc_node.parents, nProcesses, MPI_INT, proc_node.neighbours[i], 1, MPI_COMM_WORLD);
			}
		}
	}

	proc_node.updated_nr_neighbours = proc_node.nr_neighbours;

	for (i = 0; i < proc_node.nr_neighbours; i++) {
		// daca nu exista nicio legatura (de parinte sau copil) intre nodul rank si vreunul din vecini
		// sterg vecinul din lista de adiacenta a nodului(il marchez cu -1 in proc_node.neighbours)
		if (proc_node.neighbours[i] != proc_node.parents[rank] && proc_node.parents[proc_node.neighbours[i]] != rank) {
			proc_node.neighbours[i] = -1;
			proc_node.updated_nr_neighbours--;
		}
	}

	// daca numarul de vecini ramasi este egal cu 1, atunci clar nodul din arbore este frunza
	// mai putin radacina
	if (proc_node.updated_nr_neighbours == 1 && rank != 0) {
		proc_node.isLeaf = true;
	}

	// citire input din fisierul imagini.in, trimitere blocuri catre vecini
	if (rank == 0) {

		Gimp gimp;

		FILE *gimp_input = NULL, *imagini = NULL;

		imagini = fopen(argv[2], "r");

		int NF, j;

		int imgs_res = fscanf(imagini, "%d", &NF);

		// trimit numarul de imagini la copii
		for (i = 0; i < proc_node.nr_neighbours; i++) {
			if (proc_node.neighbours[i] != -1) {
				MPI_Send(&NF, 1, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD);
			}
		}

		// procesez fiecare imagine din imagini.in
		for(j = 0; j < NF; j++) {
			ImgFilterOp img_op;
			int filter_res = fscanf(imagini, "%s %s %s", img_op.filter, img_op.img_name, img_op.res_img_name);

			gimp_input = fopen(img_op.img_name, "r");

			if (gimp_input == NULL) {
				perror("Error opening file");
				exit(-1);
			}

			size_t len = 0;
			ssize_t read;

			int width, height;

			gimp.magic_number = NULL;
			gimp.comment = NULL;
			gimp.img_size = NULL;
			gimp.max_value = NULL;
			gimp.matrix = NULL;

			// preiau din fisierul imagine, primele date
			getline(&gimp.magic_number, &len, gimp_input);
			getline(&gimp.comment, &len, gimp_input);
			getline(&gimp.img_size, &len, gimp_input);
			int res = sscanf(gimp.img_size, "%d %d", &width, &height);
			getline(&gimp.max_value, &len, gimp_input);

			int l, c;

			// alocare matrice(blocurile din matrice sunt adiacente)
			gimp.matrix = (int **)malloc((height + 2) * sizeof(int *));
			int *line_data = (int *) malloc((height + 2) * (width + 2) * sizeof(int));

			for (l = 0; l < height + 2; l++) {
				gimp.matrix[l] = &(line_data[(width + 2) * l]);
			}

			// matricea este umpluta cu pixeli din imagine si bordata pe margini cu zerouri
			// citesc valoare cu valoare din fisier
			for (l = 0; l < height + 2; l++) {
				for (c = 0; c < width + 2; c++) {
					if (l == 0 || l == height + 1)
						gimp.matrix[l][c] = 0;
					else if (c == 0 || c == width + 1)
						gimp.matrix[l][c] = 0;
					else {
						int cell_res = fscanf(gimp_input, "%d", &gimp.matrix[l][c]);
					}
				}
			}

			// trimit bucati de matrice catre copii
			// frunza trebuie sa faca prelucrari si sa trimita catre parinte
			// parintele va unifica portiunile de matrice si va trimite mai departe catre parintele sau
			// care va unifica la randul lui portiunile s.a.m.d.
			// rank-ul 0 trebuie sa primeasca intreaga matrice la sfarsit si sa scrie in fisier
			int child_nr = 0, index = 1;
			int chunk_size = height / proc_node.updated_nr_neighbours;
			int temp_chunk_size = chunk_size;

			for (i = 0; i < proc_node.nr_neighbours; i++) {
				if (proc_node.neighbours[i] != -1) {
					MPI_Send(&width, 1, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD);

					// daca au mai ramas linii
					// adun restul de linii ramase la temp_chunk_size pe care le trimit ultimului copil
					if (child_nr == proc_node.updated_nr_neighbours - 1 && (height - (child_nr + 1) * temp_chunk_size) != 0) {
						temp_chunk_size += height - (child_nr + 1) * temp_chunk_size;
					}
					MPI_Send(&temp_chunk_size, 1, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD);
					MPI_Send(&index, 1, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD);
					MPI_Send(img_op.filter, 20, MPI_CHAR, proc_node.neighbours[i], 2, MPI_COMM_WORLD);

					index++;

					// matricea care va fi trimisa, va porni de la linia temp_chunk_size * child_nr
					// cand sunt la ultimul copil, mut pozitia pointerului tot cu temp_chunk_size, dar fara acel rest adunat
					MPI_Send(&((gimp.matrix + chunk_size * child_nr))[0][0], (temp_chunk_size + 2) * (width + 2), MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD);
					child_nr++;
				}
			}

			// aici trebuie sa primesc de la copii matricile si sa le unesc
			ChunkMatrix *chunk_matrix_array = NULL;
			chunk_matrix_array = (ChunkMatrix *)malloc(proc_node.updated_nr_neighbours * sizeof(ChunkMatrix));
			memset(chunk_matrix_array, -1, proc_node.updated_nr_neighbours * sizeof(*chunk_matrix_array));

			int child_nr_recv = 0;
			// modific doar temp_chunk_size acum
			temp_chunk_size = chunk_size;

			for (i = 0; i < proc_node.nr_neighbours; i++) {
				if (proc_node.neighbours[i] != -1 && proc_node.neighbours[i] != proc_node.parents[rank]) {
					chunk_matrix_array[child_nr_recv].recv_matrix = NULL;

					if (child_nr_recv == proc_node.updated_nr_neighbours - 1) {
						temp_chunk_size += height - (child_nr_recv + 1) * temp_chunk_size;
					}

					// alocare matrice
					chunk_matrix_array[child_nr_recv].recv_matrix = (int **)malloc(temp_chunk_size * sizeof(int *));
					int *recv_line_data = (int *) malloc(temp_chunk_size * width * sizeof(int));

					for (l = 0; l < temp_chunk_size; l++) {
						chunk_matrix_array[child_nr_recv].recv_matrix[l] = &(recv_line_data[width * l]);
					}

					// primesc indicele
					MPI_Recv(&chunk_matrix_array[child_nr_recv].index, 1, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD, &status);

					// primesc fasia de matrice
					MPI_Recv(&((chunk_matrix_array[child_nr_recv].recv_matrix)[0][0]), temp_chunk_size * width, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD, &status);

					child_nr_recv++;
				}
			}

			// unesc portinuile de matrice in ordinea in care le-am trimis(in functie de index)
			int **resulted_matrix = NULL;

			resulted_matrix = (int **)malloc(height * sizeof(int *));
			int *resulted_line_data = (int *) malloc(height * width * sizeof(int));

			for (l = 0; l < height; l++) {
				resulted_matrix[l] = &(resulted_line_data[width * l]);
			}

			int kk_index, ll, cc;
			ChunkMatrix aux_chunk_matrix;

			// sortez vectorul in functie de indici, folosind o simpla sortare
			for (ll = 0; ll < child_nr_recv; ll++) {
				for(cc = ll + 1; cc < child_nr_recv; cc++) {
					if (chunk_matrix_array[ll].index > chunk_matrix_array[cc].index) {
						aux_chunk_matrix = chunk_matrix_array[ll];
						chunk_matrix_array[ll] = chunk_matrix_array[cc];
						chunk_matrix_array[cc] = aux_chunk_matrix;
					}
				}
			}

			// modific doar chunk_size acum
			temp_chunk_size = chunk_size;

			// unesc matricile intr-una
			for (kk_index = 0; kk_index < child_nr_recv; kk_index++) {
				// ultimul copil a primit si restul de chunk, daca nu s-a impartit corect la toti
				if (kk_index == child_nr_recv - 1) {
					chunk_size += height - (kk_index + 1) * chunk_size;
				}
				for (l = 0; l < chunk_size; l++) {
					for (c = 0; c < width; c++) {
						resulted_matrix[(temp_chunk_size * kk_index) + l][c] = chunk_matrix_array[kk_index].recv_matrix[l][c];
					}
				}
			}

			// eliberez memoria matricilor alocat in structura
			for (kk_index = 0; kk_index < child_nr_recv; kk_index++) {
				free(chunk_matrix_array[kk_index].recv_matrix[0]);
				free(chunk_matrix_array[kk_index].recv_matrix);
			}

			FILE *gimp_output = NULL;

			gimp_output = fopen(img_op.res_img_name, "w");

			if (gimp_output == NULL) {
				perror("Error opening file");
				exit(-1);
			}

			// scriu primele linii din fisierul imagine
			fprintf(gimp_output, "%s", gimp.magic_number);
			fprintf(gimp_output, "%s", gimp.comment);
			fprintf(gimp_output, "%s", gimp.img_size);
			fprintf(gimp_output, "%s", gimp.max_value);

			// scriu matricea
			for (l = 0; l < height; l++) {
				for (c = 0; c < width; c++) {
					fprintf(gimp_output, "%d\n", resulted_matrix[l][c]);
				}
			}

			// eliberare memorie si inchidere fisiere
			fclose(gimp_input);
			fclose(gimp_output);

			free(chunk_matrix_array);
			free(resulted_matrix[0]);
			free(resulted_matrix);

			free(gimp.matrix[0]);
			free(gimp.matrix);

			free(gimp.magic_number);
			free(gimp.comment);
			free(gimp.img_size);
			free(gimp.max_value);
		}

		fclose(imagini);

		char finish_msg[7] = "FINISH";

		// trimit mesajul de terminare
		for (i = 0; i < proc_node.nr_neighbours; i++) {
			if (proc_node.neighbours[i] != -1) {
				MPI_Send(finish_msg, 7, MPI_CHAR, proc_node.neighbours[i], FINISH_TAG, MPI_COMM_WORLD);
			}
		}

		int *nodes = NULL;
		nodes = (int *)malloc(nProcesses * sizeof(int));
		memset(nodes, -1, nProcesses * sizeof(*nodes));

		// primesc numarul total de linii procesate de la copii
		for (i = 0; i < proc_node.nr_neighbours; i++) {
			if (proc_node.neighbours[i] != -1 && proc_node.neighbours[i] != proc_node.parents[rank]) {
				int recv_nodes[nProcesses];
				MPI_Recv(recv_nodes, nProcesses, MPI_INT, proc_node.neighbours[i], 4, MPI_COMM_WORLD, &status);
				nodes = merge_statistics(nodes, recv_nodes, nProcesses);
			}
		}

		// nodul radacina(0), are 0 linii procesate
		nodes[rank] = 0;

		int x;

		FILE *statistic_output = NULL;

		statistic_output = fopen(argv[3], "w");

		if (statistic_output == NULL) {
			perror("Error opening file");
			exit(-1);
		}

		// scriu statisticile in fisier
		for (x = 0; x < nProcesses; x++) {
			fprintf(statistic_output, "%d: %d\n", x, nodes[x]);
		}

		fclose(statistic_output);
		free(nodes);

	} else {
		int **matrix = NULL;
		int width, chunk_height, temp_chunk_height, l, c, index, NF, NF_index;
		char filter[20];
		char finish_msg[7];
		int nr_lines = 0;

		// am primit numarul de imagini
		MPI_Recv(&NF, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);

		// trimit numarul de imagini la copiii
		if (!proc_node.isLeaf) {
			for (i = 0; i < proc_node.nr_neighbours; i++) {
				if (proc_node.neighbours[i] != -1 && proc_node.neighbours[i] != proc_node.parents[rank]) {
					MPI_Send(&NF, 1, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD);
				}
			}
		}

		for (NF_index = 0; NF_index < NF; NF_index++) {

			// receptionez datele primite
			MPI_Recv(&width, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);
			MPI_Recv(&chunk_height, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);
			MPI_Recv(&index, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);
			MPI_Recv(filter, 20, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);

			// alocare matrice(blocurile din matrice sunt adiacente)
			matrix = (int **)malloc((chunk_height + 2) * sizeof(int *));
			int *line_data = (int *) malloc((chunk_height + 2) * (width + 2) * sizeof(int));

			for (l = 0; l < chunk_height + 2; l++) {
				matrix[l] = &(line_data[(width + 2) * l]);
			}

			// primesc fasia de matrice
			MPI_Recv(&(matrix[0][0]), (chunk_height + 2) * (width + 2), MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);

			int full_height = chunk_height;

			// daca nodul nu e frunza, scindez matricea in bucati si le trimit mai departe la copii nodului
			if (!proc_node.isLeaf) {
				int k = 0, new_index = 1;
				chunk_height /= (proc_node.updated_nr_neighbours - 1);
				temp_chunk_height = chunk_height;

				for (i = 0; i < proc_node.nr_neighbours; i++) {
					if (proc_node.neighbours[i] != -1 && proc_node.neighbours[i] != proc_node.parents[rank]) {
						MPI_Send(&width, 1, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD);

						// daca au mai ramas linii
						// adun restul de linii ramase la temp_chunk_height pe care le trimit ultimului copil
						if (k == proc_node.updated_nr_neighbours - 2 && (full_height - (k + 1) * temp_chunk_height) != 0) {
							temp_chunk_height += full_height - (k + 1) * temp_chunk_height;
						}
						MPI_Send(&temp_chunk_height, 1, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD);
						MPI_Send(&new_index, 1, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD);
						MPI_Send(filter, 20, MPI_CHAR, proc_node.neighbours[i], 2, MPI_COMM_WORLD);

						new_index++;

						// matricea care va fi trimisa, va porni de la linia temp_chunk_height * k
						// cand sunt la ultimul copil, mut pozitia pointerului tot cu chunk_size, dar fara acel rest adunat
						MPI_Send(&((matrix + chunk_height * k)[0][0]), (temp_chunk_height + 2) * (width + 2), MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD);

						k++;
					}
				}

				// aici trebuie sa primesc de la copii matricile si sa le unesc
				ChunkMatrix *chunk_matrix_array = NULL;
				chunk_matrix_array = (ChunkMatrix *)malloc((proc_node.updated_nr_neighbours - 1) * sizeof(ChunkMatrix));
				memset(chunk_matrix_array, -1, (proc_node.updated_nr_neighbours - 1) * sizeof(*chunk_matrix_array));

				int kk = 0;
				temp_chunk_height = chunk_height;
				for (i = 0; i < proc_node.nr_neighbours; i++) {
					if (proc_node.neighbours[i] != -1 && proc_node.neighbours[i] != proc_node.parents[rank]) {
						chunk_matrix_array[kk].recv_matrix = NULL;

						if (kk == proc_node.updated_nr_neighbours - 2) {
							temp_chunk_height += full_height - (kk + 1) * temp_chunk_height;
						}

						// alocare matrice
						chunk_matrix_array[kk].recv_matrix = (int **)malloc(temp_chunk_height * sizeof(int *));
						int *recv_line_data = (int *) malloc(temp_chunk_height * width * sizeof(int));

						for (l = 0; l < temp_chunk_height; l++) {
							chunk_matrix_array[kk].recv_matrix[l] = &(recv_line_data[width * l]);
						}

						// primesc indicele
						MPI_Recv(&chunk_matrix_array[kk].index, 1, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD, &status);

						// primesc fasia de matrice
						MPI_Recv(&((chunk_matrix_array[kk].recv_matrix)[0][0]), temp_chunk_height * width, MPI_INT, proc_node.neighbours[i], 2, MPI_COMM_WORLD, &status);

						kk++;
					}
				}

				// unesc fasiile si trimit indicele primit de la parinte impreuna cu matricea rezultata din unire la parinte
				int **resulted_matrix = NULL;

				resulted_matrix = (int **)malloc(full_height * sizeof(int *));
				int *resulted_line_data = (int *) malloc(full_height * width * sizeof(int));

				for (l = 0; l < full_height; l++) {
					resulted_matrix[l] = &(resulted_line_data[width * l]);
				}

				int kk_index, ll, cc;
				ChunkMatrix aux_chunk_matrix;

				// sortez vectorul in functie de indici, folosind o simpla sortare
				for (ll = 0; ll < kk; ll++) {
					for(cc = ll + 1; cc < kk; cc++) {
						if (chunk_matrix_array[ll].index > chunk_matrix_array[cc].index) {
							aux_chunk_matrix = chunk_matrix_array[ll];
							chunk_matrix_array[ll] = chunk_matrix_array[cc];
							chunk_matrix_array[cc] = aux_chunk_matrix;
						}
					}
				}

				int aux_chunk_height = chunk_height;

				// unesc matricile intr-una ca in nodul cu rank 0
				for (kk_index = 0; kk_index < kk; kk_index++) {
					if (kk_index == kk - 1) {
						chunk_height += full_height - (kk_index + 1) * chunk_height;
					}
					for (l = 0; l < chunk_height; l++) {
						for (c = 0; c < width; c++) {
							resulted_matrix[(aux_chunk_height * kk_index) + l][c] = chunk_matrix_array[kk_index].recv_matrix[l][c];
						}
					}
				}

				// eliberez memoria ocupata de matricile alocate in structura
				for (kk_index = 0; kk_index < kk; kk_index++) {
					free(chunk_matrix_array[kk_index].recv_matrix[0]);
					free(chunk_matrix_array[kk_index].recv_matrix);
				}

				MPI_Send(&index, 1, MPI_INT, proc_node.parents[rank], 2, MPI_COMM_WORLD);
				MPI_Send(&(resulted_matrix[0][0]), full_height * width, MPI_INT, proc_node.parents[rank], 2, MPI_COMM_WORLD);
				free(resulted_matrix[0]);
				free(resulted_matrix);
				free(chunk_matrix_array);
			} else { // daca nodul e frunza, aplic filtrul asupra matricii si o trimit la parinte
				int **out_matrix = NULL;

				out_matrix = (int **)malloc(chunk_height * sizeof(int *));
				int *out_line_data = (int *) malloc(chunk_height * width * sizeof(int));

				for (l = 0; l < chunk_height; l++) {
					out_matrix[l] = &(out_line_data[width * l]);
				}

				// aplic filtrul asupra fasiei de matrice
				if (!strcmp(filter, "smooth")) {
					out_matrix = convolution(matrix, out_matrix, smooth_filter, chunk_height + 2, width + 2, smooth_factor);
				} else if (!strcmp(filter, "blur")) {
					out_matrix = convolution(matrix, out_matrix, blur_filter, chunk_height + 2, width + 2, blur_factor);
				} else if (!strcmp(filter, "sharpen")) {
					out_matrix = convolution(matrix, out_matrix, sharpen_filter, chunk_height + 2, width + 2, sharpen_factor);
				} else if (!strcmp(filter, "mean_removal")) {
					out_matrix = convolution(matrix, out_matrix, mean_removal_filter, chunk_height + 2, width + 2, mean_removal_factor);
				}

				// incrementez numarul de linii care au fost procesate de frunza
				nr_lines += chunk_height;

				// trimit la parinte indexul si matricea prelucrata
				MPI_Send(&index, 1, MPI_INT, proc_node.parents[rank], 2, MPI_COMM_WORLD);
				MPI_Send(&(out_matrix[0][0]), chunk_height * width, MPI_INT, proc_node.parents[rank], 2, MPI_COMM_WORLD);
				free(out_matrix[0]);
				free(out_matrix);
			}
			free(matrix[0]);
			free(matrix);
		}

		// nodul primeste mesaj cu tag de terminare
		MPI_Recv(finish_msg, 7, MPI_CHAR, MPI_ANY_SOURCE, FINISH_TAG, MPI_COMM_WORLD, &status);

		if (!proc_node.isLeaf) {
			// trimit mesajul cu tag de terminare mai departe
			for (i = 0; i < proc_node.nr_neighbours; i++) {
				if (proc_node.neighbours[i] != -1 && proc_node.neighbours[i] != proc_node.parents[rank]) {
					MPI_Send(finish_msg, 7, MPI_CHAR, proc_node.neighbours[i], FINISH_TAG, MPI_COMM_WORLD);
				}
			}

			int *nodes = NULL;
			nodes = (int *)malloc(nProcesses * sizeof(int));
			memset(nodes, -1, nProcesses * sizeof(*nodes));

			// primesc numarul total de linii procesate de la copii de la copiii
			for (i = 0; i < proc_node.nr_neighbours; i++) {
				if (proc_node.neighbours[i] != -1 && proc_node.neighbours[i] != proc_node.parents[rank]) {
					int recv_nodes[nProcesses];
					MPI_Recv(recv_nodes, nProcesses, MPI_INT, proc_node.neighbours[i], 4, MPI_COMM_WORLD, &status);
					nodes = merge_statistics(nodes, recv_nodes, nProcesses);
				}
			}

			// nodurile care nu sunt frunze au 0
			nodes[rank] = 0;
			MPI_Send(nodes, nProcesses, MPI_INT, proc_node.parents[rank], 4, MPI_COMM_WORLD);
			free(nodes);
		} else {
			int *nodes = NULL;
			nodes = (int *)malloc(nProcesses * sizeof(int));
			memset(nodes, -1, nProcesses * sizeof(*nodes));

			// frunza trimite numarul de linii prelucrate
			nodes[rank] = nr_lines;
			MPI_Send(nodes, nProcesses, MPI_INT, proc_node.parents[rank], 4, MPI_COMM_WORLD);
			free(nodes);
		}
	}

	free(proc_node.parents);
	free(proc_node.neighbours);
	fclose(input);

	MPI_Finalize();

	return 0;
}
