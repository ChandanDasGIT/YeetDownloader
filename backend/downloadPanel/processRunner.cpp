#include "processRunner.h"

ProcessRunner::~ProcessRunner()
{
    cancel();
    if (m_thread.joinable())
        m_thread.join();
}

void ProcessRunner::emitLines(std::string &pending, bool all, const LineHandler &onLine)
{
    size_t start = 0;
    for (size_t i = 0; i < pending.size(); ++i) {
        if (pending[i] == '\n' || pending[i] == '\r') {
            if (i > start && onLine)
                onLine(pending.substr(start, i - start));
            start = i + 1;
        }
    }
    pending.erase(0, start);
    if (all && !pending.empty() && onLine) {
        onLine(pending);
        pending.clear();
    }
}
