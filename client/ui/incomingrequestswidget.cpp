#include "incomingrequestswidget.h"
#include "requestitemwidget.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>


IncomingRequestsWidget::IncomingRequestsWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    m_listWidget = new QListWidget(this);
    layout->addWidget(m_listWidget);
    setLayout(layout);
}


void IncomingRequestsWidget::updateRequests(const QJsonArray& requests)
{
    m_listWidget->clear();
    qDebug() << "start update requests, count:" << requests.size();
    for (const auto& val : requests)
    {
        QJsonObject req = val.toObject();
        addRequest(req);
    }
}


void IncomingRequestsWidget::addRequest(const QJsonObject& request)
{
    auto* itemWidget = new RequestItemWidget(request, m_listWidget);
    auto* listItem = new QListWidgetItem(m_listWidget);
    listItem->setSizeHint(QSize(300, 70));
    m_listWidget->addItem(listItem);

    m_listWidget->setItemWidget(listItem, itemWidget);

    connect(itemWidget, &RequestItemWidget::accepted, this, [this, listItem, itemWidget](const QJsonObject& req){
        m_listWidget->takeItem(m_listWidget->row(listItem));
        qDebug() <<"Accept emited";
        emit requestAccepted(req);
        itemWidget->deleteLater();
    });
    connect(itemWidget, &RequestItemWidget::rejected, this, [this, listItem, itemWidget](const QJsonObject& req){
        m_listWidget->takeItem(m_listWidget->row(listItem));
        emit requestRejected(req);
        itemWidget->deleteLater();
    });
}
