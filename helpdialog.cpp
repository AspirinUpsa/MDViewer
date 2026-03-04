#include "helpdialog.h"
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QVector>
#include <QVBoxLayout>
#include <QToolBar>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QTimer>
#include <QFile>
#include <QTextCursor>
#include <QApplication>
#include <QStyle>
#include <QAction>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QMimeData>
#include <QUrl>
#include <QScreen>
#include <QClipboard>
#include <QMouseEvent>
#include <QToolTip>

// Реализация CopyTextBrowser
CopyTextBrowser::CopyTextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    // Включаем стандартное копирование по Ctrl+C
    setContextMenuPolicy(Qt::NoContextMenu);
}

void CopyTextBrowser::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        // Правая кнопка — копируем выделенный текст
        if (textCursor().hasSelection()) {
            copy();
            
            // Создаём временное уведомление
            QLabel *notification = new QLabel("Текст скопирован в буфер обмена", this);
            notification->setStyleSheet(
                "QLabel {"
                "    background-color: rgba(0, 0, 0, 200);"
                "    color: white;"
                "    padding: 8px 12px;"
                "    border-radius: 4px;"
                "    font-size: 12px;"
                "}"
            );
            notification->setAttribute(Qt::WA_TransparentForMouseEvents);
            notification->setWindowFlags(Qt::ToolTip);
            
            // Позиционируем рядом с курсором
            QPoint pos = mapToGlobal(event->pos());
            notification->move(pos.x() + 10, pos.y() - 40);
            notification->show();
            
            // Автоматически скрываем через 1.5 секунды
            QTimer::singleShot(1500, notification, [notification]() {
                notification->deleteLater();
            });
        }
        event->accept();
        return;
    }
    // Левая кнопка — стандартное поведение (выделение)
    QTextBrowser::mousePressEvent(event);
}

void CopyTextBrowser::dragEnterEvent(QDragEnterEvent *event)
{
    // Проверяем, есть ли файлы с подходящим расширением
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        bool hasValidFile = false;
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                if (filePath.endsWith(".md", Qt::CaseInsensitive) ||
                    filePath.endsWith(".markdown", Qt::CaseInsensitive) ||
                    filePath.endsWith(".txt", Qt::CaseInsensitive)) {
                    hasValidFile = true;
                    break;
                }
            }
        }
        if (hasValidFile) {
            event->acceptProposedAction();
            return;
        }
    }
    // Для других случаев используем стандартное поведение
    QTextBrowser::dragEnterEvent(event);
}

void CopyTextBrowser::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        if (url.isLocalFile()) {
            QString filePath = url.toLocalFile();
            if (filePath.endsWith(".md", Qt::CaseInsensitive) ||
                filePath.endsWith(".markdown", Qt::CaseInsensitive) ||
                filePath.endsWith(".txt", Qt::CaseInsensitive)) {
                emit fileDropped(filePath);
                event->acceptProposedAction();
                return;
            }
        }
    }
    // Если не наш файл, используем стандартное поведение
    QTextBrowser::dropEvent(event);
}

// Реализация HelpDialog

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
    , m_textBrowser(nullptr)
    , m_toolBar(nullptr)
    , m_themeAction(nullptr)
    , m_readingModeAction(nullptr)
    , m_currentZoom(1.0)
    , m_isDarkTheme(false)
    , m_isReadingMode(true)  // По умолчанию режим чтения включен
{
    setWindowTitle("POWer Markdown Viewer - Просмотр");
    
    // Размер: высота = экран, ширина = 1/3 экрана
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int width = screenGeometry.width() / 3;
        int height = screenGeometry.height();
        resize(width, height);
    } else {
        resize(400, 600);
    }

    setupUi();
    applyLightReadingMode();  // По умолчанию применяем режим чтения
}

HelpDialog::~HelpDialog()
{
}

void HelpDialog::dragEnterEvent(QDragEnterEvent *event)
{
    // Передаём событие дочерним виджетам (CopyTextBrowser)
    QDialog::dragEnterEvent(event);
}

void HelpDialog::dropEvent(QDropEvent *event)
{
    // Передаём событие дочерним виджетам (CopyTextBrowser)
    QDialog::dropEvent(event);
}

bool HelpDialog::eventFilter(QObject *watched, QEvent *event)
{
    // Не обрабатываем drag-and-drop события — пусть их обрабатывает CopyTextBrowser
    if (event->type() == QEvent::DragEnter || event->type() == QEvent::Drop) {
        return QDialog::eventFilter(watched, event);
    }

    // Для всех остальных событий используем стандартную обработку
    return QDialog::eventFilter(watched, event);
}

