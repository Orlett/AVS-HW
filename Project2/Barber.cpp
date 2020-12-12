#include <iostream>
#include <queue>
#include <random>
#include <future>
#include<omp.h>

using namespace std;
//семафор для парикмахера
int barberSemaphore = 1;
//генератор случайных чисел
mt19937 gen((int)time(0));
uniform_int_distribution<int> uid(5, 50);
const int max_count = uid(gen);
//массив семафоров для каждого клиента
vector<bool>semaphors;

//класс клиента
class Client {
public:
	explicit Client(int i) :i(i) {}
	void action() {
		//ждем своей очереди
		while (semaphors[i] == 0)this_thread::sleep_for(10ms);
		cout << "Клиент " << i + 1 << " сел в кресло" << "\n";
		//передаем управление потоку парикмахера
		semaphors[i] = 0;
		barberSemaphore = 1;
		//ждем пока парикмахер закончит стрижку
		while (semaphors[i] == 0)this_thread::sleep_for(10ms);
	}
private:
	int i;
};

class Barber {
public:
	explicit Barber() {}
	//очередь клиентов
	queue<Client> clients;
	void action() {
		for (size_t i = 0; i < max_count; i++)
		{
			//спим пока нет посетителей
			if (clients.empty()) { cout << "Посетителей нет, можно поспать\n"; }
			while (clients.empty()) { this_thread::sleep_for(10ms); }
			cout << "\nПарикмахер приступает к клиенту " << clientsCount << "\n";
			//передаем управление потоку клиента 
			barberSemaphore = 0;
			semaphors[clientsCount - 1] = 1;
			//ждем пока клиент сядет в кресло
			while (barberSemaphore == 0)this_thread::sleep_for(10ms);
			cout << "Парикмахер стрижет клиента " << clientsCount << "\n";
			//стрижем случайное время
			this_thread::sleep_for(uid(gen) * 100ms);
			cout << "Стрижка окончена, до свидания, людей в очереди: " << clients.size() - 1 << "\n";
			//переходим к следующему клиента
			semaphors[clientsCount - 1] = 1;
			clients.pop();
			//увеличиваем количество обслуживаемых клиентов на 1
			clientsCount++;
		}
		cout << "\nРабочий день окончен";
	}
private:
	int clientsCount = 1;
};

int main()
{
	setlocale(LC_ALL, "Russian");
	cout << "Сегодня к парикмахеру записались " << max_count << " человек\n\n";
	//зануляем все семафоры клиентов
	for (size_t i = 0; i < max_count; i++)
	{
		semaphors.push_back(0);
	}
	Barber barber;
	//запускаем главный поток
	cout << "Парикмахер пришел на работу\n\n";
	future<void>barb = async([&barber]() {barber.action(); });
#pragma omp parallel num_threads(max_count)
	{
#pragma omp for
		//запускаем потоки клиентов
		for (int i = 0; i < max_count; i++)
		{
			//имитируем время через которое приходят новые клиенты
			this_thread::sleep_for(i * 2500ms);
			Client client(i);

			//будим парикмахера если он уснул
			if (barber.clients.empty()) { cout << "\nКлиент " << i + 1 << " будит парикмахера\n"; }
			barber.clients.push(client);
			client.action();
		}
	}
	return 0;
}
