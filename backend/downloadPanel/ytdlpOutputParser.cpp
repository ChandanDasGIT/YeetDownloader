#include "ytdlpOutputParser.h"

#include <cstdio>
#include <cstdlib>

namespace {

constexpr size_t maxErrorLength = 300;

bool startsWith(const std::string &s, const char *prefix)
{
    return s.rfind(prefix, 0) == 0;
}

bool contains(const std::string &s, const char *part)
{
    return s.find(part) != std::string::npos;
}

std::string formatPercent(double value)
{
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%.1f", value);
    return buffer;
}

} // namespace

void YtdlpOutputParser::reset()
{
    *this = YtdlpOutputParser{};
    m_status = "Starting download...";
}

bool YtdlpOutputParser::feedLine(const std::string &line)
{
    if (startsWith(line, "ERROR: ")) {
        m_lastError = line.substr(7);
        if (m_lastError.size() > maxErrorLength)
            m_lastError = m_lastError.substr(0, maxErrorLength) + "...";
        m_failedItems.insert(m_item);
        return false;
    }

    // "[youtube:tab] Playlist X: Downloading 9 items of 9", printed before already recorded videos are skipped
    const size_t announce = line.find(": Downloading ");
    if (startsWith(line, "[") && announce != std::string::npos) {
        int count = 0;
        char word[8] = {};
        if (std::sscanf(line.c_str() + announce + 14, "%d %7s", &count, word) == 2
            && (std::string(word) == "items" || std::string(word) == "item")) {
            m_itemCount = count;
            return false;
        }
    }

    if (!startsWith(line, "[download] "))
        return false;

    const std::string rest = line.substr(11);

    // "[download] Downloading item 37 of 175"
    int item = 0, count = 0;
    if (std::sscanf(rest.c_str(), "Downloading item %d of %d", &item, &count) == 2) {
        m_item = item;
        m_itemCount = count;
        m_percent = 0.0;
        updateRunningStatus();
        return true;
    }

    if (contains(rest, "does not pass filter (availability!=subscriber_only)")) {
        ++m_membersOnly;
        m_status = "Skipping members-only videos (" + std::to_string(m_membersOnly) + " so far)...";
        return true;
    }

    if (startsWith(rest, "Finished downloading playlist")) {
        m_reachedEnd = true;
        return false;
    }

    if (contains(rest, "has already been recorded in the archive") || contains(rest, "has already been downloaded")) {
        ++m_skipped;
        return false;
    }

    // "[download]  45.3% of ..." : the percentage must come first, so a "%" in a title never counts.
    size_t i = 0;
    while (i < rest.size() && rest[i] == ' ')
        ++i;
    char *end = nullptr;
    const double value = std::strtod(rest.c_str() + i, &end);
    if (end != rest.c_str() + i && *end == '%') {
        m_percent = value;
        updateRunningStatus();
        return true;
    }
    return false;
}

void YtdlpOutputParser::updateRunningStatus()
{
    std::string text;
    if (m_itemCount > 0)
        text = "Video " + std::to_string(m_item) + " of " + std::to_string(m_itemCount) + " · ";
    text += "Downloading... " + formatPercent(m_percent) + "%";
    m_status = text;
}

void YtdlpOutputParser::finish(int exitCode, bool cancelled)
{
    if (cancelled) {
        m_status = m_itemCount > 0
            ? "Download cancelled at video " + std::to_string(m_item) + " of " + std::to_string(m_itemCount) + "."
            : "Download cancelled.";
        return;
    }

    const int failed = failedCount();
    const std::string membersNote = m_membersOnly == 0 ? ""
        : " " + std::to_string(m_membersOnly) + (m_membersOnly == 1 ? " is" : " are")
          + " members-only (YouTube only lets the channel's paying members download them).";

    if (m_itemCount == 0) {
        if (m_membersOnly > 0)
            m_status = "Download failed: this video is members-only (YouTube only lets the channel's paying members download it).";
        else if (exitCode == 0)
            m_status = m_skipped > 0 ? "Download Complete! (already downloaded, skipped)" : "Download Complete!";
        else
            m_status = "Download failed: " + (m_lastError.empty() ? "exit code " + std::to_string(exitCode) : m_lastError);
        return;
    }

    if (exitCode == 0 && failed == 0) {
        const int available = m_itemCount - m_membersOnly;
        m_status = "Download Complete! " + std::to_string(available) + " of " + std::to_string(m_itemCount) + " videos";
        if (m_skipped > 0)
            m_status += " (" + std::to_string(m_skipped) + " already downloaded, skipped)";
        m_status += "." + membersNote;
        return;
    }

    if (failed == 0) {
        m_status = "Download failed (exit code " + std::to_string(exitCode) + ")";
    } else if (!m_reachedEnd) {
        m_status = "Download failed: stopped at video " + std::to_string(m_item) + " of " + std::to_string(m_itemCount);
    } else {
        const int done = m_itemCount - failed - m_skipped - m_membersOnly;
        m_status = "Download finished with errors: " + std::to_string(done) + " downloaded, "
                 + std::to_string(m_skipped) + " skipped, " + std::to_string(failed) + " failed of "
                 + std::to_string(m_itemCount);
    }
    if (m_lastError.empty()) {
        m_status += ".";
    } else {
        m_status += ". Last error: " + m_lastError;
        if (m_lastError.back() != '.')
            m_status += ".";
    }
    m_status += membersNote;
}
