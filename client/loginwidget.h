#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include "structures.h" // Хотя этот include здесь напрямую не используется,
// он может быть полезен для будущего расширения
// (например, если нужно будет работать со структурой User).
#include <QWidget>      // Включаем базовый класс для всех виджетов Qt.

// Прямое объявление (Forward Declaration).
// Мы используем указатель на QAction, поэтому полного определения класса не требуется.
class QAction;

// Пространство имен, генерируемое Qt UI Compiler (uic) из loginwidget.ui.
namespace Ui {
class LoginWidget;
}
/**
 * @class LoginWidget
 * @brief Виджет, отвечающий за интерфейс аутентификации и регистрации.
 *
 * Этот класс инкапсулирует всю логику, связанную с экраном входа:
 * - Отображение полей для ввода данных.
 * - Переключение между формами входа и регистрации.
 * - Сбор данных из полей и их передача в MainWindow через сигналы.
 *
 * Класс спроектирован так, чтобы быть полностью независимым от сетевой логики
 * и бизнес-логики приложения. Он только генерирует события (сигналы).
 * @author kex1tu
 */
class LoginWidget : public QWidget
{
    Q_OBJECT // Обязательный макрос для классов, использующих сигналы и слоты.

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit LoginWidget(QWidget *parent = nullptr);

    /**
     * @brief Геттер для получения имени пользователя из поля ввода.
     * @return QString Имя пользователя, введенное на форме входа.
     */
    QString username() const;

    /**
     * @brief Деструктор.
     */
    ~LoginWidget();

    // Публичные слоты - это часть API, которую могут вызывать другие классы.
public slots:
    /**
     * @brief Включает или отключает все интерактивные элементы на виджете.
     *        Вызывается, например, при потере соединения с сервером,
     *        чтобы пользователь не мог пытаться войти.
     * @param enabled true - включить, false - выключить.
     */
    void setUiEnabled(bool enabled);

    /**
     * @brief Очищает все поля ввода на обеих формах (вход и регистрация).
     */
    void clearFields();

    /**
     * @brief Слот, вызываемый после успешной регистрации.
     *        Показывает информационное сообщение и переключает на форму входа.
     */
    void onRegistrationSuccess();

    // Сигналы - способ, которым виджет сообщает внешнему миру (MainWindow) о действиях пользователя.
signals:
    /**
     * @brief Сигнал испускается, когда пользователь нажимает кнопку "Войти".
     * @param username Введенное имя пользователя.
     * @param password Введенный пароль.
     */
    void loginRequested(const QString& username, const QString& password);

    /**
     * @brief Сигнал испускается, когда пользователь нажимает кнопку "Зарегистрироваться".
     * @param username Введенное имя пользователя.
     * @param displayName Введенное отображаемое имя.
     * @param password Введенный пароль.
     */
    void registerRequested(const QString& username, const QString& displayName, const QString& password);

private:
    // Указатель на UI-форму, сгенерированную из .ui файла.
    // Позволяет получить доступ к виджетам, размещенным в Qt Designer.
    Ui::LoginWidget *ui;

    // Указатель на действие (QAction), которое представляет собой иконку "глаза"
    // в поле ввода пароля для переключения его видимости.
    QAction* m_passwordVisibilityAction;

    // Приватные слоты - используются для обработки внутренних событий виджета.
private slots:
    /**
     * @brief Переключает QStackedWidget на страницу регистрации.
     */
    void ongoToRegisterButtonclicked();

    /**
     * @brief Переключает QStackedWidget на страницу входа.
     */
    void ongoToLoginButtonclicked();

    /**
     * @brief Слот, который вызывается при клике на иконку "глаза".
     *        Переключает режим отображения пароля (QLineEdit::Password / QLineEdit::Normal).
     */
    void onTogglePasswordVisibilityTriggered();
};

#endif // LOGINWIDGET_H
