#ifndef CHATVIEWWIDGET_H
#define CHATVIEWWIDGET_H

#include <QWidget>
#include "structures.h"

class QStackedWidget;
class QLabel;
class QToolButton;
class QLineEdit;
class QListView;
class QTextEdit;
class QPropertyAnimation;

namespace Ui {
class ChatViewWidget;
}

class ChatViewWidget : public QWidget
{
    Q_OBJECT  

public:
    /**
 * @brief Конструктор ChatViewWidget — инициализирует UI, анимацию, кнопки, поля и основные сигналы/слоты.
 * @param parent Родительский QWidget
 *
 * - Инициализирует UI через setupUi
 * - Настраивает анимацию появления блока reply, скрывает блок по умолчанию
 * - Подключает сигналы для replyWidget и textEdit, обновляет высоты, фильтры
 * - Создает кнопки "вниз", "непрочитано" и прячет их по умолчанию
 * - Подключает закрытие reply, отправку сообщений, поведение истории и скролл
 * - Настраивает анимацию прокрутки чата
 */
    explicit ChatViewWidget(QWidget *parent = nullptr);
    /**
 * @brief Деструктор ChatViewWidget, освобождает ресурсы UI.
 */
    ~ChatViewWidget();
    /**
 * @brief Геттер для виджета истории сообщений (QListView).
 */
    QListView* chatHistoryView() const;
    /**
 * @brief Геттер для текстового поля ввода сообщения (QTextEdit).
 */
    QTextEdit* messageTextEdit() const;
    /**
 * @brief Проверяет, прокручен ли чат до самого низа (±5 пикселей).
 * @return true если чат долистали до конца, иначе false
 */
    bool isScrolledToBottom() const;
signals:
    /**
 * @brief Сигнал запроса на отправку нового сообщения (нажатие send).
 * @param text Текст сообщения для отправки
 */
    void sendMessageRequested(const QString& text);

    /**
 * @brief Сигнал: пользователь кликнул по заголовку чата (аватар или имя).
 */
    void headerClicked();

    /**
 * @brief Сигнал: нажатие кнопки поиска в чате.
 */
    void searchButtonClicked();

    /**
 * @brief Сигнал: пользователь начал ответ (reply) на конкретное сообщение.
 * @param messageId ID сообщения, на которое отвечают
 */
    void replyToMessageRequested(qint64 messageId);

    /**
 * @brief Сигнал отмены режима reply (пользователь закрыл reply-блок).
 */
    void replyCancelled();

    /**
 * @brief Сигнал: в поисковой строке чата введён/изменён текст.
 * @param text Строка поиска
 */
    void searchTextEntered(const QString& text);

    /**
 * @brief Сигнал: запрошена прокрутка к первому непрочитанному сообщению.
 */
    void scrollToUnreadRequested();

    /**
 * @brief Сигнал: запрошена прокрутка в конец истории чата.
 */
    void scrollToBottomRequested();

    /**
 * @brief Сигнал: пользователь инициировал вызов (аудио/видео call).
 */
    void callRequested();


    /**
 * @brief Сигнал: запрос редактирования сообщения.
 * @param messageId ID сообщения
 * @param oldText Предыдущий текст сообщения
 */
    void editMessageRequested(qint64 messageId, const QString& oldText);

