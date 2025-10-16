#ifndef SEARCHRESULTSPOPUP_H
#define SEARCHRESULTSPOPUP_H

#include <QWidget>      // Базовый класс для всех виджетов.
#include <QListWidget>  // Виджет, используемый для отображения списка.
#include <QJsonArray>   // Для получения данных о пользователях в формате JSON.

/**
 * @class SearchResultsPopup
 * @brief Всплывающий виджет для отображения списка пользователей, найденных в результате глобального поиска.
 *
 * @details Этот класс представляет собой самодостаточный, всплывающий (Qt::Popup) виджет,
 * который появляется под строкой поиска. Он получает данные в виде QJsonArray,
 * отображает их в виде списка и уведомляет родительский виджет о выборе пользователя
 * через сигнал userSelected().
 * @author kex1tu
 */
class SearchResultsPopup : public QWidget
{
    Q_OBJECT // Обязательный макрос для классов, использующих сигналы и слоты.

public:
    /**
     * @brief Конструктор класса.
     * @param parent Родительский виджет. Указание родителя важно для правильного
     *               управления памятью в системе Qt.
     */
    explicit SearchResultsPopup(QWidget *parent = nullptr);

    /**
     * @brief Заполняет виджет результатами поиска и отображает его.
     *
     * @details Этот метод очищает предыдущий список, заполняет его новыми данными из
     * `users`, динамически настраивает свою высоту в зависимости от количества
     * результатов и отображает себя на экране.
     *
     * @param users Массив JSON-объектов, где каждый объект представляет одного
     *              найденного пользователя (ожидается наличие полей "username" и "displayname").
     */
    void showResults(const QJsonArray &users);

private:
    /**
     * @brief Указатель на внутренний QListWidget.
     * @details Этот виджет используется для фактического отображения списка пользователей.
     *          SearchResultsPopup является для него оберткой и контроллером.
     */
    QListWidget *m_listWidget;

signals:
    /**
     * @brief Сигнал испускается, когда пользователь кликает на один из элементов в списке.
     * @param username Уникальное имя пользователя (`username`) выбранного контакта.
     */
    void userSelected(const QString& username);
};

#endif // SEARCHRESULTSPOPUP_H
