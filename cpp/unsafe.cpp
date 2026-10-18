// Educational undefined behavior: run only as an isolated sanitizer demo.
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    const std::string mode = argv[1];
    if (mode == "uaf") {
        int* value = new int(42); delete value;
        std::cout << *value << '\n';
    } else if (mode == "double-free") {
        void* buf = std::malloc(64); std::free(buf); std::free(buf);
    } else if (mode == "iterator") {
        std::vector<int> v{1, 2, 3, 4};
        v.shrink_to_fit();
        for (int& x : v) {
            if (x == 2) {
                // Force a reallocation regardless of the original capacity.
                v.reserve(v.capacity() + 1);
                v.push_back(99);
            }
            std::cout << x << '\n';
        }
    } else if (mode == "overflow") {
        char buf[8];
        std::strcpy(buf, "input longer than eight bytes");
        std::cout << buf << '\n';
    } else if (mode == "race") {
        int counter = 0;
        auto increment = [&] { for (int i = 0; i < 100000; ++i) ++counter; };
        std::thread a(increment), b(increment); a.join(); b.join();
        std::cout << counter << '\n'; // Any outcome is possible: undefined behavior.
    } else if (mode == "leak") {
        // Volatile pointer keeps this allocation visible to the sanitizer.
        char* volatile data = static_cast<char*>(std::malloc(1024));
        if (!data) return 3;
        data[0] = 'x';
        return 0; // Missing free on the early-return path.
    } else if (mode == "null") {
        int* user = nullptr; std::cout << *user << '\n';
    } else { std::cerr << "Unknown demo\n"; return 2; }
}
