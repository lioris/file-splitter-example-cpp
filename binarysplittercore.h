#ifndef BINARYSPLITTERCORE_H
#define BINARYSPLITTERCORE_H

#include <QObject>
#include <QJsonObject>

class BinarySplitterCore : public QObject {
    Q_OBJECT
public:
    explicit BinarySplitterCore(QObject *parent = nullptr);

    Q_INVOKABLE QJsonObject loadConfigDefaults(const QString &configFilePath = "config.json");
    Q_INVOKABLE int detectFrameSize(const QString &inputFilePath, const QString &syncWordHex,
                                    int searchChunkSize = 4096, int maxFrameSizeGuess = 65536);
    Q_INVOKABLE void splitBinaryFile(const QString &inputFilePath, double bulkSizeGb,
                                     int frameSizeBytes, const QString &outputPrefix = "output_bulk");

signals:
    void progressUpdated(int percentage);
    void splitFinished(const QString &message);
    void splitError(const QString &error);
    void stopRequested();

public slots:
    void stopOperation();

private:
    bool m_stopFlag;
};

#endif // BINARYSPLITTERCORE_H
