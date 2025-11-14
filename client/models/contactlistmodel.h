#ifndef CONTACTLISTMODEL_H
#define CONTACTLISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QMap>
#include "structures.h"

class DataService;

class ContactListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
 * @brief Дополнительные пользовательские роли для модели контактов.
 */
    enum ContactRoles {
        UsernameRole = Qt::UserRole + 1,   ///< @brief Логин пользователя (username).
        IsOnlineRole,                      ///< @brief Признак онлайн-статуса (true — online, false — offline).
        IsTypingRole,                      ///< @brief Признак, что пользователь "печатает..." сообщение.
        LastMessageRole,                   ///< @brief Последнее полученное сообщение (payload) в чате с контактом.
        UnreadCountRole                    ///< @brief Количество непрочитанных сообщений во взаимодействии с этим контактом.
    };
    /**
 * @brief Конструктор модели списка контактов.
 * @param dataService Указатель на DataService, обеспечивающий доступ к данным пользователей и чатам
 * @param parent Родительский QObject для модели
 */
    explicit ContactListModel(DataService* dataService, QObject *parent = nullptr);
    /**
 * @brief Возвращает количество контактов в списке.
 * @param parent Родительский индекс всегда невалиден для списка
 * @return Число элементов в списке контактов
 */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
 * @brief Возвращает данные контакта по индексу и роли.
 *
 * - Поддерживает все пользовательские роли: Username, Online/Typing, Последнее сообщение, Количество непрочитанных, DisplayName.
 * - Берёт User из кэша DataService; если не найден — возвращает QVariant().
 * - Для LastMessage — возвращает последний payload из ChatCache пользователя.
 * - Для UnreadCount — берёт значение из map непрочитанных DataService.
 * - Для IsTyping — возвращает false для текущего чата, если username совпадает с текущим партнёром (чтобы не светить "печатает" для самого себя).
 *
 * @param index Индекс строки в модели
 * @param role Роль, по которой требуется получить информацию
 * @return QVariant с данными по роли, либо пусто для невалидного индекса/нет данных
 */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /**
 * @brief Обновляет список контактов по новому списку usernames — корректно синхронизирует добавление, удаление и порядок.
 *
 * - Определяет каких пользователей нужно добавить/удалить через QSet.
 * - Удаляет отсутствующих, вставляет новых.
 * - Гарантирует последовательность usernames.
 * - Все операции обернуты beginInsertRows/endInsertRows и beginRemoveRows/endRemoveRows.
 *
 * @param newUsernames Новый актуальный список usernames (например, после синхронизации с сервером)
 */
    void updateContacts(const QStringList& usernames);
    /**
 * @brief Обновляет один контакт по username — вызывает dataChanged для обновления view.
 * @param username Имя пользователя для обновления
 */
    void refreshContact(const QString& username);
    /**
 * @brief Обновляет один контакт по QModelIndex — сигнатура для обновления из других слоёв.
 * @param User Индекс контакта, который нужно обновить
 */
    void refreshContact(const QModelIndex &User);
    /**
 * @brief Очищает весь список контактов — используется при дисконнекте/выходе из аккаунта.
 * Извещает view о сбросе модели.
 */
    void clear();

signals:

private:
    /**
 * @brief Указатель на сервис работы с данными (DataService) — обеспечивает получение информации о пользователях, статусах, кешах сообщений.
 */
    DataService* m_dataService;

    /**
 * @brief Список usernames, используемых как индексы для отображения контактов, сортируются и обновляются через updateContacts.
 */
    QStringList m_contactUsernames;

};

#endif
