#include "cliinterface.h"
#include <QThread>
#include <QTextStream>
#include <qcoreapplication.h>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

CliInterface::CliInterface(QObject *parent) : QObject(parent),
    m_splitter(new BinarySplitterCore),
    m_workerThread(new QThread),
    m_inputFile(""),
    m_bulkSizeGb(2.0),
    m_frameSize(1111),
    m_outputPrefix("output_bulk"),
    m_autoDetect(false),
    m_syncWord("4711") {
    // Load defaults
    QJsonObject defaults = m_splitter->loadConfigDefaults();
    m_bulkSizeGb = defaults["default_bulk_size_gb"].toDouble();
    m_frameSize = defaults["default_frame_size_bytes"].toInt();
    m_syncWord = defaults["default_sync_word_hex"].toString();

    // Set up thread and splitter
    m_splitter->moveToThread(m_workerThread);
    connect(m_splitter, &BinarySplitterCore::progressUpdated, this, &CliInterface::handleProgress);
    connect(m_splitter, &BinarySplitterCore::splitFinished, this, &CliInterface::handleFinished);
    connect(m_splitter, &BinarySplitterCore::splitError, this, &CliInterface::handleError);
    connect(this, &CliInterface::stopRequested, m_splitter, &BinarySplitterCore::stopOperation);
    m_workerThread->start();

#ifdef Q_OS_WIN
    // Windows-specific Ctrl+C handler
    SetConsoleCtrlHandler([](DWORD ctrlType) -> BOOL {
        if (ctrlType == CTRL_C_EVENT) {
            QTextStream out(stdout);
            out << "Ctrl+C received, stopping operation...\n";
            out.flush();
            QCoreApplication::instance()->quit(); // Trigger shutdown
            return TRUE;
        }
        return FALSE;
    }, TRUE);
#endif
}

CliInterface::~CliInterface() {
    stopSplit(); // Ensure operation stops before cleanup
    m_workerThread->quit();
    m_workerThread->wait();
    delete m_workerThread;
    delete m_splitter;
}

void CliInterface::setInputFile(const QString &file) {
    m_inputFile = file;
}

void CliInterface::setBulkSizeGb(double size) {
    if (size > 0 && size <= 1000) {
        m_bulkSizeGb = size;
    }
}

void CliInterface::setFrameSize(int size) {
    if (size > 0 && size <= 65536) {
        m_frameSize = size;
    }
}

void CliInterface::setOutputPrefix(const QString &prefix) {
    if (!prefix.trimmed().isEmpty()) {
        m_outputPrefix = prefix;
    }
}

void CliInterface::setAutoDetect(bool detect) {
    m_autoDetect = detect;
}

void CliInterface::setSyncWord(const QString &word) {
    if (!QByteArray::fromHex(word.toUtf8()).isEmpty()) {
        m_syncWord = word;
    }
}

void CliInterface::startSplit() {
    if (m_inputFile.isEmpty()) {
        emit operationError("Input file is required.");
        return;
    }

    QMetaObject::invokeMethod(m_splitter, "resetStopFlag", Qt::QueuedConnection);

    int frameSizeToUse;
    if (m_autoDetect) {
        bool ok = QMetaObject::invokeMethod(m_splitter, "detectFrameSize",
                                            Qt::BlockingQueuedConnection,
                                            Q_RETURN_ARG(int, frameSizeToUse),
                                            Q_ARG(QString, m_inputFile),
                                            Q_ARG(QString, m_syncWord));
        if (!ok || frameSizeToUse <= 0) {
            emit operationError("Frame size detection failed.");
            return;
        }
        m_frameSize = frameSizeToUse;
        emit statusChanged(QString("Detected frame size: %1 bytes").arg(frameSizeToUse));
    } else {
        frameSizeToUse = m_frameSize;
    }

    emit statusChanged("Splitting in progress...");
    QMetaObject::invokeMethod(m_splitter, "splitBinaryFile", Qt::QueuedConnection,
                              Q_ARG(QString, m_inputFile),
                              Q_ARG(double, m_bulkSizeGb),
                              Q_ARG(int, frameSizeToUse),
                              Q_ARG(QString, m_outputPrefix));
}

void CliInterface::stopSplit() {
    emit stopRequested();
}

void CliInterface::handleProgress(int percentage) {
    emit progressUpdated(percentage);
}

void CliInterface::handleFinished(const QString &message) {
    emit statusChanged(message);
    QCoreApplication::quit();
}

void CliInterface::handleError(const QString &error) {
    emit operationError(error);
    QCoreApplication::quit();
}
