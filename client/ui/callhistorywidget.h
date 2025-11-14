#ifndef CALLHISTORYWIDGET_H
#define CALLHISTORYWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QJsonArray>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "structures.h"

class CallHistoryWidget : public QWidget {
    Q_OBJECT
public:
    /**
 * @brief Конструктор виджета истории звонков.
 * @param parent Родительский QWidget
 */
    explicit CallHistoryWidget(QWidget* parent = nullptr);
    /**
 * @brief Деструктор — освобождает ресурсы виджета (по умолчанию ничего дополнительного).
 */
    ~CallHistoryWidget();
    /**
 * @brief Загружает в виджет новую историю звонков из JSON-массива. Очищает старое, парсит новые элементы, оформляет статистику.
 *
 * - Очищает текущую историю и виджет.
 * - Парсит каждый элемент QJsonArray в CallItem, добавляет в m_calls.
 * - Для каждого звонка создает QListWidgetItem, заполняет текст и user data.
 * - Формирует статус загрузки и статистику по завершённым, пропущенным и отклонённым.
 * - Отображает среднюю длительность завершённых вызовов.
 * - Логгирует общий размер загруженной истории.
 *
 * @param calls QJsonArray — массив объектов истории звонков
 */
    void setCallHistory(const QJsonArray& calls);
    /**
 * @brief Переводит виджет в режим загрузки: блокирует refresh и показывает текст загрузки.
 * @param loading true — показывать индикатор, false — снимаем (делаем refresh доступным)
 */
    void showLoading(bool loading);
    /**
 * @brief Отображает ошибку в статусе (выделение) и пишет в debug-вывод.
 * @param errorMsg Текст ошибки для пользователя
 */
    void showError(const QString& errorMsg);
    /**
 * @brief Полностью очищает историю вызовов и список отображения.
 */
    void clearHistory();
signals:
    /**
 * @brief Сигнал запроса обновления истории звонков из внешнего источника (например, от MainWindow/CallService).
 */
    void refreshRequested();

    /**
 * @brief Сигнал выбора конкретного звонка пользователем в списке (двойной клик).
 * @param call Объект выбраного звонка (CallItem)
 */
    void callSelected(const CallItem& call);
private slots:
    /**
 * @brief Слот для обработки нажатия кнопки Refresh: отображает статус загрузки и инициирует сигнал обновления.
 */
    void onRefreshClicked();
    /**
 * @brief Слот для обработки двойного клика по элементу истории: выбирает звонок, эмитит signal callSelected.
 * @param item Указатель на элемент списка
 */
    void onItemDoubleClicked(QListWidgetItem* item);
    /**
 * @brief Слот для обработки одиночного клика по элементу списка: выводит подробности в debug-вывод.
 * @param item Указатель на элемент списка
 */
    void onItemClicked(QListWidgetItem* item);
private:
    /**
 * @brief Инициализация UI-элементов, компоновка и подключение сигналов/слотов.
 *
 * - Главная вертикальная компоновка.
 * - Заголовок "Call History".
 * - Метка статистики (общее по звонкам).
 * - Список звонков QListWidget с настройками и подключением сигналов.
 * - Нижний блок: статус + кнопка обновления.
 */
    void setupUI();
    /**
 * @brief Форматирует CallItem в текст для списка: стрелка направления, контакт, статус, время.
 *
 * - Используется для отображения каждого звонка в QListWidget.
 * - Выводит направление (→ или ←), контакт, статус с иконкой, время (часы:минуты).
 *
 * @param item Объект истории вызова
 * @return Готовая строка для отображения в списке
 */
    QString formatCallItem(const CallItem& item) const;
    /**
 * @brief Форматирует длительность звонка из секунд в строку вида "Xm Ys"/"Xh Ym"/"Xs".
 * @param seconds Длительность вызова (в секундах)
 * @return Строка для UI: секунды, минуты:секунды, или часы:минуты
 */
    QString formatDuration(int seconds) const;
    /**
 * @brief Возвращает иконку вызова по параметрам (статус, тип). Пока заглушка.
 * @param item Объект истории звонка
 * @return QIcon (можно доработать для визуализации различий)
 */
    QIcon getCallIcon(const CallItem& item) const;
    /**
 * @brief Основной список истории звонков (визуальный компонент для вывода CallItem).
 */
    QListWidget* m_callListWidget;

    /**
 * @brief Кнопка для запуска обновления/загрузки истории звонков.
 */
    QPushButton* m_refreshBtn;

    /**
 * @brief Метка статуса: показывает ошибки, процесс загрузки или готовность интерфейса.
 */
    QLabel* m_statusLabel;

    /**
 * @brief Метка статистики: отображает краткую информацию по звонкам (всего, средняя длительность и т.д.).
 */
    QLabel* m_statsLabel;

    /**
 * @brief Список объектов истории вызовов, формирующий содержимое для вывода и выборки.
 */
    QList<CallItem> m_calls;
};

#endif
