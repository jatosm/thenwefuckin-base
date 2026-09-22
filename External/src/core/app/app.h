#pragma once
#include <atomic>
#include <cstdint>

namespace App {
bool game_open();
bool init();
std::int32_t Run();
inline std::atomic<int> stage{0};
}
