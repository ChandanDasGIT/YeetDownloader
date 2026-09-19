#include "downloader.h"
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFile>

Downloader::Downloader(QObject *parent) : QObject(parent)
{
    m_downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    m_binDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/YeetDownloader";
    QDir().mkpath(m_binDir);
    m_ytdlpPath = m_binDir + "/yt-dlp";

    connect(&m_process, &QProcess::readyReadStandardOutput, this, &Downloader::handleOutput);
    connect(&m_process, &QProcess::readyReadStandardError, this, &Downloader::handleOutput);
    connect(&m_process, &QProcess::finished, this, &Downloader::handleFinished);

    ensureBinaryReady();
}

void Downloader::setDownloadPath(const QString &path)
{
    // FolderDialog emits URL schemes (e.g. file:///home/user/...)
    QString local = path.startsWith("file://") ? QUrl(path).toLocalFile() : path;
    if (m_downloadPath != local && !local.isEmpty()) {
        m_downloadPath = local;
        emit downloadPathChanged();
    }
}

void Downloader::ensureBinaryReady()
{
    if (!QFile::exists(m_ytdlpPath)) {
        downloadInitialBinary();
    } else {
        runUpdateCheck();
    }
}

void Downloader::downloadInitialBinary()
{
    m_status = "Setting up standalone yt-dlp binary...";
    emit statusChanged();

    QUrl url("https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp");
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_netManager.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(m_ytdlpPath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();

                file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                    QFileDevice::ReadGroup | QFileDevice::ExeGroup |
                                    QFileDevice::ReadOther | QFileDevice::ExeOther);

                m_status = "yt-dlp setup complete.";
                emit statusChanged();
                runUpdateCheck();
            }
        } else {
            m_status = "Failed to fetch standalone binary: " + reply->errorString();
            emit statusChanged();
        }
        reply->deleteLater();
    });
}

void Downloader::runUpdateCheck()
{
    m_status = "Checking for yt-dlp updates...";
    emit statusChanged();

    auto *updateProc = new QProcess(this);
    connect(updateProc, &QProcess::finished, this, [this, updateProc](int exitCode) {
        if (exitCode == 0) {
            QString out = QString::fromUtf8(updateProc->readAllStandardOutput()).trimmed();
            QStringList lines = out.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
            m_status = lines.isEmpty() ? "yt-dlp is up to date." : lines.last().trimmed();
        } else {
            m_status = "Update check skipped.";
        }
        emit statusChanged();
        updateProc->deleteLater();
    });

    updateProc->start(m_ytdlpPath, QStringList() << "-U");
}

void Downloader::startDownload(const QString &url, const QStringList &extraArgs)
{
    if (url.trimmed().isEmpty() || m_isBusy)
        return;

    m_isBusy = true;
    m_progress = 0.0;
    m_status = "Starting download...";
    emit isBusyChanged();
    emit progressChanged();
    emit statusChanged();

    QStringList args;
    args << "--newline" << "--progress";
    args << extraArgs;
    args << url.trimmed();

    m_process.setWorkingDirectory(m_downloadPath);
    m_process.start(m_ytdlpPath, args);
}

void Downloader::downloadVideo(const QString &url, const QString &resolution)
{
    QString formatSelector;
    if (resolution == "Best Available") {
        formatSelector = "bestvideo+bestaudio/best";
    } else {
        // Extracts height like "1080p" -> "1080"
        QString res = resolution;
        res.remove("p");
        formatSelector = QString("bestvideo[height<=%1]+bestaudio/best[height<=%1]").arg(res);
    }

    startDownload(url, {
        "-f", formatSelector,
        "--merge-output-format", "mp4"
    });
}

void Downloader::downloadAudio(const QString &url, const QString &quality)
{
    // quality values: "0" (Best/VBR ~320k), "128k", "192k", "320k"
    startDownload(url, {
        "-x",
        "--audio-format", "mp3",
        "--audio-quality", quality
    });
}

void Downloader::openDownloadFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_downloadPath));
}

void Downloader::handleOutput()
{
    QByteArray raw = m_process.readAllStandardOutput() + m_process.readAllStandardError();
    QString text = QString::fromUtf8(raw);

    QRegularExpressionMatch match = m_percentRegex.match(text);
    if (match.hasMatch()) {
        m_progress = match.captured(1).toDouble() / 100.0;
        m_status = QString("Downloading... %1%").arg(match.captured(1));
        emit progressChanged();
        emit statusChanged();
    }
}

void Downloader::handleFinished(int exitCode, QProcess::ExitStatus)
{
    m_isBusy = false;
    emit isBusyChanged();

    if (exitCode == 0) {
        m_progress = 1.0;
        m_status = "Download Complete!";
    } else {
        m_status = QString("Download failed (Exit code %1)").arg(exitCode);
    }
    emit progressChanged();
    emit statusChanged();
}