#include "combat.h"
#include "../../globals/globals.h"

#include <chrono>
#include <thread>

namespace Combat {

void Loop()
{
    using namespace std::chrono_literals;
    while (Globals::running)
    {
        std::this_thread::sleep_for(10ms);
    }
}

}
