#include "networkservice.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonDocument>
#include <QDebug>

NetworkService::NetworkService(QObject *parent)
    : QObject(parent), m_socket(new QTcpSocket(this)), m_nextBlockSize(0)
{
    // Связь: подключение, чтение и отключение сокета — на внутренние обработчики
    connect(m_socket, &QTcpSocket::connected, this, &NetworkService::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkService::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkService::onDisconnected);
}


void NetworkService::connectToServer(const QString& host, quint16 port) {
    qDebug() << "[NetworkService] Attempting to connect to" << host << ":" << port;
    m_socket->connectToHost(host, port);
}


void NetworkService::sendJson(const QJsonObject& json)
{
    qDebug() << "---------------------------------";
    qDebug() << "[NetworkService] Preparing to send JSON of type:" << json["type"].toString();
    qDebug() << "[NetworkService] Full JSON content:" << json;
    qDebug() << "---------------------------------";

    // Сериализация объекта в QByteArray.
    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);

    // Собираем блок данных для отправки по протоколу (размер + данные)
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);

    // Зарезервировать место под размер блока
    out << (quint32)0;
    // Записать сами данные (JSON)
    out << jsonData;
    // Вернуться в начало и дополнить реальным размером блока
    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));

    // Отправляем подготовленный блок в TCP-соединение
    m_socket->write(block);
}


void NetworkService::onConnected() {
    qDebug() << "[NetworkService] Socket connected.";
    emit connected();
}


void NetworkService::onDisconnected() {
    qDebug() << "[NetworkService] Socket disconnected.";
    emit disconnected();
}


void NetworkService::onReadyRead(){
    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_6_2);

    // Внутренний цикл — обработка каждого блока (может прийти несколько подряд)
    while(true) {

        // Если пока не знаем размер блока — проверяем его наличие
        if (m_nextBlockSize == 0) {
            if (m_socket->bytesAvailable() < (qint64)sizeof(qint32)) {
                break;  // Ещё не пришёл весь размер блока
            }
            // Получаем размер следующего блока
            in >> m_nextBlockSize;
        }

        // Ждём появления полного тела сообщения
        if (m_socket->bytesAvailable() < m_nextBlockSize) {
            break;  // Данных пока недостаточно — ждём
        }

        // Читаем и декодируем JSON-body
        QByteArray jsonData;
        in >> jsonData;

        // Сброс значения размера для следующего сообщения
        m_nextBlockSize = 0;

        // Парсим JSON из прочитанных байт
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isNull() || !doc.isObject()) {
            qDebug() << "[NetworkService] Failed to parse JSON or it's not an object.";
            continue;  // Ошибка парсинга, игнорируем фрейм
        }

        QJsonObject response = doc.object();
        QString type = response["type"].toString();
        qDebug() << "[NetworkService] Processing message of type" << type;

        // Передаём событие на все подписанные компоненты (логика/слоты)
        emit jsonReceived(doc.object());
    }
}
