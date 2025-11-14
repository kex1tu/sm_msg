#include "mainwindow.h"  
#include <QFile>          
#include <QDebug>         
#include <QApplication>   
#include <dataservice.h>

/**
 * @brief Точка входа в приложение — инициализация QApplication, установка стиля, запуск главного окна.
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return Код возврата приложения
 */
int main(int argc, char *argv[])
{
    // Qt-приложение (GUI, основной EventLoop)
    QApplication a(argc, argv);

    // Чтение CSS-стилей из ресурсов
    QFile styleFile(":/styles.css");
    if (!styleFile.open(QFile::ReadOnly)) {
        // Не удалось открыть — предупредим
        qWarning() << "Warning: Could not open style file from resources.";
    } else {
        // Прочитать полностью и применить как стиль приложения
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        qDebug() << "Style sheet loaded successfully.";
        styleFile.close();
    }

    // Основной сервис данных
    DataService dataService;

    // Главное окно, DI: пробрасываем DataService
    MainWindow w(&dataService);

    // Показываем главное окно
    w.show();

    // Запускаем главный event loop (блокировка до завершения приложения)
    return a.exec();
}
