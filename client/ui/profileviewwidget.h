 

#ifndef PROFILEVIEWWIDGET_H
#define PROFILEVIEWWIDGET_H

#include <QWidget>
#include "structures.h"  
#include "dataservice.h"
#include "networkservice.h"
namespace Ui {
class ProfileViewWidget;
}

 
class ProfileViewWidget : public QWidget
{
    Q_OBJECT

public:
    /**
 * @brief Конструктор: инициализация UI, состояния, подключение кнопок.
 * @param netService Сетевой сервис для отправки команд
 * @param parent Родительский QWidget
 */
    explicit ProfileViewWidget(NetworkService* netService, QWidget* parent = nullptr);
    /**
 * @brief Сбросить состояние виджета до пустого профиля (обычно при переходах).
 */
    void reset();
    /**
 * @brief Деструктор: удаляет UI-компоненты.
 */
    ~ProfileViewWidget();
    /**
 * @brief Флаг: режим редактирования профиля (true — в режиме "Edit"/"Save").
 */
    bool m_isEditing;

    /**
 * @brief Флаг: профиль текущего аккаунта (true — это мой личный профиль).
 */
    bool m_isMyProfile;
public slots:
    /**
 * @brief Устанавливает профиль пользователя в виджет, формирует отображение и доступность элементов.
 * @param user Отображаемый пользователь
 * @param isMyProfile True — это мой профиль (разрешить редактирование)
 */
    void setUserProfile(const User& user, bool isMyProfile = false);
    /**
 * @brief Слот обработки клика по кнопке "Edit"/"Save" в профиле.
 *
 * — Вход в режим редактирования: поля становятся доступны для ввода, текст меняется на "Save".
 * — Выход из режима: если что-то изменилось — отправляется запрос обновления профиля через сеть.
 *
 * @details
 * - Если не редактировали: кнопка становится "Save", снимаются readOnly. Только своё about доступно для редактирования.
 * - Если были изменения: отправляется JSON-объект вида {"type":"update_profile", ...}.
 * - Если изменений нет, ничего не отправляется.
 */
    void onEditButtonClicked();

signals:
    /**
 * @brief Сигнал при нажатии кнопки "назад" в профиле (например, выйти из просмотра профиля).
 */
    void backButtonClicked();

private:

    /**
 * @brief Автогенерируемый UI-класс для ProfileViewWidget (Qt Designer).
 */
    Ui::ProfileViewWidget *ui;

    /**
 * @brief Текущий пользователь для отображения/редактирования в этом профиле.
 */
    User m_currentUser;

    /**
 * @brief Сетевой сервис для отправки запросов о профиле (обновление, загрузка и т.д.).
 */
    NetworkService* m_netService;


};

#endif  
