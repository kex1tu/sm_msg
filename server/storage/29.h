/********************************************************************************
** Form generated from reading UI file 'profileviewwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROFILEVIEWWIDGET_H
#define UI_PROFILEVIEWWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ProfileViewWidget
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *headerPanel;
    QHBoxLayout *horizontalLayout;
    QToolButton *backButton;
    QSpacerItem *horizontalSpacer;
    QLabel *headerLabel;
    QToolButton *editProfileButton;
    QWidget *baseInfo;
    QHBoxLayout *horizontalLayout_2;
    QLabel *avatarLabel;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *displayNameLabel;
    QLabel *lastSeenLabel;
    QFrame *line;
    QWidget *infoWidget;
    QVBoxLayout *verticalLayout_4;
    QLineEdit *usernameLabel;
    QLabel *echoUsername;
    QLineEdit *aboutLabel;
    QLabel *echoAbout;
    QFrame *line_2;
    QSpacerItem *verticalSpacer;
    QWidget *actionsPanel;
    QVBoxLayout *verticalLayout_3;
    QPushButton *editContactButton;
    QPushButton *blockContactButton;
    QPushButton *deleteChatButton;

    void setupUi(QWidget *ProfileViewWidget)
    {
        if (ProfileViewWidget->objectName().isEmpty())
            ProfileViewWidget->setObjectName("ProfileViewWidget");
        ProfileViewWidget->resize(750, 650);
        ProfileViewWidget->setAutoFillBackground(true);
        ProfileViewWidget->setStyleSheet(QString::fromUtf8(""));
        verticalLayout = new QVBoxLayout(ProfileViewWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        headerPanel = new QWidget(ProfileViewWidget);
        headerPanel->setObjectName("headerPanel");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(headerPanel->sizePolicy().hasHeightForWidth());
        headerPanel->setSizePolicy(sizePolicy);
        horizontalLayout = new QHBoxLayout(headerPanel);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 20, 0);
        backButton = new QToolButton(headerPanel);
        backButton->setObjectName("backButton");

        horizontalLayout->addWidget(backButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        headerLabel = new QLabel(headerPanel);
        headerLabel->setObjectName("headerLabel");

        horizontalLayout->addWidget(headerLabel);

        editProfileButton = new QToolButton(headerPanel);
        editProfileButton->setObjectName("editProfileButton");

        horizontalLayout->addWidget(editProfileButton);


        verticalLayout->addWidget(headerPanel);

        baseInfo = new QWidget(ProfileViewWidget);
        baseInfo->setObjectName("baseInfo");
        horizontalLayout_2 = new QHBoxLayout(baseInfo);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(20, 0, 20, 0);
        avatarLabel = new QLabel(baseInfo);
        avatarLabel->setObjectName("avatarLabel");
        avatarLabel->setMinimumSize(QSize(80, 80));
        avatarLabel->setMaximumSize(QSize(80, 80));
        avatarLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_2->addWidget(avatarLabel);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(20, -1, 20, -1);
        displayNameLabel = new QLineEdit(baseInfo);
        displayNameLabel->setObjectName("displayNameLabel");

        verticalLayout_2->addWidget(displayNameLabel);

        lastSeenLabel = new QLabel(baseInfo);
        lastSeenLabel->setObjectName("lastSeenLabel");
        QFont font;
        font.setPointSize(10);
        lastSeenLabel->setFont(font);

        verticalLayout_2->addWidget(lastSeenLabel);


        horizontalLayout_2->addLayout(verticalLayout_2);


        verticalLayout->addWidget(baseInfo);

        line = new QFrame(ProfileViewWidget);
        line->setObjectName("line");
        line->setMinimumSize(QSize(0, 20));
        line->setMaximumSize(QSize(1000, 16777215));
        line->setFrameShadow(QFrame::Shadow::Plain);
        line->setLineWidth(20);
        line->setFrameShape(QFrame::Shape::HLine);

        verticalLayout->addWidget(line);

        infoWidget = new QWidget(ProfileViewWidget);
        infoWidget->setObjectName("infoWidget");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(infoWidget->sizePolicy().hasHeightForWidth());
        infoWidget->setSizePolicy(sizePolicy1);
        infoWidget->setFont(font);
        verticalLayout_4 = new QVBoxLayout(infoWidget);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(20, 0, 20, 0);
        usernameLabel = new QLineEdit(infoWidget);
        usernameLabel->setObjectName("usernameLabel");

        verticalLayout_4->addWidget(usernameLabel);

        echoUsername = new QLabel(infoWidget);
        echoUsername->setObjectName("echoUsername");
        echoUsername->setFont(font);

        verticalLayout_4->addWidget(echoUsername);

        aboutLabel = new QLineEdit(infoWidget);
        aboutLabel->setObjectName("aboutLabel");

        verticalLayout_4->addWidget(aboutLabel);

        echoAbout = new QLabel(infoWidget);
        echoAbout->setObjectName("echoAbout");

        verticalLayout_4->addWidget(echoAbout);


        verticalLayout->addWidget(infoWidget);

        line_2 = new QFrame(ProfileViewWidget);
        line_2->setObjectName("line_2");
        line_2->setMinimumSize(QSize(0, 20));
        line_2->setFrameShadow(QFrame::Shadow::Plain);
        line_2->setLineWidth(20);
        line_2->setMidLineWidth(0);
        line_2->setFrameShape(QFrame::Shape::HLine);

        verticalLayout->addWidget(line_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        actionsPanel = new QWidget(ProfileViewWidget);
        actionsPanel->setObjectName("actionsPanel");
        verticalLayout_3 = new QVBoxLayout(actionsPanel);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        editContactButton = new QPushButton(actionsPanel);
        editContactButton->setObjectName("editContactButton");

        verticalLayout_3->addWidget(editContactButton);

        blockContactButton = new QPushButton(actionsPanel);
        blockContactButton->setObjectName("blockContactButton");

        verticalLayout_3->addWidget(blockContactButton);

        deleteChatButton = new QPushButton(actionsPanel);
        deleteChatButton->setObjectName("deleteChatButton");

        verticalLayout_3->addWidget(deleteChatButton);


        verticalLayout->addWidget(actionsPanel);


        retranslateUi(ProfileViewWidget);

        QMetaObject::connectSlotsByName(ProfileViewWidget);
    } // setupUi

    void retranslateUi(QWidget *ProfileViewWidget)
    {
        ProfileViewWidget->setWindowTitle(QCoreApplication::translate("ProfileViewWidget", "Form", nullptr));
        backButton->setText(QString());
        headerLabel->setText(QCoreApplication::translate("ProfileViewWidget", "profile", nullptr));
        editProfileButton->setText(QCoreApplication::translate("ProfileViewWidget", "EDIT", nullptr));
        avatarLabel->setText(QCoreApplication::translate("ProfileViewWidget", "TextLabel", nullptr));
        lastSeenLabel->setText(QCoreApplication::translate("ProfileViewWidget", "TextLabel", nullptr));
        echoUsername->setText(QCoreApplication::translate("ProfileViewWidget", "TextLabel", nullptr));
        echoAbout->setText(QCoreApplication::translate("ProfileViewWidget", "TextLabel", nullptr));
        editContactButton->setText(QCoreApplication::translate("ProfileViewWidget", "\320\230\320\267\320\274\320\265\320\275\320\270\321\202\321\214 \320\272\320\276\320\275\321\202\320\260\320\272\321\202", nullptr));
        blockContactButton->setText(QCoreApplication::translate("ProfileViewWidget", "\320\227\320\260\320\261\320\273\320\276\320\272\320\270\321\200\320\276\320\262\320\260\321\202\321\214", nullptr));
        deleteChatButton->setText(QCoreApplication::translate("ProfileViewWidget", "\320\243\320\264\320\260\320\273\320\270\321\202\321\214 \321\207\320\260\321\202", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ProfileViewWidget: public Ui_ProfileViewWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROFILEVIEWWIDGET_H
