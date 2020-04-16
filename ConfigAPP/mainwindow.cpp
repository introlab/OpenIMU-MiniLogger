#include "mainwindow.h"

#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QTextStream>
#include <QtDebug>
#include <QList>
#include <QIntValidator>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <qurlvalidator.h>


//Initialize static const strings
const QString MainWindow::json_label_samplerate("samplerate");
const QString MainWindow::json_label_setupaccel("setupaccel");
const QString MainWindow::json_label_setupgyro("setupgyro");
const QString MainWindow::json_label_devicename("devicename");
const QString MainWindow::json_label_openteraserver("openteraserver");
const QString MainWindow::json_label_openteraport("openteraport");
const QString MainWindow::json_label_openteratoken("openteratoken");
const QString MainWindow::json_label_wifi_ssid("wifissid");
const QString MainWindow::json_label_wifi_password("wifipassword");


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), is_saved(true), m_netManager(nullptr)
{
    m_ui.setupUi(this);

    //Sample Rate
    m_ui.SampleRatecomboBox->addItem("10");
    m_ui.SampleRatecomboBox->addItem("50");
    m_ui.SampleRatecomboBox->addItem("100");
    m_ui.SampleRatecomboBox->addItem("200");

    //Accelerometer Range
    m_ui.AccelerometercomboBox->addItem("2");
    m_ui.AccelerometercomboBox->addItem("4");
    m_ui.AccelerometercomboBox->addItem("8");
    m_ui.AccelerometercomboBox->addItem("16");

    //Gyroscope Range
    m_ui.GyroscopecomboBox->addItem("250");
    m_ui.GyroscopecomboBox->addItem("500");
    m_ui.GyroscopecomboBox->addItem("1000");
    m_ui.GyroscopecomboBox->addItem("2000");


    //Menu
    connect(m_ui.actionLoad, SIGNAL(triggered()), this, SLOT(on_loadMenuActionClicked()));


    connect(m_ui.SampleRatecomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(comboBoxchanged()));
    connect(m_ui.AccelerometercomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(comboBoxchanged()));
    connect(m_ui.GyroscopecomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(comboBoxchanged()));


    connect(m_ui.buttonRegister, SIGNAL(clicked()), this, SLOT(on_RegisterButton_clicked()));
    connect(m_ui.LoadButton, SIGNAL(clicked()), this, SLOT(on_loadMenuActionClicked()));

    //Device Name
    connect(m_ui.lineEditDeviceName, SIGNAL(textChanged(const QString&)), this, SLOT(on_lineEditDeviceName_textChanged(const QString &)));


    //Server
    connect(m_ui.lineEditServer, SIGNAL(textChanged(const QString&)), this, SLOT(on_lineEditServer_textChanged(const QString &)));
    m_ui.lineEditServer->setValidator(new QUrlValidator(this));


    //Port
    connect(m_ui.lineEditPort, SIGNAL(textChanged(const QString&)), this, SLOT(on_lineEditPort_textChanged(const QString &)));
    m_ui.lineEditPort->setValidator(new QIntValidator(1,65565, this));


    //WiFi
    connect(m_ui.lineEditWiFiSSID, SIGNAL(textChanged(const QString&)), this, SLOT(on_lineEditWiFiSSID_textChanged(const QString&)));
    connect(m_ui.lineEditWiFiPassword, SIGNAL(textChanged(const QString&)), this, SLOT(on_lineEditWiFiPassword_textChanged(const QString&)));


    //Will enable button
    verifyServerConfiguration();
}

MainWindow::~MainWindow()
{

}