    /**
 * @brief Сигнал: запрос удаления сообщения.
 * @param messageId ID сообщения
 */
    void deleteMessageRequested(qint64 messageId);
     
public slots:
    /**
 * @brief Слот: вызывается при изменении текста поиска — полезно для дебага или live-поиска.
 * @param text Строка поиска в чате
 */
    void onSearchTriggered(const QString& text);
    /**
 * @brief Показывает контекстное меню для сообщения; копировать, ответить, редактировать, удалить.
 * @param pos Позиция вызова меню
 */
    void onChatContextMenuRequested(const QPoint &pos);
    /**
 * @brief Обработка двойного клика по сообщению — показать reply-блок, инициировать replyToMessageRequested.
 * @param index QModelIndex сообщения
 */
    void onMessageDoubleClicked(const QModelIndex &index);
    /**
 * @brief Слот: вызывается при прокрутке чата. Если достигнут низ — сбрасывает непрочитанные.
 * @param value Новое значение вертикального скролла
 */
    void onChatScrolled(int value);
    /**
 * @brief Переключает header на поисковый режим, фокусирует строку поиска.
 */
    void showSearchUI();
    /**
 * @brief Скрывает поисковый header, очищает строку поиска, возвращает основной header.
 */
    void hideSearchUI();
    /**
 * @brief Обновляет header информацией о собеседнике: имя, статус, стиль.
 * @param chatPartner Объект пользователя для header
 */
    void updateHeader(const User& chatPartner);
    /**
 * @brief Переключает режим редактирования сообщения.
 * @param enabled true — режим редактирования, false — обычный режим чата.
 * @param text Текст сообщения для редактирования (отображается в input)
 *
 * - Меняет текст кнопки ("Сохранить"/"Отправить"), наполняет/очищает input
 */
    void setEditMode(bool enabled, const QString& text = QString());
    /**
 * @brief Очищает блок ответа (reply) — убирает placeholder.
 */
    void clearReplyUI();
    /**
 * @brief Показать UI-блок для ответа на сообщение (reply): имя/текст, анимация появления, автофокус.
 * @param name Имя адресата, кому отправляется reply
 * @param text Пример текста цитаты/ответа
 *
 * - Элипсирует длинный текст, чтобы не выходил за пределы блока
 * - Запускает анимацию раскрытия replyWidget
 * - Автофокус на поле ввода
 */
    void showReplyUI(const QString& name, const QString& text);
    /**
 * @brief Скрывает и очищает UI блока reply, эмитит сигнал отмены, обновляет кнопки скролла.
 *
 * - Скрывает replyWidget, вызывает clearReplyUI().
 * - Эмитит replyCancelled().
 * - Через 50мс обновляет индикатор/кнопку прокрутки (где-то мог измениться layout)
 */
    void hideReplyUI();
    /**
 * @brief Слот: вызов при появлении нового сообщения.
 *
 * - Если не прокручено до конца, увеличивает счетчик непрочитанных.
 * - Обновляет UI-индикатор кнопки scroll-to-bottom.
 */
    void onNewMessageReceived();
    /**
 * @brief Плавно скроллит чат в самый низ. Сбрасывает счетчик непрочитанных.
 *
 * - Оповещает модель о возможных изменениях.
 * - Запускает анимацию прокрутки к самому низу.
 * - Обнуляет счетчик непрочитанных и обновляет кнопку "к низу".
 */
    void scrollToBottom();
    /**
 * @brief Аккуратно прокручивает чат к нужному сообщению по индексу (если нет — в самый низ).
 * @param index Индекс сообщения
 */
    void scrollToMessage(const QModelIndex& index);

    /**
 * @brief Слот для обработки клика по кнопке "Вниз" или "Показать непрочитанные".
 *
 * - Если есть непрочитанные — инициирует прокрутку к ним (emit scrollToUnreadRequested).
 * - Если нет — скроллит в самый низ (emit scrollToBottomRequested).
 */
    void onScrollDownButtonClicked();
    /**
 * @brief Слот: обработка нажатия на кнопку звонка в header (эмитит сигнал).
 */
    void onCallButtonClicked();
     
protected:
    /**
 * @brief Обработчик события изменения размера окна.
 *
 * - Сбрасывает кэш клиента сообщений при изменении ширины (для пересчёта sizeHint).
 * - Обновляет кнопку "Вниз"/"к непрочитанным".
 * @param event QResizeEvent*
 */
    void resizeEvent(QResizeEvent *event) override;
    /**
 * @brief Фильтрует события для виджетов ChatViewWidget (обработка Enter/Shift+Enter, щелчков на userinfo).
 *
 * - Для messageTextEdit: Enter ➔ отправить сообщение (как Telegram), Shift+Enter ➔ перевод строки.
 * - Для userInfoWidget: ЛКМ — эмитим headerClicked().
 * - Если событие не обработано — передаёт дальше стандартному обработчику QWidget.
 *
 * @param watched Какой объект отслеживается
 * @param event Событие
 * @return true если обработано, false если стандартная обработка
 */
    bool eventFilter(QObject *watched, QEvent *event) override;
    /**
 * @brief Инициализирует header UI чата: обычный и поисковый header, кнопки, лейблы, layout'ы, сигналы.
 *
 * - Создаёт стек для заголовков (основной/поисковый).
 * - Строит обычный header с именем, статусом, аватаром, кнопками поиска, звонка, видео и опций.
 * - Инициализирует поисковый header (поисковое поле, кнопка закрытия).
 * - Правильно подключает layout к headerWidget (учитывает отсутствие layout).
 * - Подключает нужные сигналы для поиска, звонка, видео, опций.
 */
    void setupHeaderUI();
    /**
 * @brief Обновляет состояние и позиционирование кнопки прокрутки вниз и лейбла непрочитанных.
 *
 * - Показывает кнопку если: есть непрочитанные ИЛИ далеко от конца (>scrollThreshold).
 * - Расставляет кнопку и лейбл по координатам.
 * - Скрывает всё, если условий нет.
 */
    void updateScrollToBottomButton();

protected:
    /**
 * @brief Стек виджетов заголовка (основной/поисковый).
 */
    QStackedWidget* m_headerStack;

