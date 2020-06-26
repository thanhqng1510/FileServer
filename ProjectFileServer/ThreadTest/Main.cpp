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


#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <conio.h>
#include <chrono>


std::mutex mtx;

void CinThread() {
    while (true) {
        char c = _getch();

        mtx.lock();

        while ((int)c != 13)
            std::cout << c << "\n";

        mtx.unlock();
    }
}

void CoutThread() {
    while (true) {
        mtx.lock();
        std::cout << "cout_thread\n";
        mtx.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


int main() {
    std::thread cin_thread(CinThread);
    std::thread cout_thread(CoutThread);

    cin_thread.join();
    cout_thread.join();
}