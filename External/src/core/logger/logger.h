#pragma once
#include <string>
#include <mutex>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>
#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

namespace Logger {
    inline std::mutex mtx;
    inline std::ofstream file;
    inline std::string path;
    inline std::string crashPath;
    inline bool initialized = false;
    inline bool crashWritten = false;
    inline bool quiet = false;
    inline std::vector<std::string> recent;

    inline std::string now_str() {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto t = system_clock::to_time_t(now);
        std::tm tm{}; localtime_s(&tm, &t);
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms.count();
        return oss.str();
    }
    inline std::string stamp_file() {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::tm tm{}; localtime_s(&tm, &t);
        char b[48]; strftime(b,sizeof(b),"%Y%m%d_%H%M%S",&tm);
        char out[64]; snprintf(out,sizeof(out),"%s_%03d", b, (int)ms.count());
        return out;
    }

    inline void init() {
        std::lock_guard<std::mutex> lk(mtx);
        if (initialized) return;
        char* up = nullptr; size_t len = 0;
        _dupenv_s(&up, &len, "USERPROFILE");
        std::string desk = up ? std::string(up) : "C:\\Users\\Public";
        if (up) free(up);
        path = desk + "\\Desktop\\jatos_log.txt";
        file.open(path, std::ios::out | std::ios::app);
        initialized = true;
        if (file.is_open()) file << "\n========== jatos log start " << now_str() << " ==========\n" << std::flush;
    }

    inline void log(const std::string& tag, const std::string& msg) {
        if (!initialized) init();
        std::string line = "[" + now_str() + "] [" + tag + "] " + msg;
        {
            std::lock_guard<std::mutex> lk(mtx);
            recent.push_back(line);
            if (recent.size() > 500) recent.erase(recent.begin());
            if (!quiet) std::cout << line << std::endl;
            if (file.is_open()) { file << line << "\n" << std::flush; }
        }
        OutputDebugStringA((line + "\n").c_str());
    }

    inline void logf(const std::string& tag, const char* fmt, ...) {
        char buf[2048];
        va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
        log(tag, buf);
    }

