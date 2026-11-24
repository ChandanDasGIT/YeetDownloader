#include "ytdlpBinary.h"
#include "sha256.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace {

#ifdef _WIN32
constexpr const char *ytdlpName = "yt-dlp.exe";
constexpr const char *ffmpegName = "ffmpeg.exe";
constexpr char pathSeparator = ';';

std::string env(const wchar_t *name)
{
    const wchar_t *value = _wgetenv(name);
    return value ? fs::path(value).u8string() : "";
}
#else
constexpr const char *ytdlpName = "yt-dlp";
constexpr const char *ffmpegName = "ffmpeg";
constexpr char pathSeparator = ':';

std::string env(const char *name)
{
    const char *value = std::getenv(name);
    return value ? value : "";
}
#endif

bool isExecutable(const fs::path &file)
{
    std::error_code ec;
#ifdef _WIN32
    return fs::is_regular_file(file, ec) || fs::is_symlink(file, ec);
#else
    return fs::is_regular_file(file, ec) && ::access(file.c_str(), X_OK) == 0;
#endif
}

std::string findOnPath(const char *name)
{
#ifdef _WIN32
    std::istringstream path(env(L"PATH"));
#else
    std::istringstream path(env("PATH"));
#endif
    std::string dir;
    while (std::getline(path, dir, pathSeparator)) {
        if (dir.empty())
            continue;
        const fs::path candidate = fs::u8path(dir) / name;
        if (isExecutable(candidate))
            return candidate.u8string();
    }
    return {};
}

std::string expectedChecksum(const std::string &checksums, const std::string &fileName)
{
    std::istringstream in(checksums);
    std::string hash, name;
    while (in >> hash >> name)
        if (name == fileName)
            return hash;
    return {};
}

} // namespace

std::string appDataDir()
{
#ifdef _WIN32
    const fs::path dir = fs::u8path(env(L"LOCALAPPDATA")) / "YeetDownloader";
#else
    std::string base = env("XDG_DATA_HOME");
    if (base.empty())
        base = env("HOME") + "/.local/share";
    const fs::path dir = fs::u8path(base) / "YeetDownloader";
#endif
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir.u8string();
}

std::string ytdlpPath()
{
    return (fs::u8path(appDataDir()) / ytdlpName).u8string();
}

bool ytdlpInstalled()
{
    std::error_code ec;
    return fs::is_regular_file(fs::u8path(ytdlpPath()), ec);
}

bool ffmpegOnPath()
{
    return !findOnPath(ffmpegName).empty();
}

std::string ffmpegPath()
{
    std::string found = findOnPath(ffmpegName);
#ifdef _WIN32
    if (found.empty()) {
        const std::vector<fs::path> wingetLinks = {
            fs::u8path(env(L"LOCALAPPDATA")) / "Microsoft" / "WinGet" / "Links",
            fs::u8path(env(L"ProgramFiles")) / "WinGet" / "Links",
        };
        for (const fs::path &dir : wingetLinks) {
            if (isExecutable(dir / ffmpegName)) {
                found = (dir / ffmpegName).u8string();
                break;
            }
        }
    }
#endif
    return found;
}

std::string ffmpegInstallHint()
{
#ifdef _WIN32
    return "In a terminal: winget install Gyan.FFmpeg";
#else
    return "Fedora: sudo dnf install ffmpeg";
#endif
}

bool folderExists(const std::string &path)
{
    std::error_code ec;
    return !path.empty() && fs::is_directory(fs::u8path(path), ec);
}

std::string updateStatus(const std::string &lastLine, int exitCode)
{
    if (exitCode != 0)
        return "Update check skipped.";
    return lastLine.empty() ? "yt-dlp is up to date." : lastLine;
}

const char *ytdlpDownloadUrl()
{
#ifdef _WIN32
    return "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe";
#else
    return "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp";
#endif
}

std::string installYtdlp(const std::string &binary, const std::string &checksums)
{
    const std::string expected = expectedChecksum(checksums, ytdlpName);
    if (expected.empty())
        return std::string("no checksum published for ") + ytdlpName;
    if (sha256Hex(binary) != expected)
        return "checksum mismatch, download rejected";

    const fs::path target = fs::u8path(ytdlpPath());
    fs::path temp = target;
    temp += ".part";
    {
        std::ofstream out(temp, std::ios::binary | std::ios::trunc);
        out.write(binary.data(), std::streamsize(binary.size()));
        if (!out)
            return "could not write " + temp.u8string();
    }
    std::error_code ec;
    fs::permissions(temp, fs::perms::owner_all | fs::perms::group_read | fs::perms::group_exec
                              | fs::perms::others_read | fs::perms::others_exec, ec);
    fs::rename(temp, target, ec);
    if (ec)
        return "could not install " + target.u8string();
    return {};
}
