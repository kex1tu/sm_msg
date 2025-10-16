/**
 * @file contactlistmodel.cpp
 * @brief Реализация модели данных для списка контактов.
 * @details Этот класс действует как адаптер между данными, хранящимися в MainWindow,
 *          и представлением (QListView), которое их отображает.
 * @see ContactListModel
 * @author kex1tu
 */

#include "contactlistmodel.h"
#include "mainwindow.h"
#include <QSet> // Включаем QSet для высокопроизводительных операций сравнения списков.
#include <QDebug> // Для отладочных сообщений.

/**
 * @brief Конструктор модели.
 * @param mainWindow Указатель на MainWindow, который выступает в роли "источника правды"
 *                   (владельца всех кэшей данных).
 * @param parent Родительский объект в иерархии Qt.
 */
ContactListModel::ContactListModel(MainWindow *mainWindow, QObject *parent)
    : QAbstractListModel(parent), m_mainWindow(mainWindow)
{}

/**
 * @brief Возвращает количество строк (контактов) в модели.
 * @param parent Для одноуровневой списочной модели этот параметр всегда невалидный.
 * @return int Количество контактов.
 */
int ContactListModel::rowCount(const QModelIndex &parent) const
{
    // Если parent валидный, значит, это древовидная модель, а у нас список,
    // поэтому возвращаем 0.
    if (parent.isValid())
        return 0;

    return m_contactUsernames.count();
}

/**
 * @brief Основной метод для получения данных из модели по запросу от представления/делегата.
 * @param index Индекс элемента (строка, колонка), для которого нужны данные.
 * @param role Роль (тип) запрашиваемых данных, например, Qt::DisplayRole или кастомная роль.
 * @return QVariant Данные, обернутые в QVariant.
 */
QVariant ContactListModel::data(const QModelIndex &index, int role) const
{
    // Проверка на валидность индекса и наличие указателя на MainWindow.
    if (!index.isValid() || !m_mainWindow) return QVariant();

    // Получаем username, который является нашим ключом/идентификатором.
    QString username = m_contactUsernames.at(index.row());

    // Получаем полный объект User из кэша MainWindow.
    const User user = m_mainWindow->getUserFromCache(username);

    // В зависимости от роли, возвращаем соответствующее поле из кэшей MainWindow.
    // Это и есть суть паттерна "модель-адаптер".
    switch (role) {
    case Qt::DisplayRole:
        return user.displayName; // Текст, который будет отображаться по умолчанию.
    case UsernameRole:
        return user.username;
    case IsOnlineRole:
        return user.isOnline;
    case IsTypingRole:
        // Дополнительная логика: не показывать статус "печатает" для чата, который и так открыт.
        return (user.isTyping && username != m_mainWindow->getCurrentChatPartner().username);
    case LastMessageRole:
        // Обращаемся к кэшу истории чатов.
        if (m_mainWindow->getChatCache().contains(username)) {
            const auto& messages = m_mainWindow->getChatCache().value(username).messages;
            if (!messages.isEmpty()) {
                return messages.last().payload; // Возвращаем текст последнего сообщения.
            }
        }
        return QVariant(); // Возвращаем пустой QVariant, если истории нет.
    case UnreadCountRole:
        return m_mainWindow->getUnreadCounts().value(username, 0); // 0, если для юзера нет записей.
    }

    return QVariant();
}

/**
 * @brief Полностью очищает модель от всех данных.
 * @details Вызывается, например, при выходе пользователя из аккаунта.
 */
void ContactListModel::clear()
{
    // beginResetModel/endResetModel - это сигнал для представлений, что модель
    // будет полностью сброшена и им нужно очистить свое состояние.
    beginResetModel();
    m_contactUsernames.clear();
    endResetModel();
}

/**
 * @brief Принудительно обновляет один элемент в списке.
 * @details Этот метод вызывается, когда данные контакта изменились (например,
 *          статус "онлайн" или "печатает"), но его позиция в списке осталась прежней.
 * @param username Имя пользователя, элемент которого нужно перерисовать.
 */
void ContactListModel::refreshContact(const QString &username)
{
    // Находим индекс (номер строки) элемента по имени пользователя.
    int row = m_contactUsernames.indexOf(username);
    if (row != -1) {
        QModelIndex idx = index(row, 0);
        // Эмитируем сигнал dataChanged() для этого индекса.
        // QListView, получив этот сигнал, попросит делегата перерисовать
        // только этот конкретный элемент, что очень эффективно.
        emit dataChanged(idx, idx);
    }
}

/**
 * @brief Выполняет "умное" обновление списка контактов.
 * @details Сравнивает старый и новый списки контактов и применяет только
 *          необходимые изменения (добавление новых, удаление старых),
 *          вместо полной перезагрузки. Это сохраняет выделение и позицию
 *          прокрутки в QListView.
 * @param newUsernames Новый, уже отсортированный, список имен пользователей.
 */
void ContactListModel::updateContacts(const QStringList &newUsernames)
{
    const QStringList oldUsernames = m_contactUsernames;

    // Используем QSet для сверхбыстрой проверки наличия элемента (O(1) в среднем).
    const QSet<QString> newUsernamesSet = QSet<QString>(newUsernames.begin(), newUsernames.end());
    const QSet<QString> oldUsernamesSet = QSet<QString>(oldUsernames.begin(), oldUsernames.end());

    // --- Фаза 1: Удаление старых контактов ---
    // Итерируемся в обратном порядке (с конца к началу), чтобы индексы
    // не "съезжали" при удалении элементов из списка.
    for (int i = oldUsernames.count() - 1; i >= 0; --i) {
        if (!newUsernamesSet.contains(oldUsernames[i])) {
            // Если старого юзера нет в новом списке, удаляем его.
            beginRemoveRows(QModelIndex(), i, i); // Уведомляем view, что строка 'i' будет удалена.
            m_contactUsernames.removeAt(i);
            endRemoveRows(); // Завершаем операцию удаления.
        }
    }

    // --- Фаза 2: Добавление новых контактов ---
    for (int i = 0; i < newUsernames.count(); ++i) {
        if (!oldUsernamesSet.contains(newUsernames[i])) {
            // Если новый юзер не был в старом списке, добавляем его.
            // Так как newUsernames уже отсортирован, мы просто вставляем его
            // на ту же позицию 'i'.
            beginInsertRows(QModelIndex(), i, i); // Уведомляем view, что в позицию 'i' будет вставлена строка.
            m_contactUsernames.insert(i, newUsernames[i]);
            endInsertRows(); // Завершаем операцию вставки.
        }
    }
    // Примечание: Эта реализация не обрабатывает "перемещение" элементов,
    // а рассматривает его как удаление старого и вставку нового. Для простого
    // списка контактов этого более чем достаточно.
}
