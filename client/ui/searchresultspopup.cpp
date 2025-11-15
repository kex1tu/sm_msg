#include "searchresultspopup.h"
#include <QVBoxLayout>     
#include <QJsonObject>     
#include <QListWidgetItem> 
#include <QFontMetrics>


SearchResultsPopup::SearchResultsPopup(QWidget *parent) : QWidget(parent)
{
    // Окно в стиле popup без рамки, не захватывает фокус
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // Список с результатами поиска
    m_listWidget = new QListWidget(this);

    // Вертикальный layout, чтобы список занимал всё пространство popup
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);  // чтобы popup был компактным
    layout->addWidget(m_listWidget);
    setLayout(layout);

    // Красивое затемнённое оформление
    setStyleSheet(
        "QListWidget {"
        "background-color: #2A2A2A;"    // фон
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

    // По клику по элементу результата — эмитируем сигнал и скрываем popup
    connect(m_listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        // username кладётся в UserRole
        emit userSelected(item->data(Qt::UserRole).toString());
        hide();
    });
}


void SearchResultsPopup::showResults(const QJsonArray &users)
{
    // Очищаем прошлые результаты
    m_listWidget->clear();

    // Парсим каждый QJsonObject из массива пользователей (displayName/username)
    for (const QJsonValue &value : users) {
        QJsonObject userObj = value.toObject();
        QString displayName = userObj["displayname"].toString();
        QString username = userObj["username"].toString();

        // Итоговая строка (имя + @логин)
        QListWidgetItem *item = new QListWidgetItem(displayName + " (@" + username + ")");
        item->setData(Qt::UserRole, username);     // username для выбора
        m_listWidget->addItem(item);
    }

    // Если результатов нет — закрываем popup
    int count = m_listWidget->count();
    if (count == 0) {
        hide();
        return;
    }

    // Вычисляем высоту под количество элементов: максимум 4, остальное скролл
    QFontMetrics fm(m_listWidget->font());
    int itemHeight = fm.height() + 37;      // визуальный размер элемента
    const int maxVisibleItems = 4;          // максимум строк без скролла
    int targetHeight = 0;
    if (count > maxVisibleItems) {
        // Включить скролл и показывать только первые 4 визуально
        targetHeight = itemHeight * maxVisibleItems + m_listWidget->frameWidth() * 2;
        m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        targetHeight = itemHeight * count + m_listWidget->frameWidth() * 2;
        m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    // Устанавливаем итоговую высоту popup'а
    this->setFixedHeight(targetHeight);

    // Показываем popup только если он ещё скрыт
    if (!isVisible()) {
        show();
    }
}
