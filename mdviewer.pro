QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MDViewer
TEMPLATE = app

# Поддержка C++17
CONFIG += c++17

# Исходные файлы
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    helpdialog.cpp

# Заголовочные файлы
HEADERS += \
    mainwindow.h \
    helpdialog.h

# Ресурсы
RESOURCES += \
    resources.qrc

# Пути для заголовков
INCLUDEPATH += .

# Иконка приложения для Windows
win32 {
    RC_FILE = appicon.rc
}
