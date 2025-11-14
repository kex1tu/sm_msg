#ifndef CONTACTLISTDELEGATE_H
#define CONTACTLISTDELEGATE_H

#include <QStyledItemDelegate>

class ContactListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /**
 * @brief Конструктор ContactListDelegate — создает QStyledItemDelegate для визуального оформления списка контактов.
 * @param parent Родительский объект (обычно nullptr)
 */
    explicit ContactListDelegate(QObject *parent = nullptr);

protected:
    /**
 * @brief Рисует элемент списка контакта, поддерживает состояние selection, mouseover, online/offline, "печатает..." и badge непрочитанного.
 *
 * - Логика оформления и цветовых подсказок полностью указана внутри.
 * - Перерисовка фона по выделению/наведению.
 * - Bold для online, обычный — offline. "печатает..." выделен розовым.
 * - Elide нижней строки (короткое сообщение/статус).
 * - Badge непрочитанных сообщений отрисовывается справа.
 * - Все painter-операции обернуты .save/.restore.
 *
 * @param painter Активный QPainter
 * @param option Параметры отображения item
 * @param index Индекс строки модели (ContactListModel)
 */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    /**
 * @brief Возвращает рекомендуемый размер элемента контакта (контакт — две строки, небольшой padding).
 *
 * @param option Стиль item
 * @param index Индекс модели (не используется)
 * @return QSize элемента
 */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif
