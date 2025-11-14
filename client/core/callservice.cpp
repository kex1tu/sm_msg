#include "callservice.h"
#include "networkservice.h"
#include "dataservice.h"

#include <QMediaDevices>
#include <QAudioFormat>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>

CallService::CallService(NetworkService* networkService, DataService* dataService, QObject* parent)
    : QObject(parent),
    m_networkService(networkService),
    m_dataService(dataService),
    m_callState(Idle),
    m_currentCallId(QString()),
    m_remoteUsername(QString()),
    m_remoteIp(QString()),
    m_remotePort(0),
    m_remoteAddress(QHostAddress()),
    m_udpSocket(nullptr),
    m_localPort(0),
    m_audioSource(nullptr),
    m_audioSink(nullptr),
    m_audioInput(nullptr),
    m_audioOutput(nullptr),
    m_callTimer(new QTimer(this)),
    m_callDuration(0),
    m_audioBytesSent(0),
    m_audioPacketsSent(0),
    m_audioBytesReceived(0),
    m_audioPacketsReceived(0)
{
    /// Инициализация основных полей класса и загрузка конфигурации.

    // Выводим для отладки текущий рабочий каталог приложения.
    qDebug() << "[CallService] " << "CurrentPath:" << QDir::currentPath();

    // Формируем путь к конфигу и инициализируем чтение INI-файла настроек.
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(configPath, QSettings::IniFormat);

    // Отладочный вывод найденных параметров INI.
    qDebug() << "[CallService] " << "Current path:" << QDir::currentPath();
    qDebug() << "[CallService] " << "INI keys =" << settings.allKeys();

    // Получаем собственный IP-адрес (или задаем дефолтный).
    m_myIp = settings.value("network/myIp", "192.168.0.101").toString();
    qDebug() << "[CallService] " << "[Config] m_myIp =" << m_myIp;

    // Подключаем QTimer к слоту для обновления длительности вызова.
    connect(m_callTimer, &QTimer::timeout, this, &CallService::onCallTimerTimeout);

    // Инициализируем UDP-сокет для аудиостриминга.
    initializeUdpSocket();
}


CallService::~CallService()
{
    /// Отключаем и останавливаем все аудиостримы и устройства.
    stopAudioStreaming();
    // Освобождаем сокет, если он был инициализирован — deleteLater позволяет корректно завершить все отложенные операции Qt.
    if (m_udpSocket) {
        m_udpSocket->deleteLater();
    }
}


void CallService::initializeUdpSocket()
{
    // Проверка на уже существующий сокет: корректное отключение сигналов, закрытие устройства, освобождение памяти.
    if (m_udpSocket) {
        disconnect(m_udpSocket, nullptr, this, nullptr); // Снимаем все связи сигнал-слот во избежание конфликтов.
        m_udpSocket->close();                            // Физически закрываем сокет.
        m_udpSocket->deleteLater();                      // Сигнал к удалению объекта после завершения всех событий Qt.
    }

    /// Создаем новый экземпляр UDP-сокета для передачи аудиопотока.
    m_udpSocket = new QUdpSocket(this);

    /// Сокет привязывается к любому свободному локальному порту с поддержкой шаринга и повторного использования.
    /// Это особенно удобно при многократных перезапусках звонков.
    bool ok = m_udpSocket->bind(QHostAddress::Any, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    // Прямая обработка ошибок биндинга: если порт не назначен (занят/нет прав) — выводим предупреждение и сообщаем UI (для отображения в интерфейсе, повторного запроса или логгирования ошибки).
    if (!ok) {
        qWarning() << "[UDP] ❌ Failed to bind UDP socket";
        emit callError("UDP binding failed");
        return;
    }

    // Запоминаем выбранный порт — необходим при обмене сетевыми параметрами между клиентами.
    m_localPort = m_udpSocket->localPort();
    qDebug() << "[CallService] " << "[UDP] ✅ Socket bound to port:" << m_localPort;

    // Подключаем сигнал readyRead к слоту onAudioDataReceived — для мгновенной обработки пакетов аудио.
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &CallService::onAudioDataReceived);
    qDebug() << "[CallService] " << "[UDP] ✅ readyRead connected";
}


