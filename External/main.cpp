// discord.gg/thenwefuckin
#include <atomic>
#include <cstdint>
#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <shellapi.h>
#include "src/core/app/app.h"
#include "src/core/logger/logger.h"
#include "src/memory/memory.h"
#include "src/core/globals/globals.h"
#include "src/sdk/offsets.h"

namespace {
volatile const char g_CreditSignature[] = "discord.gg/thenwefuckin - jatos base";
constexpr WORD C_WHITE  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
constexpr WORD C_DIM    = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
constexpr WORD C_GREEN  = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
constexpr WORD C_RED    = FOREGROUND_RED | FOREGROUND_INTENSITY;
constexpr WORD C_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
constexpr WORD C_CYAN   = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
constexpr const char* kIndent = "       ";

void badge(const char* label, WORD labelColor, const char* msg) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO old{};
    GetConsoleScreenBufferInfo(h, &old);
    std::cout << kIndent;
    SetConsoleTextAttribute(h, C_WHITE);
    std::cout << "[";
    SetConsoleTextAttribute(h, labelColor);
    std::cout << label;
    SetConsoleTextAttribute(h, C_WHITE);
    std::cout << "] ";
    SetConsoleTextAttribute(h, C_DIM);
    std::cout << msg << "\n";
    SetConsoleTextAttribute(h, old.wAttributes);
}

bool is_admin() {
    BOOL admin = FALSE;
    SID_IDENTIFIER_AUTHORITY nt = SECURITY_NT_AUTHORITY;
    PSID group = nullptr;
    if (AllocateAndInitializeSid(&nt, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &group)) {
        CheckTokenMembership(nullptr, group, &admin);
        FreeSid(group);
    }
    return admin != FALSE;
}

void relaunch_as_admin() {
    wchar_t exe[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.lpVerb = L"runas";
    sei.lpFile = exe;
    sei.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&sei))
        badge("priv", C_RED, "elevation declined - run External.exe as administrator");
}

void stage_watcher(std::atomic<bool>& stop) {
    int shown = 0;
    while (!stop.load()) {
        const int s = App::stage.load();
        if (s >= 1 && shown < 1) {
            shown = 1;
            char buf[192];
            snprintf(buf, sizeof(buf), "pid=%u base=0x%llX client=%s",
                (unsigned)memory->get_process_id(),
                (unsigned long long)memory->get_module_address(),
                Offsets::ClientVersion.c_str());
            badge("attached", C_GREEN, buf);
        }
        if (s >= 2 && shown < 2) {
            shown = 2;
            badge("overlay", C_GREEN, "DirectX ready");
        }
        if (s >= 3 && shown < 3) {
            shown = 3;
            badge("ready", C_CYAN, "press INSERT for menu - have fun");
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
}

std::int32_t main() {
    SetUnhandledExceptionFilter(CrashHandler);
    AddVectoredExceptionHandler(1, [](PEXCEPTION_POINTERS ep)->LONG {
        DWORD code = ep->ExceptionRecord->ExceptionCode;
        if (code==0x406D1388 || code==0x40010006 || code==0xE06D7363) return EXCEPTION_CONTINUE_SEARCH;
        static long long lastMs=0; static int count=0;
        long long now = GetTickCount64();
        if (now - lastMs < 500) return EXCEPTION_CONTINUE_SEARCH;
        if (count++ > 20) return EXCEPTION_CONTINUE_SEARCH;
        lastMs = now;
        Logger::write_crash(ep, "VEH");
        return EXCEPTION_CONTINUE_SEARCH;
    });
    Logger::quiet = true;
    Logger::init();
    SetConsoleTitleA("jatos | discord.gg/thenwefuckin");

    std::cout << "\n";
    badge("jatos", C_CYAN, "external - INSERT to open menu");
    badge("discord", C_WHITE, "discord.gg/thenwefuckin");
    std::cout << "\n";
    if (!is_admin()) {
        badge("priv", C_YELLOW, "administrator required - relaunching elevated...");
        relaunch_as_admin();
        return 0;
    }
    badge("priv", C_GREEN, "running as administrator");
    badge("attaching", C_GREEN, "waiting for RobloxPlayerBeta.exe...");
    std::atomic<bool> stopWatcher{false};
    std::thread watcher(stage_watcher, std::ref(stopWatcher));
    const std::int32_t code = App::Run();
    stopWatcher.store(true);
    if (watcher.joinable()) watcher.join();
    if (code == 0) {
        badge("done", C_GREEN, "session ended cleanly");
    } else {
        char buf[128]; snprintf(buf, sizeof(buf), "exited with code %d - see jatos_log.txt", code);
        badge("fail", C_RED, buf);
        system("pause >nul");
    }
    return code;
}
