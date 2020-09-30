#include "mainwindow.h"
#include "ui_mainwindow.h"

#define steps 7
#define dir 0
#define en 3
#define hm_sen 4

#define LED 26
#define init 0
#define range 100

#define BARCODE_GND 21

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
    //softPwmCreate(LED, init,range);
     pinMode (en, OUTPUT) ;
     pinMode (dir, OUTPUT) ;
     pinMode (steps, OUTPUT) ;
     pinMode (hm_sen, INPUT) ;
     pinMode (BARCODE_GND, OUTPUT) ;
     //softPwmWrite(LED,0);
     digitalWrite(en,HIGH);

     digitalWrite(BARCODE_GND,LOW);
     pinMode (LED, PWM_OUTPUT);
     pwmWrite (LED, 0);
     ui->radioButton_7->setChecked(true);

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
    for (int i=0;i<15000;i++)
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
    for (int i=0;i<12000;i++)
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
    QThread::sleep(3);
if(ui->radioButton_2->isChecked())
    pwmWrite (LED, 50);
else if(ui->radioButton_3->isChecked())
    pwmWrite (LED, 100);
else if(ui->radioButton_4->isChecked())
    pwmWrite (LED, 150);
else if(ui->radioButton_5->isChecked())
    pwmWrite (LED, 200);
else if(ui->radioButton_6->isChecked())
    pwmWrite (LED, 250);
else if(ui->radioButton_7->isChecked())
    pwmWrite (LED, 300);
else if(ui->radioButton_8->isChecked())
    pwmWrite (LED, 350);
else if(ui->radioButton_9->isChecked())
    pwmWrite (LED, 400);
else if(ui->radioButton_10->isChecked())
    pwmWrite (LED, 450);
else if(ui->radioButton_11->isChecked())
    pwmWrite (LED, 500);

    digitalWrite(en,LOW);
    digitalWrite(dir,LOW);
    for (int i=0;i<12000;i++)
    {
            digitalWrite(steps, HIGH);
//            if(i>1500 && i<4000)
//                QThread::usleep(1000);
//            else
                QThread::usleep(30);
            digitalWrite(steps, LOW);
//            if(i>1500 && i<4000)
//                QThread::usleep(1000);
//            else
                QThread::usleep(30);
            read[i]=readadc();
            qDebug()<<read[i];
        }
//    for (int i=5;i<10000;i++)
//    {
//           filtdata[i]=(read[i]+read[i-1]+read[i-2]+read[i-3]+read[i-4]+read[i-5])/5;
//        }
    for (int i=0;i<11995;i=i+5)
    {
           filtdata[i]=(read[i]+read[i+1]+read[i+2]+read[i+3]+read[i+4]+read[i+5])/5;
           filtdata[i+1]=filtdata[i];
           filtdata[i+2]=filtdata[i];
           filtdata[i+3]=filtdata[i];
           filtdata[i+4]=filtdata[i];
           filtdata[i+5]=filtdata[i];

        }
     digitalWrite(en,HIGH);
     pwmWrite (LED, 0);
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
    for (int i=0; i<12000; ++i)
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
    ui->customPlot->xAxis->setRange(0, 12000);
    ui->customPlot->yAxis->setRange(0, 1500);
    ui->customPlot->replot();

}
