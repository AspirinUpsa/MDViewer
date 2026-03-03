#include "mainwindow.h"
#include "helpdialog.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPixmap>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_helpDialog(nullptr)
    , m_centralWidget(nullptr)
    , m_dropLabel(nullptr)
    , m_dropButton(nullptr)
{
    setWindowTitle("POWer Markdown Viewer");
    resize(500, 400);
    setAcceptDrops(true);

    setupUi();
    createMenuBar();
}

MainWindow::~MainWindow()
{
    if (m_helpDialog) {
        m_helpDialog->close();
        m_helpDialog->deleteLater();
    }
}

void MainWindow::showHelpDialog()
{
    if (!m_helpDialog) {
        m_helpDialog = new HelpDialog(this);
        // Скрываем главное окно при закрытии диалога
        connect(m_helpDialog, &HelpDialog::finished, this, &MainWindow::onHelpDialogClosed);
    }
    
    // Позиционируем окно справа от главного
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = screenGeometry.right() - m_helpDialog->width();
        int y = screenGeometry.top();
        m_helpDialog->move(x, y);
    }
    
    // Скрываем главное окно и показываем диалог
    this->hide();
    m_helpDialog->show();
    m_helpDialog->raise();
    m_helpDialog->activateWindow();
}

void MainWindow::onHelpDialogClosed()
{
    // Показываем главное окно при закрытии диалога
    this->show();
    this->raise();
    this->activateWindow();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        if (url.isLocalFile()) {
            QString filePath = url.toLocalFile();
            if (filePath.endsWith(".md", Qt::CaseInsensitive) ||
                filePath.endsWith(".markdown", Qt::CaseInsensitive) ||
                filePath.endsWith(".txt", Qt::CaseInsensitive)) {
                openMarkdownFile(filePath);
                break;
            }
        }
    }
    event->acceptProposedAction();
}

void MainWindow::openMarkdownFile(const QString &filePath)
{
    if (!m_helpDialog) {
        m_helpDialog = new HelpDialog(this);
        connect(m_helpDialog, &HelpDialog::finished, this, &MainWindow::onHelpDialogClosed);
    }
    m_helpDialog->loadFile(filePath);
    showHelpDialog();
}

void MainWindow::setupUi()
{
    m_centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(m_centralWidget);
    layout->setSpacing(20);
    layout->setContentsMargins(40, 40, 40, 40);

    // Заголовок
    auto *titleLabel = new QLabel("POWer Markdown Viewer", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Описание
    auto *descLabel = new QLabel(
        "Программа для чтения Markdown и текстовых файлов\n"
        "с поддержкой тем, поиска и масштабирования.", this);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    layout->addStretch();

    // Большой + в круге
    auto *circleWidget = new QWidget(this);
    circleWidget->setFixedSize(150, 150);
    
    auto *circleLayout = new QVBoxLayout(circleWidget);
    circleLayout->setContentsMargins(0, 0, 0, 0);
    
    m_dropButton = new QPushButton("+", this);
    m_dropButton->setFixedSize(120, 120);
    QFont plusFont = m_dropButton->font();
    plusFont.setPointSize(48);
    m_dropButton->setFont(plusFont);
    m_dropButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4a90d9;"
        "    color: white;"
        "    border-radius: 60px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #357abd;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #2a5f8f;"
        "}"
    );
    connect(m_dropButton, &QPushButton::clicked, this, &MainWindow::openFile);
    
    circleLayout->addWidget(m_dropButton, 0, Qt::AlignCenter);
    layout->addWidget(circleWidget, 0, Qt::AlignCenter);

    // Текст под плюсом
    m_dropLabel = new QLabel("Перетащите сюда MD-файл\nили нажмите +", this);
    m_dropLabel->setAlignment(Qt::AlignCenter);
    m_dropLabel->setStyleSheet("color: #888888; font-size: 12px;");
    layout->addWidget(m_dropLabel);

    layout->addStretch();

    // Кнопка помощи
    auto *infoButton = new QPushButton("ℹ️ Краткая помощь", this);
    infoButton->setMaximumWidth(200);
    connect(infoButton, &QPushButton::clicked, this, &MainWindow::showInfo);
    layout->addWidget(infoButton, 0, Qt::AlignCenter);

    setCentralWidget(m_centralWidget);
}

