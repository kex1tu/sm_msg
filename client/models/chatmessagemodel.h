#ifndef CHATMESSAGEMODEL_H
#define CHATMESSAGEMODEL_H

#include <QAbstractListModel>
#include "structures.h"

Q_DECLARE_METATYPE(ChatMessage)

class ChatMessageModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        DateHeaderRole = Qt::UserRole + 2
    };

    /**
 * @brief Конструктор модели сообщений чата.
 *
 * - Регистрирует тип ChatMessage для передачи через сигнал/слот (мета-тип).
 * - Инициализирует базовый QAbstractListModel.
 * @param parent Родительский QObject
 */
    explicit ChatMessageModel(QObject *parent = nullptr);
    /**
 * @brief Ищет индекс первого непрочитанного входящего сообщения (статус Delivered или Sent).
 *
 * - Перебирает m_messages в прямом порядке.
 * - Возвращает QModelIndex первого подходящего (используется для позиций скролла/автофокуса).
 * - Логгирует номер первого найденного или факт его отсутствия.
 * @return QModelIndex найденного сообщения, либо пустой, если таких нет
 */
    QModelIndex findFirstUnreadMessage() const;
    /**
 * @brief Возвращает количество сообщений в модели. Используется view для рендера.
 * @param parent Родительский индекс (для list view всегда пустой!)
 * @return Количество сообщений
 */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
 * @brief Возвращает данные модели для указанной ячейки и роли.
 *
 * - Для роли DateHeaderRole рассчитывает заголовок даты ("Сегодня", "Вчера", длинная дата), сравнивая с предыдущим или текущим сообщением.
 * - Для Qt::UserRole возвращает ChatMessage для делегатов/фильтров.
 * - Если индекс невалидный — возвращает пустой QVariant.
 * - Основная логика — форматирование дат для удобства отображения в UI.
 *
 * @param index Индекс строки и столбца (используется только row)
 * @param role Роль для возврата данных
 * @return QVariant c данными (строка, ChatMessage, пусто)
 */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /**
 * @brief Устанавливает данные для конкретного элемента — используется для обновлений от UI, редактирования.
 *
 * - Обрабатывает только Qt::UserRole. Если индекс невалиден/роль не та — возвращает false.
 * - Обновляет элемент в списке, вызывает сигнал dataChanged для переотрисовки UI.
 * - Сравнивает id сообщения, чтобы понять что обновляется именно тот объект.
 *
 * @param index Индекс строки
 * @param value Новое значение (ChatMessage), передаётся через QVariant
 * @param role Роль изменения (только UserRole)
 * @return true если изменено, иначе false
 */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

public slots:
    /**
 * @brief Добавляет одно новое сообщение в конец модели.
 *
 * - Оформление beginInsertRows/endInsertRows обеспечивает корректный update view.
 * - Обновляет список сообщений и карту по id (для быстрого поиска).
 * - Не добавляет messageMap, если id не установлен (например, draft).
 *
 * @param message Сообщение для добавления
 */
    void addMessage(const ChatMessage &message);
    /**
 * @brief Добавляет массив сообщений (bulk-вставка) в конец модели.
 *
 * - Аналогично addMessage, но оптимизировано для mass-операций.
 * - Все сообщения добавляются в messageMap по id.
 *
 * @param messages Список сообщений для массовой вставки
 */
    void addMessages(const QList<ChatMessage> &messages);
    /**
 * @brief Добавляет сообщения в начало модели (например, при подгрузке истории chunk-ами).
 *
 * - Вставляет каждое сообщение по одному в начало (order preserved!).
 * - Все сообщения добавляются в messageMap.
 *
 * @param messages Сообщения для prepend (обычно старые chunk-и)
 */
    void prependMessages(const QList<ChatMessage> &messages);
    /**
 * @brief Полностью очищает все сообщения в модели (reset для нового чата, logout и т.п.).
 *
 * - Извещает view началом/окончанием сброса.
 * - Чистит оба хранилища — список и map.
 */
    void clearMessages();
    /**
 * @brief Удаляет сообщение из модели по заданному messageId.
 *
 * - Проходит по всем сообщениям, сравнивая id.
 * - Использует beginRemoveRows/endRemoveRows для корректной работы с view.
 * - После удаления из списка и map сразу возвращает (только первая найденная!).
 * - Если id не найден — ничего не делает.
 *
 * @param messageId Идентификатор удаляемого сообщения
 */
    void removeMessage(qint64 messageId);
    /**
 * @brief Подтверждает сообщение (замена временного на пришедшее с сервера).
 *
 * - Находит message по tempId в списке.
 * - Заменяет в списке и map на новый подтверждённый объект (full id, status из confirmedMessage).
 * - Эмитит dataChanged для UI-обновления.
 *
 * @param tempId Временный идентификатор сообщения, ищем для локального эхо
 * @param confirmedMessage Реальный подтверждённый объект от сервера
 */
    void confirmMessage(const QString& tempId, const ChatMessage& confirmedMessage);
    /**
 * @brief Обновляет статус сообщения — например, после доставки или прочтения.
 *
 * - Находит по id, меняет статус, обновляет в списке и map.
 * - Эмитит dataChanged для синхронизации UI.
 *
 * @param messageId Идентификатор сообщения
 * @param newStatus Новый статус (enum)
 */
    void updateMessageStatus(qint64 messageId, ChatMessage::MessageStatus newStatus);
    /**
 * @brief Редактирует текст (payload) уже существующего сообщения по id.
 *
 * - Обновляет payload и флаг isEdited в списке/карте.
 * - Эмитит dataChanged для UI.
 *
 * @param messageId Идентификатор сообщения
 * @param newPayload Новый текст, который будет сохранён
 */
    void editMessage(qint64 messageId, const QString& newPayload);
    /**
 * @brief Возвращает сообщение по id, если оно существует в messageMap (быстрый доступ для цитат/поиска).
 *
 * @param id Идентификатор сообщения для поиска
 * @param msg Ссылка для возврата результата
 * @return true если найдено, иначе false
 */
    bool getMessageById(qint64 id, ChatMessage &msg) const;

signals:
    /**
 * @brief Сигнал для запроса отправки read-receipt (подтверждение прочтения) конкретного сообщения из UI/делегата/логики.
 * @param messageId Идентификатор сообщения, требующего отметки "прочитано"
 */
    void messageNeedsReadReceipt(qint64 messageId);

private:
    /**
 * @brief Массив всех сообщений чата, отображаемых в текущей модели.
 * Используется для вывода списка сообщений, хранения порядка, передачи в делегаты.
 */
    QList<ChatMessage> m_messages;

    /**
 * @brief Карта сообщений по их server_id — для быстрого поиска по id, цитирования, обновления статуса.
 * Ключ — уникальный идентификатор сообщения (id), значение — сам ChatMessage.
 */
    QMap<qint64, ChatMessage> m_messageMap;
};

#endif
