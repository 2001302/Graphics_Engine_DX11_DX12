#include "graphics_core.h"

#include <filesystem>
#include <knownfolders.h>
#include <shlobj.h>

namespace graphics {

bool GpuCore::Initialize() {

    InitializePix();
    InitializeDevice();
    InitializeCommand();
    InitializeSwapchain();
    InitializeHeap();
    InitializeBuffer();
    return true;
}

static std::wstring GetLatestWinPixGpuCapturerPath_Cpp17() {
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL,
                         &programFilesPath);

    std::filesystem::path pixInstallationPath = programFilesPath;
    pixInstallationPath /= "Microsoft PIX";

    std::wstring newestVersionFound;

    for (auto const &directory_entry :
         std::filesystem::directory_iterator(pixInstallationPath)) {
        if (directory_entry.is_directory()) {
            if (newestVersionFound.empty() ||
                newestVersionFound <
                    directory_entry.path().filename().c_str()) {
                newestVersionFound = directory_entry.path().filename().c_str();
            }
        }
    }

    if (newestVersionFound.empty()) {
        // TODO: Error, no PIX installation found
    }

    return pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
}

bool GpuCore::InitializePix() {
	if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0) {
		LoadLibrary(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
	}
	return true;
}
} // namespace graphics
