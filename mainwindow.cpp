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

static double read[20000];
static double filtdata[20000];

static int conc[100];
static int dat[100];
static int len=0;

static int opt=0;




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
     ui->stackedWidget->setCurrentIndex(0);
     QSqlDatabase sqdb = QSqlDatabase::addDatabase("QSQLITE");
     sqdb.setDatabaseName("/home/pi/git/stepper_test/FIA.db");
     if(!sqdb.open())
         {
             qDebug() << "Can't Connect to DB !";
         }
         else
         {
             qDebug() << "Connected Successfully to DB !";
     }
     setWindowFlags(Qt::FramelessWindowHint);
     ui->customPlot->setBackground(QColor(0,0,26));
     ui->customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
     ui->customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
     ui->customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
     ui->customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
//     ui->customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
//     ui->customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
     ui->customPlot->xAxis->setTickLabelColor(Qt::white);
     ui->customPlot->yAxis->setTickLabelColor(Qt::white);
//     ui->customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
//     ui->customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
//     ui->customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
//     ui->customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
//     ui->customPlot->xAxis->grid()->setSubGridVisible(true);
//     ui->customPlot->yAxis->grid()->setSubGridVisible(true);
//     ui->customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
//     ui->customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
//     ui->customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
//     ui->customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    //Homing
    unsigned long homing_speed=0;
    QSqlQuery query;
    query.prepare("select homespeed from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        homing_speed=query.value(0).toUInt();
    }
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
            QThread::usleep(homing_speed);
            digitalWrite(steps, LOW);
            QThread::usleep(homing_speed);
        }

    }
    digitalWrite(en,HIGH);
}

void MainWindow::on_pushButton_2_clicked()
{
    //Initialization
    unsigned long homing_speed=0;
    QSqlQuery query;
    query.prepare("select homespeed from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        homing_speed=query.value(0).toUInt();
    }
    on_pushButton_clicked();

    digitalWrite(en,LOW);
    digitalWrite(dir,LOW);
    for (int i=0;i<12000;i++)
    {
            digitalWrite(steps, HIGH);
            QThread::usleep(homing_speed);
            digitalWrite(steps, LOW);
            QThread::usleep(homing_speed);
        }
     digitalWrite(en,HIGH);

}

void MainWindow::on_pushButton_3_clicked()
{


    //reading
    int intensity=0, win_start=0,win_end=0;
    unsigned long reading_speed=0;
    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select intensity, samprate, cutoff,readspeed,startregion,endregion from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        intensity=query.value(0).toInt();
        samplingrate=query.value(1).toDouble();
        cutoff_frequency=query.value(2).toDouble();
        reading_speed=query.value(3).toUInt();
        win_start=query.value(4).toInt();
        win_end=query.value(5).toInt();
    }
    on_pushButton_clicked();
