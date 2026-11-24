#pragma once

#include <set>
#include <string>

// Follows a yt-dlp download from its output lines and turns it into the
// progress and status the user sees.
class YtdlpOutputParser
{
public:
    void reset();

    // True when something the user sees changed.
    bool feedLine(const std::string &line);

    // Call once yt-dlp has exited; sets the final status.
    void finish(int exitCode, bool cancelled);

    double progress() const { return m_percent / 100.0; } // current video, 0..1
    int item() const { return m_item; }                    // 0 for a single video
    int itemCount() const { return m_itemCount; }
    int failedCount() const { return int(m_failedItems.size()); }
    int skippedCount() const { return m_skipped; }
    const std::string &lastError() const { return m_lastError; }
    const std::string &status() const { return m_status; }

private:
    void updateRunningStatus();

    double m_percent = 0.0;
    int m_item = 0;
    int m_itemCount = 0;
    int m_skipped = 0;
    std::set<int> m_failedItems;
    std::string m_lastError;
    std::string m_status;
};
