
# Архитектурный рефакторинг: Переход на мультипротокольную модель (TCP + WebSocket)

Этот документ описывает ключевые изменения, внесенные в архитектуру сервера, в результате которых была осуществлена миграция от однопротокольной модели на основе наследования (`QTcpServer`) к более гибкой мультипротокольной модели на основе композиции (`QObject`).

## 1. Мотивация для изменений

Изначальная архитектура сервера была проста и эффективна, но имела существенное ограничение: она поддерживала **только TCP-соединения**. Это исключало возможность прямого взаимодействия с веб-клиентами (браузерами), которые для двусторонней связи в реальном времени используют протокол **WebSocket**.

Основная цель рефакторинга — расширить функциональность сервера, добавив поддержку WebSocket, и заложить фундамент для дальнейшего масштабирования. Это позволяет нашему приложению обслуживать не только нативные десктопные клиенты, но и веб-приложения, значительно расширяя его охват.

## 2. Сравнение архитектур "До" и "После"

#### Старая архитектура (Версия 2)

*   **Принцип:** Наследование. Класс `Server` наследовался напрямую от `QTcpServer` (`class Server : public QTcpServer`).
*   **Протокол:** Только TCP.
*   **Обработка подключений:** Переопределение виртуального метода `incomingConnection()`.
*   **Хранение клиентов:** Использовался `QMap<QString, QTcpSocket*>`, что жестко привязывало логику к одному типу сокета.

#### Новая архитектура (Версия 1)

*   **Принцип:** Композиция. Класс `Server` является наследником `QObject` и *управляет* экземплярами `QTcpServer` и `QWebSocketServer` как своими членами.
*   **Протоколы:** TCP и WebSocket.
*   **Обработка подключений:** Использование сигналов (`newConnection`) от каждого серверного объекта и их привязка к соответствующим слотам.
*   **Хранение клиентов:** Использование `QMap<QString, QObject*>`. Это позволяет хранить указатели на сокеты разных типов (`QTcpSocket*`, `QWebSocket*`) полиморфно.

## 3. Ключевые этапы перехода и изменения в коде

Переход потребовал нескольких фундаментальных изменений в кодовой базе.

### Шаг 1: Изменение базового класса

Первым делом был изменен базовый класс `Server`.

**Было:**
```cpp
// server.h (Версия 1)
class Server : public QTcpServer { ... };
```

**Стало:**
```cpp
// server.h (Версия 2)
class Server : public QObject { ... };
```
**Почему?** Класс `Server` перестал *быть* TCP-сервером. Вместо этого он стал *управленцем* серверов. `QObject` является идеальным базовым классом для этого, так как он предоставляет механизм сигналов и слотов, необходимый для асинхронной обработки событий.

### Шаг 2: Введение серверных объектов (Композиция)

Вместо наследования мы добавили указатели на `QTcpServer` и `QWebSocketServer` как члены класса.

**Было:**
Класс сам был сервером.

**Стало:**
```cpp
// server.h (Версия 2)
private:
    QTcpServer *m_tcpServer;
    QWebSocketServer *m_webSocketServer;
```
В конструкторе эти объекты инициализируются:
```cpp
// server.cpp (Версия 2)
Server::Server(QObject *parent) : QObject(parent){
    m_tcpServer = new QTcpServer(this);
    m_webSocketServer = new QWebSocketServer("MessengerServer", QWebSocketServer::NonSecureMode, this);
    // ...
}
```

### Шаг 3: Обновление логики запуска сервера

Метод `listen()` был адаптирован для запуска обоих серверов.

**Было:**
`listen()` был унаследован от `QTcpServer` и вызывался напрямую.

**Стало:**
```cpp
// server.cpp (Версия 2)
bool Server::listen(const QHostAddress &address, quint16 tcpPort, quint16 wsPort)
{
    bool tcpSuccess = m_tcpServer->listen(address, tcpPort);
    bool wsSuccess = m_webSocketServer->listen(address, wsPort);
    return tcpSuccess && wsSuccess;
}
```

### Шаг 4: Новый механизм обработки подключений

Метод `incomingConnection()` был удален. Вместо него мы используем сигналы и слоты для каждого протокола.

**Было:**
```cpp
// server.cpp (Версия 1)
void Server::incomingConnection(qintptr socketDescriptor) { ... }
```

**Стало:**
В конструкторе мы соединяем сигналы `newConnection` с новыми слотами:
```cpp
// server.cpp (Версия 2)
connect(m_tcpServer, &QTcpServer::newConnection, this, &Server::onNewTcpConnection);
connect(m_webSocketServer, &QWebSocketServer::newConnection, this, &Server::onNewWebSocketConnection);
```
И реализуем эти слоты:
```cpp
// server.cpp (Версия 2)
void Server::onNewTcpConnection() {
    QTcpSocket *socket = m_tcpServer->nextPendingConnection();
    // ... логика для TCP
}

void Server::onNewWebSocketConnection() {
    QWebSocket *socket = m_webSocketServer->nextPendingConnection();
    // ... логика для WebSocket
}
```

### Шаг 5: Унификация управления клиентами

Для хранения сокетов разных типов `QMap` был параметризован базовым классом `QObject*`.

**Было:**
```cpp
// server.h (Версия 1)
QMap<QString, QTcpSocket*> loggedInUsers;
```

**Стало:**
```cpp
// server.h (Версия 2)
QMap<QString, QObject*> m_clients; // Для хранения сокетов по имени пользователя
QMap<QObject*, QString> m_clientsReverse; // Для обратного поиска имени по сокету
```
**Почему?** `QObject` является общим предком для `QTcpSocket` и `QWebSocket`, что позволяет хранить их в одной коллекции. Двусторонняя карта (`m_clientsReverse`) упрощает поиск пользователя при получении события от сокета (например, `disconnected` или `readyRead`).

### Шаг 6: Абстрагирование отправки данных

Функция `sendJson` была модифицирована для работы с обоими типами сокетов с использованием `qobject_cast`.

**Было:**
```cpp
// server.cpp (Версия 1)
void Server::sendJson(QTcpSocket* socket, const QJsonObject& response) {
    socket->write(block);
}
```

**Стало:**
```cpp
// server.cpp (Версия 2)
void Server::sendJson(QObject* socket, const QJsonObject& json)
{
    if (!socket) return;
    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);

    // Пытаемся привести к QTcpSocket
    if (auto tcpSocket = qobject_cast<QTcpSocket*>(socket)) {
        // Логика отправки для TCP с указанием размера блока
        tcpSocket->write(block);
    } 
    // Если не получилось, пытаемся привести к QWebSocket
    else if (auto webSocket = qobject_cast<QWebSocket*>(socket)) {
        webSocket->sendTextMessage(QString::fromUtf8(jsonData));
    }
}
```

### Шаг 7: Адаптация обработчиков (Handlers)

Сигнатуры всех функций-обработчиков были изменены, чтобы принимать `QObject*` вместо `QTcpSocket*`. Это делает их независимыми от протокола.

**Было:**
```cpp
// server.h (Версия 1)
using Handler = void (Server::*)(QTcpSocket*, const QJsonObject&);
void handleLogin(QTcpSocket* socket, const QJsonObject& request);
```

**Стало:**
```cpp
// server.h (Версия 2)
using Handler = void (Server::*)(QObject*, const QJsonObject&);
void handleLogin(QObject* socket, const QJsonObject& request);
```

## 4. Преимущества новой архитектуры

1.  **Поддержка WebSocket:** Основная цель достигнута. Сервер теперь может обслуживать веб-клиентов.
2.  **Гибкость и масштабируемость:** Архитектура на основе композиции позволяет легко добавлять новые протоколы в будущем, не затрагивая существующую логику.
3.  **Четкое разделение ответственности:** Логика обработки конкретного протокола (установка соединения, чтение данных) изолирована в своих слотах, в то время как общая бизнес-логика (аутентификация, отправка сообщений) остается универсальной.

Этот рефакторинг превратил простой TCP-сервер в мощную и гибкую платформу для построения кросс-платформенного мессенджера.