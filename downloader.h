#pragma once

#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class Downloader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath NOTIFY downloadPathChanged)

public:
    explicit Downloader(QObject *parent = nullptr);

    Q_INVOKABLE void downloadVideo(const QString &url, const QString &resolution);
    Q_INVOKABLE void downloadAudio(const QString &url, const QString &quality);
    Q_INVOKABLE void openDownloadFolder();

    double progress() const { return m_progress; }
    QString status() const { return m_status; }
    bool isBusy() const { return m_isBusy; }
    QString downloadPath() const { return m_downloadPath; }
    void setDownloadPath(const QString &path);

signals:
    void progressChanged();
    void statusChanged();
    void isBusyChanged();
    void downloadPathChanged();

private slots:
    void handleOutput();
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void ensureBinaryReady();
    void downloadInitialBinary();
    void runUpdateCheck();
    void startDownload(const QString &url, const QStringList &extraArgs);

    QNetworkAccessManager m_netManager;
    QProcess m_process;
    QRegularExpression m_percentRegex{R"((\d+(?:\.\d+)?)%)"};

    double m_progress = 0.0;
    QString m_status = "Initializing...";
    bool m_isBusy = false;

    QString m_downloadPath;
    QString m_binDir;
    QString m_ytdlpPath;
};