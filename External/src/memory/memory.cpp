#include "memory.h"
#include <Psapi.h>

std::unique_ptr<memory_t> memory = std::make_unique<memory_t>();

static std::wstring to_wide(const std::string& value) {
    if (value.empty())
        return {};
    int needed = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    if (needed <= 0)
        return {};
    std::wstring out(static_cast<std::size_t>(needed), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, out.data(), needed);
    out.pop_back();
    return out;
}

std::uint32_t memory_t::find_process_id(const std::string& process_name) {
    std::uint32_t found = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;
    const std::wstring want = to_wide(process_name);
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snap, &entry)) {
        do {
            if (CompareStringOrdinal(want.c_str(), -1, entry.szExeFile, -1, TRUE) == CSTR_EQUAL) {
                found = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
    if (found)
        process_id = found;
    return found;
}

std::uint64_t memory_t::find_module_address(const std::string& module_name) {
    if (!process_handle || process_handle == INVALID_HANDLE_VALUE)
        return 0;
    const DWORD pid = GetProcessId(process_handle);
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;
    const std::wstring want = to_wide(module_name);
    MODULEENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    std::uint64_t addr = 0;
    if (Module32FirstW(snap, &entry)) {
        do {
            if (CompareStringOrdinal(want.c_str(), -1, entry.szModule, -1, TRUE) == CSTR_EQUAL) {
                addr = reinterpret_cast<std::uint64_t>(entry.modBaseAddr);
                break;
            }
        } while (Module32NextW(snap, &entry));
    }
    CloseHandle(snap);
    if (addr)
        base_address = addr;
    return addr;
}

bool memory_t::attach_to_process(const std::string& process_name) {
    const std::uint32_t pid = find_process_id(process_name);
    if (!pid)
        return false;
    HANDLE h = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!h || h == INVALID_HANDLE_VALUE)
        return false;
    if (process_handle && process_handle != INVALID_HANDLE_VALUE)
        CloseHandle(process_handle);
    process_handle = h;
    process_id = pid;
    return true;
}

bool memory_t::IsConnected() const {
    return process_handle && process_handle != INVALID_HANDLE_VALUE;
}

bool memory_t::read_raw(std::uint64_t address, void* buffer, std::size_t size) const {
    if (!IsConnected() || !buffer || !size)
        return false;
    SIZE_T done = 0;
    return ReadProcessMemory(process_handle, reinterpret_cast<LPCVOID>(address), buffer, size, &done) && done == size;
}

bool memory_t::write_raw(std::uint64_t address, const void* buffer, std::size_t size) const {
    if (!IsConnected() || !buffer || !size)
        return false;
    SIZE_T done = 0;
    return WriteProcessMemory(process_handle, reinterpret_cast<LPVOID>(address), buffer, size, &done) && done == size;
}

std::string memory_t::read_string(std::uint64_t address) {
    if (!address)
        return "Unknown";
    const std::int32_t len = read<std::int32_t>(address + 0x10);
    if (len <= 0 || len > 255)
        return "Unknown";
    const std::uint64_t ptr = (len >= 16) ? read<std::uint64_t>(address) : address;
    if (!ptr)
        return "Unknown";
    std::vector<char> buf(static_cast<std::size_t>(len) + 1, 0);
    if (!read_raw(ptr, buf.data(), static_cast<std::size_t>(len)))
        return "Unknown";
    return std::string(buf.data(), static_cast<std::size_t>(len));
}