void HelpDialog::handleFileDrop(const QString &filePath)
{
    loadFile(filePath);
    show();
    raise();
    activateWindow();
}

void HelpDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Панель инструментов
    m_toolBar = new QToolBar(this);
    m_toolBar->setMovable(false);
    m_toolBar->setIconSize(QSize(16, 16));

    // Кнопка поиска
    auto *searchLabel = new QLabel("Поиск:", m_toolBar);
    m_toolBar->addWidget(searchLabel);

    auto *searchEdit = new QLineEdit(m_toolBar);
    searchEdit->setPlaceholderText("Введите текст для поиска...");
    searchEdit->setMaximumWidth(150);
    // Поиск по нажатию Enter - подсвечивает все совпадения
    connect(searchEdit, &QLineEdit::returnPressed, this, [this, searchEdit]() {
        QString text = searchEdit->text();
        
        // Снимаем предыдущую подсветку
        clearSearchHighlight();
        
        if (!text.isEmpty()) {
            // Подсвечиваем все совпадения
            highlightAllOccurrences(text);
        }
    });
    m_toolBar->addWidget(searchEdit);

    // Кнопка поиска - подсвечивает все совпадения
    auto *searchBtn = new QToolButton(m_toolBar);
    searchBtn->setText("🔍");
    searchBtn->setToolTip("Найти все совпадения");
    connect(searchBtn, &QToolButton::clicked, this, [this, searchEdit]() {
        QString text = searchEdit->text();
        
        // Снимаем предыдущую подсветку
        clearSearchHighlight();
        
        if (!text.isEmpty()) {
            // Подсвечиваем все совпадения
            highlightAllOccurrences(text);
        }
    });
    m_toolBar->addWidget(searchBtn);

    m_toolBar->addSeparator();

    // Переключатель темы
    m_themeAction = new QAction("🌙 Тёмная", this);
    m_themeAction->setToolTip("Переключить тему");
    connect(m_themeAction, &QAction::triggered, this, &HelpDialog::toggleTheme);
    m_toolBar->addAction(m_themeAction);

    m_toolBar->addSeparator();

    // Режим чтения (по умолчанию включен)
    m_readingModeAction = new QAction("❌ Выход из режима чтения", this);
    m_readingModeAction->setToolTip("Приятная цветовая схема для чтения (включена)");
    connect(m_readingModeAction, &QAction::triggered, this, &HelpDialog::toggleReadingMode);
    m_toolBar->addAction(m_readingModeAction);

    m_toolBar->addSeparator();

    // Кнопки масштабирования
    auto *zoomInBtn = new QToolButton(m_toolBar);
    zoomInBtn->setText("A+");
    zoomInBtn->setToolTip("Увеличить шрифт");
    connect(zoomInBtn, &QToolButton::clicked, this, &HelpDialog::zoomIn);
    m_toolBar->addWidget(zoomInBtn);

    auto *zoomOutBtn = new QToolButton(m_toolBar);
    zoomOutBtn->setText("A-");
    zoomOutBtn->setToolTip("Уменьшить шрифт");
    connect(zoomOutBtn, &QToolButton::clicked, this, &HelpDialog::zoomOut);
    m_toolBar->addWidget(zoomOutBtn);

    auto *zoomNormalBtn = new QToolButton(m_toolBar);
    zoomNormalBtn->setText("A");
    zoomNormalBtn->setToolTip("Сбросить масштаб");
    connect(zoomNormalBtn, &QToolButton::clicked, this, &HelpDialog::zoomNormal);
    m_toolBar->addWidget(zoomNormalBtn);

    mainLayout->addWidget(m_toolBar);

    // Текстовый браузер с поддержкой копирования правой кнопкой
    m_textBrowser = new CopyTextBrowser(this);
    m_textBrowser->setOpenExternalLinks(true);
    m_textBrowser->setOpenLinks(true);
    m_textBrowser->setSearchPaths(QStringList() << ":/");
    m_textBrowser->setAcceptDrops(true);
    // Подключаем к handleFileDrop для правильного отображения окна
    connect(m_textBrowser, &CopyTextBrowser::fileDropped, this, &HelpDialog::handleFileDrop);

    mainLayout->addWidget(m_textBrowser);

    setLayout(mainLayout);
}