void MainWindow::createMenuBar()
{
    // Меню "Файл"
    auto *fileMenu = menuBar()->addMenu("Файл");

    auto *openAction = fileMenu->addAction("📂 Открыть MD файл...");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    fileMenu->addSeparator();

    auto *exitAction = fileMenu->addAction("Выход");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // Меню "Справка"
    auto *helpMenu = menuBar()->addMenu("Справка");

    auto *helpAction = helpMenu->addAction("Помощь");
    helpAction->setShortcut(QKeySequence::HelpContents);
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);

    helpMenu->addSeparator();

    auto *infoAction = helpMenu->addAction("Краткая помощь");
    connect(infoAction, &QAction::triggered, this, &MainWindow::showInfo);

    helpMenu->addSeparator();

    auto *aboutAction = helpMenu->addAction("О программе");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Открыть файл",
        QString(),
        "Текстовые файлы (*.md *.markdown *.txt);;Markdown-файлы (*.md *.markdown);;Текстовые файлы (*.txt);;Все файлы (*.*)"
    );

    if (!fileName.isEmpty()) {
        if (!m_helpDialog) {
            m_helpDialog = new HelpDialog(this);
            connect(m_helpDialog, &HelpDialog::finished, this, &MainWindow::onHelpDialogClosed);
        }
        m_helpDialog->loadFile(fileName);
        showHelpDialog();
    }
}

void MainWindow::showHelp()
{
    if (!m_helpDialog) {
        m_helpDialog = new HelpDialog(this);
        connect(m_helpDialog, &HelpDialog::finished, this, &MainWindow::onHelpDialogClosed);
    }
    m_helpDialog->loadFile(":/help.md");
    showHelpDialog();
}

void MainWindow::showInfo()
{
    QMessageBox::information(this, "Краткая помощь",
        "<h3>Как использовать POWer Markdown Viewer</h3>"
        "<ul>"
        "<li><b>Перетащите файл</b> на окно программы</li>"
        "<li><b>Нажмите +</b> для выбора файла</li>"
        "<li><b>Файл → Открыть</b> (Ctrl+O) — открыть файл</li>"
        "</ul>"
        "<h4>В окне просмотра:</h4>"
        "<ul>"
        "<li><b>🌙 / ☀️</b> — переключение темы</li>"
        "<li><b>📖 / ❌</b> — режим чтения (вкл/выкл, по умолчанию включен)</li>"
        "<li><b>A+ / A- / A</b> — масштабирование шрифта</li>"
        "<li><b>Поиск</b> — поиск по тексту</li>"
        "<li><b>Правая кнопка мыши</b> — копировать выделенный текст</li>"
        "</ul>"
        "<p><i>Поддерживаются файлы .md, .markdown и .txt</i></p>"
    );
}

void MainWindow::showAbout()
{
    auto *aboutBox = new QMessageBox(this);
    aboutBox->setWindowTitle("О программе");
    aboutBox->setIconPixmap(QPixmap(":/appicon.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    aboutBox->setText("<h3>POWer Markdown Viewer</h3>"
        "<p>Программа для просмотра Markdown и текстовых файлов.</p>"
        "<p><b>Версия:</b> 1.0</p>"
        "<p><b>Qt версия:</b> " QT_VERSION_STR "</p>"
        "<p><b>Год:</b> 2026</p>");
    aboutBox->setInformativeText(
        "<hr>"
        "<p><b>Copyright © POWer, Samara</b></p>"
        "<p><i>Поддержка: drag-and-drop, темы, поиск, масштабирование, копирование</i></p>");
    aboutBox->exec();
    aboutBox->deleteLater();
}

void MainWindow::updateDropPlaceholder()
{
    if (m_dropLabel) {
        m_dropLabel->setText("Перетащите сюда MD-файл\nили нажмите +");
    }
}