    /**
 * @brief Обычный header (с аватаром, именем, статусом, кнопками).
 */
    QWidget* m_normalHeaderWidget;

    /**
 * @brief Header для поиска по истории чата.
 */
    QWidget* m_searchHeaderWidget;

    /**
 * @brief Ui-класс, сгенерированный Qt Designer для ChatViewWidget.
 */
    Ui::ChatViewWidget *ui;

    /**
 * @brief Лейбл имени собеседника (в header).
 */
    QLabel* m_nameLabel;

    /**
 * @brief Лейбл статуса собеседника (онлайн, занят, last seen и т.д.).
 */
    QLabel* m_statusLabel;

    /**
 * @brief Кнопка поиска в чате.
 */
    QToolButton* m_searchButton;

    /**
 * @brief Кнопка вызова (аудио) в чате.
 */
    QToolButton* m_callButton;

    /**
 * @brief Кнопка видеозвонка.
 */
    QToolButton* m_videoCallButton;

    /**
 * @brief Кнопка дополнительных опций (меню, dots).
 */
    QToolButton* m_moreOptionsButton;

    /**
 * @brief Кнопка закрытия reply-блока (цитата сообщения).
 */
    QToolButton* m_closeReplyButton;

    /**
 * @brief Поле для ввода строки поиска.
 */
    QLineEdit* m_searchLineEdit;

    /**
 * @brief Кнопка закрытия поискового режима.
 */
    QToolButton* m_closeSearchButton;

    /**
 * @brief Кнопка прокрутки вниз (и к непрочитанным).
 */
    QToolButton* m_scrollToBottomButton;

    /**
 * @brief Лейбл количества непрочитанных (над кнопкой "вниз").
 */
    QLabel* m_unreadCountLabel;

    /**
 * @brief Число непрочитанных сообщений (для UI).
 */
    int m_unreadMessageCount = 0;

    /**
 * @brief Анимация раскрытия и сворачивания reply-блока.
 */
    QPropertyAnimation* m_replyAnimation;

    /**
 * @brief Визуальный блок с инфой о пользователе (аватар, имя, статус).
 */
    QWidget* m_userInfoWidget;

    /**
 * @brief Анимация плавной прокрутки истории чата.
 */
    QPropertyAnimation* m_scrollAnimation;


};

#endif
/**
 * @brief Русская функция склонения слова по числу (1 сообщение, 2 сообщения, 5 сообщений).
 * @param n число для склонения
 * @param form1 единственное (сообщение)
 * @param form2 родительный (сообщения)
 * @param form5 множественное (сообщений)
 */
QString pluralize(int n, const QString& form1, const QString& form2, const QString& form5);
/**
 * @brief Форматирует статус "онлайн/был(а) недавно" для пользователя.
 * @param user Структура пользователя с датой lastSeen
 * @return Человеко-понятная строка статуса
 *
 * - Передает склонение в pluralize (минута/минуты/минут)
 * - Учитывает только что/минуты/сегодня/вчера/дата-время
 */
QString formatLastSeen(const User &user);