void CallService::initiateCall(const QString& toUser)
{
    // Логируем попытку инициации — удобно для отслеживания истории и отладки сетевых запросов.
    qDebug() << "[CallService] " << "[CALL] >>> INITIATING CALL TO:" << toUser;

    /// Проверяем, что нет уже активного вызова (single active call protection).
    if (m_callState != Idle) {
        // Если занято — выводим предупреждение, генерируем событие ошибки (UI/интерфейс может показать диалог/текст).
        qWarning() << "[CALL] ❌ Already in a call";
        emit callError("Already in a call");
        return;
    }

    // Запоминаем параметры партнёра и стартовые данные сессии.
    m_remoteUsername = toUser;
    m_currentCallId = QUuid::createUuid().toString(); // Уникальный идентификатор сессии, необходим для синхронизации сторон.
    m_callState = Calling;                            // Смена состояния: звонок запущен и ожидает ответа.
    m_callDuration = 0;                              // Сброс таймера звонка.

    // Отправляем сетевой запрос о начале звонка (через NetworkService/сервер).
    sendCallRequest(toUser);

    /// Генерируем сигнал для UI – нужный экран звонка, индикаторы и кнопки.
    emit outgoingCallShow();

    // Лог финализации процедуры — удобно для статистики и дебага.
    qDebug() << "[CallService] " << "[CALL] ✅ Outgoing call initiated";
}


void CallService::sendCallRequest(const QString& toUser)
{
    // Получаем имя текущего пользователя — без него нельзя инициировать вызов извне
    QString fromUser = m_dataService ? m_dataService->getCurrentUser()->username : QString();

    // Проверка авторизации: если пользователь не идентифицирован, сигнализируем ошибку, возвращаем управление UI.
    if (fromUser.isEmpty()) {
        qWarning() << "[CALL] ❌ User not logged in";
        emit callError("Not logged in");
        return;
    }

    // Формируем JSON-запрос с типом и ключевыми параметрами сессии (имена, id, сетевые адреса)
    QJsonObject request;
    request["type"] = "call_request";
    request["from"] = fromUser;
    request["to"] = toUser;
    request["call_id"] = m_currentCallId;     // Уникальный идентификатор для синхронизации звонка
    request["caller_ip"] = m_myIp;            // Текущий внешний IP, чтобы партнёр мог подключиться
    request["caller_port"] = (int)m_localPort;// Порт, на котором клиент ждёт аудиопотока

    // Отправляем пакет через сетевой сервис (реализация может быть абстрактной – TCP-сервис, WebSocket, и т.д.)
    m_networkService->sendJson(request);

    // Финальный лог: отправка успешна, звонок можно ожидать
    qDebug() << "[CallService] " << "[CALL] ✅ Call request sent with port:" << m_localPort;
}


void CallService::onCallRequestReceived(const QString& from, const QString& callId,
                                        const QString& ip, quint16 port)
{
    // Логируем пришедший звонок, фиксируем параметры для отладки сетевого протокола.
    qDebug() << "[CallService] " << "[CALL] <<< INCOMING CALL FROM:" << from << "IP:" << ip << "PORT:" << port;

    /// Проверяем, что не происходят параллельные сессии — если сервис не свободен, выводим ошибку.
    if (m_callState != Idle) {
        qWarning() << "[CALL] ❌ Not in Idle state";
        return;
    }

    // Сохраняем все ключевые параметры входящего вызова, которые понадобятся при соединении аудиотрафика.
    m_remoteUsername = from;
    m_currentCallId = callId;
    m_remoteIp = ip;
    m_remotePort = port;
    m_remoteAddress = QHostAddress(ip); // Преобразуем строковый IP в тип Qt для последующего обмена пакетами.
    qDebug() << "[CallService] " << "m_remoteAddress" << m_remoteAddress;
    m_callState = Ringing; // Ожидание реакции пользователя (Accept/Reject).

    // Генерируем событие для UI: показываем окно со входящим звонком, деталями, кнопками ответа.
    emit incomingCallShow(from);

    // Логируем успешную обработку — удобно для аналитики и истории взаимодействий
    qDebug() << "[CallService] " << "[CALL] ✅ Incoming call signal emitted";
}



