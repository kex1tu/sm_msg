#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H
#include "structures.h"
#include <QWidget>

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    QString username() const;
    ~LoginWidget();
public slots:
    void setUiEnabled(bool enabled);
    void clearFields();
    void onRegistrationSuccess();
signals:
    void loginRequested(const QString& username, const QString& password);
    void registerRequested(const QString& username, const QString& displayName, const QString& password);

private:
    Ui::LoginWidget *ui;
private slots:
    void ongoToRegisterButtonclicked();
    void ongoToLoginButtonclicked();

};

#endif // LOGINWIDGET_H
