#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <QNetworkAccessManager>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:



    static const QString json_label_samplerate;
    static const QString json_label_setupaccel;
    static const QString json_label_setupgyro;
    static const QString json_label_devicename;
    static const QString json_label_openteraserver;
    static const QString json_label_openteraport;
    static const QString json_label_openteratoken;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Ui
    void on_ExitButton_clicked();
    void on_SaveButton_clicked();
    void comboBoxchanged();
    void saveChanges(QString path);
    void saveChangesJSON(QString path);
    void on_RegisterButton_clicked();
    void on_lineEditDeviceName_textChanged(const QString &text);
    void on_lineEditServer_textChanged(const QString &text);
    void on_lineEditPort_textChanged(const QString &text);

    //Menu
    void on_loadMenuActionClicked();

    //Networking
    void on_networkFinished(QNetworkReply *reply);
    void on_networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void on_networkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

private:

    bool verifyServerConfiguration();


    Ui::MainWindow m_ui;
    bool is_saved= false;

    QNetworkAccessManager*  m_netManager;



};
#endif // MAINWINDOW_H
