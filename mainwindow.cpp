#include "mainwindow.h"
#include "ui_mainwindow.h"

#define steps 7
#define dir 0
#define en 3

#define BASE 100
#define SPI_CHAN 0

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    wiringPiSetup () ;
    mcp3004Setup (BASE, SPI_CHAN) ;
     pinMode (en, OUTPUT) ;
     pinMode (dir, OUTPUT) ;
     pinMode (steps, OUTPUT) ;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    //forward
    digitalWrite(en,LOW);
    digitalWrite(dir,LOW);
    for (int i=0;i<5000;i++)
    {
        digitalWrite(steps, HIGH);
        QThread::usleep(10);
        digitalWrite(steps, LOW);
        QThread::usleep(10);
    }
    digitalWrite(en,HIGH);
    qDebug()<<"Done";
}

void MainWindow::on_pushButton_2_clicked()
{
    //forward
    digitalWrite(en,LOW);
    digitalWrite(dir,HIGH);
    for (int i=0;i<5000;i++)
    {
        digitalWrite(steps, HIGH);
        QThread::usleep(10);
        digitalWrite(steps, LOW);
        QThread::usleep(10);
    }
    digitalWrite(en,HIGH);
    qDebug()<<"Done";

}

void MainWindow::on_pushButton_3_clicked()
{
//    int x=0;
//    for (int i = 0 ; i<10 ; i++)
//    {
//        x = analogRead (BASE) ;
//    qDebug()<<x;
//    QThread::msleep(100);
//    }

    for (int i = 0 ; i<10 ; i++)
    {
        int status;
        status=wiringPiSPISetup(0, 500000);
        unsigned char data[3]={0x06,0x00,0x00};
        status=wiringPiSPIDataRW(0,data,3);
        int adcValue=(data[1]&15)<<8|data[2];
        qDebug()<<adcValue;
        QThread::usleep(1);
    }

}