    inline void write_crash(EXCEPTION_POINTERS* ep, const char* source) {
        std::lock_guard<std::mutex> lk(mtx);
        char* up = nullptr; size_t len = 0;
        _dupenv_s(&up, &len, "USERPROFILE");
        std::string desk = up ? std::string(up) : "C:\\Users\\Public";
        if (up) free(up);
        crashPath = desk + "\\Desktop\\jatos_" + stamp_file() + ".crash";
        char exePath[MAX_PATH]={0};
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::string exeDir = exePath;
        auto slash = exeDir.find_last_of("\\/");
        if (slash != std::string::npos) exeDir = exeDir.substr(0, slash);
        std::string buildCrash = exeDir + "\\crash.crash";
        std::ofstream cf(crashPath, std::ios::out);
        std::ofstream bf(buildCrash, std::ios::out);
        if (!cf.is_open() && !bf.is_open()) return;
        auto write_all = [&](std::ofstream& out){
            if (!out.is_open()) return;
            out << "========== JATOS CRASH REPORT ==========\n";
            out << "time: " << now_str() << " source: " << source << "\n";
            if (ep && ep->ExceptionRecord) {
                out << "exception code: 0x" << std::hex << ep->ExceptionRecord->ExceptionCode << std::dec << "\n";
                out << "exception addr: 0x" << std::hex << (uintptr_t)ep->ExceptionRecord->ExceptionAddress << std::dec << "\n";
                out << "exception flags: 0x" << std::hex << ep->ExceptionRecord->ExceptionFlags << std::dec << "\n";
                out << "params: " << ep->ExceptionRecord->NumberParameters;
                for (DWORD i=0;i<ep->ExceptionRecord->NumberParameters;i++) out << " 0x" << std::hex << ep->ExceptionRecord->ExceptionInformation[i] << std::dec;
                out << "\n";
            }
            if (ep && ep->ContextRecord) {
#ifdef _M_X64
                auto c = ep->ContextRecord;
                out << "\n--- registers (x64) ---\n";
                out << "RAX=" << std::hex << c->Rax << " RBX=" << c->Rbx << " RCX=" << c->Rcx << " RDX=" << c->Rdx << "\n";
                out << "RSI=" << c->Rsi << " RDI=" << c->Rdi << " RBP=" << c->Rbp << " RSP=" << c->Rsp << "\n";
                out << "R8 =" << c->R8 << " R9 =" << c->R9 << " R10=" << c->R10 << " R11=" << c->R11 << "\n";
                out << "R12=" << c->R12 << " R13=" << c->R13 << " R14=" << c->R14 << " R15=" << c->R15 << "\n";
                out << "RIP=" << c->Rip << " EFL=" << c->EFlags << "\n";
#endif
            }
            out << "\n--- callstack ---\n";
            {
                void* st[64]; USHORT n = CaptureStackBackTrace(0, 64, st, nullptr);
                HANDLE proc = GetCurrentProcess();
                SymInitialize(proc, nullptr, TRUE);
                for (USHORT i=0;i<n;i++) {
                    DWORD64 addr = (DWORD64)st[i];
                    char symBuf[sizeof(SYMBOL_INFO)+256]; SYMBOL_INFO* sym=(SYMBOL_INFO*)symBuf;
                    sym->SizeOfStruct=sizeof(SYMBOL_INFO); sym->MaxNameLen=255;
                    DWORD64 disp=0;
                    if (SymFromAddr(proc, addr, &disp, sym)) out << "#" << i << " 0x" << std::hex << addr << std::dec << " " << sym->Name << "+0x" << std::hex << disp << std::dec << "\n";
                    else out << "#" << i << " 0x" << std::hex << addr << std::dec << "\n";
                }
            }
            out << "\n--- recent log (" << recent.size() << " lines) ---\n";
            for (auto &l: recent) out << l << "\n";
            out << "\n--- modules ---\n";
            {
                HMODULE mods[256]; DWORD need=0;
                HANDLE proc = GetCurrentProcess();
                if (EnumProcessModules(proc, mods, sizeof(mods), &need)) {
                    for (size_t i=0;i<need/sizeof(HMODULE);i++) {
                        char name[MAX_PATH]={0}; GetModuleFileNameA(mods[i], name, MAX_PATH);
                        MODULEINFO mi{}; GetModuleInformation(proc, mods[i], &mi, sizeof(mi));
                        out << std::hex << (uintptr_t)mi.lpBaseOfDll << "-" << (uintptr_t)mi.lpBaseOfDll+mi.SizeOfImage << std::dec << " " << name << "\n";
                    }
                }
            }
            out << "\n--- system ---\n";
#pragma warning(push)
#pragma warning(disable:4996)
            { OSVERSIONINFOA vi{}; vi.dwOSVersionInfoSize=sizeof(vi); GetVersionExA(&vi); out << "windows " << vi.dwMajorVersion << "." << vi.dwMinorVersion << " build " << vi.dwBuildNumber << "\n"; }
#pragma warning(pop)
            out << "\n========== END ==========\n";
        };
        write_all(cf); write_all(bf);
        cf.flush(); bf.flush();
        crashWritten = true;
        if (file.is_open()) { file << "\n!!! CRASH written to " << crashPath << " + " << buildCrash << " !!!\n" << std::flush; }
        if (!quiet) std::cout << "\n!!! CRASH written to " << crashPath << " + " << buildCrash << " !!!\n";
    }
}

#define LOG(tag, msg) Logger::log(tag, msg)
#define LOGF(tag, fmt, ...) Logger::logf(tag, fmt, __VA_ARGS__)

inline LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep) {
    Logger::log("CRASH", "unhandled exception!");
    if (ep && ep->ExceptionRecord) {
        char buf[256]; snprintf(buf, sizeof(buf), "code=0x%08X addr=0x%p", (unsigned)ep->ExceptionRecord->ExceptionCode, ep->ExceptionRecord->ExceptionAddress);
        Logger::log("CRASH", buf);
    }
    Logger::write_crash(ep, "UEF");
    if (Logger::file.is_open()) Logger::file.flush();
    std::string msg = "jatos crashed!\n\nCrash report:\n" + Logger::crashPath + "\n\nLog:\n" + Logger::path;
    MessageBoxA(nullptr, msg.c_str(), "jatos crash", MB_ICONERROR);
    return EXCEPTION_EXECUTE_HANDLER;
}
