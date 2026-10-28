#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <deque>
#include <future>
#include <chrono>
#include <cmath>
#include <string>

// ==========================================
// ДОПОМІЖНІ ФУНКЦІЇ ДЛЯ ОБЧИСЛЕННЯ ПРОСТИХ ЧИСЕЛ
// ==========================================

// Перевірка числа на простоту
bool isPrime(long long n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// Знаходження n-го простого числа
long long getNthPrime(int n) {
    if (n <= 0) return 0;
    int count = 0;
    long long num = 1;
    while (count < n) {
        num++;
        if (isPrime(num)) {
            count++;
        }
    }
    return num;
}

// ==========================================
// ЗАВДАННЯ 1.2.1
// ==========================================
namespace Task1 {
    std::queue<int> data_queue;
    std::mutex mtx;
    bool is_ready = false;

    void DataPreparation() {
        std::cout << "--- [DataPreparation] Введіть цілі числа (введіть -1 для завершення):\n";
        int val;
        while (std::cin >> val && val != -1) {
            std::lock_guard<std::mutex> lock(mtx);
            data_queue.push(val);
        }
        {
            std::lock_guard<std::mutex> lock(mtx);
            is_ready = true;
        }
        std::cout << "--- [DataPreparation] Введення завершено. Прапорець встановлено.\n";
    }

    void DataProcessing() {
        std::cout << "--- [DataProcessing] Очікування на готовність даних...\n";
        while (true) {
            std::unique_lock<std::mutex> lk(mtx);
            if (is_ready) {
                break;
            }
            lk.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "--- [DataProcessing] Перевірка чисел у черзі на простоту:\n";
        std::lock_guard<std::mutex> lock(mtx);
        while (!data_queue.empty()) {
            int num = data_queue.front();
            data_queue.pop();
            if (isPrime(num)) {
                std::cout << "Число " << num << " є ПРОСТИМ.\n";
            }
            else {
                std::cout << "Число " << num << " НЕ є простим.\n";
            }
        }
    }

    void run() {
        std::cout << "\n=== ЗАВДАННЯ 1.2.1 ===\n";
        is_ready = false;
        while (!data_queue.empty()) data_queue.pop();

        std::thread tPrep(DataPreparation);
        tPrep.detach();

        std::thread tProc(DataProcessing);
        tProc.join();
    }
}

// ==========================================
// ЗАВДАННЯ 1.2.2
// ==========================================
namespace Task2 {
    std::mutex cv_m;
    std::condition_variable cv;
    int i = 0;

    void Waits(int id) {
        std::unique_lock<std::mutex> lk(cv_m);
        std::cout << "[Thread " << id << "] Входить у стан очікування (i = " << i << ")...\n";
        cv.wait(lk, [] { return i == 1; });
        std::cout << "[Thread " << id << "] Очікування завершене! (i = " << i << ")\n";
    }

    void Awake() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            std::lock_guard<std::mutex> lk(cv_m);
            std::cout << "\n[Awake] Перша спроба notify_all() (значення i = " << i << ")...\n";
        }
        cv.notify_all();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            std::lock_guard<std::mutex> lk(cv_m);
            i = 1;
            std::cout << "[Awake] Змінній i присвоєно значення 1. Друга спроба notify_all()...\n";
        }
        cv.notify_all();
    }

    void run() {
        std::cout << "\n=== ЗАВДАННЯ 1.2.2 ===\n";
        i = 0;
        std::thread t1(Waits, 1);
        std::thread t2(Waits, 2);
        std::thread t3(Waits, 3);
        std::thread tAwake(Awake);

        t1.join();
        t2.join();
        t3.join();
        tAwake.join();
    }
}

// ==========================================
// ЗАВДАННЯ 1.2.3
// ==========================================
namespace Task3 {
    std::mutex cv_m;
    std::condition_variable cv;
    int i = 0;

    void ThreadWorker(int id) {
        std::unique_lock<std::mutex> lk(cv_m);
        cv.wait(lk, [] { return i == 1; });
        std::cout << "Повідомлення з потоку " << id << std::endl;
    }

    void Notify() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            std::lock_guard<std::mutex> lk(cv_m);
            i = 1;
            std::cout << "[Notify] Встановлено i = 1, викликаємо notify_one()...\n";
        }
        cv.notify_one();
    }

    void run() {
        std::cout << "\n=== ЗАВДАННЯ 1.2.3 ===\n";
        i = 0;
        std::thread t1(ThreadWorker, 1);
        std::thread t2(ThreadWorker, 2);
        std::thread t3(ThreadWorker, 3);
        std::thread tNotify(Notify);

        tNotify.join();

        // Розблоковуємо решту потоків для завершення роботи
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        cv.notify_all();

        t1.join();
        t2.join();
        t3.join();
    }
}

