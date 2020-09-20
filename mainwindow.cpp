#include "mainwindow.h"
#include "ui_mainwindow.h"

#define steps 7
#define dir 0
#define en 3
#define hm_sen 4

#define LED 26
#define init 0
#define range 100

#define BASE 100
#define SPI_CHAN 0

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    wiringPiSetup () ;
    mcp3004Setup (BASE, SPI_CHAN) ;
    softPwmCreate(LED, init,range);
     pinMode (en, OUTPUT) ;
     pinMode (dir, OUTPUT) ;
     pinMode (steps, OUTPUT) ;
     pinMode (hm_sen, INPUT) ;
     softPwmWrite(LED,0);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    //Homing
    digitalWrite(en,LOW);
    digitalWrite(dir,HIGH);
    for (int i=0;i<25000;i++)
    {
        if(digitalRead(hm_sen)==1)
        {
            break;
        }
        else
        {
            digitalWrite(steps, HIGH);
            QThread::usleep(15);
            digitalWrite(steps, LOW);
            QThread::usleep(15);
        }

    }
    digitalWrite(en,HIGH);
}

void MainWindow::on_pushButton_2_clicked()
{
    //Initialization
    on_pushButton_clicked();

    digitalWrite(en,LOW);
    digitalWrite(dir,LOW);
    for (int i=0;i<20000;i++)
    {
            digitalWrite(steps, HIGH);
            QThread::usleep(15);
            digitalWrite(steps, LOW);
            QThread::usleep(15);
        }
     digitalWrite(en,HIGH);

}

void MainWindow::on_pushButton_3_clicked()
{


    //reading
    on_pushButton_clicked();
if(ui->radioButton_2->isChecked())
    softPwmWrite(LED,10);
if(ui->radioButton_3->isChecked())
    softPwmWrite(LED,20);
if(ui->radioButton_4->isChecked())
    softPwmWrite(LED,30);
if(ui->radioButton_5->isChecked())
    softPwmWrite(LED,40);
if(ui->radioButton_6->isChecked())
    softPwmWrite(LED,50);
if(ui->radioButton_7->isChecked())
    softPwmWrite(LED,60);
if(ui->radioButton_8->isChecked())
    softPwmWrite(LED,70);
if(ui->radioButton_9->isChecked())
    softPwmWrite(LED,80);
if(ui->radioButton_10->isChecked())
    softPwmWrite(LED,90);
if(ui->radioButton_11->isChecked())
    softPwmWrite(LED,100);

    digitalWrite(en,LOW);
    digitalWrite(dir,LOW);
    for (int i=0;i<20000;i++)
    {
            digitalWrite(steps, HIGH);
            QThread::usleep(15);
            digitalWrite(steps, LOW);
            //QThread::usleep(20);
            readadc();
        }
     digitalWrite(en,HIGH);
     softPwmWrite(LED,0);

}

void MainWindow::readadc()
{
    unsigned char data[3]={0x06,0x00,0x00};
    wiringPiSPIDataRW(0,data,3);
    int adcValue=(data[1]&15)<<8|data[2];
    qDebug()<<adcValue;
}
