import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    visible: true
    width: 600
    height: 400
    title: "Binary File Splitter"

    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        anchors.margins: 10

        RowLayout {
            Label { text: "Input File:" }
            TextField {
                id: inputFileField
                Layout.fillWidth: true
                text: mainWindow.inputFile
                onTextChanged: {
                    if (text !== mainWindow.inputFile) {
                        mainWindow.inputFile = text
                    }
                }
            }
            Button {
                text: "Browse"
                onClicked: mainWindow.browseFile()
            }
        }

        RowLayout {
            CheckBox {
                text: "Auto-detect Frame Size"
                checked: mainWindow.autoDetect
                onCheckedChanged: mainWindow.autoDetect = checked
            }
            Label { text: "Frame Size (bytes):" }
            TextField {
                id: frameSizeField
                enabled: !mainWindow.autoDetect
                text: mainWindow.frameSize
                onTextChanged: {
                    let value = parseInt(text)
                    if (!isNaN(value) && value >= 0 && text !== mainWindow.frameSize.toString()) {
                        mainWindow.frameSize = value
                    }
                }
                validator: IntValidator { bottom: 0 }
            }
        }

        RowLayout {
            Label { text: "Sync Word (Hex):" }
            TextField {
                id: syncWordField
                enabled: mainWindow.autoDetect
                text: mainWindow.syncWord
                onTextChanged: {
                    if (text !== mainWindow.syncWord) {
                        mainWindow.syncWord = text
                    }
                }
            }
        }

        RowLayout {
            Label { text: "Bulk Size (GB):" }
            TextField {
                id: bulkSizeField
                text: mainWindow.bulkSizeGb.toString()
                onTextChanged: {
                    let value = parseFloat(text)
                    if (!isNaN(value) && value > 0 && text !== mainWindow.bulkSizeGb.toString()) {
                        mainWindow.bulkSizeGb = value
                    }
                }
                validator: DoubleValidator { bottom: 0.1; decimals: 2 }
            }
        }

        RowLayout {
            Label { text: "Output Prefix:" }
            TextField {
                id: outputPrefixField
                text: mainWindow.outputPrefix
                placeholderText: "Default: Input filename"
                onTextChanged: {
                    if (text !== mainWindow.outputPrefix) {
                        mainWindow.outputPrefix = text
                    }
                }
            }
        }

        RowLayout {
            Button {
                text: "Split File"
                onClicked: mainWindow.startSplit()
            }
            Button {
                text: "Stop"
                enabled: mainWindow.progress > 0 && mainWindow.progress < 100
                onClicked: mainWindow.stopSplit()
            }
        }

        ProgressBar {
            Layout.fillWidth: true
            value: mainWindow.progress / 100
        }

        Label {
            text: mainWindow.status
            Layout.fillWidth: true
        }
    }

    Connections {
        target: mainWindow
        function onInputFileChanged() {
            if (inputFileField.text !== mainWindow.inputFile) {
                inputFileField.text = mainWindow.inputFile
            }
        }
        function onFrameSizeChanged() {
            if (frameSizeField.text !== mainWindow.frameSize.toString()) {
                frameSizeField.text = mainWindow.frameSize
            }
        }
        function onSyncWordChanged() {
            if (syncWordField.text !== mainWindow.syncWord) {
                syncWordField.text = mainWindow.syncWord
            }
        }
        function onBulkSizeGbChanged() {
            if (bulkSizeField.text !== mainWindow.bulkSizeGb.toString()) {
                bulkSizeField.text = mainWindow.bulkSizeGb.toString()
            }
        }
        function onOutputPrefixChanged() {
            if (outputPrefixField.text !== mainWindow.outputPrefix) {
                outputPrefixField.text = mainWindow.outputPrefix
            }
        }
    }
}