// ==========================================
// ЗАВДАННЯ 1.2.4
// ==========================================
namespace Task4 {
    std::queue<int> data_queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool is_finished = false;

    void DataPreparation() {
        std::cout << "--- [DataPreparation] Введіть цілі числа (введіть -1 для завершення):\n";
        int val;
        while (std::cin >> val && val != -1) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                data_queue.push(val);
            }
            cv.notify_one();
        }
        {
            std::lock_guard<std::mutex> lock(mtx);
            is_finished = true;
        }
        cv.notify_all();
    }

    void DataProcessing() {
        while (true) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [] { return !data_queue.empty() || is_finished; });

            while (!data_queue.empty()) {
                int num = data_queue.front();
                data_queue.pop();
                lk.unlock();

                if (isPrime(num)) {
                    std::cout << "[DataProcessing] Число " << num << " є ПРОСТИМ.\n";
                }
                else {
                    std::cout << "[DataProcessing] Число " << num << " НЕ є простим.\n";
                }

                lk.lock();
            }

            if (is_finished && data_queue.empty()) {
                break;
            }
        }
        std::cout << "--- [DataProcessing] Обробку завершено.\n";
    }

    void run() {
        std::cout << "\n=== ЗАВДАННЯ 1.2.4 ===\n";
        is_finished = false;
        while (!data_queue.empty()) data_queue.pop();

        std::thread tPrep(DataPreparation);
        tPrep.detach();

        std::thread tProc(DataProcessing);
        tProc.join();
    }
}

// ==========================================
// ЗАВДАННЯ 1.2.5
// ==========================================
namespace Task5 {
    void runExperiment(std::launch policy, const std::string& policyName, int n) {
        std::cout << "\n--------------------------------------------\n";
        std::cout << "Запуск з політикою: " << policyName << " для n = " << n << "\n";

        auto t_start = std::chrono::high_resolution_clock::now();

        std::future<long long> fut = std::async(policy, getNthPrime, n);

        std::cout << "Оберіть математичну функцію від n (1 - sqrt, 2 - sin, 3 - ln): ";
        int choice;
        std::cin >> choice;

        double mathRes = 0.0;
        if (choice == 1) mathRes = std::sqrt(n);
        else if (choice == 2) mathRes = std::sin(n);
        else if (choice == 3) mathRes = std::log(n);

        std::cout << "Результат математичної функції: " << mathRes << std::endl;

        std::cout << "Очікування результату обчислення (fut.get())...\n";
        auto get_start = std::chrono::high_resolution_clock::now();
        long long primeRes = fut.get();
        auto get_end = std::chrono::high_resolution_clock::now();

        auto total_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> get_duration = get_end - get_start;
        std::chrono::duration<double, std::milli> total_duration = total_end - t_start;

        std::cout << "Знайдено " << n << "-те просте число: " << primeRes << "\n";
        std::cout << "Час очікування у fut.get(): " << get_duration.count() << " ms\n";
        std::cout << "Загальний час виконання: " << total_duration.count() << " ms\n";
    }

    void run() {
        std::cout << "\n=== ЗАВДАННЯ 1.2.5 ===\n";
        int n;
        std::cout << "Введіть номер простого числа (n, наприклад 150000): ";
        std::cin >> n;

        runExperiment(std::launch::deferred, "std::launch::deferred", n);
        runExperiment(std::launch::async, "std::launch::async", n);
    }
}

// ==========================================
// ЗАВДАННЯ 1.2.6
// ==========================================
namespace Task6 {
    std::deque<std::packaged_task<long long()>> task_deque;
    std::deque<int> n_deque;
    std::mutex deque_mtx;
    std::condition_variable cv;
    bool stop_worker = false;

    void WorkerThread() {
        while (true) {
            std::packaged_task<long long()> task;
            int n_val = 0;

            {
                std::unique_lock<std::mutex> lk(deque_mtx);
                cv.wait(lk, [] { return !task_deque.empty() || stop_worker; });

                if (stop_worker && task_deque.empty()) {
                    break;
                }

                task = std::move(task_deque.front());
                task_deque.pop_front();
                n_val = n_deque.front();
                n_deque.pop_front();
            }

            std::future<long long> fut = task.get_future();
            task();

            long long result = fut.get();
            {
                std::lock_guard<std::mutex> lock(deque_mtx);
                std::cout << "\n[Worker] " << n_val << "-те просте число дорівнює: " << result << std::endl;
            }
        }
    }

