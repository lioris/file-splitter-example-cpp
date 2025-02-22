#include "mainwindow.h"
#include <QFileDialog>
#include <QThread>

MainWindow::MainWindow(QObject *parent) : QObject(parent),
    m_splitter(new BinarySplitterCore),
    m_inputFile(""),
    m_bulkSizeGb(2.0),
    m_frameSize(1111),
    m_outputPrefix("output_bulk"),
    m_autoDetect(false),
    m_syncWord("4711"),
    m_progress(0),
    m_status("Ready"),
    m_isRunning(false),
    m_workerThread(new QThread) {

    QJsonObject defaults = m_splitter->loadConfigDefaults();
    m_bulkSizeGb = defaults["default_bulk_size_gb"].toDouble();
    m_frameSize = defaults["default_frame_size_bytes"].toInt();
    m_syncWord = defaults["default_sync_word_hex"].toString();

    m_splitter->moveToThread(m_workerThread);
    connect(m_splitter, &BinarySplitterCore::progressUpdated, this, &MainWindow::updateProgress);
    connect(m_splitter, &BinarySplitterCore::splitFinished, this, &MainWindow::operationFinished);
    connect(m_splitter, &BinarySplitterCore::splitError, this, &MainWindow::operationError);
    m_workerThread->start();
}

MainWindow::~MainWindow() {
    m_workerThread->quit();
    m_workerThread->wait();
    delete m_workerThread;
    delete m_splitter;
}

void MainWindow::setInputFile(const QString &file) {
    m_inputFile = file;
    emit inputFileChanged();
}

void MainWindow::setBulkSizeGb(double size) {
    if (size > 0 && size <= 1000) {
        m_bulkSizeGb = size;
        emit bulkSizeGbChanged();
    } else {
        m_status = "Bulk size must be between 0 and 1000 GB.";
        emit statusChanged();
    }
}

void MainWindow::setFrameSize(int size) {
    if (size > 0 && size <= 65536) {
        m_frameSize = size;
        emit frameSizeChanged();
    } else {
        m_status = "Frame size must be between 1 and 65536 bytes.";
        emit statusChanged();
    }
}

void MainWindow::setOutputPrefix(const QString &prefix) {
    if (!prefix.trimmed().isEmpty()) {
        m_outputPrefix = prefix;
        emit outputPrefixChanged();
    } else {
        m_status = "Output prefix cannot be empty.";
        emit statusChanged();
    }
}

void MainWindow::setAutoDetect(bool detect) {
    m_autoDetect = detect;
    emit autoDetectChanged();
}

void MainWindow::setSyncWord(const QString &word) {
    QByteArray test = QByteArray::fromHex(word.toUtf8());
    if (!test.isEmpty()) {
        m_syncWord = word;
        emit syncWordChanged();
    } else {
        m_status = "Sync word must be valid hexadecimal.";
        emit statusChanged();
    }
}

void MainWindow::startSplit() {
    if (m_inputFile.isEmpty()) {
        m_status = "Please select an input file.";
        emit statusChanged();
        return;
    }

    int frameSizeToUse;
    if (m_autoDetect) {
        bool ok = QMetaObject::invokeMethod(m_splitter, "detectFrameSize",
                                            Qt::BlockingQueuedConnection,
                                            Q_RETURN_ARG(int, frameSizeToUse),
                                            Q_ARG(QString, m_inputFile),
                                            Q_ARG(QString, m_syncWord));
        if (!ok || frameSizeToUse <= 0) {
            m_status = "Frame size detection failed.";
            emit statusChanged();
            return;
        }
        setFrameSize(frameSizeToUse);
        m_status = QString("Detected frame size: %1 bytes").arg(frameSizeToUse);
    } else {
        frameSizeToUse = m_frameSize;
    }

    setIsRunning(true);
    m_progress = 0;
    m_status = "Splitting in progress...";
    emit progressChanged();
    emit statusChanged();
    QMetaObject::invokeMethod(m_splitter, "splitBinaryFile", Qt::QueuedConnection,
                              Q_ARG(QString, m_inputFile),
                              Q_ARG(double, m_bulkSizeGb),
                              Q_ARG(int, frameSizeToUse),
                              Q_ARG(QString, m_outputPrefix));
}

void MainWindow::stopSplit() {
    if (m_isRunning) {
        QMetaObject::invokeMethod(m_splitter, "stopOperation", Qt::QueuedConnection);
    }
}

void MainWindow::browseFile() {
    QString file = QFileDialog::getOpenFileName(nullptr, "Select Binary File");
    if (!file.isEmpty()) setInputFile(file);
}

void MainWindow::updateProgress(int percentage) {
    m_progress = percentage;
    emit progressChanged();
}

void MainWindow::operationFinished(const QString &message) {
    m_status = message;
    setIsRunning(false);
    emit statusChanged();
}

void MainWindow::operationError(const QString &error) {
    m_status = "Error: " + error;
    setIsRunning(false);
    emit statusChanged();
}

void MainWindow::setIsRunning(bool running) {
    if (m_isRunning != running) {
        m_isRunning = running;
        emit isRunningChanged();
    }
}