QThread::sleep(1);
const int order = 2; // 4th order (=2 biquads)
Iir::Butterworth::LowPass<order> f;
//const float samplingrate = 1500; // Hz
//const float cutoff_frequency = 4; // Hz
f.setup (samplingrate, cutoff_frequency);

    digitalWrite(en,LOW);
    digitalWrite(dir,LOW);
    for (int i=0;i<12000;i++)
    {
         if(i>win_start && i<win_end)
         {
             pwmWrite (LED, intensity);
         }
         else
         {
             pwmWrite (LED, 0);
         }
         digitalWrite(steps, HIGH);
         QThread::usleep(reading_speed);
         digitalWrite(steps, LOW);
         QThread::usleep(reading_speed);

         read[i]=readadc();
         filtdata[i]=f.filter(read[i]);
         //qDebug()<<read[i];
    }
    digitalWrite(en,HIGH);
    pwmWrite (LED, 0);
    if(ui->radioButton_2->isChecked())
    {
        double x1=0,y1=0,x2=0,y2=0;
        QSqlQuery query;
        query.prepare("select minraw, maxraw, mincal, maxcal from FIA where sno=1");
        query.exec();
        while(query.next())
        {
            x1=query.value(0).toInt();
            x2=query.value(1).toInt();
            y1=query.value(2).toInt();
            y2=query.value(3).toInt();
        }
        double slope=(y2-y1)/(x2-x1);
        double intercept=y1-(slope*x1);
        for (int i=0;i<12000;i++)
        {
            filtdata[i]=(slope*(filtdata[i]+intercept));
            if(filtdata[i]<0)
                filtdata[i]=0;
            qDebug()<<filtdata[i];
        }
    }


        query.prepare("select conc, data from FIA");
        query.exec();
        len=0;
        while(query.next())
        {
            conc[len]=query.value(0).toInt();
            dat[len]=query.value(1).toInt();
            len++;
        }
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
    int win_start=0,win_end=0;
    QSqlQuery query;
    query.prepare("select startregion, endregion from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        win_start=query.value(0).toInt();
        win_end=query.value(1).toInt();
    }
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

    int control=0, test=0, cr=0,cr1=0,cr2=0,tr=0,tr1=0,tr2=0;


    for(int i=pos1-10;i<pos1+10;i++)
    {
        control+=y1[i];
    }
   control=control/20;
   for(int i=pos1-220;i<pos1-200;i++)
   {
       cr1+=y1[i];
   }
    cr1=cr1/20;
    for(int i=pos1+200;i<pos1+220;i++)
    {
        cr2+=y1[i];
    }
     cr2=cr2/20;
     cr=(cr1+cr2)/2;
    for(int i=pos2-10;i<pos2+10;i++)
    {
        test+=y1[i];
    }
    test=test/20;

    for(int i=pos2-220;i<pos2-200;i++)
    {
        tr1+=y1[i];
    }
     tr1=tr1/20;
     for(int i=pos2+200;i<pos2+220;i++)
     {
         tr2+=y1[i];
     }
      tr2=tr2/20;
      tr=(tr1+tr2)/2;

      if(ui->radioButton_6->isChecked())
      {
          qDebug()<<control<<cr<<test<<tr;
          control=control-cr;
          if(control<0)
              control=0;
          test=test-tr;
          if(test<0)
              test=0;
      }

    int max=0;
    if(control>test)
        max=control;
    else max=test;

    //Qantitative readout
    int point=0;
    for(int i=0;i<len;i++)
    {
        if(test<dat[i])
        {
            point=i;
            break;
        }
        else if(test>dat[len-1])
        {
            point=len;
            break;
        }
    }
    qDebug()<<"POINT="<<point;
    double result=0;
    if(point==0)
    {
        result=conc[0];
        QString res=QString::number(result);
        ui->label_24->setText("< "+res);
    }

    else if(point==len)
    {
        result=conc[len];
        QString res=QString::number(result);
        ui->label_24->setText("> "+res);
    }
    else
    {
        double x1=0,y1=0,x2=0,y2=0;

        x1=dat[point-1];
        x2=dat[point];
        y1=conc[point-1];
        y2=conc[point];
        double slope=(y2-y1)/(x2-x1);
        double intercept=y1-(slope*x1);
        result=slope*(test+intercept);
        ui->label_24->setText(QString::number(result, 'f', 2));
    }


    ui->label->setNum(test);
     ui->label_2->setNum(control);
    // create graph and assign data to it:
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(x,y);
    ui->customPlot->graph(0)->setVisible(false);
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(QPen(Qt::red,3));
    ui->customPlot->graph(1)->setData(x,y1);
    //vertical  line 2
    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setPen(QPen(Qt::white,3));
    ui->customPlot->graph(2)->setData(xv1,yv1);
    //vertical line 2
    ui->customPlot->addGraph();
    ui->customPlot->graph(3)->setPen(QPen(Qt::white,3));
    ui->customPlot->graph(3)->setData(xv2,yv2);

    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(win_start, win_end);
    ui->customPlot->yAxis->setRange(0, max+100);
    ui->customPlot->replot();

}

