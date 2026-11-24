#pragma once

#include <string>

// All paths are UTF-8 strings.

// Linux: ~/.local/share/YeetDownloader (or under $XDG_DATA_HOME).
// Windows: %LOCALAPPDATA%\YeetDownloader. Created if missing.
std::string appDataDir();

std::string ytdlpPath();
bool ytdlpInstalled();

// Full path of ffmpeg: on PATH, or (Windows) where winget puts it, which a
// program started before ffmpeg was installed doesn't have on its PATH yet.
// Empty if not found.
std::string ffmpegPath();
bool ffmpegOnPath();

// How to install ffmpeg on this platform, for the error message.
std::string ffmpegInstallHint();

bool folderExists(const std::string &path);

// Status shown after "yt-dlp -U", from the last line it printed.
std::string updateStatus(const std::string &lastLine, int exitCode);

// Where the app fetches yt-dlp from, and its published checksums.
const char *ytdlpDownloadUrl();
inline constexpr const char *ytdlpChecksumsUrl = "https://github.com/yt-dlp/yt-dlp/releases/latest/download/SHA2-256SUMS";

// Installs the downloaded yt-dlp only if it matches its line in SHA2-256SUMS.
// Returns an empty string on success, otherwise what went wrong.
std::string installYtdlp(const std::string &binary, const std::string &checksums);
