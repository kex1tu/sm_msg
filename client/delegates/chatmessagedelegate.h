#ifndef CHATMESSAGEDELEGATE_H
#define CHATMESSAGEDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QMap>
#include <QStaticText>
#include <QTextDocument>
#include "structures.h"

class ChatMessageModel;
class QSvgRenderer;

class ChatMessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    /**
 * @brief Конструктор делегата сообщений. Подключает модель и инициализирует SVG-рендереры статусов.
 * @param model Модель сообщений чата (read-only)
 * @param parent Родительский QObject (обычно nullptr)
 */
    explicit ChatMessageDelegate(const ChatMessageModel* model, QObject *parent = nullptr);
    /**
 * @brief Деструктор — очищает QTextDocument кеш (важно для предотвращения утечек памяти).
 */
    ~ChatMessageDelegate();
    /**
 * @brief Очищает все кеши QTextDocument (при очистке модели, смене темы, перерисовке).
 */
    void clearCaches();
    /**
 * @brief Отрисовывает одно сообщение чата, учитывая исходящий/входящий тип, наличие цитаты, статусы и оформление.
 *
 * - Включает bubble, цитаты, metaString, иконку статуса, обработку отправителя/получателя.
 * - Максимально оптимизирован по кешу QTextDocument.
 * - Поддерживает визуальные режимы Outgoing/Incoming, разные цвета иконок, корректно работает с длинными и короткими payload/meta.
 * - Дебаги отмечают ключевые изменения внешнего вида и вычисления размеров.
 *
 * @param painter Указатель на painter виджета
 * @param option Опции элемента view
 * @param index Индекс в модели, даёт ChatMessage
 */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    /**
 * @brief Рассчитывает размер message bubble для одного сообщения (разные случаи: обычное, с цитатой, с meta/status), с учётом кеша QTextDocument.
 *
 * - Использует кешируемый QTextDocument для ускорения multiple вызовов (scroll/resize).
 * - Максимально аккуратно учитывает quote, meta-информацию, ширину и padding.
 * - Применяет системный font и завязки на стилизацию.
 * - Логика полностью согласована с paint.
 *
 * @param option QStyleOptionViewItem (структура настроек/размера item)
 * @param index Индекс элемента модели (даёт ChatMessage)
 * @return Рассчитанный размер item (QSize)
 */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

public slots:
    /**
 * @brief Очищает кеш sizeHint (например, при ресайзе/изменении view).
 */
    void clearSizeHintCache();

private:/**
 * @brief Указатель на модель сообщений (ChatMessageModel), используемую делегатом для получения данных.
 */
    const ChatMessageModel* m_model;

    /**
 * @brief Мутируемый кеш рассчитанных размеров sizeHint для сообщений (по id), для ускорения list view.
 */
    mutable QMap<qint64, QSize> m_sizeHintCache;

    /**
 * @brief Глобальный кеш SVG-рендереров статусов сообщений.
 * Используется всеми экземплярами делегата для рисования иконок (отправлено, доставлено и т.д.).
 */
    static QMap<ChatMessage::MessageStatus, QSvgRenderer*> m_statusRenderers;

    /**
 * @brief Флаг отметки — была ли уже глобально создана таблица рендереров (однократно на всё приложение).
 */
    static bool m_renderersInitialized;

    /**
 * @brief Мутируемый кеш QTextDocument для быстрого получения rich-форматирования и рендера текста.
 * Ключ — пара (id сообщения или row, max width).
 */
    mutable QMap<QPair<qint64, int>, QTextDocument*> m_documentCache;

    /**
 * @brief Статическая инициализация SVG-рендереров для каждого MessageStatus.
 *
 * - Проверяет, была ли инициализация ранее (m_renderersInitialized).
 * - Создаёт SVG-рендереры по статусам (".../clock_icon.svg" и т.д.) и сохраняет с static-временем жизни.
 * - Родитель — любой существующий QObject, чтобы обеспечить автоматический cleanup.
 * - Логгирует все действия, чтобы видеть этапы и id объектов в debug выводе.
 *
 * @param parent Родительский QObject для SVG-рендереров
 */
    static void initRenderers(QObject* parent);
};

#endif
