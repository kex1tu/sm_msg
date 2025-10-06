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
        "background-color: #2A2A2A;" /* Цвет фона всего списка */
        "border-radius: 10px;"      /* Скругляем углы контейнера */
        "padding: 4px;"             /* Небольшой отступ от краев контейнера до элементов */
        "border: none;"             /* Убираем стандартную рамку, чтобы не мешала */
        "}"

        "QListWidget::item {"
        /* --- КЛЮЧЕВОЕ РЕШЕНИЕ --- */
        "border-radius: 8px;"      /* Скругляем углы самого элемента! */

        "padding: 5px;"            /* Внутренний отступ для текста */
        "margin: 1px 0px;"         /* Внешний отступ, чтобы элементы не слипались */
        "color: white;"            /* Цвет текста */
        "}"

        "QListWidget::item:hover {"
        "background-color: #444444;" /* Цвет подсветки при наведении */
        "}"

        "QListWidget::item:selected {"
        "background-color: #00557F;" /* Цвет для выделенного (кликнутого) элемента */
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
    int itemHeight = fm.height() + 37;//test
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