void CallService::acceptCall()
{
    // Лог: попытка принятия звонка (кнопка Answer или событие в протоколе).
    qDebug() << "[CallService] " << "[CALL] >>> ACCEPTING CALL";

    // Проверка корректного состояния: звонок можно принять только из режима Ringing.
    if (m_callState != Ringing) {
        qWarning() << "[CALL] ❌ Not in Ringing state";
        return;
    }

    // Переводим сервис в состояние установленного соединения.
    m_callState = Connected;

    // Отправляем партнёру сообщение подтверждения вызова (сетевой протокол).
    sendCallAccepted();

    // Запуск аудиостриминга — конфигурирование и включение микрофона/динамика, открытие каналов данных.
    startAudioStreaming();

    // Запуск QTimer — раз в 1000 мс обновляет длительность звонка для интерфейса.
    m_callTimer->start(1000);

    // Сигнализируем UI: звонок успешно установлен, можно обновить интерфейс.
    emit callConnected();
    qDebug() << "[CallService] " << "[CALL] ✅ Call connected";
}


void CallService::sendCallAccepted()
{
    // Формируем объект JSON-ответа: сообщает параметры для соединения аудиоканалов партнёру.
    QJsonObject response;
    response["type"] = "call_accepted";
    response["from"] = m_dataService ? m_dataService->getCurrentUser()->username : ""; // Имя пользователя — важно для аудитории с несколькими аккаунтами.
    response["call_id"] = m_currentCallId;
    response["callee_ip"] = m_myIp;                      // IP-адрес, на который следует слать аудиотрафик.
    response["callee_port"] = (int)m_localPort;          // Порт нашего сокета, открытого для передачи.

    // Отправляем ответ через сетевой сервис — TCP/WebSocket/UDP-шлюз.
    m_networkService->sendJson(response);

    // Лог подтверждения — позволяет выявлять ошибки подключения и видеть информацию в консоли.
    qDebug() << "[CallService] " << "[CALL] ✅ Call accepted sent with port:" << m_localPort;
}


void CallService::rejectCall()
{
    // Логируем начало процедуры отказа.
    qDebug() << "[CallService] " << "[CALL] >>> REJECTING CALL";

    // Проверяем текущее состояние — отклонить можно только во время ожидания ответа (Ringing).
    if (m_callState != Ringing) {
        qWarning() << "[CALL] ❌ Not in Ringing state";
        return;
    }

    // Завершаем звонок: сброс параметров, остановка аудио, отправка сигнала partner'у, состояние Idle.
    endCall();

    /// Сообщаем UI о завершении вызова (после отказа пользователь получает обновление интерфейса).
    emit callEnded();

    // Подробный лог завершающего этапа — статистика, отладка, аналитика.
    qDebug() << "[CallService] " << "[CALL] ✅ Call rejected";
}


void CallService::sendCallRejected()
{
    // Формируем объект JSON для передачи причины отказа партнёру или серверу.
    QJsonObject response;
    response["type"] = "call_rejected";
    response["call_id"] = m_currentCallId;
    response["to"] = m_dataService ? m_dataService->getCurrentUser()->username : "";

    // Отправляем JSON через сетевой сервис.
    m_networkService->sendJson(response);
}


