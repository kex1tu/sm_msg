#include "mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    QFile styleFile(":/styles.css"); // Путь к файлу в ресурсах
    if (!styleFile.open(QFile::ReadOnly)) {
        qWarning() << "Warning: Could not open style file from resources.";
    } else {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        qDebug() << "Style sheet loaded successfully.";
        styleFile.close();
    }
    //a.setStyleSheet(styleSheet);

    MainWindow w;
    w.show();
    return a.exec();
}