void HelpDialog::loadFile(const QString &filePath)
{
    // Снимаем предыдущую подсветку поиска
    clearSearchHighlight();

    if (filePath.isEmpty()) {
        m_textBrowser->setHtml(
            "<h2>Справка</h2><p>Выберите <b>Файл → Открыть файл</b> для загрузки файла.</p>"
        );
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_textBrowser->setHtml(
            QString("<h2>Ошибка</h2><p>Не удалось открыть файл: %1</p>").arg(filePath)
        );
        return;
    }

    QString content = file.readAll();
    file.close();

    // Проверяем расширение файла
    bool isMarkdown = filePath.endsWith(".md", Qt::CaseInsensitive) ||
                      filePath.endsWith(".markdown", Qt::CaseInsensitive);

    // Используем встроенный парсер Markdown из Qt 5.14+ для MD файлов
    QTextDocument document;
    if (isMarkdown) {
        document.setMarkdown(content);
    } else {
        // Для TXT файлов используем простой текст
        document.setPlainText(content);
    }

    m_textBrowser->setDocument(document.clone());

    // Настраиваем базовый шрифт
    QFont font = m_textBrowser->font();
    font.setPointSize(9);
    m_textBrowser->setFont(font);

    // Применяем текущий режим
    if (m_isReadingMode) {
        if (m_isDarkTheme) {
            applyDarkReadingMode();
        } else {
            applyLightReadingMode();
        }
    } else {
        if (m_isDarkTheme) {
            applyDarkTheme();
        } else {
            applyLightTheme();
        }
    }
}

void HelpDialog::toggleTheme()
{
    // Снимаем подсветку перед переключением темы
    clearSearchHighlight();
    
    m_isDarkTheme = !m_isDarkTheme;

    if (m_isDarkTheme) {
        m_themeAction->setText("☀️ Светлая");
        if (m_isReadingMode) {
            applyDarkReadingMode();
        } else {
            applyDarkTheme();
        }
    } else {
        m_themeAction->setText("🌙 Тёмная");
        if (m_isReadingMode) {
            applyLightReadingMode();
        } else {
            applyLightTheme();
        }
    }
}

void HelpDialog::toggleReadingMode()
{
    // Снимаем подсветку перед переключением режима
    clearSearchHighlight();
    
    m_isReadingMode = !m_isReadingMode;

    if (m_isReadingMode) {
        m_readingModeAction->setText("❌ Выход из режима чтения");
        if (m_isDarkTheme) {
            applyDarkReadingMode();
        } else {
            applyLightReadingMode();
        }
    } else {
        m_readingModeAction->setText("📖 Режим чтения");
        if (m_isDarkTheme) {
            applyDarkTheme();
        } else {
            applyLightTheme();
        }
    }
}

void HelpDialog::applyLightTheme()
{
    m_textBrowser->setStyleSheet(
        "QTextBrowser {"
        "    background-color: #ffffff;"
        "    color: #000000;"
        "    border: none;"
        "    padding: 10px;"
        "}"
        "QTextBrowser QScrollBar:vertical {"
        "    background: #f0f0f0;"
        "    width: 12px;"
        "    border: none;"
        "    margin: 0px;"
        "}"
        "QTextBrowser QScrollBar::handle:vertical {"
        "    background: #c0c0c0;"
        "    min-height: 20px;"
        "    border-radius: 6px;"
        "}"
        "QTextBrowser QScrollBar::handle:vertical:hover {"
        "    background: #a0a0a0;"
        "}"
        "QTextBrowser QScrollBar::add-line:vertical,"
        "QTextBrowser QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QTextBrowser QScrollBar::add-page:vertical,"
        "QTextBrowser QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
    );

    m_toolBar->setStyleSheet(
        "QToolBar {"
        "    background-color: #f0f0f0;"
        "    border-bottom: 1px solid #cccccc;"
        "    spacing: 3px;"
        "}"
        "QLabel { color: #000000; }"
        "QLineEdit {"
        "    background-color: #ffffff;"
        "    color: #000000;"
        "    border: 1px solid #cccccc;"
        "    padding: 2px 5px;"
        "}"
        "QToolButton {"
        "    background-color: #e0e0e0;"
        "    color: #000000;"
        "    border: 1px solid #cccccc;"
        "    padding: 2px 8px;"
        "}"
        "QToolButton:hover { background-color: #d0d0d0; }"
    );
}