void CallService::onCallAcceptedReceived(const QString& ip, quint16 port)
{
    qDebug() << "[CallService] " << "[CALL] <<< CALL ACCEPTED FROM REMOTE USER IP:" << ip << "PORT:" << port;

    // Проверка: звонок должен быть инициирован с нашей стороны, ожидать подтверждения.
    if (m_callState != Calling) {
        qWarning() << "[CALL] ❌ Not in Calling state";
        return;
    }

    // Обновляем параметры соединения с полученными от собеседника данными.
    m_remoteIp = ip;
    m_remotePort = port;
    m_remoteAddress = QHostAddress(m_remoteIp);
    m_callState = Connected;

    // Запуск media-сессии (захват и передача звука).
    startAudioStreaming();

    // Включение отсчета времени звонка (обновляет интерфейс каждую секунду).
    m_callTimer->start(1000);

    // Сигнализируем в интерфейс о подтверждении соединения и старте разговора.
    emit callConnected();
    qDebug() << "[CallService] " << "[CALL] ✅ Call connected";
}


void CallService::onCallRejectedReceived()
{
    qDebug() << "[CallService] " << "[CALL] <<< CALL REJECTED";

    // Сигнализируем интерфейсу ошибку — отображается пользователю
    emit callError("Call rejected");

    // Корректно завершаем звонок: очистка состояний, отключение аудиоустройств
    endCall();
}


void CallService::onCallEndedReceived()
{
    qDebug() << "[CallService] " << "[CALL] <<< CALL ENDED BY REMOTE USER";
    // Сброс состояния, остановка устройств, возвращение UI в исходное положение
    endCall();
}


void CallService::endCall()
{
    /// Если вызов уже завершён — предотвращаем дублирование событий и действий.
    if (m_callState == Idle) return;

    m_callState = Idle;           // Сброс состояния
    /// Останавливаем таймер продолжительности звонка.
    m_callTimer->stop();
    // Сообщаем партнёру или серверу о завершении звонка — протокол VoIP.
    sendCallEnd();
    /// Полностью отключаем аудиоустройства, поток и освобождаем ресурсы.
    stopAudioStreaming();
    /// Сигнализируем UI о завершении сессии (к примеру, скрыть диалог вызова).
    emit callEnded();

    // Лог успешного завершения — для истории и анализа отказов.
    qDebug() << "[CallService] " << "[CALL] ✅ Call ended";
}


void CallService::cancelOutgoingCall()
{
    // Только если идёт исходящий вызов (в процессе ожидания ответа)
    if (m_callState == Calling) {
        // Корректно информируем партнёра: завершение ожидания соединения.
        sendCallEnd();
    }
}


void CallService::sendCallEnd()
{
    // Формируем пакет завершения сессии — необходим для синхронизации состояния у партнёра
    QJsonObject msg;
    msg["type"] = "call_end";
    msg["from"] = m_dataService ? m_dataService->getCurrentUser()->username : ""; // Имя пользователя (может быть пустым, если произошла ошибка авторизации)
    msg["call_id"] = m_currentCallId; // ID сессии — используется для идентификации звонка

    // Отправляем через общий сетевой сервис — сервер или напрямую
    m_networkService->sendJson(msg);
}


void CallService::onCallTimerTimeout()
{
    // Инкрементируем счетчик секунд текущего звонка
    m_callDuration++;
    // Обновляем UI — пользователь видит актуальное время разговора
    emit callDurationUpdated(m_callDuration);
}


