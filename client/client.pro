QT += core gui network
QT += network sql svg


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatfilterproxymodel.cpp \
    chatmessagedelegate.cpp \
    chatmessagemodel.cpp \
    chatviewwidget.cpp \
    contactlistdelegate.cpp \
    contactlistmodel.cpp \
    loginwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    searchresultspopup.cpp \
    smoothlistview.cpp \
    smoothtextedit.cpp

HEADERS += \
    chatfilterproxymodel.h \
    chatmessagedelegate.h \
    chatmessagemodel.h \
    chatviewwidget.h \
    contactlistdelegate.h \
    contactlistmodel.h \
    loginwidget.h \
    mainwindow.h \
    searchresultspopup.h \
    smoothlistview.h \
    smoothtextedit.h \
    structures.h

FORMS += \
    chatviewwidget.ui \
    loginwidget.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    styles.css \
    styles.css

RESOURCES += \
    resources.qrc
