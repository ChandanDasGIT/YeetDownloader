#include "downloadPanelWrap.h"

#include "backend/downloadPanel/ytdlpBinary.h"

#include <QDesktopServices>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QUrl>

#include <memory>

downloadPanelWrap::downloadPanelWrap(QObject *parent) : QObject(parent)
{
    m_downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    m_preparing = true;
    if (ytdlpInstalled())
        runUpdateCheck();
    else
        fetchYtdlp();
}

void downloadPanelWrap::setDownloadPath(const QString &path)
{
    // FolderDialog gives file:// URLs
    const QString local = path.startsWith("file://") ? QUrl(path).toLocalFile() : path;
    if (local != m_downloadPath && folderExists(local.toStdString())) {
        m_downloadPath = local;
        emit downloadPathChanged();
    }
}

void downloadPanelWrap::fetchYtdlp()
{
    setStatus("Setting up standalone yt-dlp binary...");

    auto get = [this](const char *url) {
        QNetworkRequest request{QUrl(QString::fromLatin1(url))};
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        // Over HTTP/2, GitHub's redirect makes Qt log "QIODevice::read (QSslSocket): device not open"
        request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
        return m_net.get(request);
    };
    struct Fetch
    {
        int pending = 2;
        QByteArray binary, sums;
        QString error;
    };
    auto fetch = std::make_shared<Fetch>();

    auto finished = [this, fetch](QNetworkReply *reply, QByteArray Fetch::*into) {
        if (reply->error() == QNetworkReply::NoError)
            (*fetch).*into = reply->readAll();
        else if (fetch->error.isEmpty())
            fetch->error = reply->errorString();
        reply->deleteLater();
        if (--fetch->pending > 0)
            return;

        if (!fetch->error.isEmpty()) {
            setStatus("Failed to fetch standalone binary: " + fetch->error);
        } else {
            const std::string problem = installYtdlp(fetch->binary.toStdString(), fetch->sums.toStdString());
            setStatus(problem.empty() ? "yt-dlp setup complete." : "yt-dlp setup failed: " + QString::fromStdString(problem));
        }
        if (ytdlpInstalled())
            runUpdateCheck();
        else
            preparationDone();
    };
    QNetworkReply *binary = get(ytdlpDownloadUrl());
    QNetworkReply *sums = get(ytdlpChecksumsUrl);
    connect(binary, &QNetworkReply::finished, this, [finished, binary] { finished(binary, &Fetch::binary); });
    connect(sums, &QNetworkReply::finished, this, [finished, sums] { finished(sums, &Fetch::sums); });
}

void downloadPanelWrap::runUpdateCheck()
{
    setStatus("Checking for yt-dlp updates...");
    m_updateLastLine.clear();

    const bool started = m_updater.start(
        {ytdlpPath(), "-U"}, {},
        [this](const std::string &line) {
            if (line.rfind("WARNING", 0) == 0)
                return;
            const QString text = QString::fromStdString(line).trimmed();
            QMetaObject::invokeMethod(this, [this, text] { m_updateLastLine = text; }, Qt::QueuedConnection);
        },
        [this](int exitCode, bool) {
            QMetaObject::invokeMethod(this, [this, exitCode] {
                setStatus(QString::fromStdString(updateStatus(m_updateLastLine.toStdString(), exitCode)));
                preparationDone();
            }, Qt::QueuedConnection);
        });
    if (!started) {
        setStatus("Update check skipped.");
        preparationDone();
    }
}

void downloadPanelWrap::preparationDone()
{
    m_preparing = false;
    if (m_waiting) {
        const DownloadRequest waiting = *m_waiting;
        m_waiting.reset();
        startDownload(waiting);
    }
}

void downloadPanelWrap::downloadVideo(const QString &url, const QString &resolution, bool pauseBetweenVideos)
{
    DownloadRequest r;
    r.kind = DownloadKind::Video;
    r.url = url.trimmed().toStdString();
    r.resolution = resolution.toStdString();
    r.pauseBetweenVideos = pauseBetweenVideos;
    request(r);
}

void downloadPanelWrap::downloadAudio(const QString &url, const QString &quality, bool pauseBetweenVideos)
{
    DownloadRequest r;
    r.kind = DownloadKind::Audio;
    r.url = url.trimmed().toStdString();
    r.audioQuality = quality.toStdString();
    r.pauseBetweenVideos = pauseBetweenVideos;
    request(r);
}

void downloadPanelWrap::request(DownloadRequest request)
{
    if (request.url.empty() || m_isBusy)
        return;

    setBusy(true);
    setProgress(0.0);
    if (m_preparing) {
        m_waiting = std::move(request);
        setStatus("Waiting for the yt-dlp update check to finish...");
        return;
    }
    startDownload(request);
}

void downloadPanelWrap::startDownload(const DownloadRequest &request)
{
    auto fail = [this](const QString &message) {
        setStatus(message);
        setBusy(false);
    };
    if (!ytdlpInstalled())
        return fail("Download failed: yt-dlp is not installed. Restart the app to set it up again.");
    const std::string ffmpeg = ffmpegPath();
    if (ffmpeg.empty())
        return fail("Download failed: ffmpeg is not installed. It is needed to make MP4 and MP3 files ("
                    + QString::fromStdString(ffmpegInstallHint()) + ").");

    const std::string folder = m_downloadPath.toStdString();
    seedArchive(folder, request.kind);

    m_parser.reset();
    setStatus(QString::fromStdString(m_parser.status()));

    const bool started = m_downloader.start(
        downloadArgs(ytdlpPath(), folder, request, ffmpegOnPath() ? std::string() : ffmpeg), folder,
        [this](const std::string &line) {
            QMetaObject::invokeMethod(this, [this, line] {
                if (m_parser.feedLine(line)) {
                    setProgress(m_parser.progress());
                    setStatus(QString::fromStdString(m_parser.status()));
                }
            }, Qt::QueuedConnection);
        },
        [this](int exitCode, bool cancelled) {
            QMetaObject::invokeMethod(this, [this, exitCode, cancelled] {
                m_parser.finish(exitCode, cancelled);
                if (!cancelled && exitCode == 0)
                    setProgress(1.0);
                setStatus(QString::fromStdString(m_parser.status()));
                setBusy(false);
            }, Qt::QueuedConnection);
        });
    if (!started)
        fail("Download failed: could not start yt-dlp.");
}

void downloadPanelWrap::cancel()
{
    if (m_waiting) {
        m_waiting.reset();
        setStatus("Download cancelled.");
        setBusy(false);
        return;
    }
    if (m_downloader.running()) {
        setStatus("Cancelling...");
        m_downloader.cancel();
    }
}

QString downloadPanelWrap::downloadFolderUrl() const
{
    return QUrl::fromLocalFile(m_downloadPath).toString();
}

void downloadPanelWrap::openDownloadFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_downloadPath));
}

void downloadPanelWrap::setStatus(const QString &status)
{
    if (status != m_status) {
        m_status = status;
        emit statusChanged();
    }
}

void downloadPanelWrap::setProgress(double progress)
{
    if (progress != m_progress) {
        m_progress = progress;
        emit progressChanged();
    }
}

void downloadPanelWrap::setBusy(bool busy)
{
    if (busy != m_isBusy) {
        m_isBusy = busy;
        emit isBusyChanged();
    }
}
