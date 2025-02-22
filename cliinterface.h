#ifndef CLIINTERFACE_H
#define CLIINTERFACE_H

#include <QObject>
#include <QString>
#include "binarysplittercore.h"

class QThread;

class CliInterface : public QObject {
    Q_OBJECT
public:
    explicit CliInterface(QObject *parent = nullptr);
    ~CliInterface();

    // Setters for parameters
    void setInputFile(const QString &file);
    void setBulkSizeGb(double size);
    void setFrameSize(int size);
    void setOutputPrefix(const QString &prefix);
    void setAutoDetect(bool detect);
    void setSyncWord(const QString &word);
    QString inputFile() const { return m_inputFile; } // For validation in main

    void startSplit();
    void stopSplit();

signals:
    void progressUpdated(int percentage);
    void statusChanged(const QString &status);
    void operationFinished(const QString &message);
    void operationError(const QString &error);
    void stopRequested();

private slots:
    void handleProgress(int percentage);
    void handleFinished(const QString &message);
    void handleError(const QString &error);

private:
    BinarySplitterCore *m_splitter;
    QThread *m_workerThread;
    QString m_inputFile;
    double m_bulkSizeGb;
    int m_frameSize;
    QString m_outputPrefix;
    bool m_autoDetect;
    QString m_syncWord;
};

#endif // CLIINTERFACE_H
