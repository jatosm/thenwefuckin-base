#include <cstdint>
#include <iostream>
#include <windows.h>
#include "src/core/app/app.h"

std::int32_t main() {
    SetConsoleTitleA("jatos");
    std::cout << "jatos external\n";
    std::cout << "waiting for roblox...\n\n";
    const std::int32_t code = App::Run();
    if (code != 0) {
        std::cout << "\nexited with code " << code << "\n";
        system("pause");
    }
    return code;
}
