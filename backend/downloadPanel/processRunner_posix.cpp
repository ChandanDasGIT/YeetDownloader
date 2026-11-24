// Linux: posix_spawn in its own process group, output read through one pipe.
#include "processRunner.h"

#include <chrono>
#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

namespace {

constexpr auto killGrace = std::chrono::seconds(5);

} // namespace

bool ProcessRunner::start(const std::vector<std::string> &args, const std::string &workDir,
                          LineHandler onLine, FinishHandler onFinish)
{
    if (args.empty() || m_running)
        return false;
    if (m_thread.joinable())
        m_thread.join();

    int fds[2] = {-1, -1};
    if (pipe2(fds, O_CLOEXEC) != 0)
        return false;

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, fds[1], STDERR_FILENO);
    posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    if (!workDir.empty())
        posix_spawn_file_actions_addchdir_np(&actions, workDir.c_str());

    // Own process group, so cancel() also reaches the programs it starts.
    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);
    posix_spawnattr_setpgroup(&attr, 0);

    std::vector<char *> argv;
    for (const std::string &arg : args)
        argv.push_back(const_cast<char *>(arg.c_str()));
    argv.push_back(nullptr);

    pid_t pid = -1;
    const int spawned = posix_spawn(&pid, argv[0], &actions, &attr, argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attr);
    ::close(fds[1]);
    if (spawned != 0) {
        ::close(fds[0]);
        return false;
    }

    {
        std::lock_guard lock(m_mutex);
        m_pid = pid;
    }
    m_cancelled = false;
    m_running = true;

    const int fd = fds[0];
    m_thread = std::thread([this, fd, pid, onLine = std::move(onLine), onFinish = std::move(onFinish)] {
        std::string pending;
        char buffer[8192];
        std::chrono::steady_clock::time_point cancelSeen{};
        bool killed = false;

        pollfd pfd{fd, POLLIN, 0};
        for (;;) {
            if (m_cancelled && !killed) {
                const auto now = std::chrono::steady_clock::now();
                if (cancelSeen == std::chrono::steady_clock::time_point{}) {
                    cancelSeen = now;
                } else if (now - cancelSeen > killGrace) {
                    ::kill(-pid, SIGKILL);
                    killed = true;
                }
            }
            const int ready = poll(&pfd, 1, 200);
            if (ready < 0)
                break;
            if (ready == 0)
                continue;
            const ssize_t n = ::read(fd, buffer, sizeof(buffer));
            if (n <= 0)
                break;
            pending.append(buffer, size_t(n));
            emitLines(pending, false, onLine);
        }
        emitLines(pending, true, onLine);
        ::close(fd);

        int status = 0;
        waitpid(pid, &status, 0);
        const int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        {
            std::lock_guard lock(m_mutex);
            m_pid = -1;
        }
        const bool cancelled = m_cancelled;
        m_running = false;
        if (onFinish)
            onFinish(exitCode, cancelled);
    });
    return true;
}

void ProcessRunner::cancel()
{
    std::lock_guard lock(m_mutex);
    if (m_pid > 0 && !m_cancelled) {
        m_cancelled = true;
        ::kill(-m_pid, SIGINT);
    }
}
