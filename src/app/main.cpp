#include "mainwindow.h"
#include "futuristic_theme.h"
#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QLocale>
#include <QDebug>

class SavvyCANApplication : public QApplication
{
public:
    MainWindow *mainWindow;
    
    SavvyCANApplication(int &argc, char **argv) : QApplication(argc, argv)
    {
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen)
        {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            mainWindow->handleDroppedFile(openEvent->file());
        }

        return QApplication::event(event);
    }
};

int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    // Uncomment for verbose debug: qputenv("QT_FATAL_WARNINGS", "1");
#endif

    SavvyCANApplication a(argc, argv);

    //Add a local path for Qt extensions, to allow for per-application extensions.
    a.addLibraryPath("plugins");

    //These things are used by QSettings to set up setting storage
    a.setOrganizationName("EVTV");
    a.setApplicationName("SavvyCAN");
    a.setOrganizationDomain("evtv.me");

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings settings;

    const QIcon appIcon(":/icons/icon.png");
    a.setWindowIcon(appIcon);
    a.setApplicationDisplayName("SavvyCAN");

    // ── Apply saved theme ────────────────────────────────────────────
    QString theme = settings.value("Main/Theme", "dark").toString();
    if (theme == "light") {
        a.setStyleSheet(FuturisticTheme::lightStyleSheet());
        a.setPalette(FuturisticTheme::lightPalette());
    } else if (theme == "highcontrast") {
        a.setStyleSheet(FuturisticTheme::highContrastStyleSheet());
        a.setPalette(FuturisticTheme::highContrastPalette());
    } else {
        a.setStyleSheet(FuturisticTheme::darkStyleSheet());
        a.setPalette(FuturisticTheme::darkPalette());
    }

    QString localeString = settings.value("Main/Language").toString();
    if (localeString.isEmpty()) {
        QLocale sysLocale = QLocale::system();
        localeString = sysLocale.name();
        settings.setValue("Main/Language", localeString);
    }

    QTranslator translator;
    QLocale locale(localeString);
    QString lang = locale.name();
    QString shortLang = locale.name().left(2);

    // Try multiple paths for translation files
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/translations",
        QCoreApplication::applicationDirPath() + "/../translations",
    };

    bool loaded = false;
    for (const QString &dir : searchPaths) {
        if (translator.load("SavvyCAN_" + lang, dir)) {
            loaded = true;
            break;
        }
    }
    if (!loaded) {
        for (const QString &dir : searchPaths) {
            if (translator.load("SavvyCAN_" + shortLang, dir)) {
                loaded = true;
                break;
            }
        }
    }
    a.installTranslator(&translator);

    qInfo() << "Locale Value is:" << locale.name();

    a.mainWindow = new MainWindow();

    int fontSize = settings.value("Main/FontSize", 9).toUInt();
    QFont sysFont = QFont(); //get default font
    sysFont.setPointSize(fontSize);
    a.setFont(sysFont);

    a.mainWindow->show();

    int retCode = a.exec();

    delete a.mainWindow; a.mainWindow = nullptr;

    return retCode;
}
