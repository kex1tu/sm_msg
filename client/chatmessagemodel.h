#ifndef CHATMESSAGEMODEL_H
#define CHATMESSAGEMODEL_H

#include <QAbstractListModel> // Включаем базовый класс для списочных моделей Qt.
#include "structures.h"       // Включаем определение структуры ChatMessage.

/**
 * @brief Макрос Qt, регистрирующий кастомный тип ChatMessage в мета-объектной системе Qt.
 *
 * Это необходимо для того, чтобы мы могли:
 * 1. Оборачивать объекты ChatMessage в QVariant (чтобы передавать их через data() модели).
 * 2. Использовать этот тип в механизме сигналов и слотов.
 * @author kex1tu
 */
Q_DECLARE_METATYPE(ChatMessage)

/**
 * @class ChatMessageModel
 * @brief Модель данных для списка сообщений в одном чате.
 *
 * Этот класс является "источником правды" для `QListView`, который отображает чат.
 * Он наследуется от QAbstractListModel, реализуя его виртуальные методы для
 * предоставления данных представлению (View).
 *
 * Модель ничего не знает о том, как данные будут отображаться; она только хранит их
 * и уведомляет представление об изменениях.
 */
class ChatMessageModel : public QAbstractListModel
{
    Q_OBJECT // Обязательный макрос для любого класса, использующего сигналы и слоты.

public:
    /**
     * @brief Конструктор класса.
     * @param parent Родительский объект (стандартно для Qt).
     */
    explicit ChatMessageModel(QObject *parent = nullptr);

    // --- Переопределенные обязательные методы QAbstractListModel ---

    /**
     * @brief Возвращает количество элементов (сообщений) в модели.
     * @param parent Для списочной модели всегда должен быть невалидным QModelIndex().
     * @return int Количество строк в списке.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Основной метод, через который представление (View) запрашивает данные.
     * @param index Индекс элемента (строка и колонка), для которого нужны данные.
     * @param role Тип запрашиваемых данных (например, текст для отображения, цвет, иконка).
     *             Мы используем Qt::UserRole для передачи всего объекта ChatMessage целиком.
     * @return QVariant Данные, обернутые в QVariant.
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Метод для изменения данных в модели извне (например, из делегата).
     *        Хотя в данной версии он не используется активно для изменения, его реализация
     *        важна для полноты API модели и может пригодиться в будущем.
     * @param index Индекс изменяемого элемента.
     * @param value Новое значение.
     * @param role Роль, по которой изменяются данные.
     * @return bool true в случае успеха, иначе false.
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Открытые слоты являются частью публичного API класса и могут быть вызваны из других объектов.
public slots:
    // --- API для управления списком сообщений ---

    void addMessage(const ChatMessage &message);          // Добавляет одно сообщение в конец списка.
    void addMessages(const QList<ChatMessage> &messages);  // Добавляет несколько сообщений в конец (для первоначальной загрузки).
    void prependMessages(const QList<ChatMessage> &messages); // Добавляет несколько сообщений в начало (для подгрузки старой истории).
    void clearMessages();                                  // Полностью очищает модель.
    void removeMessage(qint64 messageId);                  // Удаляет сообщение по его уникальному ID.

    // --- API для обновления существующих сообщений ---

    // Подтверждает сообщение, отправленное клиентом. Находит его по временному ID (tempId)
    // и заменяет на подтвержденное сообщение от сервера (с настоящим ID и timestamp).
    void confirmMessage(const QString& tempId, const ChatMessage& confirmedMessage);

    // Обновляет статус сообщения (Sending -> Sent -> Delivered -> Read).
    void updateMessageStatus(qint64 messageId, ChatMessage::MessageStatus newStatus);

    // Изменяет текст сообщения и устанавливает флаг isEdited.
    void editMessage(qint64 messageId, const QString& newPayload);

    // --- Вспомогательные методы ---

    // Быстрый поиск сообщения по ID с использованием QMap. Возвращает true, если найдено.
    bool getMessageById(qint64 id, ChatMessage &msg) const;

    // Слот, вызываемый делегатом, когда сообщение становится видимым.
    // Он проверяет, нужно ли отправить уведомление о прочтении, и если да,
    // испускает сигнал messageNeedsReadReceipt.
    void markMessageAsRead(const QModelIndex &index);

    // Сигналы - это способ, которым модель уведомляет другие части приложения о событиях.
signals:
    /**
     * @brief Сигнал, который модель испускает, когда считает, что для сообщения
     *        с указанным ID нужно отправить на сервер уведомление о прочтении.
     *
     * Этот сигнал будет пойман в MainWindow, который уже сформирует и отправит
     * соответствующий JSON-запрос.
     * @param messageId Уникальный ID сообщения, которое было прочитано.
     */
    void messageNeedsReadReceipt(qint64 messageId);

private:
    /**
     * @brief Основное хранилище сообщений в виде списка.
     *
     * QList обеспечивает быстрый доступ по индексу и сохраняет порядок,
     * что необходимо для отображения в QListView.
     */
    QList<ChatMessage> m_messages;

    /**
     * @brief Дополнительное хранилище для оптимизации.
     *
     * QMap (хэш-таблица) хранит те же сообщения, но индексирует их по уникальному ID.
     * Это позволяет осуществлять сверхбыстрый поиск сообщения по ID (O(log n)),
     * что критически важно для функции ответов, где делегату нужно быстро найти
     * исходное цитируемое сообщение.
     */
    QMap<qint64, ChatMessage> m_messageMap;
};

#endif // CHATMESSAGEMODEL_H