void MainWindow::on_ExitButton_clicked()
{

    if (is_saved)
    {
        qDebug()<<"is saved true";
        QApplication::quit();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.resize(640,480);
        msgBox.setText("Changed not saved");
        msgBox.setInformativeText("Are you sure you want to exit without saving your changes?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        switch (ret)
        {
            case QMessageBox::Yes:
            {
                QApplication::quit();
                break;
            }
            case QMessageBox::No:
            {
                break;
            }
        }
    }

}

void MainWindow::on_SaveButton_clicked()
{
    QString drive=QFileDialog::getExistingDirectory(this,"Select base directory");

    if (drive=="")
        QMessageBox::warning(this,"","No drive found to save the file, make sure the sdCard is connected");

    else
    {
        QDir directory(drive +"/ParameterFolder");

        if(!directory.exists())
        {
            statusBar()->showMessage("Folder not found");
            directory.mkpath(drive +"/ParameterFolder");
        }

        //DL - save configuration in a json document
        saveChangesJSON(directory.path() + "/StartingParameter.json");
    }
    is_saved=true;
}

void MainWindow::comboBoxchanged()
{
    is_saved=false;
}

void MainWindow::saveChangesJSON(QString path)
{
    QJsonDocument doc;
    QJsonObject json_object;

    //Fill object fields

    //IMU
    json_object.insert(MainWindow::json_label_samplerate, QJsonValue(m_ui.SampleRatecomboBox->currentText().toInt()));
    json_object.insert(MainWindow::json_label_setupaccel, QJsonValue(m_ui.AccelerometercomboBox->currentText().toInt()));
    json_object.insert(MainWindow::json_label_setupgyro, QJsonValue(m_ui.GyroscopecomboBox->currentText().toInt()));

    //WiFi
    json_object.insert(MainWindow::json_label_wifi_ssid, QJsonValue(m_ui.lineEditWiFiSSID->text()));
    json_object.insert(MainWindow::json_label_wifi_password, QJsonValue(m_ui.lineEditWiFiPassword->text()));

    //OpenTera
    json_object.insert(MainWindow::json_label_devicename, QJsonValue(m_ui.lineEditDeviceName->text()));
    json_object.insert(MainWindow::json_label_openteraserver, QJsonValue(m_ui.lineEditServer->text()));
    json_object.insert(MainWindow::json_label_openteraport, QJsonValue(m_ui.lineEditPort->text().toInt()));
    json_object.insert(MainWindow::json_label_openteratoken, QJsonValue(m_ui.textEditToken->toPlainText()));

    doc.setObject(json_object);

    QFile saveFile(path);
    if (!saveFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this,"Save JSON file", "Couldn't open save file : " + path);
        return;
    }

    saveFile.write(doc.toJson());
    saveFile.close();

    QMessageBox::information(this, "Save JSON file", "File saved!");
}

void MainWindow::on_RegisterButton_clicked()
{

    qDebug() << "MainWindow::on_RegisterButton_clicked()";

    if (m_netManager)
        m_netManager->deleteLater();

    m_netManager = new QNetworkAccessManager(this);

    // Connect signals from QNetworkAccessManager
    connect(m_netManager, &QNetworkAccessManager::finished, this, &MainWindow::on_networkFinished);
    connect(m_netManager, &QNetworkAccessManager::networkAccessibleChanged, this, &MainWindow::on_networkAccessibleChanged);
    connect(m_netManager, &QNetworkAccessManager::sslErrors, this, &MainWindow::on_networkSslErrors);

    QUrl url("https://" + m_ui.lineEditServer->text() + ":" + m_ui.lineEditPort->text() + "/api/device/device_register");
    qDebug() << url.toString();
    QNetworkRequest request(url);

    request.setRawHeader("Content-Type", "application/json");

    QJsonDocument doc;
    QJsonObject json_root_object;
    QJsonObject json_name_object;

    json_name_object.insert("device_name", QJsonValue(m_ui.lineEditDeviceName->text()));
    json_root_object.insert("device_info", QJsonValue(json_name_object));
    doc.setObject(json_root_object);

    m_netManager->post(request,doc.toJson());
}

void MainWindow::on_lineEditDeviceName_textChanged(const QString &text)
{
    Q_UNUSED(text)
    is_saved = false;
    verifyServerConfiguration();
}

void MainWindow::on_lineEditServer_textChanged(const QString &text)
{
    Q_UNUSED(text)
    is_saved = false;
    verifyServerConfiguration();
}

void MainWindow::on_lineEditPort_textChanged(const QString &text)
{
    Q_UNUSED(text)
    is_saved = false;
    verifyServerConfiguration();
}

