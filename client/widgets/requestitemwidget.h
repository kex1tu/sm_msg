#ifndef REQUESTITEMWIDGET_H
#define REQUESTITEMWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QJsonObject>

class RequestItemWidget : public QWidget {
    Q_OBJECT
public:
    /**
 * @brief Виджет отдельного входящего запроса (например, на добавление в друзья).
 * Показывает имя, username и кнопки принять/отклонить, эмитит сигналы accepted/rejected.
 * @param req QJsonObject запроса (fromDisplayName/fromUsername и др.)
 * @param parent Родительский виджет
 */
    RequestItemWidget(const QJsonObject& req, QWidget* parent);


signals:

    /**
 * @brief Сигнал о принятии запроса.
 * @param request Объект запроса (QJsonObject), который был принят
 */
    void accepted(const QJsonObject& request);

    /**
 * @brief Сигнал об отклонении запроса.
 * @param request Объект запроса (QJsonObject), который был отклонён
 */
    void rejected(const QJsonObject& request);

private:
    /**
 * @brief QJsonObject, представляющий данные конкретного запроса (friend/invite и т.д.).
 */
    QJsonObject m_request;
};

#endif  
