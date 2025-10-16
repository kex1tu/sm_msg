/**
 * @file loginwidget.cpp
 * @brief Реализация виджета для аутентификации и регистрации пользователя.
 * @see LoginWidget
 * @author kex1tu
 */

#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QMessageBox> // Для показа информационных сообщений.
#include <QAction>     // Для создания действия "показать/скрыть пароль".
#include <QIcon>       // Для установки иконок на QAction.
#include <QDebug>      // Для отладочных сообщений.

/**
 * @brief Конструктор LoginWidget.
 * @details Инициализирует UI из формы, настраивает виджеты (например, кнопку
 *          видимости пароля) и соединяет все внутренние сигналы и слоты,
 *          а также сигналы, которые будут отправлены наружу (в MainWindow).
 * @param parent Родительский виджет.
 */
LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    // Инициализация UI из сгенерированного uic файла (loginwidget.ui).
    ui->setupUi(this);

    // --- Настройка кнопки "Показать/Скрыть пароль" ---
    // 1. Создаем QAction, который будет служить "кнопкой" внутри QLineEdit.
    m_passwordVisibilityAction = new QAction(this);
    // 2. Устанавливаем начальную иконку (пароль скрыт).
    m_passwordVisibilityAction->setIcon(QIcon(":/icons/crossed_eye.png"));

    // 3. Добавляем QAction в поле ввода пароля. Он будет отображаться справа (TrailingPosition).
    ui->loginPasswordEdit->addAction(m_passwordVisibilityAction, QLineEdit::TrailingPosition);

    // 4. Устанавливаем начальный режим отображения пароля (скрыт символами).
    ui->loginPasswordEdit->setEchoMode(QLineEdit::Password);

    // --- Соединение сигналов и слотов (Connects) ---
    // Соединяем сигнал triggered() от QAction с нашим слотом для переключения видимости.
    connect(m_passwordVisibilityAction, &QAction::triggered, this, &LoginWidget::onTogglePasswordVisibilityTriggered);

    // Соединяем клики по кнопкам навигации с соответствующими слотами.
    connect(ui->goToRegisterButton, &QPushButton::clicked, this, &LoginWidget::ongoToRegisterButtonclicked);
    connect(ui->goToLoginButton, &QPushButton::clicked, this, &LoginWidget::ongoToLoginButtonclicked);

    // Соединяем клик по кнопке "Войти" с лямбда-функцией, которая собирает данные и эмитирует сигнал.
    connect(ui->loginButton, &QPushButton::clicked, this, [this](){
        qDebug() << "LoginWidget: loginButton clicked. Emitting loginRequested signal.";

        // Получаем и очищаем данные из полей ввода.
        QString username = ui->loginUsernameEdit->text().trimmed();
        QString password = ui->loginPasswordEdit->text();

        // Отправляем сигнал наружу (в MainWindow).
        emit loginRequested(username, password);
    });

    // Аналогично для кнопки "Зарегистрироваться".
    connect(ui->registerButton, &QPushButton::clicked, this, [this](){
        qDebug() << "LoginWidget: registerButton clicked. Emitting registerRequested signal.";

        QString username = ui->registerUsernameEdit->text().trimmed();
        QString displayName = ui->registerDisplayNameEdit->text().trimmed();
        QString password = ui->registerPasswordEdit->text();

        emit registerRequested(username, displayName, password);
    });
}

/**
 * @brief Деструктор.
 */
LoginWidget::~LoginWidget()
{
    delete ui;
}

/**
 * @brief Слот, переключающий видимость пароля в поле ввода.
 * @details Вызывается при клике на иконку "глаза" (m_passwordVisibilityAction).
 */
void LoginWidget::onTogglePasswordVisibilityTriggered()
{
    // Проверяем текущий режим отображения.
    if (ui->loginPasswordEdit->echoMode() == QLineEdit::Password) {
        // Если пароль был скрыт -> показываем его.
        ui->loginPasswordEdit->setEchoMode(QLineEdit::Normal);
        // Меняем иконку на "открытый глаз".
        m_passwordVisibilityAction->setIcon(QIcon(":/icons/eye.png"));
    } else {
        // Если пароль был виден -> скрываем его.
        ui->loginPasswordEdit->setEchoMode(QLineEdit::Password);
        // Меняем иконку обратно на "закрытый глаз".
        m_passwordVisibilityAction->setIcon(QIcon(":/icons/crossed_eye.png"));
    }
}

/**
 * @brief Возвращает имя пользователя, введенное на форме входа.
 * @return QString Имя пользователя.
 */
QString LoginWidget::username() const
{
    return ui->loginUsernameEdit->text().trimmed();
}

/**
 * @brief Слот, вызываемый после успешной регистрации.
 * @details Показывает пользователю сообщение и переключает на форму входа.
 */
void LoginWidget::onRegistrationSuccess()
{
    QMessageBox::information(this, "Регистрация успешна", "Теперь вы можете войти, используя свои данные.");

    // Очищаем поля формы регистрации.
    ui->registerUsernameEdit->clear();
    ui->registerDisplayNameEdit->clear();
    ui->registerPasswordEdit->clear();

    // Переключаем QStackedWidget на страницу входа (индекс 1).
    ui->stackedWidget->setCurrentIndex(1);
}

/**
 * @brief Переключает виджет на страницу регистрации.
 */
void LoginWidget::ongoToRegisterButtonclicked()
{
    ui->stackedWidget->setCurrentIndex(0); // Индекс страницы регистрации.
}

/**
 * @brief Переключает виджет на страницу входа.
 */
void LoginWidget::ongoToLoginButtonclicked()
{
    ui->stackedWidget->setCurrentIndex(1); // Индекс страницы входа.
}

/**
 * @brief Очищает все поля ввода на обеих формах.
 */
void LoginWidget::clearFields()
{
    ui->loginUsernameEdit->clear();
    ui->loginPasswordEdit->clear();
    ui->registerUsernameEdit->clear();
    ui->registerPasswordEdit->clear();
    ui->registerDisplayNameEdit->clear();
}

/**
 * @brief Включает или отключает все интерактивные элементы на виджете.
 * @details Используется для блокировки UI, например, во время ожидания ответа от сервера
 *          или при потере соединения.
 * @param enabled `true` чтобы включить элементы, `false` чтобы выключить.
 */
void LoginWidget::setUiEnabled(bool enabled)
{
    // Просто устанавливаем свойство `enabled` для каждого элемента управления.
    ui->loginUsernameEdit->setEnabled(enabled);
    ui->loginPasswordEdit->setEnabled(enabled);
    ui->loginButton->setEnabled(enabled);
    ui->goToRegisterButton->setEnabled(enabled);
    ui->registerUsernameEdit->setEnabled(enabled);
    ui->registerPasswordEdit->setEnabled(enabled);
    ui->registerDisplayNameEdit->setEnabled(enabled);
    ui->registerButton->setEnabled(enabled);
}
