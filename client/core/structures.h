#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QString>
#include <QList>

/**
 * @brief Структура, описывающая пользователя мессенджера.
 */
struct User {
    qint64 id = 0;                   ///< Уникальный идентификатор пользователя (БД или сервер).
    QString username;                ///< Юзернейм (логин) пользователя.
    QString displayName;             ///< Отображаемое имя для UI.
    QString lastSeen;                ///< Метка времени последней активности.
    QString avatarUrl;               ///< Ссылка на аватарку.
    QString statusMessage;           ///< Статус/подпись пользователя.

    bool isTyping = false;           ///< Флаг: пользователь сейчас печатает сообщение.
    bool isOnline = false;           ///< Флаг: пользователь онлайн.
};

/**
 * @brief Структура сообщения в чате.
 */
struct ChatMessage {

    /**
     * @brief Тип содержимого сообщения.
     */
    enum MessageType {
        Text,     ///< Текстовое сообщение.
        Image,    ///< Сообщение с изображением.
        File,     ///< Сообщение-файл.
        Sticker,  ///< Стикер/эмодзи.
        System    ///< Системное сообщение (например, сервисное событие).
    };

    /**
     * @brief Статус сообщения: доставка, чтение и ошибки.
     */
    enum MessageStatus {
        Sending,    ///< В процессе отправки.
        Sent,       ///< Отправлено на сервер.
        Delivered,  ///< Доставлено адресату.
        Read,       ///< Прочитано адресатом.
        Error       ///< Ошибка доставки/отправки.
    };

    qint64 id = 0;                   ///< Уникальный идентификатор на сервере.
    QString tempId;                  ///< Локальный временный id для сопоставления с эхо.
    QString fromUser;                ///< Логин отправителя.
    QString toUser;                  ///< Логин получателя.
    QString payload;                 ///< Основное содержимое/текст.
    QString timestamp;               ///< Метка времени (отправки/сохранения).

    bool isEdited = false;           ///< Было ли сообщение отредактировано после отправки.
    qint64 replyToId = 0;            ///< ID сообщения, на которое дан reply.
    User forwardedFrom;              ///< Кто переслал (если переслано).
    MessageType messageType;         ///< Тип сообщения по содержимому.
    MessageStatus status;            ///< Статус сообщения.
    QString mediaUrl;                ///< Ссылка/путь к медиа-ресурсу (для Image/File).
    bool isOutgoing;                 ///< Исходящее ли сообщение (от текущего пользователя).
};

/**
 * @brief Оператор сравнения по ID сообщений.
 */
inline bool operator==(const ChatMessage& lhs, const ChatMessage& rhs) {
    return lhs.id == rhs.id;
}

/**
 * @brief Кеш сообщений чата и вспомогательные флаги.
 */
struct ChatCache {
    QList<ChatMessage> messages;      ///< Список сообщений данного чата.
    qint64 oldestMessageId = -1;      ///< ID самого старого сообщения (история).
    bool allMessagesLoaded = false;   ///< Флаг: вся история чата уже загружена.
};

/**
 * @brief Структура для истории звонков.
 */
struct CallItem {
    QString callId;           ///< Уникальный идентификатор звонка (UUID).
    QString caller;           ///< Кто инициировал звонок.
    QString callee;           ///< Кто принимал звонок.
    QString status;           ///< Статус звонка ("completed", "missed", ...).
    QString callType;         ///< Тип звонка ("incoming"/"outgoing").
    QString startTime;        ///< Время начала звонка.
    QString endTime;          ///< Время окончания звонка.
    int durationSeconds;      ///< Длительность звонка в секундах.
};

#endif
