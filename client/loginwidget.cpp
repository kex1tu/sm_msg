#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QMessageBox>
#include <QAction>
#include <QIcon>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);


    // --- НАСТРОЙКА КНОПКИ "ПОКАЗАТЬ ПАРОЛЬ" ---

    // 1. Создаем QAction
    m_passwordVisibilityAction = new QAction(this);
    // Изначально пароль скрыт, поэтому ставим иконку "закрытого глаза"
    m_passwordVisibilityAction->setIcon(QIcon(":/icons/crossed_eye.png"));

    // 2. Добавляем QAction в поле ввода пароля справа
    ui->loginPasswordEdit->addAction(m_passwordVisibilityAction, QLineEdit::TrailingPosition);

    // 3. Устанавливаем начальный режим отображения пароля
    ui->loginPasswordEdit->setEchoMode(QLineEdit::Password);

    // 4. Подключаем сигнал от QAction к нашему слоту
    connect(m_passwordVisibilityAction, &QAction::triggered, this, &LoginWidget::on_togglePasswordVisibility_triggered);

    connect(ui->goToRegisterButton, &QPushButton::clicked, this, &LoginWidget::ongoToRegisterButtonclicked);
    connect(ui->goToLoginButton, &QPushButton::clicked, this, &LoginWidget::ongoToLoginButtonclicked);
    connect(ui->loginButton, &QPushButton::clicked, this, [this](){
        qDebug() << "LoginWidget: loginButton clicked. Emitting loginRequested signal.";

        QString username = ui->loginUsernameEdit->text().trimmed();
        QString password = ui->loginPasswordEdit->text();

        emit loginRequested(username, password);
    });

    connect(ui->registerButton, &QPushButton::clicked, this, [this](){
        qDebug() << "LoginWidget: registerButton clicked. Emitting registerRequested signal.";

        QString username = ui->registerUsernameEdit->text().trimmed();
        QString displayName = ui->registerDisplayNameEdit->text().trimmed();
        QString password = ui->registerPasswordEdit->text();

        emit registerRequested(username, displayName, password);
    });


}



void LoginWidget::on_togglePasswordVisibility_triggered()
{
    // Проверяем текущий режим отображения
    if (ui->loginPasswordEdit->echoMode() == QLineEdit::Password) {
        // Если пароль был скрыт -> показываем его
        ui->loginPasswordEdit->setEchoMode(QLineEdit::Normal);
        // Меняем иконку на "открытый глаз"
        m_passwordVisibilityAction->setIcon(QIcon(":/icons/eye.png"));
    } else {
        // Если пароль был виден -> скрываем его
        ui->loginPasswordEdit->setEchoMode(QLineEdit::Password);
        // Меняем иконку на "закрытый глаз"
        m_passwordVisibilityAction->setIcon(QIcon(":/icons/crossed_eye.png"));
    }
}

QString LoginWidget::username() const
{
    return ui->loginUsernameEdit->text().trimmed();
}
void LoginWidget::onRegistrationSuccess()
{
    QMessageBox::information(this, "Регистрация успешна", "Теперь вы можете войти, используя свои данные.");

    ui->registerUsernameEdit->clear();
    ui->registerDisplayNameEdit->clear();
    ui->registerPasswordEdit->clear();

    ui->stackedWidget->setCurrentIndex(1);
}

void LoginWidget::ongoToRegisterButtonclicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void LoginWidget::ongoToLoginButtonclicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void LoginWidget::clearFields()
{
    ui->loginUsernameEdit->clear();
    ui->loginPasswordEdit->clear();
    ui->registerUsernameEdit->clear();
    ui->registerPasswordEdit->clear();
    ui->registerDisplayNameEdit->clear();
}

void LoginWidget::setUiEnabled(bool enabled)
{
    ui->loginUsernameEdit->setEnabled(enabled);
    ui->loginPasswordEdit->setEnabled(enabled);
    ui->loginButton->setEnabled(enabled);
    ui->goToRegisterButton->setEnabled(enabled);
    ui->registerUsernameEdit->setEnabled(enabled);
    ui->registerPasswordEdit->setEnabled(enabled);
    ui->registerDisplayNameEdit->setEnabled(enabled);
    ui->registerButton->setEnabled(enabled);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}
