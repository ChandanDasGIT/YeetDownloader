#pragma once

#include <string>
#include <vector>

enum class DownloadKind { Video, Audio };

struct DownloadRequest
{
    DownloadKind kind = DownloadKind::Video;
    std::string url;
    std::string resolution;   // "Best Available" or e.g. "1080p"
    std::string audioQuality; // "0" (best VBR), "320k", ...
    bool pauseBetweenVideos = false;
};

// Record of finished videos, one per kind, kept in the download folder.
std::string archivePath(const std::string &folder, DownloadKind kind);

// Adds the videos already in the folder (files named "... [<id>].mp4/.mp3")
// to the record, so they are skipped. Returns how many were added.
int seedArchive(const std::string &folder, DownloadKind kind);

// ffmpegLocation: only when ffmpeg isn't on PATH (see ffmpegPath()).
std::vector<std::string> downloadArgs(const std::string &ytdlpPath, const std::string &folder,
                                      const DownloadRequest &request, const std::string &ffmpegLocation = {});
