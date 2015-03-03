#include "tests/lib.h"
#include "proc/syscall.h"

int main() {
    int mus = syscall_exec("[disk]mus");
    int kat = syscall_exec("[disk]kat");

    syscall_join(mus);
    syscall_join(kat);

    return 0;
}
