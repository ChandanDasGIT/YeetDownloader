// Windows: CreateProcess inside a job object (so cancel() also stops ffmpeg),
// output read through one pipe. The child inherits only its own pipe ends.
#include "processRunner.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdlib>

namespace {

std::wstring wide(const std::string &utf8)
{
    if (utf8.empty())
        return {};
    const int size = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), int(utf8.size()), nullptr, 0);
    std::wstring out(size_t(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), int(utf8.size()), out.data(), size);
    return out;
}

// One argument as the C runtime reads it back (CommandLineToArgvW's rules).
void appendQuoted(std::wstring &line, const std::wstring &arg)
{
    if (!arg.empty() && arg.find_first_of(L" \t\n\v\"") == std::wstring::npos) {
        line += arg;
        return;
    }
    line += L'"';
    for (size_t i = 0;; ++i) {
        size_t backslashes = 0;
        while (i < arg.size() && arg[i] == L'\\') {
            ++i;
            ++backslashes;
        }
        if (i == arg.size()) {
            line.append(backslashes * 2, L'\\');
            break;
        }
        if (arg[i] == L'"')
            line.append(backslashes * 2 + 1, L'\\');
        else
            line.append(backslashes, L'\\');
        line += arg[i];
    }
    line += L'"';
}

} // namespace

bool ProcessRunner::start(const std::vector<std::string> &args, const std::string &workDir,
                          LineHandler onLine, FinishHandler onFinish)
{
    if (args.empty() || m_running)
        return false;
    if (m_thread.joinable())
        m_thread.join();

    // yt-dlp is a Python program: make it write UTF-8 into the pipe
    _wputenv_s(L"PYTHONIOENCODING", L"utf-8");

    SECURITY_ATTRIBUTES inherit{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE readEnd = nullptr, writeEnd = nullptr;
    if (!CreatePipe(&readEnd, &writeEnd, &inherit, 0))
        return false;
    SetHandleInformation(readEnd, HANDLE_FLAG_INHERIT, 0);
    HANDLE nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &inherit,
                             OPEN_EXISTING, 0, nullptr);

    HANDLE inherited[2] = {writeEnd, nul};
    SIZE_T attrSize = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &attrSize);
    std::vector<char> attrBuffer(attrSize);
    auto *attrs = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrBuffer.data());
    InitializeProcThreadAttributeList(attrs, 1, 0, &attrSize);
    UpdateProcThreadAttribute(attrs, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inherited, sizeof(inherited), nullptr, nullptr);

    STARTUPINFOEXW si{};
    si.StartupInfo.cb = sizeof(si);
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    si.StartupInfo.hStdInput = nul;
    si.StartupInfo.hStdOutput = writeEnd;
    si.StartupInfo.hStdError = writeEnd;
    si.lpAttributeList = attrs;

    std::wstring commandLine;
    for (const std::string &arg : args) {
        if (!commandLine.empty())
            commandLine += L' ';
        appendQuoted(commandLine, wide(arg));
    }
    const std::wstring dir = wide(workDir);

    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));

    // CREATE_NO_WINDOW: a hidden console that yt-dlp's own children (ffmpeg) share
    PROCESS_INFORMATION pi{};
    const BOOL created = CreateProcessW(nullptr, commandLine.data(), nullptr, nullptr, TRUE,
                                        CREATE_NO_WINDOW | CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT,
                                        nullptr, dir.empty() ? nullptr : dir.c_str(), &si.StartupInfo, &pi);
    DeleteProcThreadAttributeList(attrs);
    CloseHandle(writeEnd);
    if (nul != INVALID_HANDLE_VALUE)
        CloseHandle(nul);
    if (!created) {
        CloseHandle(readEnd);
        CloseHandle(job);
        return false;
    }
    AssignProcessToJobObject(job, pi.hProcess);
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);

    {
        std::lock_guard lock(m_mutex);
        m_job = job;
    }
    m_cancelled = false;
    m_running = true;

    HANDLE process = pi.hProcess;
    m_thread = std::thread([this, readEnd, process, onLine = std::move(onLine), onFinish = std::move(onFinish)] {
        std::string pending;
        char buffer[8192];
        DWORD got = 0;
        while (ReadFile(readEnd, buffer, sizeof(buffer), &got, nullptr) && got > 0) {
            pending.append(buffer, got);
            emitLines(pending, false, onLine);
        }
        emitLines(pending, true, onLine);
        CloseHandle(readEnd);

        WaitForSingleObject(process, INFINITE);
        DWORD code = DWORD(-1);
        GetExitCodeProcess(process, &code);
        CloseHandle(process);
        {
            std::lock_guard lock(m_mutex);
            CloseHandle(static_cast<HANDLE>(m_job));
            m_job = nullptr;
        }
        const bool cancelled = m_cancelled;
        m_running = false;
        if (onFinish)
            onFinish(int(code), cancelled);
    });
    return true;
}

void ProcessRunner::cancel()
{
    // Windows has no gentle Ctrl+C for a program without a console window: stop it.
    std::lock_guard lock(m_mutex);
    if (m_job && !m_cancelled) {
        m_cancelled = true;
        TerminateJobObject(static_cast<HANDLE>(m_job), 1);
    }
}
