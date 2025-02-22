#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
#include "mainwindow.h"
#include "cliinterface.h"

int main(int argc, char *argv[]) {
    // Avoid creating an application object until we know the mode

    // Set up command-line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(
        "Binary File Splitter\n"
        "Splits large binary files into smaller chunks based on bulk size or frame size.\n"
        "\nUsage examples:\n"
        "  CLI mode with explicit frame size:\n"
        "    binarysplitter --cli --input file.bin --bulk-size 2.0 --frame-size 1111\n"
        "  CLI mode with auto-detected frame size:\n"
        "    binarysplitter --cli --input file.bin --auto-detect --sync-word 4711\n"
        "  GUI mode:\n"
        "    binarysplitter\n"
        );
    parser.addHelpOption(); // Adds --help and -h

    // Define CLI mode option
    parser.addOption(QCommandLineOption("cli", "Run in command-line interface mode (non-GUI)"));

    // Define CLI-specific options
    parser.addOption(QCommandLineOption("input", "Input binary file (required in CLI mode)", "file"));
    parser.addOption(QCommandLineOption("bulk-size", "Bulk size in GB (default: 2.0)", "size"));
    parser.addOption(QCommandLineOption("frame-size", "Frame size in bytes (default: 1111)", "bytes"));
    parser.addOption(QCommandLineOption("output-prefix", "Output file prefix (default: input filename)", "prefix"));
    parser.addOption(QCommandLineOption("auto-detect", "Auto-detect frame size using sync word"));
    parser.addOption(QCommandLineOption("sync-word", "Sync word in hex for auto-detection (default: 4711)", "hex"));

    // Parse arguments manually without an application object
    parser.parse(QCoreApplication::arguments());

    // Handle help option first
    if (parser.isSet("help")) {
        QTextStream out(stdout);
        out << parser.helpText() << "\n";
        return 0; // Exit after showing help
    }

    // Decide between CLI and GUI mode
    if (parser.isSet("cli")) {
        // CLI mode: Create QCoreApplication
        QCoreApplication app(argc, argv);

        CliInterface cli;

        // Validate and set CLI options
        if (!parser.isSet("input")) {
            QTextStream err(stderr);
            err << "Error: Input file is required in CLI mode. Use --input <file>\n";
            err << parser.helpText() << "\n";
            return 1; // Exit with error
        }
        cli.setInputFile(parser.value("input"));

        if (parser.isSet("bulk-size")) {
            bool ok;
            double size = parser.value("bulk-size").toDouble(&ok);
            if (ok && size > 0) cli.setBulkSizeGb(size);
        }

        if (parser.isSet("frame-size")) {
            bool ok;
            int size = parser.value("frame-size").toInt(&ok);
            if (ok && size > 0) cli.setFrameSize(size);
        }

        if (parser.isSet("output-prefix")) {
            cli.setOutputPrefix(parser.value("output-prefix"));
        }

        if (parser.isSet("auto-detect")) {
            cli.setAutoDetect(true);
            if (parser.isSet("sync-word")) {
                cli.setSyncWord(parser.value("sync-word"));
            }
        } else {
            cli.setAutoDetect(false);
        }

        // Connect signals for console output
        QTextStream out(stdout);
        QObject::connect(&cli, &CliInterface::progressUpdated, [&out](int percentage) {
            out << QString("Progress: %1%").arg(percentage, 3, 10, QChar(' ')) << "\r";
            out.flush();
        });
        QObject::connect(&cli, &CliInterface::statusChanged, [&out](const QString &status) {
            out << "\n" << status << "\n";
            out.flush();
        });
        QObject::connect(&cli, &CliInterface::operationError, [&out](const QString &error) {
            out << "\nError: " << error << "\n";
            out.flush();
        });

        // Start CLI operation
        cli.startSplit();
        return app.exec();
    } else {
        // GUI mode: Create QApplication
        QApplication app(argc, argv);
        qmlRegisterType<MainWindow>("BinarySplitter", 1, 0, "MainWindow");
        MainWindow mainWindow;
        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty("mainWindow", &mainWindow);
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        if (engine.rootObjects().isEmpty()) {
            return -1; // Exit if QML fails to load
        }
        return app.exec();
    }
}
