#include "ytdlpCommand.h"

#include <filesystem>
#include <fstream>
#include <set>

namespace fs = std::filesystem;

namespace {

constexpr size_t youtubeIdLength = 11;

bool isIdChar(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
}

// "Title [VPMzCi6TZ5E]" -> "VPMzCi6TZ5E"
std::string idFromStem(const std::string &stem)
{
    if (stem.size() < youtubeIdLength + 2 || stem.back() != ']')
        return {};
    const size_t open = stem.size() - youtubeIdLength - 2;
    if (stem[open] != '[')
        return {};
    std::string id = stem.substr(open + 1, youtubeIdLength);
    for (char c : id)
        if (!isIdChar(c))
            return {};
    return id;
}

} // namespace

std::string archivePath(const std::string &folder, DownloadKind kind)
{
    const char *name = kind == DownloadKind::Video ? ".yeetdownloader-archive-mp4.txt"
                                                   : ".yeetdownloader-archive-mp3.txt";
    return (fs::u8path(folder) / name).u8string();
}

int seedArchive(const std::string &folder, DownloadKind kind)
{
    const std::string path = archivePath(folder, kind);
    std::set<std::string> known;
    {
        std::ifstream in(fs::u8path(path));
        std::string line;
        while (std::getline(in, line))
            known.insert(line);
    }

    const std::string wantedExt = kind == DownloadKind::Video ? ".mp4" : ".mp3";
    std::vector<std::string> found;
    std::error_code ec;
    for (const auto &entry : fs::directory_iterator(fs::u8path(folder), ec)) {
        if (!entry.is_regular_file(ec) || entry.path().extension() != wantedExt)
            continue;
        const std::string id = idFromStem(entry.path().stem().u8string());
        if (id.empty())
            continue;
        const std::string line = "youtube " + id;
        if (known.insert(line).second)
            found.push_back(line);
    }

    if (!found.empty()) {
        std::ofstream out(fs::u8path(path), std::ios::app);
        for (const std::string &line : found)
            out << line << '\n';
    }
    return int(found.size());
}

std::vector<std::string> downloadArgs(const std::string &ytdlpPath, const std::string &folder,
                                      const DownloadRequest &request, const std::string &ffmpegLocation)
{
    std::vector<std::string> args{ytdlpPath, "--newline", "--progress"};

    if (request.kind == DownloadKind::Video) {
        std::string format = "bestvideo+bestaudio/best";
        if (request.resolution != "Best Available") {
            std::string height = request.resolution;
            if (!height.empty() && height.back() == 'p')
                height.pop_back();
            format = "bestvideo[height<=" + height + "]+bestaudio/best[height<=" + height + "]";
        }
        args.insert(args.end(), {"-f", format, "--merge-output-format", "mp4"});
    } else {
        args.insert(args.end(), {"-x", "--audio-format", "mp3", "--audio-quality", request.audioQuality});
    }

    // Playlist videos get their zero-padded position in front ("001 - "); single videos keep yt-dlp's default name.
    args.insert(args.end(), {"-o", "%(playlist_index|)s%(playlist_index& - |)s%(title)s [%(id)s].%(ext)s"});
    args.insert(args.end(), {"--download-archive", archivePath(folder, request.kind)});

    if (!ffmpegLocation.empty())
        args.insert(args.end(), {"--ffmpeg-location", ffmpegLocation});

    if (request.pauseBetweenVideos)
        args.insert(args.end(), {"--sleep-interval", "5", "--max-sleep-interval", "15"});

    args.insert(args.end(), {"--", request.url});
    return args;
}
