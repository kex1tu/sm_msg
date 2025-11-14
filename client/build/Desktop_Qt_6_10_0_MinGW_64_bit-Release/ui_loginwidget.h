/********************************************************************************
** Form generated from reading UI file 'loginwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINWIDGET_H
#define UI_LOGINWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginWidget
{
public:
    QGridLayout *gridLayout_3;
    QStackedWidget *stackedWidget;
    QWidget *RegisterPage;
    QGridLayout *gridLayout;
    QSpacerItem *verticalSpacer_4;
    QSpacerItem *horizontalSpacer_3;
    QVBoxLayout *verticalLayout_6;
    QHBoxLayout *horizontalLayout_16;
    QLabel *RegDisplayNameLabel;
    QLineEdit *registerDisplayNameEdit;
    QHBoxLayout *horizontalLayout_20;
    QLabel *RegUsernameLabel;
    QLineEdit *registerUsernameEdit;
    QHBoxLayout *horizontalLayout_17;
    QLabel *RegPasswordLabel;
    QLineEdit *registerPasswordEdit;
    QHBoxLayout *horizontalLayout_18;
    QPushButton *registerButton;
    QPushButton *goToLoginButton;
    QSpacerItem *verticalSpacer_3;
    QSpacerItem *horizontalSpacer_4;
    QWidget *loginPage;
    QGridLayout *gridLayout_2;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *verticalSpacer_2;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *verticalSpacer;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_7;
    QLabel *loginUsernameLabel_3;
    QLineEdit *loginUsernameEdit;
    QHBoxLayout *horizontalLayout_8;
    QLabel *loginPasswordLabel_3;
    QLineEdit *loginPasswordEdit;
    QHBoxLayout *horizontalLayout_9;
    QPushButton *loginButton;
    QPushButton *goToRegisterButton;

    void setupUi(QWidget *LoginWidget)
    {
        if (LoginWidget->objectName().isEmpty())
            LoginWidget->setObjectName("LoginWidget");
        LoginWidget->resize(910, 718);
        gridLayout_3 = new QGridLayout(LoginWidget);
        gridLayout_3->setObjectName("gridLayout_3");
        stackedWidget = new QStackedWidget(LoginWidget);
        stackedWidget->setObjectName("stackedWidget");
        RegisterPage = new QWidget();
        RegisterPage->setObjectName("RegisterPage");
        gridLayout = new QGridLayout(RegisterPage);
        gridLayout->setObjectName("gridLayout");
        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout->addItem(verticalSpacer_4, 0, 1, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 1, 0, 1, 1);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName("verticalLayout_6");
        verticalLayout_6->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName("horizontalLayout_16");
        RegDisplayNameLabel = new QLabel(RegisterPage);
        RegDisplayNameLabel->setObjectName("RegDisplayNameLabel");

        horizontalLayout_16->addWidget(RegDisplayNameLabel);

        registerDisplayNameEdit = new QLineEdit(RegisterPage);
        registerDisplayNameEdit->setObjectName("registerDisplayNameEdit");

        horizontalLayout_16->addWidget(registerDisplayNameEdit);


        verticalLayout_6->addLayout(horizontalLayout_16);

        horizontalLayout_20 = new QHBoxLayout();
        horizontalLayout_20->setObjectName("horizontalLayout_20");
        RegUsernameLabel = new QLabel(RegisterPage);
        RegUsernameLabel->setObjectName("RegUsernameLabel");

        horizontalLayout_20->addWidget(RegUsernameLabel);

        registerUsernameEdit = new QLineEdit(RegisterPage);
        registerUsernameEdit->setObjectName("registerUsernameEdit");

        horizontalLayout_20->addWidget(registerUsernameEdit);


        verticalLayout_6->addLayout(horizontalLayout_20);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName("horizontalLayout_17");
        RegPasswordLabel = new QLabel(RegisterPage);
        RegPasswordLabel->setObjectName("RegPasswordLabel");

        horizontalLayout_17->addWidget(RegPasswordLabel);

        registerPasswordEdit = new QLineEdit(RegisterPage);
        registerPasswordEdit->setObjectName("registerPasswordEdit");
        registerPasswordEdit->setEchoMode(QLineEdit::EchoMode::Password);

        horizontalLayout_17->addWidget(registerPasswordEdit);


        verticalLayout_6->addLayout(horizontalLayout_17);

        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setObjectName("horizontalLayout_18");
        registerButton = new QPushButton(RegisterPage);
        registerButton->setObjectName("registerButton");

        horizontalLayout_18->addWidget(registerButton);

        goToLoginButton = new QPushButton(RegisterPage);
        goToLoginButton->setObjectName("goToLoginButton");

        horizontalLayout_18->addWidget(goToLoginButton);


        verticalLayout_6->addLayout(horizontalLayout_18);


        gridLayout->addLayout(verticalLayout_6, 1, 1, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout->addItem(verticalSpacer_3, 2, 1, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer_4, 1, 2, 1, 1);

        stackedWidget->addWidget(RegisterPage);
        loginPage = new QWidget();
        loginPage->setObjectName("loginPage");
        gridLayout_2 = new QGridLayout(loginPage);
        gridLayout_2->setObjectName("gridLayout_2");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 2, 0, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout_2->addItem(verticalSpacer_2, 3, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 2, 3, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout_2->addItem(verticalSpacer, 0, 1, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName("horizontalLayout_7");
        loginUsernameLabel_3 = new QLabel(loginPage);
        loginUsernameLabel_3->setObjectName("loginUsernameLabel_3");
        loginUsernameLabel_3->setMaximumSize(QSize(16777215, 50));

        horizontalLayout_7->addWidget(loginUsernameLabel_3);

        loginUsernameEdit = new QLineEdit(loginPage);
        loginUsernameEdit->setObjectName("loginUsernameEdit");

        horizontalLayout_7->addWidget(loginUsernameEdit);


        verticalLayout_3->addLayout(horizontalLayout_7);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName("horizontalLayout_8");
        loginPasswordLabel_3 = new QLabel(loginPage);
        loginPasswordLabel_3->setObjectName("loginPasswordLabel_3");
        loginPasswordLabel_3->setMaximumSize(QSize(16777215, 50));

        horizontalLayout_8->addWidget(loginPasswordLabel_3);

        loginPasswordEdit = new QLineEdit(loginPage);
        loginPasswordEdit->setObjectName("loginPasswordEdit");
        loginPasswordEdit->setEchoMode(QLineEdit::EchoMode::Password);

        horizontalLayout_8->addWidget(loginPasswordEdit);


        verticalLayout_3->addLayout(horizontalLayout_8);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName("horizontalLayout_9");
        loginButton = new QPushButton(loginPage);
        loginButton->setObjectName("loginButton");

        horizontalLayout_9->addWidget(loginButton);

        goToRegisterButton = new QPushButton(loginPage);
        goToRegisterButton->setObjectName("goToRegisterButton");

        horizontalLayout_9->addWidget(goToRegisterButton);


        verticalLayout_3->addLayout(horizontalLayout_9);


        gridLayout_2->addLayout(verticalLayout_3, 2, 1, 1, 1);

        stackedWidget->addWidget(loginPage);

        gridLayout_3->addWidget(stackedWidget, 0, 0, 1, 1);


        retranslateUi(LoginWidget);

        stackedWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(LoginWidget);
    } // setupUi

    void retranslateUi(QWidget *LoginWidget)
    {
        LoginWidget->setWindowTitle(QCoreApplication::translate("LoginWidget", "Form", nullptr));
        RegDisplayNameLabel->setText(QCoreApplication::translate("LoginWidget", "DisplayName", nullptr));
        RegUsernameLabel->setText(QCoreApplication::translate("LoginWidget", "Username", nullptr));
        RegPasswordLabel->setText(QCoreApplication::translate("LoginWidget", "password", nullptr));
        registerButton->setText(QCoreApplication::translate("LoginWidget", "Reg", nullptr));
        goToLoginButton->setText(QCoreApplication::translate("LoginWidget", "Go To Log", nullptr));
        loginUsernameLabel_3->setText(QCoreApplication::translate("LoginWidget", "Username", nullptr));
        loginPasswordLabel_3->setText(QCoreApplication::translate("LoginWidget", "password", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginWidget", "Login", nullptr));
        goToRegisterButton->setText(QCoreApplication::translate("LoginWidget", "Go To Reg", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginWidget: public Ui_LoginWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINWIDGET_H
