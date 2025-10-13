#include "searchresultspopup.h"
#include <QVBoxLayout>
#include <QJsonObject>
#include <QListWidgetItem>

SearchResultsPopup::SearchResultsPopup(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);

    m_listWidget = new QListWidget(this);
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_listWidget);
    setLayout(layout);

    setStyleSheet(
        "QListWidget {"
        "background-color: #2A2A2A;"  
        "border-radius: 10px;"       
        "padding: 4px;"              
        "border: none;"              
        "}"

        "QListWidget::item {"
         
        "border-radius: 8px;"       

        "padding: 5px;"             
        "margin: 1px 0px;"          
        "color: white;"             
        "}"

        "QListWidget::item:hover {"
        "background-color: #444444;"  
        "}"

        "QListWidget::item:selected {"
        "background-color: #00557F;"  
        "}"
        );

    connect(m_listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        emit userSelected(item->data(Qt::UserRole).toString());
        hide();
    });
}

void SearchResultsPopup::showResults(const QJsonArray &users)
{
    m_listWidget->clear();
    for (const QJsonValue &value : users) {
        QJsonObject userObj = value.toObject();
        QString displayName = userObj["displayname"].toString();
        QString username = userObj["username"].toString();

        QListWidgetItem *item = new QListWidgetItem(displayName + " (@" + username + ")");
        item->setData(Qt::UserRole, username);
        m_listWidget->addItem(item);
    }
    int count = m_listWidget->count();
    if (count == 0) {
        hide();
        return;
    }

    QFontMetrics fm(m_listWidget->font());
    int itemHeight = fm.height() + 37; 
    const int maxVisibleItems = 4;

    int targetHeight = 0;
    if (count > maxVisibleItems) {
        targetHeight = itemHeight * maxVisibleItems + m_listWidget->frameWidth() * 2;
        m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        targetHeight = itemHeight * count + m_listWidget->frameWidth() * 2;
        m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    this->setFixedHeight(targetHeight);

    if (!isVisible()) {
        show();
    }
}