void HelpDialog::applyDarkTheme()
{
    m_textBrowser->setStyleSheet(
        "QTextBrowser {"
        "    background-color: #1e1e1e;"
        "    color: #d4d4d4;"
        "    border: none;"
        "    padding: 10px;"
        "}"
        "QTextBrowser QScrollBar:vertical {"
        "    background: #2d2d2d;"
        "    width: 12px;"
        "    border: none;"
        "    margin: 0px;"
        "}"
        "QTextBrowser QScrollBar::handle:vertical {"
        "    background: #555555;"
        "    min-height: 20px;"
        "    border-radius: 6px;"
        "}"
        "QTextBrowser QScrollBar::handle:vertical:hover {"
        "    background: #666666;"
        "}"
        "QTextBrowser QScrollBar::add-line:vertical,"
        "QTextBrowser QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QTextBrowser QScrollBar::add-page:vertical,"
        "QTextBrowser QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
    );

    m_toolBar->setStyleSheet(
        "QToolBar {"
        "    background-color: #2d2d2d;"
        "    border-bottom: 1px solid #3c3c3c;"
        "    spacing: 3px;"
        "}"
        "QLabel { color: #d4d4d4; }"
        "QLineEdit {"
        "    background-color: #3c3c3c;"
        "    color: #d4d4d4;"
        "    border: 1px solid #555555;"
        "    padding: 2px 5px;"
        "}"
        "QToolButton {"
        "    background-color: #3c3c3c;"
        "    color: #d4d4d4;"
        "    border: 1px solid #555555;"
        "    padding: 2px 8px;"
        "}"
        "QToolButton:hover { background-color: #4a4a4a; }"
    );
}

void HelpDialog::applyLightReadingMode()
{
    // Приятная схема для чтения: тёплый кремовый фон, тёмно-серый текст
    m_textBrowser->setStyleSheet(
        "QTextBrowser {"
        "    background-color: #f5f0e6;"
        "    color: #2c2c2c;"
        "    border: none;"
        "    padding: 15px 25px;"
        "    font-family: Georgia, 'Times New Roman', serif;"
        "    line-height: 1.6;"
        "}"
        "QTextBrowser QScrollBar:vertical {"
        "    background: #e8e0d5;"
        "    width: 12px;"
        "    border: none;"
        "    margin: 0px;"
        "}"
        "QTextBrowser QScrollBar::handle:vertical {"
        "    background: #c4b8a8;"
        "    min-height: 20px;"
        "    border-radius: 6px;"
        "}"
        "QTextBrowser QScrollBar::handle:vertical:hover {"
        "    background: #b4a898;"
        "}"
        "QTextBrowser QScrollBar::add-line:vertical,"
        "QTextBrowser QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QTextBrowser QScrollBar::add-page:vertical,"
        "QTextBrowser QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
        "QTextBrowser h1, QTextBrowser h2, QTextBrowser h3 {"
        "    color: #1a1a1a;"
        "    border-bottom: 1px solid #d4c8b8;"
        "    padding-bottom: 5px;"
        "}"
        "QTextBrowser a { color: #0066cc; }"
        "QTextBrowser code {"
        "    background-color: #e8e4dc;"
        "    padding: 2px 6px;"
        "    border-radius: 3px;"
        "}"
        "QTextBrowser pre {"
        "    background-color: #2d2d2d;"
        "    color: #d4d4d4;"
        "    padding: 10px;"
        "    border-radius: 5px;"
        "}"
    );

    m_toolBar->setStyleSheet(
        "QToolBar {"
        "    background-color: #e8e0d5;"
        "    border-bottom: 1px solid #d4c8b8;"
        "    spacing: 3px;"
        "}"
        "QLabel { color: #2c2c2c; }"
        "QLineEdit {"
        "    background-color: #ffffff;"
        "    color: #2c2c2c;"
        "    border: 1px solid #d4c8b8;"
        "    padding: 2px 5px;"
        "}"
        "QToolButton {"
        "    background-color: #d4c8b8;"
        "    color: #2c2c2c;"
        "    border: 1px solid #c4b8a8;"
        "    padding: 2px 8px;"
        "}"
        "QToolButton:hover { background-color: #c4b8a8; }"
    );
}

