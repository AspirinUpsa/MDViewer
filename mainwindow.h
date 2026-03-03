#pragma once

#include <QMainWindow>

class HelpDialog;
class QLabel;
class QPushButton;

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
    QPushButton *m_dropButton;
};
