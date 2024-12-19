#include "memory_manager.h"
#include "variable_manager.h"
#include "tcp_server.h"

int main() {
    init_memory();

    // Пример использования
    DEFINE_VAR(a, int);
    DEFINE_ARRAY(arr, double, 5);

    SET_VAR(a, 42);
    SET_ARRAY(arr, 0, 1.0);
    SET_ARRAY(arr, 1, 2.0);

    int value;
    GET_VAR(a, &value);
    printf("a = %d\n", value);

    double elem;
    GET_ARRAY(arr, 1, &elem);
    printf("arr[1] = %.2f\n", elem);

    // Запуск сервера
    start_server(8080);

    cleanup_memory();
    return 0;
}
