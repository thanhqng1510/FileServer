//#include <atomic>
//#include <thread>
//#include <iostream>
//#include <chrono>
//#include <mutex>
//#include <conio.h>
//
//
//std::mutex mtx;
//
//void CinThread(std::atomic<int>& disp_mode) {
//    while (disp_mode.load() != 3) {
//        char c = _getch();
//
//        mtx.lock();
//
//        std::cout << c;
//        disp_mode.store((int)c - 48);
//        c = _getch();
//        system("cls");
//
//        mtx.unlock();
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
//    std::thread cin_thread(CinThread, std::ref(disp_mode));
//
//    while (disp_mode.load() != 3) {
//        if (disp_mode.load() == 1) {
//            mtx.lock();
//            std::cout << "activity mode\n";
//            mtx.unlock();
//        }
//        else {    // 2
//            mtx.lock();
//            std::cout << "list mode\n";
//            mtx.unlock();
//        }
//
//        std::this_thread::sleep_for(std::chrono::seconds(1));
//    }
//
//    cin_thread.join();
//}


#include <iostream>
#include <fstream>
#include <string>


int main() {
	std::ifstream fin("Test.txt");

	std::string s;
	int cnt = 0;
	while (std::getline(fin, s)) {
		std::cout << cnt << " " << s << "\n";
		++cnt;
	}
}