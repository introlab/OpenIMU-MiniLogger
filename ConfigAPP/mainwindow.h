#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_ExitButton_clicked();

    void on_SaveButton_clicked();

    void comboBoxchanged();

    void saveChanges(QString path);

private:
    Ui::MainWindow *ui;
    bool is_saved= false;
};
#endif // MAINWINDOW_H
