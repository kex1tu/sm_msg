#ifndef CHATVIEWWIDGET_H
#define CHATVIEWWIDGET_H

#include <QWidget>
#include "structures.h" // Включаем определения структур User и ChatMessage.

// Прямые объявления (Forward Declarations) для классов Qt.
// Это позволяет использовать указатели на них без включения полных заголовочных файлов,
// что ускоряет компиляцию.
class QStackedWidget;
class QLabel;
class QToolButton;
class QLineEdit;
class QListView;
class QTextEdit;
class QPropertyAnimation;

// Пространство имен, генерируемое Qt UI Compiler (uic).
// Содержит класс для доступа к виджетам, созданным в Qt Designer.
namespace Ui {
class ChatViewWidget;
}

/**
 * @class ChatViewWidget
 * @brief Виджет, отвечающий за отображение интерфейса одного чата.
 *
 * Этот класс инкапсулирует всю визуальную часть диалога: заголовок,
 * историю сообщений и панель ввода. Он не содержит бизнес-логики, а только
 * отображает данные и уведомляет `MainWindow` о действиях пользователя через сигналы.
 * @author kex1tu
 */
class ChatViewWidget : public QWidget
{
    Q_OBJECT // Обязательный макрос для классов, использующих сигналы и слоты.

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit ChatViewWidget(QWidget *parent = nullptr);
    ~ChatViewWidget(); // Деструктор.

    // --- Методы доступа (Getters) к ключевым элементам UI ---
    // Предоставляют `MainWindow` доступ к виджетам для настройки (например, установки модели).
    QListView* chatHistoryView() const; // Возвращает указатель на список сообщений.
    QTextEdit* messageTextEdit() const; // Возвращает указатель на поле ввода текста.

    // --- Вспомогательные методы ---
    /**
     * @brief Проверяет, прокручен ли список сообщений до самого низа.
     * @return true, если пользователь видит последние сообщения.
     */
    bool isScrolledToBottom() const;

    // Публичные слоты - это часть API виджета, которую могут вызывать другие классы (например, MainWindow).
public slots:
    void updateHeader(const User& chatPartner); // Обновляет заголовок чата (имя, статус).
    void setEditMode(bool enabled, const QString& text = QString()); // Включает/выключает режим редактирования сообщения.
    void clearReplyUI(); // Очищает UI ответа (сбрасывает placeholder).
    void showReplyUI(const QString& name, const QString& text); // Показывает панель ответа с анимацией.
    void hideReplyUI(); // Скрывает панель ответа.
    void onNewMessageReceived(); // Слот, вызываемый при получении нового сообщения, когда чат не прокручен вниз.
    void scrollToBottom(); // Прокручивает список сообщений в самый низ.

    // Сигналы - способ, которым этот виджет сообщает внешнему миру (MainWindow) о действиях пользователя.
signals:
    void sendMessageRequested(const QString& text); // Пользователь нажал "Отправить".
    void headerClicked();                          // Пользователь кликнул по заголовку (для показа профиля).
    void searchButtonClicked();                    // Пользователь нажал кнопку поиска в чате.
    void replyToMessageRequested(qint64 messageId); // Пользователь выбрал "Ответить" на сообщение.
    void editMessageRequested(qint64 messageId, const QString& oldText); // Пользователь выбрал "Редактировать".
    void deleteMessageRequested(qint64 messageId); // Пользователь выбрал "Удалить".
    void replyCancelled();                         // Пользователь закрыл панель ответа.

    // Приватные слоты - используются для обработки внутренних событий виджета.
private slots:
    void onSearchTriggered(const QString& text); // (Заготовка) Обработка ввода текста в поле поиска.
    void onChatContextMenuRequested(const QPoint &pos); // Вызывается по правому клику на списке сообщений.
    void onMessageDoubleClicked(const QModelIndex &index); // Вызывается по двойному клику (для быстрого ответа).
    void onChatScrolled(int value); // Вызывается при прокрутке списка сообщений.

    // Защищенные методы - переопределение виртуальных методов базового класса QWidget.
protected:
    void resizeEvent(QResizeEvent *event) override; // Вызывается при изменении размера виджета (для позиционирования кнопки "вниз").
    bool eventFilter(QObject *watched, QEvent *event) override; // Используется для перехвата нажатия Enter в поле ввода.

    // Приватные поля и методы - внутренняя реализация класса.
private:
    QString formatLastSeen(const User &user); // Форматирует строку "был в сети...".
    void setupHeaderUI(); // Инициализирует и компонует виджеты в заголовке чата.

    // Приватный метод для обновления положения кнопки "вниз" и счетчика.
    void updateScrollToBottomButton();

    // --- Указатели на элементы UI ---
    Ui::ChatViewWidget *ui; // Указатель на UI-форму, сгенерированную из .ui файла.

    // Виджеты заголовка (создаются в коде)
    QLabel* m_nameLabel;
    QLabel* m_statusLabel;
    QToolButton* m_searchButton;
    QToolButton* m_callButton;
    QToolButton* m_videoCallButton;
    QToolButton* m_moreOptionsButton;

    // Виджеты поиска (заготовки)
    QLineEdit* m_searchLineEdit;
    QToolButton* m_closeSearchButton;

    // Виджеты для UX "непрочитанных сообщений"
    QToolButton* m_scrollToBottomButton; // Кнопка "прокрутить вниз".
    QLabel* m_unreadCountLabel;         // Ярлык со счетчиком непрочитанных.
    int m_unreadMessageCount = 0;           // Внутренний счетчик.

    // Анимация для панели ответа
    QPropertyAnimation* m_replyAnimation;

};

#endif // CHATVIEWWIDGET_H
