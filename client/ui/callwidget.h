#ifndef CALLWIDGET_H
#define CALLWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QCloseEvent>
#include "callservice.h"

class CallWidget : public QWidget
{
    Q_OBJECT
protected:
    /**
 * @brief Обработчик закрытия окна: завершает звонок через сервис и закрывает окно.
 * @param event QCloseEvent — событие закрытия
 */
    void closeEvent(QCloseEvent *event) override;
public:
    /**
 * @brief Конструктор виджета звонка, создает панель для входящего/исходящего вызова, инициализирует UI и связи.
 * @param service Указатель на CallService для управления вызовом
 * @param parent Родительский QWidget
 *
 * - Настраивает основные лейблы, кнопки: принять, отклонить, завершить
 * - Задает стили, режим отображения always-on-top
 * - Запускает connect для всех кнопок с debug-выводом
 */
    explicit CallWidget(CallService* service, QWidget* parent = nullptr);
    /**
 * @brief Устанавливает имя звонящего в лейбл и выводит debug.
 * @param name Имя/логин собеседника
 */
    void setCallerName(const QString& name);
    /**
 * @brief Изменяет состояние звонка — статус, режим показа кнопок. Выводит debug.
 * @param state Строка состояния ("Входящий", "Ожидание", "Разговор" и т.п.)
 */
    void setCallState(const QString& state);
    /**
 * @brief Слот обновления длительности разговора в UI.
 * @param duration Строка в формате "MM:SS" или "HH:MM"
 */
    void onDurationChanged(const QString& duration);

signals:
    /**
 * @brief Сигнал, испускаемый при нажатии кнопки "Принять звонок".
 */
    void acceptClicked();

    /**
 * @brief Сигнал, испускаемый при нажатии кнопки "Отклонить звонок".
 */
    void rejectClicked();

    /**
 * @brief Сигнал, испускаемый при нажатии кнопки "Завершить (отбой) звонок".
 */
    void endCallClicked();
private:
    /**
 * @brief Сервис для управления звонками, передачи команд (accept, reject, end).
 */
    CallService* m_callService;

    /**
 * @brief Лейбл, отображающий имя звонящего или информацию о звонке.
 */
    QLabel* m_callerLabel;

    /**
 * @brief Лейбл состояния звонка ("Ожидание", "Входящий", "Разговор").
 */
    QLabel* m_stateLabel;

    /**
 * @brief Лейбл для показа длительности текущего звонка ("00:00", "05:23").
 */
    QLabel* m_durationLabel;

    /**
 * @brief Кнопка для принятия входящего звонка.
 */
    QPushButton* m_acceptBtn;

    /**
 * @brief Кнопка для отклонения входящего звонка.
 */
    QPushButton* m_rejectBtn;

    /**
 * @brief Кнопка для завершения активного звонка.
 */
    QPushButton* m_endBtn;

};

#endif  