void CallService::startAudioStreaming()
{
    qDebug() << "[CallService] " << "[AUDIO] >>> STARTING AUDIO STREAMING";

    // Проверка: аудиостриминг может быть запущен только в состоянии Connected и с назначенным удалённым портом.
    if (m_remotePort == 0 || m_callState != Connected) {
        qWarning() << "[AUDIO] ❌ Not ready - remotePort:" << m_remotePort << "state:" << m_callState;
        return;
    }

    // Получаем устройства ввода (микрофон) и вывода (динамики): QMediaDevices гарантирует кроссплатформенность.
    QList<QAudioDevice> inputs = QMediaDevices::audioInputs();
    QList<QAudioDevice> outputs = QMediaDevices::audioOutputs();
    // Подробный лог для диагностики: вывод списка всех доступных устройств.
    qDebug() << "[CallService] " << "[CALL] === AVAILABLE INPUT DEVICES ===";
    for (int i = 0; i < inputs.size(); i++) {
        qDebug() << "[CallService] " << QString("[CALL] [%1]").arg(i) << inputs.at(i).description();
    }
    qDebug() << "[CallService] " << "[CALL] === END OF LIST ===";

    qDebug() << "[CallService] " << "[CALL] === AVAILABLE OUTPUT DEVICES ===";
    for (int i = 0; i < outputs.size(); i++) {
        qDebug() << "[CallService] " << QString("[CALL] [%1]").arg(i) << outputs.at(i).description();
    }
    qDebug() << "[CallService] " << "[CALL] === END OF LIST ===";

    /// Без устройств нет смысла продолжать — уведомляем UI и завершаем стриминг.
    if (inputs.isEmpty() || outputs.isEmpty()) {
        qWarning() << "[AUDIO] ❌ No audio devices found";
        emit callError("No audio devices");
        return;
    }

    /// Формируем аудиоформат под VoIP: моно, 16кГц, Integer PCM.
    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    /// Создаём и запускаем QAudioSource на первом доступном устройстве ввода.
    m_audioSource = new QAudioSource(inputs.at(0), format, this);

    /// Запускаем захват аудио — поток будет читаться из m_audioInput.
    m_audioInput = m_audioSource->start();

    // Проверка корректного старта микрофона — если ошибка, завершаем работу и очищаем объекты.
    if (!m_audioInput) {
        qWarning() << "[AUDIO] ❌ Failed to start audio input";
        delete m_audioSource;
        m_audioSource = nullptr;
        return;
    }

    // Подключаем обработчик фрагментов аудиоданных — readyRead.
    connect(m_audioInput, &QIODevice::readyRead, this, &CallService::onAudioInputReady);

    /// Запускаем QAudioSink — вывод аудиоданных на первое доступное устройство.
    m_audioSink = new QAudioSink(outputs.at(0), format, this);
    m_audioSink->setVolume(1.0);
    m_audioOutput = m_audioSink->start();

    // Проверка корректного запуска динамика/вывода, при ошибке завершает аудиостриминг.
    if (!m_audioOutput) {
        qWarning() << "[AUDIO] ❌ Failed to start audio output";
        delete m_audioSink;
        m_audioSink = nullptr;
        /// Вызываем остановку и очистку аудиостриминга, предотвращаем утечку ресурсов.
        stopAudioStreaming();
        return;
    }

    // Логи успешного старта аудиостриминга, вывод данных о соединении для диагностики.
    qDebug() << "[CallService] " << "[AUDIO] ✅ AUDIO STREAMING STARTED";
    qDebug() << "[CallService] " << "[AUDIO] Remote:" << m_remoteAddress.toString() << ":" << m_remotePort;
}


void CallService::stopAudioStreaming()
{
    qDebug() << "[CallService] " << "[AUDIO] Stopping audio streaming...";

    // Если работал микрофон (input), корректно отключаем и удаляем объект
    if (m_audioInput) {
        disconnect(m_audioInput, nullptr, this, nullptr); // Снимаем все сигнал-слот связи с данным входом
        delete m_audioSource;                            // Удаляем источник (микрофон)
        m_audioSource = nullptr;                         // Указатель обнуляем для безопасности
        m_audioInput = nullptr;
    }

    // Если работал динамик/выход (output), корректно отключаем и удаляем объект
    if (m_audioOutput) {
        disconnect(m_audioOutput, nullptr, this, nullptr); // Сброс обработчиков данных и сигналов
        delete m_audioSink;                                // Удаляем приемник (динамик/колонки)
        m_audioSink = nullptr;
        m_audioOutput = nullptr;
    }

    // Финальный лог — подтверждает чистоту ресурсов, предотвращает утечки
    qDebug() << "[CallService] " << "[AUDIO] ✅ Audio streaming stopped";
}