int MainWindow::linearity(int val)
{
    int x1=0,y1=0,x2=0,y2=0;
    QSqlQuery query;
    query.prepare("select minraw, maxraw, mincal, maxcal from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        x1=query.value(0).toInt();
        x2=query.value(1).toInt();
        y1=query.value(2).toInt();
        y2=query.value(3).toInt();
    }
    int slope=(y2-y1)/(x2-x1);
    int intercept=y1-(slope*x1);
    int result=slope*(val+intercept);
    return result;

}

void MainWindow::on_pushButton_19_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Intensity[0-1024]");
    opt=1;
    int intensity=0;
    QSqlQuery query;
    query.prepare("select intensity from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        intensity=query.value(0).toInt();
    }
    QString ity=QString::number(intensity);
    ui->lineEdit_9->setText(ity);
}

void MainWindow::on_toolButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->toolButton_2->setChecked(false);
    ui->toolButton_4->setChecked(false);
}

void MainWindow::on_toolButton_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->toolButton->setChecked(false);
    ui->toolButton_4->setChecked(false);
    int intensity=0,samprate=0,cutoff=0,homing_speed=0,reading_speed=0,win_start=0,win_end=0;

    QSqlQuery query;
    query.prepare("select intensity, samprate, cutoff, homespeed,readspeed, startregion,endregion from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        intensity=query.value(0).toInt();
        samprate=query.value(1).toInt();
        cutoff=query.value(2).toInt();
        homing_speed=query.value(3).toInt();
        reading_speed=query.value(4).toInt();
        win_start=query.value(5).toInt();
        win_end=query.value(6).toInt();
    }
    QString ity=QString::number(intensity);
    QString srt=QString::number(samprate);
    QString cut=QString::number(cutoff);
    QString hms=QString::number(homing_speed);
    QString rds=QString::number(reading_speed);
    QString wis=QString::number(win_start);
    QString wie=QString::number(win_end);

    ui->lineEdit->setText(ity);
    ui->lineEdit_3->setText(srt);
    ui->lineEdit_4->setText(cut);
    ui->lineEdit_5->setText(hms);
    ui->lineEdit_6->setText(rds);
    ui->lineEdit_7->setText(wis);
    ui->lineEdit_8->setText(wie);

}


void MainWindow::on_pushButton_21_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Sampling Rate");
    opt=2;
    int samprate=0;
    QSqlQuery query;
    query.prepare("select samprate from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        samprate=query.value(0).toInt();
    }
    QString srt=QString::number(samprate);
    ui->lineEdit_9->setText(srt);
}

void MainWindow::on_pushButton_22_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Cut Off Freq.");
    opt=3;
    int cutoff=0;
    QSqlQuery query;
    query.prepare("select cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        cutoff=query.value(0).toInt();
    }
    QString cut=QString::number(cutoff);
    ui->lineEdit_9->setText(cut);
}

void MainWindow::on_pushButton_23_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Homing Speed");
    opt=4;
    int homespeed=0;
    QSqlQuery query;
    query.prepare("select homespeed from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        homespeed=query.value(0).toInt();
    }
    QString hms=QString::number(homespeed);
    ui->lineEdit_9->setText(hms);
}

void MainWindow::on_pushButton_24_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Reading Speed");
    opt=5;
    int readspeed=0;
    QSqlQuery query;
    query.prepare("select readspeed from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        readspeed=query.value(0).toInt();
    }
    QString rds=QString::number(readspeed);
    ui->lineEdit_9->setText(rds);
}

void MainWindow::on_pushButton_25_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Starting Region");
    opt=6;
    int startregion=0;
    QSqlQuery query;
    query.prepare("select startregion from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        startregion=query.value(0).toInt();
    }
    QString wis=QString::number(startregion);
    ui->lineEdit_9->setText(wis);
}

void MainWindow::on_pushButton_26_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Ending Region");
    opt=7;
    int endregion=0;
    QSqlQuery query;
    query.prepare("select endregion from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        endregion=query.value(0).toInt();
    }
    QString wie=QString::number(endregion);
    ui->lineEdit_9->setText(wie);
}

void MainWindow::on_pushButton_4_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"1");
}

void MainWindow::on_pushButton_18_clicked()
{
    ui->lineEdit_9->backspace();
}

void MainWindow::on_pushButton_5_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"2");
}

