// discord.gg/thenwefuckin
#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

class memory_t final {
public:
    memory_t() = default;
    ~memory_t() = default;

    memory_t(const memory_t&) = delete;
    memory_t& operator=(const memory_t&) = delete;

    std::uint32_t find_process_id(const std::string& process_name);
    std::vector<std::uint32_t> find_all_process_ids(const std::string& process_name);
    std::uint64_t find_module_address(const std::string& module_name);
    bool attach_to_process(const std::string& process_name);
    bool attach_to_pid(std::uint32_t pid);

    std::string read_string(std::uint64_t address);
    bool IsConnected() const;

    template <typename T>
    T read(std::uint64_t address) const {
        T buffer{};
        if (process_handle && process_handle != INVALID_HANDLE_VALUE)
            ReadProcessMemory(process_handle, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), nullptr);
        return buffer;
    }

    template <typename T>
    void write(std::uint64_t address, const T& value) const {
        if (process_handle && process_handle != INVALID_HANDLE_VALUE)
            WriteProcessMemory(process_handle, reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr);
    }

    bool read_raw(std::uint64_t address, void* buffer, std::size_t size) const;
    bool write_raw(std::uint64_t address, const void* buffer, std::size_t size) const;

    std::uint32_t get_process_id() const { return process_id; }
    std::uint64_t get_module_address() const { return base_address; }
    HANDLE get_process_handle() const { return process_handle; }

private:
    std::uint32_t process_id = 0;
    std::uint64_t base_address = 0;
    HANDLE process_handle = nullptr;
};

extern std::unique_ptr<memory_t> memory;
