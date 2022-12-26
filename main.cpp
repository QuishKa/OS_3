#include <windows.h>
#include <iostream>
#include <string>
#include <bitset>
#include <math.h>
#include <omp.h>

using namespace std;

void ThreadVariant();
void OpenMPVariant();

const int N = 100000000;
const double invertN = 1.0 / N;
const int BLOCK = 308080; // номер студ. билета 030808
volatile LONG iteration = -1;

double* results;

double realPI = 3.1415926535;

DWORD WINAPI ThreadProc(LPVOID lpParameter) {
	int i, start, end, tmp, id;
	double x, res = 0;
	bool stop = false;
	id = *((int*)lpParameter);

	while (!stop) {
		tmp = InterlockedIncrement(&iteration);
		start = BLOCK * tmp; // start end calcs
		end = BLOCK * (tmp + 1);
		if (end > N) {
			end = N;
			stop = true;
		}
		// PI calc
		for (i = start; i < end; ++i) {
			x = (i + 0.5) * invertN;
			res += 4.0 / (1.0 + x * x);
		}
	}
	results[id] = res;

	return 0;
}

int main()
{
	int input;
	bool work = true;
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	while (work) {
		system("cls");
		printf("Выберите метод рассчета:\n"
			"1 - WinAPI\n"
			"2 - OpenMP\n"
			"0 - выход\n");
		cin >> input;

		switch (input)
		{
		case 1:
			ThreadVariant();
			break;
		case 2:
			OpenMPVariant();
			break;
		case 0:
			work = false;
			break;
		default:
			printf("Неверный ввод.\n");
			break;
		}
		printf("\nНажмите любую кнопку...");

		getchar();
		getchar();
	}

	return 0;
}

void ThreadVariant() {
	int threads;
	int* id;
	double PI = 0;
	HANDLE* THREADS;
	DWORD* THREADSID;
	DWORD startTime, endTime;

	printf("Введите количество потоков: ");
	cin >> threads;

	THREADS = new HANDLE[threads];
	THREADSID = new DWORD[threads];
	id = new int[threads];
	results = new double[threads];

	for (int i = 0; i < threads; ++i) {
		id[i] = i;
		results[i] = 0;
		THREADS[i] = CreateThread(NULL, 0, ThreadProc, &id[i], CREATE_SUSPENDED, &THREADSID[i]);
	}

	startTime = (DWORD)GetTickCount64();

	for (int i = 0; i < threads; ++i) {
		ResumeThread(THREADS[i]);
	}

	WaitForMultipleObjects(threads, THREADS, true, INFINITE);

	for (int i = 0; i < threads; ++i) {
		PI += results[i];
	}

	PI *= invertN;

	endTime = (DWORD)GetTickCount64();

	printf("Затрачено времени: %3.3f\n"
		"     PI: %3.11f...\n"
		"Real PI: %3.10f...\n",
		(float)(endTime - startTime) / 1000, PI, realPI);
	for (int i = 0; i < threads; ++i) {
		CloseHandle(THREADS[i]);
	}
	iteration = -1;
}

void OpenMPVariant() {
	int threads;
	double PI = 0, x;
	DWORD startTime, endTime;

	printf("Введите количество потоков: ");
	cin >> threads;

	startTime = (DWORD)GetTickCount64();

	#pragma omp parallel for schedule(dynamic,BLOCK) private(x) num_threads(threads) reduction (+:PI)
	
	for (long i = 0; i < N; ++i) {
		x = (i + 0.5) * invertN;
		PI = PI + 4.0 / (1.0 + x * x);
	}

	PI *= invertN;

	endTime = (DWORD)GetTickCount64();

	printf("Затрачено времени: %3.3f\n"
		"     PI: %3.11f...\n"
		"Real PI: %3.10f...\n",
		(float)(endTime - startTime) / 1000, PI, realPI);
}