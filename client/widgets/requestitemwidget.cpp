#include "requestitemwidget.h"

RequestItemWidget::RequestItemWidget(const QJsonObject& req, QWidget* parent)
    : QWidget(parent), m_request(req)
{
    // Получаем имя и username отправителя запроса
    QString name = req.value("fromDisplayName").toString();
    QString username = req.value("fromUsername").toString();

    // Горизонтальная компоновка: имя/кнопки
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);    // компактно
    layout->setSpacing(16);                 // отступ между кнопками и меткой

    // Метка — имя и username, расширяется влево
    QLabel* label = new QLabel(QString("%1 (%2)").arg(name, username));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setFont(QFont("Arial", 13));
    label->setStyleSheet("color: white;");

    // Кнопки принять/отклонить:
    QToolButton* acceptBtn = new QToolButton();
    QToolButton* rejectBtn = new QToolButton();
    acceptBtn->setText("Y");
    rejectBtn->setText("N");

    // По клику эмитим соответствующий сигнал с исходным request-объектом
    connect(acceptBtn, &QToolButton::clicked, this, [this] {
        emit accepted(m_request);
    });
    connect(rejectBtn, &QToolButton::clicked, this, [this] {
        emit rejected(m_request);
    });

    acceptBtn->setFixedSize(32,32);
    rejectBtn->setFixedSize(32,32);

    // Компоновка: метка — растяжка — кнопки
    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(acceptBtn);
    layout->addWidget(rejectBtn);
    setLayout(layout);
}
