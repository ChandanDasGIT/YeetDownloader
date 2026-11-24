#pragma once

#include "backend/downloadPanel/processRunner.h"
#include "backend/downloadPanel/ytdlpCommand.h"
#include "backend/downloadPanel/ytdlpOutputParser.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

#include <optional>

class downloadPanelWrap : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath NOTIFY downloadPathChanged)
    Q_PROPERTY(QString downloadFolderUrl READ downloadFolderUrl NOTIFY downloadPathChanged)

public:
    explicit downloadPanelWrap(QObject *parent = nullptr);

    Q_INVOKABLE void downloadVideo(const QString &url, const QString &resolution, bool pauseBetweenVideos);
    Q_INVOKABLE void downloadAudio(const QString &url, const QString &quality, bool pauseBetweenVideos);
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void openDownloadFolder();

    double progress() const { return m_progress; }
    QString status() const { return m_status; }
    bool isBusy() const { return m_isBusy; }
    QString downloadPath() const { return m_downloadPath; }
    void setDownloadPath(const QString &path);
    QString downloadFolderUrl() const; // file:// URL, right on Windows too (file:///C:/...)

signals:
    void progressChanged();
    void statusChanged();
    void isBusyChanged();
    void downloadPathChanged();

private:
    void fetchYtdlp();
    void runUpdateCheck();
    void request(DownloadRequest request);
    void startDownload(const DownloadRequest &request);
    void preparationDone();

    void setStatus(const QString &status);
    void setProgress(double progress);
    void setBusy(bool busy);

    QNetworkAccessManager m_net;
    ProcessRunner m_updater;
    ProcessRunner m_downloader;
    YtdlpOutputParser m_parser;

    bool m_preparing = false; // fetching or updating yt-dlp
    std::optional<DownloadRequest> m_waiting;
    QString m_updateLastLine;

    double m_progress = 0.0;
    QString m_status = "Initializing...";
    bool m_isBusy = false;
    QString m_downloadPath;
};
