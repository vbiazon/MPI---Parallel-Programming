#include <iostream>
#include <random>
#include <chrono>
#include <math.h>
#include <mpi.h>
#define MASTER 0
using namespace std;
using namespace std::chrono;
using std::scientific;
using std::fixed;
using std::ios;


float random(float init, float max) { //utiliza o mersenne twister 19937 para gerar numeros pseudo randomicos
	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 generator(seed1);
	double random = (double)generator() / generator.max();
	return init + (double)random * (max - init);
}

float funcao(float x, float y, float z) { //  calcula o valor da funcao para entradas x, y e z
	return pow(z, 2) + pow(sqrt(pow(x, 2) + pow(y, 2)) - 3, 2);
}

bool isToroide(float resultado) { //verifica se o resultado da funcao esta dentro do toroide
	return resultado <= 1;
}

	//define intervalos de integracao
	float xUm = 1;
	float xD = 4;

	float yUm = -3;
	float yD = 4;

	float zUm = -1;
	float zD = 1;

	float totaltask = 0;
	float f2task = 0;

void MonteCarloIntegral(int numeros){ // calcula taxa de acerto no toroide pelo metodo de montecarlo
	float x = 0;
	float y = 0;
	float z = 0;
	float resultado = 0;
	float contador = 0;
	for (; contador <= numeros; contador++) { //calcula n numeros de amostras com entradas x y e z randomicas e verifica se esta no toroide
		x = random(xUm, xD);
		y = random(yUm, yD);
		z = random(zUm, zD);

		resultado = funcao(x, y, z);

		if (isToroide(resultado)) // se estiver no toroide soma 1 no total
			totaltask += 1;
			f2task += pow(1, 2);
	}
}


int main(int argc, char *argv[]) {
	int tasks, taskid;

	MPI_Init(&argc, &argv); // inicializa o MPI
	MPI_Comm_size(MPI_COMM_WORLD, &tasks); //determina numero de tasks
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid); //assume numeracao de tasks

	int numeros = 100000; //define variaveis auxiliares 
	if(argc > 1)
		numeros = atoi(argv[1]);
	int contador = 0;
	float total = 0;
	float f2 = 0;
	float f = 0;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(t2 - t1).count();

		t1 = high_resolution_clock::now();
	MonteCarloIntegral(numeros / tasks); //chama a funcao e divide a tarefa entre as tasks
	MPI_Reduce(&totaltask, &total, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD); //recebe informacao das variaveis totaltask para a variavel total
	MPI_Reduce(&f2task, &f2, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD); //recebe informacao das variaveis f2task para a variavel f2
		t2 = high_resolution_clock::now();

	if(taskid == MASTER) { //verifica se a task é o MASTER (taskid = 0) para executar apenas uma vez o o calculo final e print na tela
		f2 = f2 / numeros; // calcula media das amostras quadradas
		f = total / numeros; // calcula media de acertos dentro do toroide das amostras
		float V = (xD - xUm) * (yD - yUm) * (zD - zUm); //calcula volume de integracao
		float erro = V * sqrt((f2 - (float)pow(f, 2)) / numeros); // calcula erro estimado
		//mostra na tela taxa de acerto dentro do toroide, resultado da integracao e erro estimado
		cout << "Taxa de acerto no toroide: "<< f << " Resultado: " << V * (total / numeros) << endl;
		cout << "Erro: " << erro << endl;
		cout << "Numero de amostras: " << numeros << endl;
		cout << "Tasks: " << tasks << endl;
		duration = duration_cast<microseconds>(t2 - t1).count();
		cout << "Tempo de execucao BFS: " << duration / 1000 << "ms" << endl << endl;
		}
	MPI_Finalize(); //finaliza execucao do MPI
}
