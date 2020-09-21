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

unsigned int read[20000];
unsigned int filtdata[20000];


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
else if(ui->radioButton_3->isChecked())
    softPwmWrite(LED,20);
else if(ui->radioButton_4->isChecked())
    softPwmWrite(LED,30);
else if(ui->radioButton_5->isChecked())
    softPwmWrite(LED,40);
else if(ui->radioButton_6->isChecked())
    softPwmWrite(LED,50);
else if(ui->radioButton_7->isChecked())
    softPwmWrite(LED,60);
else if(ui->radioButton_8->isChecked())
    softPwmWrite(LED,70);
else if(ui->radioButton_9->isChecked())
    softPwmWrite(LED,80);
else if(ui->radioButton_10->isChecked())
    softPwmWrite(LED,90);
else if(ui->radioButton_11->isChecked())
    softPwmWrite(LED,100);

    digitalWrite(en,LOW);
    digitalWrite(dir,LOW);
    for (int i=0;i<20000;i++)
    {
            digitalWrite(steps, HIGH);
            QThread::usleep(15);
            digitalWrite(steps, LOW);
            //QThread::usleep(20);
            read[i]=readadc();
            qDebug()<<read[i];
        }
    for (int i=5;i<20000;i++)
    {
           filtdata[i]=(read[i]+read[i-1]+read[i-2]+read[i-3]+read[i-4]+read[i-5])/5;
        }
     digitalWrite(en,HIGH);
     softPwmWrite(LED,0);
     makePlot();

}

int MainWindow::readadc()
{
    unsigned char data[3]={0x06,0x00,0x00};
    wiringPiSPIDataRW(0,data,3);
    int adcValue=(data[1]&15)<<8|data[2];
    return adcValue;
    //qDebug()<<adcValue;
}


void MainWindow::makePlot()
{
    // generate some data:
    QVector<double> x(20000), y(20000), y1(20000);// initialize with entries 0..100
    for (int i=0; i<20000; ++i)
    {
//      x[i] = i/50.0 - 1; // x goes from -1 to 1
//      y[i] = x[i]*x[i]; // let's plot a quadratic function
        x[i]=i;
        y[i]=read[i];
        y1[i]=filtdata[i];
    }
    // create graph and assign data to it:
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(x,y);
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(QPen(Qt::red));
    ui->customPlot->graph(1)->setData(x,y1);
    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(3000, 7000);
    ui->customPlot->yAxis->setRange(0, 100);
    ui->customPlot->replot();

}
