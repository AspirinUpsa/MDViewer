#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDropEvent>

class HelpDialog;
class QLabel;

/**
 * @brief Кнопка, пропускающая drag-and-drop события к родительскому виджету
 */
class DropButton : public QPushButton
{
    Q_OBJECT

public:
    explicit DropButton(const QString &text, QWidget *parent = nullptr)
        : QPushButton(text, parent) {}

protected:
    void dragEnterEvent(QDragEnterEvent *event) override
    {
        // Передаём событие родительскому виджету
        QPushButton::dragEnterEvent(event);
    }

    void dropEvent(QDropEvent *event) override
    {
        // Передаём событие родительскому виджету
        QPushButton::dropEvent(event);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void showHelp();
    void showAbout();
    void showInfo();
    void onHelpDialogClosed();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setupUi();
    void createMenuBar();
    void openMarkdownFile(const QString &filePath);
    void updateDropPlaceholder();
    void showHelpDialog();

    HelpDialog *m_helpDialog;
    QWidget *m_centralWidget;
    QLabel *m_dropLabel;
    DropButton *m_dropButton;
};
