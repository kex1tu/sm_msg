#ifndef CONTACTLISTMODEL_H
#define CONTACTLISTMODEL_H

#include <QAbstractListModel> // Базовый класс для списочных моделей Qt.
#include <QStringList>        // Для хранения списка имен пользователей.
#include <QMap>
#include "structures.h"

// Прямое объявление (Forward Declaration) класса MainWindow.
// Позволяет использовать указатель MainWindow* без включения mainwindow.h,
// что предотвращает циклические зависимости (т.к. MainWindow включает contactlistmodel.h).
class MainWindow;

/**
 * @class ContactListModel
 * @brief Кастомная модель данных для списка контактов.
 *
 * Этот класс выступает в роли адаптера (Proxy) между централизованными кэшами данных,
 * хранящимися в MainWindow, и представлением (QListView).
 *
 * Он не дублирует данные, а лишь хранит отсортированный список идентификаторов (usernames)
 * и предоставляет унифицированный интерфейс для делегата (ContactListDelegate)
 * для получения информации о контактах через кастомные роли.
 * @author kex1tu
 */
class ContactListModel : public QAbstractListModel
{
    Q_OBJECT // Обязательный макрос для системы сигналов и слотов Qt.

public:
    /**
     * @enum ContactRoles
     * @brief Определяет кастомные роли данных для запроса информации делегатом.
     *
     * Qt::DisplayRole используется для основного отображаемого имени.
     * Эти роли используются для получения дополнительной контекстной информации.
     */
    enum ContactRoles {
        UsernameRole = Qt::UserRole + 1, ///< Уникальное имя пользователя (QString).
        IsOnlineRole,                    ///< Онлайн-статус (bool).
        IsTypingRole,                    ///< Печатает ли пользователь сообщение (bool).
        LastMessageRole,                 ///< Текст последнего сообщения в чате (QString).
        UnreadCountRole                  ///< Количество непрочитанных сообщений (int).
    };

    /**
     * @brief Конструктор.
     * @param mainWindow Указатель на главное окно. Необходим модели для доступа
     *                   к центральным кэшам данных (пользователей, истории, счетчиков).
     * @param parent Родительский объект (стандартно для Qt).
     */
    explicit ContactListModel(MainWindow* mainWindow, QObject *parent = nullptr);

    // --- Реализация обязательных методов QAbstractListModel ---

    /**
     * @brief Возвращает количество контактов в списке.
     * @param parent Для списочной модели всегда должен быть невалидным QModelIndex().
     * @return int Количество строк (контактов).
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Основной метод доступа к данным.
     *
     * Делегат вызывает этот метод, передавая индекс и роль, чтобы получить
     * конкретную информацию для отрисовки. Модель берет username по индексу
     * из своего списка, а затем запрашивает детальную информацию у MainWindow
     * в зависимости от роли.
     *
     * @param index Индекс элемента.
     * @param role Роль запрашиваемых данных (из Qt::ItemDataRole или ContactRoles).
     * @return QVariant Запрошенные данные.
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // --- Публичный API для управления моделью (вызывается из MainWindow) ---

    /**
     * @brief Интеллектуально обновляет список контактов.
     *
     * Сравнивает текущий список с новым, вычисляет разницу (diff) и
     * применяет только необходимые изменения (вставка/удаление строк).
     * Это позволяет избежать полной перерисовки списка и сброса выделения.
     * @param usernames Новый отсортированный список имен пользователей.
     */
    void updateContacts(const QStringList& usernames);

    /**
     * @brief Принудительно обновляет данные одного контакта.
     *
     * Вызывается, когда изменяется состояние контакта (например, пришел статус "печатает",
     * новое сообщение, или изменился счетчик непрочитанных), но порядок в списке не меняется.
     * Эмитирует сигнал dataChanged() для конкретной строки, заставляя представление перерисовать её.
     * @param username Имя пользователя, данные которого нужно обновить.
     */
    void refreshContact(const QString& username);

    /**
     * @brief Полностью очищает модель.
     * Вызывается, например, при выходе из аккаунта.
     */
    void clear();

private:
    // Указатель на MainWindow. Используется как источник данных.
    MainWindow* m_mainWindow;

    // Единственные данные, которыми владеет сама модель:
    // упорядоченный список имен пользователей (первичных ключей).
    QStringList m_contactUsernames;
};

#endif // CONTACTLISTMODEL_H