    void run() {
        std::cout << "\n=== ЗАВДАННЯ 1.2.6 ===\n";
        stop_worker = false;
        task_deque.clear();
        n_deque.clear();

        std::thread worker(WorkerThread);

        std::string input;
        std::cout << "Введіть n для обчислення n-го простого числа (або 'stop' для завершення):\n";

        while (true) {
            std::cout << "> ";
            std::cin >> input;
            if (input == "stop") break;

            try {
                int n = std::stoi(input);
                std::packaged_task<long long()> task([n]() { return getNthPrime(n); });

                {
                    std::lock_guard<std::mutex> lock(deque_mtx);
                    task_deque.push_back(std::move(task));
                    n_deque.push_back(n);
                }
                cv.notify_one();
            }
            catch (...) {
                std::cout << "Некоректне введення! Спробуйте ще раз або введіть 'stop'.\n";
            }
        }

        {
            std::lock_guard<std::mutex> lock(deque_mtx);
            stop_worker = true;
        }
        cv.notify_all();
        worker.join();
    }
}

// ==========================================
// ЗАВДАННЯ 1.2.7
// ==========================================
namespace Task7 {
    void Thread1(int n,
        std::promise<long long> prom1,
        std::promise<bool> promSync,
        std::promise<long long> prom2)
    {
        long long primeN = getNthPrime(n);
        prom1.set_value(primeN);

        promSync.set_value(true);

        long long primeN10 = getNthPrime(n * 10);
        prom2.set_value(primeN10);
    }

    void Thread2(int n, std::future<bool> futSync) {
        bool startSignal = futSync.get();
        if (startSignal) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            double sqrtN = std::sqrt(n);
            std::cout << "[Thread 2] Квадратний корінь від " << n << " дорівнює: " << sqrtN << std::endl;
        }
    }

    void run() {
        std::cout << "\n=== ЗАВДАННЯ 1.2.7 ===\n";
        int n;
        std::cout << "Введіть число n: ";
        std::cin >> n;

        std::promise<long long> prom1;
        std::promise<bool> promSync;
        std::promise<long long> prom2;

        std::future<long long> fut1 = prom1.get_future();
        std::future<bool> futSync = promSync.get_future();
        std::future<long long> fut2 = prom2.get_future();

        std::thread t1(Thread1, n, std::move(prom1), std::move(promSync), std::move(prom2));
        std::thread t2(Thread2, n, std::move(futSync));

        std::cout << "[Main] " << n << "-те просте число: " << fut1.get() << std::endl;
        std::cout << "[Main] " << (n * 10) << "-те просте число: " << fut2.get() << std::endl;

        t1.join();
        t2.join();
    }
}

// ==========================================
// ГОЛОВНЕ МЕНЮ ПРОГРАМИ
// ==========================================
int main() {
    // Встановлення коректного відображення символів у консолі
    std::setlocale(LC_ALL, "Ukrainian");

    int choice = 0;
    while (true) {
        std::cout << "\n============================================\n";
        std::cout << "         ЛАБОРАТОРНА РОБОТА №2              \n";
        std::cout << "============================================\n";
        std::cout << "1. Завдання 1.2.1 (Прапорець + sleep_for)\n";
        std::cout << "2. Завдання 1.2.2 (condition_variable + notify_all)\n";
        std::cout << "3. Завдання 1.2.3 (condition_variable + notify_one)\n";
        std::cout << "4. Завдання 1.2.4 (Переробка 1.2.1 на condition_variable)\n";
        std::cout << "5. Завдання 1.2.5 (std::async: deferred vs async)\n";
        std::cout << "6. Завдання 1.2.6 (std::packaged_task + деки)\n";
        std::cout << "7. Завдання 1.2.7 (std::promise / std::future)\n";
        std::cout << "0. Вихід\n";
        std::cout << "Виберіть номер завдання: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 0) break;

        switch (choice) {
        case 1: Task1::run(); break;
        case 2: Task2::run(); break;
        case 3: Task3::run(); break;
        case 4: Task4::run(); break;
        case 5: Task5::run(); break;
        case 6: Task6::run(); break;
        case 7: Task7::run(); break;
        default: std::cout << "Некоректна опція. Спробуйте ще раз.\n"; break;
        }
    }

    std::cout << "Програму завершено.\n";
    return 0;
}