void CallService::onAudioInputReady()
{
    // Проверка корректности состояния перед попыткой чтения и отправки аудиоданных.
    if (!m_audioInput || m_remotePort == 0 || m_callState != Connected) {
        qDebug() << "[CallService] " << "ERROR m_audioInput is nullptr orm_remotePort == 0 or m_callState != Connected ";
        return;
    }

    // Считываем данные аудио с микрофона — формируем буфер для отправки.
    QByteArray audioData = m_audioInput->readAll();

    // Если буфер не пустой — начинаем обработку для отправки по UDP.
    if (!audioData.isEmpty()) {
        // Проверяем наличие сокета (возможна ситуация, когда он был закрыт по ошибке).
        if(!m_udpSocket){
            qWarning() << "UDP socket is nullptr!";
            return;
        }
        // Отправляем аудиобуфер по UDP (remoteAddress и port) — ядро VoIP-стриминга.
        qint64 sent = m_udpSocket->writeDatagram(audioData, m_remoteAddress, m_remotePort);
        // При ошибке записи — подробный лог для диагностики (например, отсутствие сети, проблемы с портом).
        if (sent < 0) {
            qWarning() << "UDP write failure:" << m_udpSocket->errorString();
        }
        // Обновляем статистику передачи: количество отправленных байт и пакетов.
        m_audioBytesSent += audioData.size();
        m_audioPacketsSent++;

        /// Каждые 50 пакетов выводим расширенную информацию для анализа сети, мониторинга производительности.
        if (m_audioPacketsSent % 50 == 0) {
            qDebug() << "[CallService] " << "[AUDIO] Sent" << m_audioPacketsSent << "packets"
                     << "(" << m_audioBytesSent / 1024 << "KB) from" << m_udpSocket->localPort() << " to " << m_remotePort;
        }
    }
}


void CallService::onAudioDataReceived()
{
    /// Цикл для обработки всех входящих UDP пакетов (клиент может получать несколько за тик таймера)
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize()); // Подготавливаем буфер для принятия аудиоданных
        QHostAddress senderAddress;
        quint16 senderPort;

        // Читаем пакет и сохраняем адрес отправителя (для статистики/инженерных целей)
        qint64 read = m_udpSocket->readDatagram(datagram.data(), datagram.size(),
                                                &senderAddress, &senderPort);
        if (read < 0) {
            // При ошибке чтения выводим информацию — важно для выявления потерь UDP
            qWarning() << "UDP read failure:" << m_udpSocket->errorString();
        }
        // Сохраняем статистику всего полученного аудио (байты, количество пакетов)
        m_audioBytesReceived += datagram.size();
        m_audioPacketsReceived++;

        /// Проверка: выходное аудиоустройство должно быть инициализировано.
        if (!m_audioOutput) {
            qWarning() << "audioOutput is nullptr!";
            return;
        }
        // Отправляем полученные данные в аудиовыход (проигрываем звук для пользователя).
        qint64 written = m_audioOutput->write(datagram);

        if (written < 0) {
            // Возможные ситуации — устройство недоступно/ошибка записи
            qWarning() << "[AUDIO] ❌ Failed to write to output";
        }

        /// Каждые 50 пакетов логгируем полную статистику — графики, диагностика, аналитика VoIP.
        if (m_audioPacketsReceived % 50 == 0) {
            qDebug() << "[CallService] " << "[AUDIO] Received" << m_audioPacketsReceived << "packets"
                     << "(" << m_audioBytesReceived / 1024 << "KB)";
        }
    }
}


void CallService::playMusicalScale()
{
    if (m_callState != Connected) {
        qWarning() << "[MUSIC] ❌ Not connected";
        return;
    }

    // Лог: начало воспроизведения звуковой гаммы.
    qDebug() << "[CallService] " << "\n[MUSIC] 🎵 Playing scale...";

    int notes[] = {262, 294, 329, 349, 392}; // Частоты соответствуют нотам "ДО", "РЕ", "МИ", "ФА", "СОЛЬ"
    QString names[] = {"ДО", "РЕ", "МИ", "ФА", "СОЛЬ"};

    /// Перебираем все ноты, проигрываем каждую, выводим лог названия для пользователя и отладки.
    for (int i = 0; i < 5; ++i) {
        sendSineWaveTone(notes[i], 500);    // Отправка аудиотона через собственную сеть
        qDebug() << "[CallService] " << "[MUSIC]" << names[i]; // Лог текущей ноты
        QThread::msleep(600);               // Задержка между тонами
    }

    qDebug() << "[CallService] " << "[MUSIC] ✅ Scale finished\n";
}


