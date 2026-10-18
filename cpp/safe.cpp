#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>
std::optional<std::string> find_user(unsigned id) {
    if (id == 42) return "Ada";
    return std::nullopt;
}
void process(bool error) {
    auto data = std::make_unique<char[]>(1024);
    if (error) return; // RAII also handles early returns in C++.
    data[0] = 'x';
}
int main() {
    auto owner = std::make_unique<std::string>("hello");
    auto moved = std::move(owner);
    assert(!owner); std::cout << *moved << '\n';
    std::vector<int> v{1,2,3,4}, additions;
    for (int x : v) if (x == 2) additions.push_back(99);
    v.insert(v.end(), additions.begin(), additions.end());
    assert(v.size() == 5);
    try { (void)v.at(100); }
    catch (const std::out_of_range&) { std::cout << "out of range\n"; }
    std::atomic<int> counter{0};
    auto increment = [&] { for (int i=0; i<100000; ++i)
        counter.fetch_add(1, std::memory_order_relaxed); };
    std::thread a(increment), b(increment); a.join(); b.join();
    assert(counter.load() == 200000);
    process(true); process(false);
    if (auto name = find_user(42)) std::cout << *name << '\n';
    assert(!find_user(0));
    std::cout << "C++ safe examples passed\n";
}
