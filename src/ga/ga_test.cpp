#include "GAEngine.hpp"

int main() {
#ifdef GA_TEST_MODE
    std::cout << "Running GA in TEST MODE\n";
#endif

    GAEngine ga;
    ga.run();

    return 0;
}
