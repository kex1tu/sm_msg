 #include "profileviewwidget.h"
#include "chatviewwidget.h"
#include "ui_profileviewwidget.h"
#include <QJsonObject>
#include <QPixmap>



ProfileViewWidget::ProfileViewWidget(NetworkService* netService, QWidget* parent)
    : QWidget(parent), ui(new Ui::ProfileViewWidget), m_netService(netService)
{
    ui->setupUi(this);
    m_isMyProfile = false;
    m_isEditing = false;
    m_currentUser = User();

    // Кнопки навигации и редактирования профиля
    connect(ui->backButton, &QToolButton::clicked, this, &ProfileViewWidget::backButtonClicked);
    connect(ui->editProfileButton, &QToolButton::clicked, this, &ProfileViewWidget::onEditButtonClicked);
}


ProfileViewWidget::~ProfileViewWidget()
{
    delete ui;
}


void ProfileViewWidget::setUserProfile(const User& user, bool isMyProfile)
{
    m_isEditing = false;
    m_isMyProfile = isMyProfile;
    m_currentUser = user;

    ui->editProfileButton->setText("edit");
    ui->backButton->setIcon(QIcon(":/icons/backArrow"));

    ui->displayNameLabel->setText(user.displayName);
    ui->usernameLabel->setText("@" + user.username);
    ui->echoUsername->setText("Имя пользователя");
    ui->aboutLabel->setText(user.statusMessage.isEmpty()? "ЕЩЁ ничего нет" : user.statusMessage);
    ui->echoAbout->setText("О себе");

    QString statusText = user.isOnline ? "Online" : formatLastSeen(user);
    ui->lastSeenLabel->setText(statusText);
    ui->avatarLabel->setText("Нет\nаватара");

    ui->aboutLabel->setReadOnly(true);
    ui->usernameLabel->setReadOnly(true);
    ui->displayNameLabel->setReadOnly(true);

    ui->editProfileButton->setVisible(m_isMyProfile);
    ui->blockContactButton->setVisible(!m_isMyProfile);
    ui->deleteChatButton->setVisible(!m_isMyProfile);
    ui->editContactButton->setVisible(!m_isMyProfile);
}


void ProfileViewWidget::reset(){
    m_isEditing = false;
    m_isMyProfile = false;
    m_currentUser = User();

    ui->editProfileButton->setText("edit");
    ui->backButton->setIcon(QIcon(":/icons/backArrow"));
    ui->displayNameLabel->setText(m_currentUser.displayName);
    ui->usernameLabel->setText("@" + m_currentUser.username);
    ui->echoUsername->setText("Имя пользователя");
    ui->aboutLabel->setText(m_currentUser.statusMessage.isEmpty()? "ЕЩЁ ничего нет" : m_currentUser.statusMessage);
    ui->echoAbout->setText("О себе");
    QString statusText = m_currentUser.isOnline ? "Online" : formatLastSeen(m_currentUser);
    ui->lastSeenLabel->setText(statusText);
    ui->avatarLabel->setText("Нет\nаватара");

    ui->aboutLabel->setReadOnly(true);
    ui->usernameLabel->setReadOnly(true);
    ui->displayNameLabel->setReadOnly(true);

    ui->editProfileButton->setVisible(m_isMyProfile);
    ui->blockContactButton->setVisible(!m_isMyProfile);
    ui->deleteChatButton->setVisible(!m_isMyProfile);
    ui->editContactButton->setVisible(!m_isMyProfile);
}

void ProfileViewWidget::onEditButtonClicked() {
    if(m_isEditing == false){
        // Включаем режим редактирования: активируем поля и меняем текст кнопки
        m_isEditing = true;
        ui->editProfileButton->setVisible(true);
        ui->editProfileButton->setText("Save");

        if(m_isMyProfile){
            ui->aboutLabel->setReadOnly(false);
        }
        ui->displayNameLabel->setReadOnly(false);
        return;
    }

    // Здесь режим "Save": проверяем, были ли изменения, выключаем режим
    m_isEditing = false;
    ui->editProfileButton->setVisible(true);
    ui->editProfileButton->setText("Edit");
    ui->aboutLabel->setReadOnly(true);
    ui->displayNameLabel->setReadOnly(true);

    if(m_isMyProfile == false){
        return;
    }

    QString newDisplayName = ui->displayNameLabel->text();
    QString newAbout       = ui->aboutLabel->text();

    bool changed = false;
    User updated = m_currentUser;

    // Проверка на изменения
    if (newDisplayName != m_currentUser.displayName) {
        updated.displayName = newDisplayName;
        changed = true;
    }
    if (newAbout != m_currentUser.statusMessage) {
        updated.statusMessage = newAbout;
        changed = true;
    }
    if (!changed)
        return;

    // Формируем JSON-запрос для NetworkService
    QJsonObject req;
    req["type"]           = "update_profile";
    req["username"]       = updated.username;
    req["display_name"]   = updated.displayName;
    req["status_message"] = updated.statusMessage;
    req["avatar_url"]     = updated.avatarUrl;
    m_netService->sendJson(req);
}
