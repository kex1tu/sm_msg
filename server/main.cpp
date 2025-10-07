#include <QCoreApplication>
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server server;
    if (!server.listen(QHostAddress::Any, 1234)) {
        qCritical() << "Server could not start!";
        return 1;
    }
    qInfo() << "Server started on port 1234.";
    return a.exec();
}