void MainWindow::on_lineEditWiFiSSID_textChanged(const QString &text)
{
    Q_UNUSED(text)
    is_saved = false;
}

void MainWindow::on_lineEditWiFiPassword_textChanged(const QString &text)
{
    Q_UNUSED(text)
    is_saved = false;
}

void MainWindow::on_loadMenuActionClicked()
{
    //qDebug() << "void MainWindow::on_loadMenuActionClicked()";
    QString path=QFileDialog::getExistingDirectory(this,"Select base directory") + "/ParameterFolder/StartingParameter.json";

    if (!path.isEmpty())
    {
        QFile inputFile(path);
        inputFile.open(QFile::ReadOnly);

        if (inputFile.isReadable())
        {

            QJsonDocument doc = QJsonDocument::fromJson(inputFile.readAll());

            if (!doc.isEmpty())
            {

                //Restore all fields if possible.
                QJsonObject config_object = doc.object();

                //IMU
                if (config_object.contains(MainWindow::json_label_samplerate))
                    m_ui.SampleRatecomboBox->setCurrentText(QString::number(config_object[MainWindow::json_label_samplerate].toInt()));
                if (config_object.contains(MainWindow::json_label_setupaccel))
                    m_ui.AccelerometercomboBox->setCurrentText(QString::number(config_object[MainWindow::json_label_setupaccel].toInt()));
                if (config_object.contains(MainWindow::json_label_setupgyro))
                    m_ui.GyroscopecomboBox->setCurrentText(QString::number(config_object[MainWindow::json_label_setupgyro].toInt()));

                //WiFi
                if (config_object.contains(MainWindow::json_label_wifi_ssid))
                    m_ui.lineEditWiFiSSID->setText(config_object[MainWindow::json_label_wifi_ssid].toString());
                if (config_object.contains(MainWindow::json_label_wifi_password))
                    m_ui.lineEditWiFiPassword->setText(config_object[MainWindow::json_label_wifi_password].toString());

                //OpenTera
                if (config_object.contains(MainWindow::json_label_devicename))
                    m_ui.lineEditDeviceName->setText(config_object[MainWindow::json_label_devicename].toString());
                if (config_object.contains(MainWindow::json_label_openteraserver))
                    m_ui.lineEditServer->setText(config_object[MainWindow::json_label_openteraserver].toString());
                if (config_object.contains(MainWindow::json_label_openteraport))
                    m_ui.lineEditPort->setText(QString::number(config_object[MainWindow::json_label_openteraport].toInt()));
                if (config_object.contains(MainWindow::json_label_openteratoken))
                    m_ui.textEditToken->setText(config_object[MainWindow::json_label_openteratoken].toString());


                is_saved = true;

                QMessageBox::information(this, "Load JSON file", "File loaded!");

                return;
            }
        }

        inputFile.close();
    }

    QMessageBox::warning(this,"Load JSON file", "Couldn't load save file : " + path);

}

bool MainWindow::verifyServerConfiguration()
{
    if (m_ui.lineEditPort->text().isEmpty() ||
            m_ui.lineEditServer->text().isEmpty() ||
            m_ui.lineEditDeviceName->text().isEmpty()
            )
    {
        m_ui.buttonRegister->setEnabled(false);
        return false;
    }

    //Every field is filled, enable button
    m_ui.buttonRegister->setEnabled(true);

    return true;
}


void MainWindow::on_networkFinished(QNetworkReply *reply)
{
    // Valid reply?
    if (reply->error() == QNetworkReply::NoError)
    {
        //Read reply
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

        //Get JSON object and extract "token"
        QString token = doc.object()["token"].toString();

        //Put token in textEdit
        m_ui.textEditToken->setText(token);

        is_saved = false;
    }

    reply->deleteLater();
}

void MainWindow::on_networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible)
{
    Q_UNUSED(accessible)

    qDebug() << "ComManager::onNetworkAccessibleChanged";
    //TODO: ???
}

void MainWindow::on_networkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    qDebug() << "Ignoring SSL errors, this is unsafe";
    reply->ignoreSslErrors();
    for(QSslError error : errors){
        qDebug() << "Ignored: " + error.errorString();
    }
}






