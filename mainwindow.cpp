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

static int read[20000];
static int filtdata[20000];
static int win_start=6800;
static int win_end=9000;
static int speed=30;



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
QThread::sleep(1);
const int order = 4; // 4th order (=2 biquads)
Iir::Butterworth::LowPass<order> f;
const double samplingrate = 1000; // Hz
const double cutoff_frequency = 5; // Hz
f.setup (samplingrate, cutoff_frequency);

    digitalWrite(en,LOW);
    digitalWrite(dir,LOW);
    for (int i=0;i<12000;i++)
    {
         if(i>win_start && i<win_end)
             if(ui->radioButton_2->isChecked())
                 pwmWrite (LED, 300);
             else if(ui->radioButton_3->isChecked())
                 pwmWrite (LED, 310);
             else if(ui->radioButton_4->isChecked())
                 pwmWrite (LED, 320);
             else if(ui->radioButton_5->isChecked())
                 pwmWrite (LED, 330);
             else if(ui->radioButton_6->isChecked())
                 pwmWrite (LED, 340);
             else if(ui->radioButton_7->isChecked())
                 pwmWrite (LED, 350);
             else if(ui->radioButton_8->isChecked())
                 pwmWrite (LED, 360);
             else if(ui->radioButton_9->isChecked())
                 pwmWrite (LED, 370);
             else if(ui->radioButton_10->isChecked())
                 pwmWrite (LED, 380);
             else
                 pwmWrite (LED, 390);
         else {
             pwmWrite (LED, 0);
         }
            digitalWrite(steps, HIGH);
//            if(i>1500 && i<4000)
//                QThread::usleep(1000);
//            else
                QThread::usleep(speed);
            digitalWrite(steps, LOW);
//            if(i>1500 && i<4000)
//                QThread::usleep(1000);
//            else
                QThread::usleep(speed);
//                if(i%50==0)
//                    read[i]=readadc();
//                else {
//                    read[i]=read[i-1];
//                }
                read[i]=readadc();
                filtdata[i]=f.filter(read[i]);
            qDebug()<<read[i];
        }
//    for (int i=5;i<10000;i++)
//    {
//           filtdata[i]=(read[i]+read[i-1]+read[i-2]+read[i-3]+read[i-4]+read[i-5])/5;
//        }
//    for (int i=0;i<12000;i++)
//    {

//            for(int t=0;t<=100;t++)
//            {
//                filtdata[i]+=read[i+t];
//            }
//            filtdata[i]=filtdata[i]/100;

//        }
     digitalWrite(en,HIGH);
     //pwmWrite (LED, 0);
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
    for (int i=0; i<12000; i++)
    {
//      x[i] = i/50.0 - 1; // x goes from -1 to 1
//      y[i] = x[i]*x[i]; // let's plot a quadratic function
        x[i]=i;
        y[i]=read[i];
        y1[i]=filtdata[i];
    }
    double temp1=0;
    int pos1=0;
    for (int i=win_start;i<(win_start+(win_end-win_start)/2)-50;i++)
    {

      if(temp1<y1[i])
      {
          temp1=y1[i];
          pos1=i;
      }

    }

    double temp2=0;
    int pos2=0;
    for (int i=(win_start+(win_end-win_start)/2)+50;i<win_end;i++)
    {

      if(temp2<y1[i])
      {
          temp2=y1[i];
          pos2=i;
      }

    }
    qDebug()<<temp1<<temp2;
    qDebug()<<pos1<<pos2;
    QVector<double> xv1(2);
    QVector<double> yv1(2);
    QVector<double> xv2(2);
    QVector<double> yv2(2);
    xv1[0]=xv1[1]=pos1;
    xv2[0]=xv2[1]=pos2;
    yv1[0]=yv2[0]=0;
    yv1[1]=temp1;
    yv2[1]=temp2;

    int test=0;
    for(int i=pos1-10;i<pos1+10;i++)
    {
        test+=y1[i];
    }
    test=test/20;
    int control=0;
    for(int i=pos2-10;i<pos2+10;i++)
    {
        control+=y1[i];
    }
    control=control/20;

    int max=0;
    if(control>test)
        max=control;
    else max=test;

    ui->label->setNum(test);
     ui->label_2->setNum(control);
    // create graph and assign data to it:
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(x,y);
    ui->customPlot->graph(0)->setVisible(false);
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(QPen(Qt::red));
    ui->customPlot->graph(1)->setData(x,y1);
    //vertical  line 2
    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setPen(QPen(Qt::blue));
    ui->customPlot->graph(2)->setData(xv1,yv1);
    //vertical line 2
    ui->customPlot->addGraph();
    ui->customPlot->graph(3)->setPen(QPen(Qt::blue));
    ui->customPlot->graph(3)->setData(xv2,yv2);

    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(win_start, win_end);
    ui->customPlot->yAxis->setRange(0, max+100);
    ui->customPlot->replot();

}
