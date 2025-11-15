#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include "structures.h"
#include <QWidget>       

 
 
class QAction;

 
namespace Ui {
class LoginWidget;
}
 
class LoginWidget : public QWidget
{
    Q_OBJECT  

public:
    /**
 * @brief Конструктор LoginWidget — инициализирует UI, экшены, сигналы/слоты.
 * @param parent Родительский QWidget
 */
    explicit LoginWidget(QWidget *parent = nullptr);
    /**
 * @brief Получает username из поля ввода. Используется для авто-ввода/логина.
 */
    QString username() const;
    /**
 * @brief Деструктор LoginWidget — удаляет ui и логирует.
 */
    ~LoginWidget();

public slots:
    /**
 * @brief Активирует или блокирует доступность всех полей и кнопок.
 * @param enabled true — поля доступны, false — заблокированы
 */
    void setUiEnabled(bool enabled);
    /**
 * @brief Очищает все поля формы регистрации и входа.
 */
    void clearFields();
    /**
 * @brief Слот для успешной регистрации — показывает messagebox, очищает поля, переключает на экран входа.
 */
    void onRegistrationSuccess();   
signals:
    /**
 * @brief Сигнал авторизации пользователя (логина).
 * @param username Имя пользователя
 * @param password Пароль (не шифрованный — UI layer)
 */
    void loginRequested(const QString& username, const QString& password);

    /**
 * @brief Сигнал регистрации нового пользователя.
 * @param username Имя пользователя (логин)
 * @param displayName Отображаемое имя
 * @param password Пароль пользователя
 */
    void registerRequested(const QString& username, const QString& displayName, const QString& password);

private:
    /**
 * @brief Автогенерируемый UI-класс для LoginWidget (Qt Designer).
 */
    Ui::LoginWidget *ui;

    /**
 * @brief Экшен для переключения видимости пароля (глазик/перечёркнутый глазик).
 */
    QAction* m_passwordVisibilityAction;

private slots:
    /**
 * @brief Переход к форме регистрации.
 */
    void ongoToRegisterButtonclicked();
    /**
 * @brief Переход к форме входа.
 */
    void ongoToLoginButtonclicked();
    /**
 * @brief Инвертирует режим видимости пароля и меняет иконку.
 */
    void onTogglePasswordVisibilityTriggered();
};

#endif  
