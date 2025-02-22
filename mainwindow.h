#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QString>
#include "binarysplittercore.h"

class QThread;

class MainWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString inputFile READ inputFile WRITE setInputFile NOTIFY inputFileChanged)
    Q_PROPERTY(double bulkSizeGb READ bulkSizeGb WRITE setBulkSizeGb NOTIFY bulkSizeGbChanged)
    Q_PROPERTY(int frameSize READ frameSize WRITE setFrameSize NOTIFY frameSizeChanged)
    Q_PROPERTY(QString outputPrefix READ outputPrefix WRITE setOutputPrefix NOTIFY outputPrefixChanged)
    Q_PROPERTY(bool autoDetect READ autoDetect WRITE setAutoDetect NOTIFY autoDetectChanged)
    Q_PROPERTY(QString syncWord READ syncWord WRITE setSyncWord NOTIFY syncWordChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)

public:
    explicit MainWindow(QObject *parent = nullptr);
    ~MainWindow();

    QString inputFile() const { return m_inputFile; }
    double bulkSizeGb() const { return m_bulkSizeGb; }
    int frameSize() const { return m_frameSize; }
    QString outputPrefix() const { return m_outputPrefix; }
    bool autoDetect() const { return m_autoDetect; }
    QString syncWord() const { return m_syncWord; }
    int progress() const { return m_progress; }
    QString status() const { return m_status; }
    bool isRunning() const { return m_isRunning; }

public slots:
    void setInputFile(const QString &file);
    void setBulkSizeGb(double size);
    void setFrameSize(int size);
    void setOutputPrefix(const QString &prefix);
    void setAutoDetect(bool detect);
    void setSyncWord(const QString &word);
    void startSplit();
    void stopSplit();
    void browseFile();

signals:
    void inputFileChanged();
    void bulkSizeGbChanged();
    void frameSizeChanged();
    void outputPrefixChanged();
    void autoDetectChanged();
    void syncWordChanged();
    void progressChanged();
    void statusChanged();
    void isRunningChanged();

private slots:
    void updateProgress(int percentage);
    void operationFinished(const QString &message);
    void operationError(const QString &error);

private:
    void setIsRunning(bool running);

    BinarySplitterCore *m_splitter;
    QString m_inputFile;
    double m_bulkSizeGb;
    int m_frameSize;
    QString m_outputPrefix;
    bool m_autoDetect;
    QString m_syncWord;
    int m_progress;
    QString m_status;
    bool m_isRunning;
    QThread *m_workerThread;
};

#endif // MAINWINDOW_H
