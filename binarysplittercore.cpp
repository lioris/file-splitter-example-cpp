#include "binarysplittercore.h"
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QCoreApplication>  // For processEvents()

BinarySplitterCore::BinarySplitterCore(QObject *parent) : QObject(parent), m_stopFlag(false) {}

QJsonObject BinarySplitterCore::loadConfigDefaults(const QString &configFilePath) {
    QJsonObject defaults;
    defaults["default_frame_size_bytes"] = 1111;
    defaults["default_bulk_size_gb"] = 2.0;
    defaults["default_sync_word_hex"] = "4711";

    QFile file(configFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject config = doc.object();
            for (const QString &key : config.keys()) {
                defaults[key] = config[key];
            }
        }
    }
    return defaults;
}

int BinarySplitterCore::detectFrameSize(const QString &inputFilePath, const QString &syncWordHex,
                                        int searchChunkSize, int maxFrameSizeGuess) {
    QByteArray syncWordBytes = QByteArray::fromHex(syncWordHex.toUtf8());
    if (syncWordBytes.isEmpty()) return -1;

    QFile file(inputFilePath);
    if (!file.open(QIODevice::ReadOnly)) return -1;

    QByteArray chunk = file.read(searchChunkSize);
    int firstSyncIndex = chunk.indexOf(syncWordBytes);
    if (firstSyncIndex == -1) return -1;

    file.seek(firstSyncIndex + syncWordBytes.length());
    QByteArray nextChunk = file.read(maxFrameSizeGuess);
    int secondSyncIndex = nextChunk.indexOf(syncWordBytes);

    if (secondSyncIndex == -1) return -1;
    int detectedFrameSize = secondSyncIndex + syncWordBytes.length();

    return (detectedFrameSize > syncWordBytes.length()) ? detectedFrameSize : -1;
}

void BinarySplitterCore::splitBinaryFile(const QString &inputFilePath, double bulkSizeGb,
                                         int frameSizeBytes, const QString &outputPrefix) {
    const qint64 bufferSize = 1024 * 1024; // 1MB buffer
    qint64 bulkSizeBytes = static_cast<qint64>(bulkSizeGb * 1024 * 1024 * 1024);
    int bulkCount = 1;
    qint64 currentBulkSize = 0;
    QString outputDir = QFileInfo(inputFilePath).absolutePath();
    QString prefix = (outputPrefix == "output_bulk") ?
                         QFileInfo(inputFilePath).baseName() : outputPrefix;

    QFile inFile(inputFilePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        emit splitError("Input file not found: " + inputFilePath);
        return;
    }

    qint64 totalSize = inFile.size();
    qint64 bytesProcessed = 0;
    QFile *outFile = nullptr;
    int lastPercentage = -1;
    QByteArray buffer;

    while (!inFile.atEnd()) {
        if (m_stopFlag) {
            emit splitFinished("Split operation stopped.");
            if (outFile) delete outFile;
            return;
        }

        // Read a large chunk
        buffer = inFile.read(bufferSize);
        if (buffer.isEmpty()) break;

        qint64 offset = 0;
        while (offset < buffer.size()) {
            qint64 remaining = buffer.size() - offset;
            qint64 bytesToProcess = qMin(remaining, static_cast<qint64>(frameSizeBytes));
            QByteArray frameData = buffer.mid(offset, bytesToProcess);

            if (!outFile || (currentBulkSize + frameData.length() > bulkSizeBytes)) {
                if (outFile) {
                    outFile->close();
                    delete outFile;
                }
                QString outPath = QString("%1/%2_%3.bin").arg(outputDir, prefix).arg(bulkCount++, 3, 10, QChar('0'));
                outFile = new QFile(outPath);
                if (!outFile->open(QIODevice::WriteOnly)) {
                    emit splitError("Cannot create output file: " + outPath);
                    delete outFile;
                    return;
                }
                currentBulkSize = 0;
            }

            outFile->write(frameData);
            currentBulkSize += frameData.length();
            bytesProcessed += frameData.length();
            offset += frameData.length();

            if (totalSize > 0) {
                int percentage = static_cast<int>((bytesProcessed * 100) / totalSize);
                if (percentage > lastPercentage) {
                    emit progressUpdated(percentage);
                    lastPercentage = percentage;
                }
            }
        }

        QCoreApplication::processEvents(); // Process events to handle stopOperation
    }

    if (outFile) {
        outFile->close();
        delete outFile;
    }
    emit splitFinished("Binary file splitting complete.");
}

void BinarySplitterCore::stopOperation() {
    m_stopFlag = true;
    emit stopRequested();
}

void BinarySplitterCore::resetStopFlag() {
    m_stopFlag = false; // Reset the stop flag for a new operation
}
