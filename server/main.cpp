#include <QCoreApplication>
#include "server.h"

// main.cpp
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server server;

    if (!server.listen()) {
        qCritical() << "Server could not start!";
        return 1;
    }
    return a.exec();
}
