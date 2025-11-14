/********************************************************************************
** Form generated from reading UI file 'chatviewwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHATVIEWWIDGET_H
#define UI_CHATVIEWWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <smoothlistview.h>
#include <smoothtextedit.h>

QT_BEGIN_NAMESPACE

class Ui_ChatViewWidget
{
public:
    QVBoxLayout *verticalLayout_2;
    QWidget *headerWidget;
    QHBoxLayout *headerLayout;
    SmoothListView *chatHistoryView;
    QWidget *messageInputWidget;
    QGridLayout *gridLayout;
    SmoothTextEdit *messageTextEdit;
    QPushButton *sendButton;
    QPushButton *attachButton;
    QWidget *replyWidget;
    QHBoxLayout *horizontalLayout;
    QFrame *replyColorBar;
    QVBoxLayout *verticalLayout_4;
    QLabel *replyNameLabel;
    QLabel *replyTextLabel;
    QToolButton *closeReplyButton;

    void setupUi(QWidget *ChatViewWidget)
    {
        if (ChatViewWidget->objectName().isEmpty())
            ChatViewWidget->setObjectName("ChatViewWidget");
        ChatViewWidget->resize(830, 585);
        verticalLayout_2 = new QVBoxLayout(ChatViewWidget);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        headerWidget = new QWidget(ChatViewWidget);
        headerWidget->setObjectName("headerWidget");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(60);
        sizePolicy.setHeightForWidth(headerWidget->sizePolicy().hasHeightForWidth());
        headerWidget->setSizePolicy(sizePolicy);
        headerLayout = new QHBoxLayout(headerWidget);
        headerLayout->setObjectName("headerLayout");

        verticalLayout_2->addWidget(headerWidget);

        chatHistoryView = new SmoothListView(ChatViewWidget);
        chatHistoryView->setObjectName("chatHistoryView");

        verticalLayout_2->addWidget(chatHistoryView);

        messageInputWidget = new QWidget(ChatViewWidget);
        messageInputWidget->setObjectName("messageInputWidget");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(messageInputWidget->sizePolicy().hasHeightForWidth());
        messageInputWidget->setSizePolicy(sizePolicy1);
        messageInputWidget->setMaximumSize(QSize(16777215, 16777215));
        gridLayout = new QGridLayout(messageInputWidget);
        gridLayout->setSpacing(5);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(5, 5, 5, 5);
        messageTextEdit = new SmoothTextEdit(messageInputWidget);
        messageTextEdit->setObjectName("messageTextEdit");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::MinimumExpanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(messageTextEdit->sizePolicy().hasHeightForWidth());
        messageTextEdit->setSizePolicy(sizePolicy2);
        messageTextEdit->setMinimumSize(QSize(0, 40));
        messageTextEdit->setMaximumSize(QSize(16777215, 150));

        gridLayout->addWidget(messageTextEdit, 3, 1, 1, 1);

        sendButton = new QPushButton(messageInputWidget);
        sendButton->setObjectName("sendButton");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(sendButton->sizePolicy().hasHeightForWidth());
        sendButton->setSizePolicy(sizePolicy3);
        sendButton->setMinimumSize(QSize(0, 40));

        gridLayout->addWidget(sendButton, 3, 2, 1, 1, Qt::AlignmentFlag::AlignBottom);

        attachButton = new QPushButton(messageInputWidget);
        attachButton->setObjectName("attachButton");
        attachButton->setMinimumSize(QSize(0, 40));

        gridLayout->addWidget(attachButton, 3, 0, 1, 1, Qt::AlignmentFlag::AlignBottom);

        replyWidget = new QWidget(messageInputWidget);
        replyWidget->setObjectName("replyWidget");
        QSizePolicy sizePolicy4(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(replyWidget->sizePolicy().hasHeightForWidth());
        replyWidget->setSizePolicy(sizePolicy4);
        replyWidget->setMaximumSize(QSize(16777215, 16777215));
        horizontalLayout = new QHBoxLayout(replyWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(5, 5, 5, 5);
        replyColorBar = new QFrame(replyWidget);
        replyColorBar->setObjectName("replyColorBar");
        replyColorBar->setFrameShape(QFrame::Shape::VLine);
        replyColorBar->setFrameShadow(QFrame::Shadow::Sunken);

        horizontalLayout->addWidget(replyColorBar);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName("verticalLayout_4");
        replyNameLabel = new QLabel(replyWidget);
        replyNameLabel->setObjectName("replyNameLabel");

        verticalLayout_4->addWidget(replyNameLabel);

        replyTextLabel = new QLabel(replyWidget);
        replyTextLabel->setObjectName("replyTextLabel");

        verticalLayout_4->addWidget(replyTextLabel);


        horizontalLayout->addLayout(verticalLayout_4);

        closeReplyButton = new QToolButton(replyWidget);
        closeReplyButton->setObjectName("closeReplyButton");
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/cross.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        closeReplyButton->setIcon(icon);

        horizontalLayout->addWidget(closeReplyButton);


        gridLayout->addWidget(replyWidget, 1, 0, 1, 3);


        verticalLayout_2->addWidget(messageInputWidget);

        verticalLayout_2->setStretch(1, 1);

        retranslateUi(ChatViewWidget);

        QMetaObject::connectSlotsByName(ChatViewWidget);
    } // setupUi

    void retranslateUi(QWidget *ChatViewWidget)
    {
        ChatViewWidget->setWindowTitle(QCoreApplication::translate("ChatViewWidget", "Form", nullptr));
        sendButton->setText(QCoreApplication::translate("ChatViewWidget", "send", nullptr));
        attachButton->setText(QCoreApplication::translate("ChatViewWidget", "add", nullptr));
        replyNameLabel->setText(QCoreApplication::translate("ChatViewWidget", "replyName", nullptr));
        replyTextLabel->setText(QCoreApplication::translate("ChatViewWidget", "replyText", nullptr));
        closeReplyButton->setText(QCoreApplication::translate("ChatViewWidget", "...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChatViewWidget: public Ui_ChatViewWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHATVIEWWIDGET_H
