#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QTextStream>
#include <QtDebug>
#include <QList>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Sample Rate
    ui->SampleRatecomboBox->addItem("10");
    ui->SampleRatecomboBox->addItem("50");
    ui->SampleRatecomboBox->addItem("100");
    ui->SampleRatecomboBox->addItem("200");

    //Accelerometer Range
    ui->AccelerometercomboBox->addItem("2");
    ui->AccelerometercomboBox->addItem("4");
    ui->AccelerometercomboBox->addItem("8");
    ui->AccelerometercomboBox->addItem("16");

    //Gyroscope Range
    ui->GyroscopecomboBox->addItem("250");
    ui->GyroscopecomboBox->addItem("500");
    ui->GyroscopecomboBox->addItem("1000");
    ui->GyroscopecomboBox->addItem("2000");

    //QFile file("C/Stage3IT/QtLesson/Configfile.txt");

    connect(ui->SampleRatecomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(comboBoxchanged()));
    connect(ui->AccelerometercomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(comboBoxchanged()));
    connect(ui->GyroscopecomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(comboBoxchanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
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

    //QDir directory("I:/ParameterFolder");
    //QFile file("I:/ParameterFolder/StartingParameter.txt");
    QString drive="";

    foreach( QFileInfo itdrive, QDir::drives() )
      {
        qDebug() << "Drive: " << itdrive.absolutePath();

        if (itdrive.absolutePath()=="I:/" || itdrive.absolutePath()=="E:/")
        {
            qDebug() << "Drive found:" << itdrive.absolutePath();
            drive=itdrive.absolutePath();
            break;
        }

      }
    if (drive=="")
        QMessageBox::warning(this,"","No drive found to save the file, make sure the sdCard is connected");

    else
    {
        QDir directory(drive +"ParameterFolder");

        if(!directory.exists())
        {
           statusBar()->showMessage("Folder not found");
           directory.mkpath(drive +"ParameterFolder");
        }

        saveChanges(directory.path() + "/StartingParameter.txt");
    }
    is_saved=true;
}

void MainWindow::comboBoxchanged()
{
    is_saved=false;
}

void MainWindow::saveChanges(QString path)
{

    QFile file(path);
    qDebug() <<"inside savechange";

    if (file.exists())
    {
        file.remove();
    }

    if(!file.open(QIODevice::WriteOnly |QIODevice::Text ))
    {
        QMessageBox::warning(this,"","Can't create file");
    }

    QTextStream _file(&file);
    _file<<"samplerate "<<ui->SampleRatecomboBox->currentText().toInt()<<"\n";
    _file<<"setupaccel "<<ui->AccelerometercomboBox->currentText().toInt()<<"\n";
    _file<<"setupgyro "<<ui->GyroscopecomboBox->currentText().toInt()<<"\n";
    file.flush();
    file.close();

}







