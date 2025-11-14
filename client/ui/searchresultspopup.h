#ifndef SEARCHRESULTSPOPUP_H
#define SEARCHRESULTSPOPUP_H

#include <QWidget>       
#include <QListWidget>   
#include <QJsonArray>    

 
class SearchResultsPopup : public QWidget
{
    Q_OBJECT  

public:
    /**
 * @brief Конструктор SearchResultsPopup — создание всплывающего списка результатов поиска пользователей.
 * @param parent Родительский QWidget
 *
 * - Устанавливает флаги окна (Popup, Frameless).
 * - Создаёт QListWidget для отображения результатов.
 * - Настраивает стили (тёмный фон, подсветка выбранного и наведения).
 * - Подключает обработчик клика по пользователю: эмитит userSelected, прячет окно.
 */
    explicit SearchResultsPopup(QWidget *parent = nullptr);
    /**
 * @brief Отображает результаты поиска (пользователей) во всплывающем списке.
 * @param users QJsonArray с найденными пользователями
 *
 * - Очищает список, добавляет новые элементы.
 * - Формирует текст: displayName + username.
 * - Эмитирует userSelected при выборе, а также корректно прячет popup если результатов нет.
 * - Размер pop-up ограничен maxVisibleItems (прокрутка, если много результатов).
 */
    void showResults(const QJsonArray &users);

private:
    /**
     * @brief Список для отображения найденных пользователей.
     * Хранит элементы, к которым привязаны username через Qt::UserRole.
     */
    QListWidget *m_listWidget;

signals:
    /**
     * @brief Сигнал при выборе пользователя из результатов поиска.
     * @param username Имя пользователя (username), выбранного из списка
     */
    void userSelected(const QString& username);
};
#endif  
