#ifndef SEARCHRESULTSPOPUP_H
#define SEARCHRESULTSPOPUP_H

#include <QWidget>
#include <QListWidget>
#include <QJsonArray>

class SearchResultsPopup : public QWidget
{
    Q_OBJECT
public:
    explicit SearchResultsPopup(QWidget *parent = nullptr);
    void showResults(const QJsonArray &users);

private:
    QListWidget *m_listWidget;

signals:
    void userSelected(const QString& username);
};
#endif // SEARCHRESULTSPOPUP_H