void HelpDialog::applyDarkReadingMode()
{
    // Приятная схема для чтения в тёмном режиме: тёмно-серый фон, светлый текст
    m_textBrowser->setStyleSheet(
        "QTextBrowser {"
        "    background-color: #2d313a;"
        "    color: #c9d1d9;"
        "    border: none;"
        "    padding: 15px 25px;"
        "    font-family: Georgia, 'Times New Roman', serif;"
        "    line-height: 1.6;"
        "}"
        "QTextBrowser QScrollBar:vertical {"
        "    background: #21262d;"
        "    width: 12px;"
        "    border: none;"
        "    margin: 0px;"
        "}"
        "QTextBrowser QScrollBar::handle:vertical {"
        "    background: #4a5058;"
        "    min-height: 20px;"
        "    border-radius: 6px;"
        "}"
        "QTextBrowser QScrollBar::handle:vertical:hover {"
        "    background: #5a6068;"
        "}"
        "QTextBrowser QScrollBar::add-line:vertical,"
        "QTextBrowser QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QTextBrowser QScrollBar::add-page:vertical,"
        "QTextBrowser QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
        "QTextBrowser h1, QTextBrowser h2, QTextBrowser h3 {"
        "    color: #e6edf3;"
        "    border-bottom: 1px solid #3c4048;"
        "    padding-bottom: 5px;"
        "}"
        "QTextBrowser a { color: #58a6ff; }"
        "QTextBrowser code {"
        "    background-color: #3c4048;"
        "    padding: 2px 6px;"
        "    border-radius: 3px;"
        "}"
        "QTextBrowser pre {"
        "    background-color: #1a1d23;"
        "    color: #c9d1d9;"
        "    padding: 10px;"
        "    border-radius: 5px;"
        "}"
    );

    m_toolBar->setStyleSheet(
        "QToolBar {"
        "    background-color: #21262d;"
        "    border-bottom: 1px solid #3c4048;"
        "    spacing: 3px;"
        "}"
        "QLabel { color: #c9d1d9; }"
        "QLineEdit {"
        "    background-color: #3c4048;"
        "    color: #c9d1d9;"
        "    border: 1px solid #555a63;"
        "    padding: 2px 5px;"
        "}"
        "QToolButton {"
        "    background-color: #3c4048;"
        "    color: #c9d1d9;"
        "    border: 1px solid #555a63;"
        "    padding: 2px 8px;"
        "}"
        "QToolButton:hover { background-color: #555a63; }"
    );
}

void HelpDialog::zoomIn()
{
    m_currentZoom *= 1.2;
    QFont font = m_textBrowser->font();
    font.setPointSizeF(font.pointSizeF() * 1.2);
    m_textBrowser->setFont(font);
}

void HelpDialog::zoomOut()
{
    m_currentZoom /= 1.2;
    QFont font = m_textBrowser->font();
    font.setPointSizeF(font.pointSizeF() / 1.2);
    m_textBrowser->setFont(font);
}

void HelpDialog::zoomNormal()
{
    m_currentZoom = 1.0;
    QFont font = m_textBrowser->font();
    font.setPointSize(9);
    m_textBrowser->setFont(font);
}

void HelpDialog::copySelectedText()
{
    if (m_textBrowser->textCursor().hasSelection()) {
        m_textBrowser->copy();
    }
}

void HelpDialog::highlightAllOccurrences(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }

    // Формат подсветки: ярко-жёлтый фон, чёрный текст
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(QColor(255, 255, 0));  // Ярко-жёлтый фон
    highlightFormat.setForeground(QColor(0, 0, 0));      // Чёрный текст

    // Сохраняем позиции всех найденных совпадений
    m_searchCursors.clear();
    
    QTextCursor cursor = m_textBrowser->document()->find(text, 0);
    int position = 0;
    
    // Находим и подсвечиваем все вхождения
    while (!cursor.isNull()) {
        // Сохраняем позицию начала выделения
        m_searchCursors.append(cursor);
        
        // Применяем подсветку
        cursor.mergeCharFormat(highlightFormat);
        
        // Запоминаем позицию для следующего поиска
        position = cursor.position();
        
        // Ищем следующее вхождение
        cursor = m_textBrowser->document()->find(text, position);
    }

    // Прокручиваем к первому найденному совпадению без изменения форматирования
    if (!m_searchCursors.isEmpty()) {
        QTextCursor firstCursor = m_searchCursors.first();
        // Просто перемещаем курсор и прокручиваем, не выделяя текст
        QTextCursor moveCursor = m_textBrowser->textCursor();
        moveCursor.setPosition(firstCursor.selectionStart());
        m_textBrowser->setTextCursor(moveCursor);
        m_textBrowser->ensureCursorVisible();
    }
}

void HelpDialog::clearSearchHighlight()
{
    if (m_searchCursors.isEmpty()) {
        return;
    }

    // Снимаем подсветку: проходим по всем сохранённым позициям
    QTextCharFormat normalFormat;
    normalFormat.setBackground(Qt::transparent);
    
    for (const QTextCursor &cursor : m_searchCursors) {
        // Создаем новый курсор с теми же позициями
        QTextCursor formatCursor(m_textBrowser->document());
        formatCursor.setPosition(cursor.selectionStart());
        formatCursor.setPosition(cursor.selectionEnd(), QTextCursor::KeepAnchor);
        formatCursor.setCharFormat(normalFormat);
    }

    m_searchCursors.clear();
}
