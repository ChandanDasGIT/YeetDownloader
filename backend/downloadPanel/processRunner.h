#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Runs one program at a time without a shell, merging stdout and stderr and
// reporting them line by line. The handlers are called on a worker thread.
// One implementation per platform: processRunner_posix.cpp, processRunner_windows.cpp.
class ProcessRunner
{
public:
    using LineHandler = std::function<void(const std::string &line)>;
    using FinishHandler = std::function<void(int exitCode, bool cancelled)>;

    ProcessRunner() = default;
    ProcessRunner(const ProcessRunner &) = delete;
    ProcessRunner &operator=(const ProcessRunner &) = delete;
    ~ProcessRunner();

    // False if a program is already running or it could not be started.
    bool start(const std::vector<std::string> &args, const std::string &workDir,
               LineHandler onLine, FinishHandler onFinish);

    // Stops the program and everything it started (e.g. ffmpeg).
    void cancel();

    bool running() const { return m_running; }

private:
    // Calls onLine for each complete line in pending (and the rest too if all).
    static void emitLines(std::string &pending, bool all, const LineHandler &onLine);

    std::thread m_thread;
    std::mutex m_mutex;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_cancelled{false};
#ifdef _WIN32
    void *m_job = nullptr; // the program and its children
#else
    int m_pid = -1;
#endif
};
