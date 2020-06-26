//#include <atomic>
//#include <thread>
//#include <iostream>
//#include <chrono>
//
//
//void ReadCin(std::atomic<int>& disp_mode) {
//    int inp;
//
//    while (disp_mode.load() != 3) {
//        std::cin >> inp;
//        disp_mode.store(inp);
//
//        system("cls");
//    }
//}
//
//
//int main() {
//    std::atomic<int> disp_mode(1);
//    // 1 -> activity
//    // 2 -> list
//    // 3 -> quit
//
//    std::thread cin_thread(ReadCin, std::ref(disp_mode));
//
//    while (disp_mode.load() != 3) {
//        if (disp_mode.load() == 1)
//            std::cout << "activity mode\n";
//        else    // 2
//            std::cout << "list mode\n";
//
//        std::this_thread::sleep_for(std::chrono::seconds(1));
//    }
//
//    cin_thread.join();
//}


//#include <iostream>
//#include <thread>
//#include <string>
//#include <chrono>
//#include <mutex>
//using namespace std;
//
//
//mutex mtx;
//
//void foo() {
//    string x;
//
//    while (true) {
//        std::this_thread::sleep_for(std::chrono::seconds(1));
//
//        mtx.lock();
//        cin >> x;
//        cout << x << "\n";
//        mtx.unlock();
//    }
//}
//
//
//int main() {
//    thread t1(foo);
//
//    while (true) {
//        std::this_thread::sleep_for(std::chrono::seconds(1));
//
//        mtx.lock();
//        cout << "Cake\n";
//        mtx.unlock();
//    }
//
//    t1.join();
//}


#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <mutex>
using namespace std;

mutex mtx;

void foo()
{
    while (1)
    {
        mtx.lock();
        cout << "Cake\n";
        mtx.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main()
{
    thread t1(foo);

    string x;
    while (1)
    {
        char c;
        while (cin.get(c))
        {
            mtx.lock();
            cout << c;      

            if (int(c) == 13)
                mtx.unlock();
        }
    }

    t1.join();
}