void CallService::testFrequencyRange()
{
    if (m_callState != Connected) {
        qWarning() << "[TEST] ❌ Not connected";
        return;
    }

    // Лог: начало тестирования диапазона звуковых частот.
    qDebug() << "[CallService] " << "\n[TEST] Testing frequency range...";

    int testFreqs[] = {200, 440, 880, 1000, 2000, 4000};

    // Для каждой частоты в тестовом диапазоне формируем и отправляем аудиотон.
    for (int freq : testFreqs) {
        qDebug() << "[CallService] " << "[TEST]" << freq << "Hz...";       // Лог частоты для пользователя
        sendSineWaveTone(freq, 300);                                       // Генерация и отправка тона
        QThread::msleep(400);                                              // Задержка для восприятия/анализа
    }

    qDebug() << "[CallService] " << "[TEST] ✅ Range test finished\n";
}


void CallService::sendSineWaveTone(int frequencyHz, int durationMs)
{
    // Проверка всех условий для корректной работы (активная сессия, порт, сокет)
    if (m_callState != Connected || m_remotePort == 0 || !m_udpSocket) {
        qWarning() << "[SINE] ❌ Not ready";
        return;
    }

    // Параметры аудиосигнала
    const int sampleRate = 16000;                                   // Частота дискретизации (16кГц)
    const int totalSamples = (sampleRate * durationMs) / 1000;      // Количество сэмплов для заданной длительности
    const float amplitude = 32767.0f * 0.3f;                        // Максимальная амплитуда для PCM 16 бит

    // Буфер для аудиоданных — 16 бит на каждый сэмпл, memory для UDP-send
    QByteArray audioBuffer(totalSamples * 2, 0);
    qint16* samples = (qint16*)audioBuffer.data();

    /// Генерируем синусоиду во всех отсчётах для PCM.
    for (int i = 0; i < totalSamples; ++i) {
        float t = (float)i / sampleRate;
        float phase = 2.0f * M_PI * frequencyHz * t;    // Фаза = 2πft
        float sampleValue = sin(phase) * amplitude;     // Значение амплитуды в текущий момент
        samples[i] = (qint16)sampleValue;               // Записываем 16-битный PCM
    }

    // Отправка всего буфера как одного UDP-пакета
    m_udpSocket->writeDatagram(audioBuffer, m_remoteAddress, m_remotePort);
    // Обновляем переданные байты/пакеты для статистики
    m_audioBytesSent += audioBuffer.size();
    m_audioPacketsSent++;

    // Финальный диагностический лог — размер пакета, частота
    qDebug() << "[CallService] " << "[SINE] ✅ Sent" << audioBuffer.size() << "bytes at" << frequencyHz << "Hz";
}


void CallService::resetCallData()
{
    // Корректно завершаем возможный активный звонок (обработка аудиоустройств, протокола)
    endCall();
    // Явный сброс состояния (Idle) — даже если функция вызвана вне активной сессии
    m_callState = Idle;
    // Очищаем все ключевые параметры текущего звонка
    m_currentCallId.clear();
    m_remoteUsername.clear();
    m_remoteIp.clear();
    m_remotePort = 0;
    m_remoteAddress = QHostAddress(); // Сброс адреса (отправка "0.0.0.0")
    m_callDuration = 0;               // Обнуляем таймер

    // Статистика аудиотрафика (подсчёты для анализа и UI) — все параметры в ноль
    m_audioBytesSent = 0;
    m_audioPacketsSent = 0;
    m_audioBytesReceived = 0;
    m_audioPacketsReceived = 0;
}
