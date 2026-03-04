#pragma once

#include <QDialog>
#include <QTextBrowser>
#include <QToolBar>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QVector>

class QAction;
class QLabel;
class QLineEdit;

/**
 * @brief Текстовый браузер с поддержкой копирования правой кнопкой мыши
 */
class CopyTextBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    explicit CopyTextBrowser(QWidget *parent = nullptr);

signals:
    void rightMouseClicked();
    void fileDropped(const QString &filePath);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

/**
 * @brief Диалоговое окно для отображения справки из Markdown-файла
 *
 * Поддерживает:
 * - Рендеринг Markdown через QTextDocument
 * - Поиск по тексту
 * - Переход по внешним ссылкам
 * - Переключение темы (светлая/тёмная)
 * - Режим чтения с приятной цветовой схемой
 * - Загрузку MD-файла через метод loadFile()
 * - Drag-and-drop для открытия файлов
 * - Копирование выделенного текста правой кнопкой мыши
 */
class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);
    ~HelpDialog() override;

public slots:
    void loadFile(const QString &filePath);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void toggleTheme();
    void toggleReadingMode();
    void zoomIn();
    void zoomOut();
    void zoomNormal();
    void copySelectedText();

private:
    void setupUi();
    void applyLightTheme();
    void applyDarkTheme();
    void applyLightReadingMode();
    void applyDarkReadingMode();
    void handleFileDrop(const QString &filePath);
    void highlightAllOccurrences(const QString &text);
    void clearSearchHighlight();

    CopyTextBrowser *m_textBrowser;
    QToolBar *m_toolBar;
    QAction *m_themeAction;
    QAction *m_readingModeAction;
    qreal m_currentZoom;
    bool m_isDarkTheme;
    bool m_isReadingMode;
    QVector<QTextCursor> m_searchCursors;  // Сохраняем курсоры для снятия подсветки
};
