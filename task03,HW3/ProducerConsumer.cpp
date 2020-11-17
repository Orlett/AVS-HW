#include <iostream>
#include <queue>
#include <mutex>
#include <random>
#include <future>

using namespace std;

mutex m;
//переменная для контроля потоков petrov и necheporchuk
int semaphore = 1;
//очередь для подсчета груза
queue<int> taken_property;
//количество добычи
int loads = 0;
//генератор случайных чисел
mt19937 gen((int)time(0));
uniform_int_distribution<int> uid(5, 30);
int max_count = uid(gen);

//class producer
class Ivanov {
public:
	explicit Ivanov() {}
	void action() {
		//симуляции кражи первого груза
		this_thread::sleep_for(5000ms);
		while (true) {
			m.lock();
			//ценность груза
			int elem = uid(gen);
			//добавляем в очередь
			taken_property.push(elem);
			++loads;
			cout << loads << " load. Ivanov delivered a load of value " << elem << "\n";
			m.unlock();
			//отправляем ivanov за следующим грузом
			this_thread::sleep_for(5000ms);
			//если склад опустел
			if (loads == max_count)break;
			//заставляем ждать, если товарищи еще не доделали свои дела
			while (!taken_property.empty()) this_thread::sleep_for(10ms);
		}
	}
};
//class consumer petrov
class Petrov {
public:
	explicit Petrov() {}
	void action() {
		while (true) {
			m.lock();
			//если ivanov принес груз
			if (!taken_property.empty()) {
				//время на погрузку в грузовик
				this_thread::sleep_for(3000ms);
				int elem = taken_property.front();
				cout << "Petrov shippped a load into truck into truck of value " << elem << "\n";
				m.unlock();
				//уведомляем necheporchuk, что груз доставлен в грузовик
				--semaphore;
			}
			else m.unlock();
			//если груз закончился завершаем поток
			if (loads == max_count)break;
			//ждем, если necheporchuk еще не подсчитал предыдущий груз
			while (!semaphore) this_thread::sleep_for(10ms);
		}
	}
};
//class consumer necheporchuk
class Necheporchuk {
public:
	explicit Necheporchuk() {}
	void action() {
		while (true) {
			//ждем, если petrov еще не доставил груз в грузовик
			while (semaphore)this_thread::sleep_for(10ms);
			//время на подсчет суммы
			this_thread::sleep_for(2000ms);
			m.lock();
			int elem = taken_property.front();
			//удаляем груз из очереди
			taken_property.pop();
			total_cash += elem;
			cout << "Necheporchuk noticed load of value " << elem << ". Total cash: " << total_cash << "\n";
			//сообщаем petrov, что готовы считать следующий груз
			++semaphore;
			m.unlock();
			//завершаем поток, если груз закончился
			if (loads == max_count)break;
			//если груза еще нету, ждем
			while (taken_property.empty())this_thread::sleep_for(10ms);
		}
	}
private:
	long long total_cash = 0;
};

int main()
{
	cout << "The store has " << max_count << " loads\n";
	Ivanov ivanov;
	Petrov petrov;
	Necheporchuk necheporchuk;
	//ассинхронно вызываем потоки
	future<void>f1 = async([&ivanov]() {ivanov.action(); });
	future<void>f2 = async([&petrov]() {petrov.action(); });
	future<void>f3 = async([&necheporchuk]() {necheporchuk.action(); });
	return 0;
}
