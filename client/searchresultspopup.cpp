/**
 * @file searchresultspopup.cpp
 * @brief Реализация всплывающего виджета для отображения результатов поиска пользователей.
 * @see SearchResultsPopup
 * @author kex1tu
 */

#include "searchresultspopup.h"
#include <QVBoxLayout>    // Для компоновки внутреннего QListWidget.
#include <QJsonObject>    // Для разбора данных о пользователе.
#include <QListWidgetItem>// Для создания элементов списка.
#include <QFontMetrics>   // Для расчета высоты элементов.

/**
 * @brief Конструктор SearchResultsPopup.
 * @details Настраивает флаги окна, чтобы оно вело себя как всплывающее меню,
 *          создает внутренние виджеты, устанавливает стили и соединяет сигналы.
 * @param parent Родительский виджет.
 */
SearchResultsPopup::SearchResultsPopup(QWidget *parent) : QWidget(parent)
{
    // --- 1. Настройка поведения окна ---
    // Qt::Popup: Делает виджет всплывающим окном верхнего уровня, которое
    // автоматически закрывается при клике вне его области.
    // Qt::FramelessWindowHint: Убирает стандартную рамку окна (заголовок, кнопки).
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

    // Qt::WA_ShowWithoutActivating: Позволяет виджету появиться на экране,
    // не забирая фокус ввода у другого виджета (в нашем случае, у QLineEdit поиска).
    setAttribute(Qt::WA_ShowWithoutActivating);

    // --- 2. Создание и компоновка внутреннего UI ---
    m_listWidget = new QListWidget(this);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // Убираем отступы, чтобы список занимал все место.
    layout->addWidget(m_listWidget);
    setLayout(layout);

    // --- 3. Установка стилей (QSS) ---
    // Стили заданы прямо в коде, так как это небольшой, самодостаточный компонент.
    // Альтернатива - вынести их в общий styles.css, используя селектор по имени объекта.
    setStyleSheet(
        "QListWidget {"
        "background-color: #2A2A2A;"  /* Темный фон списка */
        "border-radius: 10px;"       /* Скругленные углы */
        "padding: 4px;"              /* Внутренний отступ */
        "border: none;"              /* Без рамки */
        "}"
        "QListWidget::item {"
        "border-radius: 8px;"        /* Скругленные углы для каждого элемента */
        "padding: 5px;"
        "margin: 1px 0px;"
        "color: white;"
        "}"
        "QListWidget::item:hover {"
        "background-color: #444444;" /* Цвет при наведении */
        "}"
        "QListWidget::item:selected {"
        "background-color: #00557F;" /* Цвет при клике (хотя мы сразу закрываем) */
        "}"
        );

    // --- 4. Соединение сигналов и слотов ---
    // Соединяем сигнал itemClicked от QListWidget с лямбда-функцией.
    connect(m_listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        // Когда пользователь кликает на элемент:
        // 1. Извлекаем сохраненный 'username' из данных элемента.
        // 2. Эмитируем сигнал userSelected наружу (для MainWindow).
        emit userSelected(item->data(Qt::UserRole).toString());
        // 3. Скрываем всплывающее окно.
        hide();
    });
}

/**
 * @brief Заполняет виджет результатами поиска, настраивает его размер и отображает.
 * @param users Массив JSON-объектов, представляющих найденных пользователей.
 */
void SearchResultsPopup::showResults(const QJsonArray &users)
{
    // 1. Очищаем список от предыдущих результатов.
    m_listWidget->clear();

    // 2. Заполняем список новыми данными.
    for (const QJsonValue &value : users) {
        QJsonObject userObj = value.toObject();
        QString displayName = userObj["displayname"].toString();
        QString username = userObj["username"].toString();

        // Создаем текстовое представление для отображения.
        QListWidgetItem *item = new QListWidgetItem(displayName + " (@" + username + ")");
        // Сохраняем `username` в `Qt::UserRole`. Это позволяет нам легко
        // получить идентификатор пользователя при клике, не парся строку.
        item->setData(Qt::UserRole, username);
        m_listWidget->addItem(item);
    }

    // 3. Если результатов нет, просто скрываем окно и выходим.
    int count = m_listWidget->count();
    if (count == 0) {
        hide();
        return;
    }

    // --- 4. Динамический расчет высоты виджета ---
    QFontMetrics fm(m_listWidget->font());
    int itemHeight = fm.height() + 37; // Расчетная высота одного элемента (подобрана экспериментально).
    const int maxVisibleItems = 4;     // Максимальное количество элементов без прокрутки.

    int targetHeight = 0;
    if (count > maxVisibleItems) {
        // Если элементов больше максимума, ограничиваем высоту и показываем скроллбар.
        targetHeight = itemHeight * maxVisibleItems + m_listWidget->frameWidth() * 2;
        m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        // Если элементов меньше, делаем высоту точно по их количеству и скрываем скроллбар.
        targetHeight = itemHeight * count + m_listWidget->frameWidth() * 2;
        m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    // Устанавливаем рассчитанную высоту для всего виджета.
    this->setFixedHeight(targetHeight);

    // 5. Показываем виджет, если он был скрыт.
    if (!isVisible()) {
        show();
    }
}
