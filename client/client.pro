QT += core gui
QT += network sql svg httpserver
QT += multimedia network
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += \
    $$PWD/core \
    $$PWD/models \
    $$PWD/delegates \
    $$PWD/ui \
    $$PWD/widgets \
    $$PWD/utils \
    $$PWD/resources

SOURCES += \
    core/callservice.cpp \
    core/databaseservice.cpp \
    main.cpp \
    mainwindow.cpp \
    core/dataservice.cpp \
    core/networkservice.cpp \
    models/chatmessagemodel.cpp \
    models/contactlistmodel.cpp \
    models/chatfilterproxymodel.cpp \
    delegates/chatmessagedelegate.cpp \
    delegates/contactlistdelegate.cpp \
    ui/callwidget.cpp \
    ui/chatviewwidget.cpp \
    ui/loginwidget.cpp \
    ui/profileviewwidget.cpp \
    ui/incomingrequestswidget.cpp \
    ui/searchresultspopup.cpp \
    ui/smoothlistview.cpp \
    ui/smoothtextedit.cpp \
    ui/callhistorywidget.cpp \
    widgets/requestitemwidget.cpp \


HEADERS += \
    core/callservice.h \
    core/databaseservice.h \
    mainwindow.h \
    core/dataservice.h \
    core/networkservice.h \
    core/structures.h \
    models/chatmessagemodel.h \
    models/contactlistmodel.h \
    models/chatfilterproxymodel.h \
    delegates/chatmessagedelegate.h \
    delegates/contactlistdelegate.h \
    ui/callwidget.h \
    ui/chatviewwidget.h \
    ui/loginwidget.h \
    ui/profileviewwidget.h \
    ui/incomingrequestswidget.h \
    ui/searchresultspopup.h \
    ui/smoothlistview.h \
    ui/smoothtextedit.h \
    ui/callhistorywidget.h \
    widgets/requestitemwidget.h \



FORMS += \
    forms/chatviewwidget.ui \
    forms/loginwidget.ui \
    forms/mainwindow.ui \
    forms/profileviewwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QMAKE_CXXFLAGS_CLAZY += -Wno-clazy-qcolor-from-literal

RESOURCES += \
    resources/resources.qrc
