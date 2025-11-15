#ifndef INCOMINGREQUESTSWIDGET_H
#define INCOMINGREQUESTSWIDGET_H

 
#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QJsonObject>  

class IncomingRequestsWidget : public QWidget {
    Q_OBJECT
public:
    /**
 * @brief Конструктор виджета входящих запросов на добавление в друзья (или приглашения).
 * @param parent Родительский QWidget
 *
 * - Создаёт QVBoxLayout, QListWidget.
 */
    explicit IncomingRequestsWidget(QWidget* parent = nullptr);
public slots:
    /**
 * @brief Обновляет и отображает список входящих запросов.
 * @param requests QJsonArray с массивом объектов запросов (каждый — QJsonObject)
 *
 * - Очищает старые элементы, добавляет новые через addRequest().
 */
    void updateRequests(const QJsonArray& requests);
    /**
 * @brief Добавляет один запрос в список и связывает слоты accept/reject для удаления из списка.
 * @param request QJsonObject c информацией по запросу
 *
 * - Создаёт RequestItemWidget и QListWidgetItem, помещает widget в item.
 * - Подключает обработчики: при accept/reject убирает элемент из списка, эмитит сигнал и удаляет widget.
 */
    void addRequest(const QJsonObject& request);
signals:
    /**
 * @brief Сигнал, испускаемый при принятии входящего запроса пользователем.
 * @param request QJsonObject — принятый запрос (с деталями)
 */
    void requestAccepted(const QJsonObject& request);

    /**
 * @brief Сигнал, испускаемый при отклонении входящего запроса пользователем.
 * @param request QJsonObject — отклонённый запрос
 */
    void requestRejected(const QJsonObject& request);


private:
    /**
 * @brief Виджет-список для отображения входящих запросов (RequestItemWidget).
 */
    QListWidget* m_listWidget;

    /**
 * @brief Коллекция (очередь) ожидающих обработки входящих запросов.
 * Могут быть использованы для дополнительной логики (например, батч-обработка).
 */
    QVector<QJsonObject> m_pendingRequests;
};


#endif  
