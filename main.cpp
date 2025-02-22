#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
#include "mainwindow.h"
#include "cliinterface.h"

int main(int argc, char *argv[]) {
    // Initialize with QCoreApplication first to parse args
    QCoreApplication coreApp(argc, argv);

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
    parser.addHelpOption(); // Adds --help and -h automatically

    // Define CLI mode option
    parser.addOption(QCommandLineOption("cli", "Run in command-line interface mode (non-GUI)"));

    // Define CLI-specific options
    parser.addOption(QCommandLineOption("input", "Input binary file (required in CLI mode)", "file"));
    parser.addOption(QCommandLineOption("bulk-size", "Bulk size in GB (default: 2.0)", "size"));
    parser.addOption(QCommandLineOption("frame-size", "Frame size in bytes (default: 1111)", "bytes"));
    parser.addOption(QCommandLineOption("output-prefix", "Output file prefix (default: input filename)", "prefix"));
    parser.addOption(QCommandLineOption("auto-detect", "Auto-detect frame size using sync word"));
    parser.addOption(QCommandLineOption("sync-word", "Sync word in hex for auto-detection (default: 4711)", "hex"));

    // Process arguments
    parser.process(coreApp);

    // Check for help first
    if (parser.isSet("help")) {
        parser.showHelp(0); // Exit with success code 0 after showing help
    }

    // Decide between CLI and GUI mode
    if (parser.isSet("cli")) {
        CliInterface cli;

        // Validate and set CLI properties
        if (parser.isSet("input")) {
            cli.setInputFile(parser.value("input"));
        } else {
            QTextStream err(stderr);
            err << "Error: Input file is required in CLI mode. Use --input <file>\n";
            parser.showHelp(1); // Exit with error code 1
        }

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
            out << "Progress: " << percentage << "%\n";
            out.flush();
        });
        QObject::connect(&cli, &CliInterface::statusChanged, [&out](const QString &status) {
            out << status << "\n";
            out.flush();
        });
        QObject::connect(&cli, &CliInterface::operationError, [&out](const QString &error) {
            out << "Error: " << error << "\n";
            out.flush();
        });

        // Start splitting
        cli.startSplit();
        return coreApp.exec();
    } else {
        // GUI mode
        QApplication app(argc, argv); // Upgrade to QApplication for GUI
        qmlRegisterType<MainWindow>("BinarySplitter", 1, 0, "MainWindow");
        MainWindow mainWindow;
        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty("mainWindow", &mainWindow);
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        if (engine.rootObjects().isEmpty()) {
            return -1;
        }
        return app.exec();
    }
}
