
#ifndef CHATMESSAGEDELEGATE_H // Защита от двойного включения (Include Guard),
#define CHATMESSAGEDELEGATE_H // стандартная практика для заголовочных файлов C++.

#include <QObject>
#include <QStyledItemDelegate> // Включаем базовый класс для нашего делегата.
#include <QMap>                // Используется для хранения SVG-рендереров.
#include "structures.h"        // Включаем определение структуры ChatMessage.

// Прямое объявление (Forward Declaration) классов.
// Это позволяет нам использовать указатели на эти классы (ChatMessageModel*, QSvgRenderer*)
// без необходимости включать их полные заголовочные файлы (.h).
// Такой подход ускоряет компиляцию и уменьшает зависимости между файлами.
class ChatMessageModel;
class QSvgRenderer;

/**
 * @class ChatMessageDelegate
 * @brief Кастомный делегат для отрисовки сообщений в виде "пузырей" в QListView.
 *
 * Этот класс полностью берет на себя ответственность за визуальное представление
 * каждого элемента в списке сообщений. Он наследуется от QStyledItemDelegate,
 * чтобы иметь доступ к стилям приложения, но переопределяет методы paint() и sizeHint()
 * для создания уникального внешнего вида.
 * @author kex1tu
 */
class ChatMessageDelegate : public QStyledItemDelegate
{
public:
    /**
     * @brief Конструктор класса.
     * @param model Указатель на модель данных. Необходим для получения информации
     *              о цитируемых сообщениях (ответах), так как делегат имеет доступ
     *              только к текущему элементу (index).
     * @param parent Родительский объект (стандартно для Qt).
     */
    explicit ChatMessageDelegate(const ChatMessageModel* model, QObject *parent = nullptr);

    /**
     * @brief Основной метод отрисовки. Вызывается для каждого видимого элемента.
     * @param painter "Холст" (QPainter), на котором происходит вся отрисовка.
     * @param option Содержит информацию о состоянии элемента (выделен, наведен и т.д.)
     *               и его геометрию (прямоугольник, в котором нужно рисовать).
     * @param index Индекс элемента в модели, предоставляющий доступ к данным (ChatMessage).
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * @brief Метод для расчета размера элемента. Вызывается представлением (QListView)
     *        чтобы определить, сколько места выделить для каждого сообщения.
     * @param option Опции представления, включая доступную ширину.
     * @param index Индекс элемента для доступа к его данным (тексту, наличию цитаты).
     * @return QSize Рекомендуемый размер (ширина и высота) для этого элемента.
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    // Указатель на модель. `const`, потому что делегат не должен изменять данные,
    // он их только читает для отрисовки.
    const ChatMessageModel* m_model;

    // --- Статические члены для эффективного управления ресурсами ---

    /**
     * @brief Статическая карта для хранения предзагруженных SVG-рендереров.
     *
     * Ключ - статус сообщения (Sending, Sent, Delivered), значение - указатель на рендерер.
     * `static` означает, что эта карта будет ОДНА на все экземпляры класса ChatMessageDelegate.
     * Это критически важная оптимизация: мы загружаем и парсим SVG-файлы только один раз
     * при первом создании делегата, а не при отрисовке каждого сообщения.
     */
    static QMap<ChatMessage::MessageStatus, QSvgRenderer*> m_statusRenderers;

    /**
     * @brief Статический флаг, который предотвращает повторную инициализацию рендереров.
     */
    static bool m_renderersInitialized;

    /**
     * @brief Статический метод-инициализатор.
     *
     * Безопасно создает и кэширует рендереры в `m_statusRenderers`.
     * Вызывается в конструкторе делегата только один раз благодаря флагу `m_renderersInitialized`.
     * @param parent Родительский объект, которому будут принадлежать созданные рендереры.
     *               Это обеспечивает автоматическое управление памятью (Qt Parent-Child system).
     */
    static void initRenderers(QObject* parent);
};

#endif // CHATMESSAGEDELEGATE_H

