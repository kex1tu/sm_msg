#ifndef NETWORKSERVICE_H
#define NETWORKSERVICE_H

#include <QObject>
#include <QJsonObject>

class QTcpSocket;

class NetworkService : public QObject
{
    Q_OBJECT
public:
    /**
 * @brief Конструктор NetworkService — создание TCP-сокета и связывание сигналов.
 *
 * - Создаёт объект QTcpSocket.
 * - Связывает сокетные сигналы (connected, disconnected, readyRead) на приватные слоты класса.
 * - Подготавливает сервис к приёму/отправке команд сетевого протокола.
 */
    explicit NetworkService(QObject *parent = nullptr);

public slots:
    /**
 * @brief Подключается к серверу по заданному адресу и порту.
 *
 * - Логгирует параметры подключения.
 * - Запускает процедуру открытия TCP-соединения.
 *
 * @param host Адрес сервера (IP/FQDN)
 * @param port Порт сервера
 */
    void connectToServer(const QString& host, quint16 port);
    /**
 * @brief Отправляет сериализованный JSON-объект по TCP-соединению в виде бинарного блока.
 *
 * - Логгирует тип сообщения и полный текст JSON для аудита обмена.
 * - Сериализует JSON в QByteArray.
 * - Формирует блок с префиксом размера для корректного декодирования на сервере.
 * - Записывает получившийся блок в сокет.
 *
 * @param json JSON-объект для отправки
 */
    void sendJson(const QJsonObject& json);

signals:
    /**
     * @brief Сигнал успешного подключения к серверу (TCP-сессия установлена).
     */
    void connected();

    /**
     * @brief Сигнал разрыва TCP-соединения с сервером.
     */
    void disconnected();

    /**
     * @brief Сигнал поступления нового сообщения в виде JSON-объекта (универсальный обмен).
     * @param jsonDoc Принятый JSON-объект
     */
    void jsonReceived(const QJsonObject& jsonDoc);

    /**
     * @brief Сигнал о входящем запросе звонка от другого пользователя.
     * @param fromUser От кого поступил запрос
     * @param callId Идентификатор звонка
     * @param callerIp IP-адрес вызывающей стороны
     * @param callerPort UDP-порт вызывающей стороны
     */
    void callRequestReceived(const QString& fromUser, const QString& callId,
                             const QString& callerIp, quint16 callerPort);

    /**
     * @brief Сигнал получения подтверждения принятия звонка со стороны собеседника.
     * @param calleeIp IP-адрес согласившейся стороны
     * @param calleePort UDP-порт согласившейся стороны
     */
    void callAcceptedReceived(const QString& calleeIp, quint16 calleePort);

    /**
     * @brief Сигнал получения отказа (reject) на звонок.
     */
    void callRejectedReceived();

    /**
     * @brief Сигнал завершения текущего звонка (call end).
     */
    void callEndedReceived();

    /**
     * @brief Сигнал отказа при запросе звонка — с текстовой причиной для UI.
     * @param reason Объяснение причины отказа
     */
    void callRequestFailure(const QString& reason);


private slots:
    /**
 * @brief Слот обработки события подключения сокета.
 *
 * - Логгирует факт подключения.
 * - Эмитит сигнал connected для всех слушателей.
 */
    void onConnected();
    /**
 * @brief Слот обработки события отключения сокета.
 *
 * - Логгирует факт отключения.
 * - Эмитит сигнал disconnected для всех слушателей.
 */
    void onDisconnected();
    /**
 * @brief Слот чтения новых данных из сокета — декодирует блоки и извлекает JSON-команды.
 *
 * - Внутренний цикл разбирает несколько сообщений, если пришли в одном фрейме.
 * - Проверяет наличие данных для полного блока (размер).
 * - Читает и парсит JSON, логгирует ошибки парсинга.
 * - Эмитит jsonReceived для основной системы (прокладка в другие слои).
 */
    void onReadyRead();

private:
    /**
 * @brief TCP-сокет для подключения к серверу и обмена всеми служебными сообщениями (JSON, команды VoIP).
 *
 * Управляет всей передачей данных на уровне TCP — автоматически очищается/destruct при удалении NetworkService.
 */
    QTcpSocket *m_socket;

    /**
 * @brief Размер следующего ожидаемого блока данных (байты, префикс протокола).
 *
 * Используется для корректной пакетной обработки — сначала читается размер, затем принимается весь блок.
 */
    quint32 m_nextBlockSize;

};

#endif