void MainWindow::on_pushButton_6_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"3");
}

void MainWindow::on_pushButton_11_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"4");
}

void MainWindow::on_pushButton_12_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"5");
}

void MainWindow::on_pushButton_10_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"6");
}

void MainWindow::on_pushButton_14_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"7");
}

void MainWindow::on_pushButton_15_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"8");
}

void MainWindow::on_pushButton_13_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"9");
}

void MainWindow::on_pushButton_16_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"0");
}

void MainWindow::on_pushButton_17_clicked()
{
        QString val=ui->lineEdit_9->text();
        QSqlQuery query;

        if(opt==1)
            query.prepare("update FIA set intensity=:val where sno=1");
        else if(opt==2)
             query.prepare("update FIA set samprate=:val where sno=1");
        else if(opt==3)
            query.prepare("update FIA set cutoff=:val where sno=1");
        else if(opt==4)
            query.prepare("update FIA set homespeed=:val where sno=1");
        else if(opt==5)
            query.prepare("update FIA set readspeed=:val where sno=1");
        else if(opt==6)
            query.prepare("update FIA set startregion=:val where sno=1");
        else if(opt==7)
            query.prepare("update FIA set endregion=:val where sno=1");
        else if(opt==8)
            query.prepare("update FIA set minraw=:val where sno=1");
        else if(opt==9)
            query.prepare("update FIA set maxraw=:val where sno=1");
        else if(opt==10)
            query.prepare("update FIA set mincal=:val where sno=1");
        else if(opt==11)
            query.prepare("update FIA set maxcal=:val where sno=1");

        query.bindValue(":val",val);
        query.exec();
        if(opt==8||opt==9||opt==10||opt==11)
        {
            on_toolButton_4_clicked();
        }
        else
        {
          on_toolButton_2_clicked();
        }
}

void MainWindow::on_toolButton_3_clicked()
{
    qApp->exit();
}

void MainWindow::on_toolButton_4_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->toolButton->setChecked(false);
    ui->toolButton_2->setChecked(false);
    int minraw=0,maxraw=0,mincal=0,maxcal=0;

    QSqlQuery query;
    query.prepare("select minraw,maxraw,mincal,maxcal from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        minraw=query.value(0).toInt();
        maxraw=query.value(1).toInt();
        mincal=query.value(2).toInt();
        maxcal=query.value(3).toInt();

    }
    QString min_raw=QString::number(minraw);
    QString max_raw=QString::number(maxraw);
    QString min_cal=QString::number(mincal);
    QString max_cal=QString::number(maxcal);

    ui->lineEdit_2->setText(min_raw);
    ui->lineEdit_11->setText(max_raw);
    ui->lineEdit_12->setText(min_cal);
    ui->lineEdit_13->setText(max_cal);
}

void MainWindow::on_pushButton_20_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Min Raw data");
    opt=8;
    int intensity=0;
    QSqlQuery query;
    query.prepare("select minraw from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        intensity=query.value(0).toInt();
    }
    QString ity=QString::number(intensity);
    ui->lineEdit_9->setText(ity);
}

void MainWindow::on_pushButton_28_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Max Raw data");
    opt=9;
    int intensity=0;
    QSqlQuery query;
    query.prepare("select maxraw from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        intensity=query.value(0).toInt();
    }
    QString ity=QString::number(intensity);
    ui->lineEdit_9->setText(ity);
}

void MainWindow::on_pushButton_29_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Min Cal. data");
    opt=10;
    int intensity=0;
    QSqlQuery query;
    query.prepare("select mincal from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        intensity=query.value(0).toInt();
    }
    QString ity=QString::number(intensity);
    ui->lineEdit_9->setText(ity);
}

void MainWindow::on_pushButton_30_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Max Cal. data");
    opt=11;
    int intensity=0;
    QSqlQuery query;
    query.prepare("select maxcal from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        intensity=query.value(0).toInt();
    }
    QString ity=QString::number(intensity);
    ui->lineEdit_9->setText(ity);
}

void MainWindow::on_toolButton_5_clicked()
{
    setWindowState(Qt::WindowMinimized);
}
