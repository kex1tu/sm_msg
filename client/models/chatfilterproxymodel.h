#ifndef CHATFILTERPROXYMODEL_H
#define CHATFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QRegularExpressionMatch>

class ChatFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    /**
 * @brief Конструктор ChatFilterProxyModel — стандартный для QSortFilterProxyModel, используется для поиска по истории.
 * @param parent Родительский QObject
 */
    explicit ChatFilterProxyModel(QObject *parent = nullptr);

protected:
    /**
 * @brief Принимает ли строка чата (сообщение) в результат поиска — ищет по payload через QRegularExpression.
 *
 * - Достает ChatMessage из модели по source_row.
 * - Проверяет валидность индекса.
 * - Берёт текущий фильтр (filterRegularExpression).
 * - Если фильтр пустой/невалидный — возвращает все строки (нет фильтрации).
 * - Если валидный — принимает только те строки, где payload совпадает с регуляркой.
 *
 * @param source_row Строка в исходной модели
 * @param source_parent Родительский index
 * @return true — строка подходит, false — фильтруется
 */
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif
