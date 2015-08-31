#include "../time_library.h"

int main() {
    print_current_time_with_ms();
    get_next_interval_start(100);
    while (1) {
        get_next_interval_start(500);
        get_next_interval_start(500);
        get_next_interval_start(500);
    }
}
