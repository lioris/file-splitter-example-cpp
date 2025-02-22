#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
#include "mainwindow.h"
#include "cliinterface.h"
#include <QTextStream>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
    // Step 1: Manually check for --cli and --help
    bool isCli = false;
    bool showHelp = false;
    for (int i = 1; i < argc; ++i) {
        if (qstrcmp(argv[i], "--cli") == 0) {
            isCli = true;
        }
        if (qstrcmp(argv[i], "--help") == 0 || qstrcmp(argv[i], "-h") == 0) {
            showHelp = true;
            break; // Exit loop once help is detected
        }
    }

    // Step 2: Show help and exit if requested
    if (showHelp) {
#ifdef Q_OS_WIN
        // Ensure a console is available for help output on Windows
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();
        }
        FILE *dummy;
        if (freopen_s(&dummy, "CONOUT$", "w", stdout) != 0) {
            return 1; // Exit if console redirection fails
        }
#else \
    // On Unix-like systems, console is already available via shell
#endif
        QTextStream out(stdout);
        out << "Binary File Splitter\n"
            << "Splits large binary files into smaller chunks based on bulk size or frame size.\n\n"
            << "Usage examples:\n"
            << "  CLI mode with explicit frame size:\n"
            << "    " << QFileInfo(argv[0]).fileName() << " --cli --input file.bin --bulk-size 2.0 --frame-size 1111\n"
            << "  CLI mode with auto-detected frame size:\n"
            << "    " << QFileInfo(argv[0]).fileName() << " --cli --input file.bin --auto-detect --sync-word 4711\n"
            << "  GUI mode:\n"
            << "    " << QFileInfo(argv[0]).fileName() << "\n\n"
            << "Options:\n"
            << "  --cli                       Run in command-line interface mode (non-GUI)\n"
            << "  --input <file>              Input binary file (required in CLI mode)\n"
            << "  --bulk-size <size>          Bulk size in GB (default: 2.0)\n"
            << "  --frame-size <bytes>        Frame size in bytes (default: 1111)\n"
            << "  --output-prefix <prefix>    Output file prefix (default: input filename)\n"
            << "  --auto-detect               Auto-detect frame size using sync word\n"
            << "  --sync-word <hex>           Sync word in hex for auto-detection (default: 4711)\n"
            << "  --help, -h                  Show this help message\n";
        out.flush();
#ifdef Q_OS_WIN
        FreeConsole(); // Clean up console after help
#endif
        return 0; // Exit successfully
    }

    // Step 3: Create the appropriate application object
    if (isCli) {
        // CLI mode
#ifdef Q_OS_WIN
        // Allocate a console for CLI mode
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();
        }
        FILE *dummy;
        if (freopen_s(&dummy, "CONOUT$", "w", stdout) != 0 || freopen_s(&dummy, "CONOUT$", "w", stderr) != 0) {
            return 1; // Exit if console redirection fails
        }
#endif
        QCoreApplication app(argc, argv);

        // Step 4: Use QCommandLineParser after app creation
        QCommandLineParser parser;
        parser.addOption(QCommandLineOption("cli", "Run in command-line interface mode (non-GUI)"));
        parser.addOption(QCommandLineOption("input", "Input binary file (required in CLI mode)", "file"));
        parser.addOption(QCommandLineOption("bulk-size", "Bulk size in GB (default: 2.0)", "size"));
        parser.addOption(QCommandLineOption("frame-size", "Frame size in bytes (default: 1111)", "bytes"));
        parser.addOption(QCommandLineOption("output-prefix", "Output file prefix (default: input filename)", "prefix"));
        parser.addOption(QCommandLineOption("auto-detect", "Auto-detect frame size using sync word"));
        parser.addOption(QCommandLineOption("sync-word", "Sync word in hex for auto-detection (default: 4711)", "hex"));
        parser.process(app);

        // Set up CLI functionality
        CliInterface cli;
        if (!parser.isSet("input")) {
            QTextStream err(stderr);
            err << "Error: Input file is required in CLI mode. Use --input <file>\n";
            return 1;
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

        cli.startSplit();
        int result = app.exec();
#ifdef Q_OS_WIN
        FreeConsole(); // Clean up console after CLI mode
#endif
        return result;
    } else {
        // GUI mode - No console allocation
        QApplication app(argc, argv);
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
