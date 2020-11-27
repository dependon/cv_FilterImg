#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QPainter>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString path=QFileDialog::getOpenFileName();
    if(m_img)
    {
        delete m_img;
        m_img=nullptr;
    }
    m_img=new QImage(path);

    ui->label->setPixmap(QPixmap::fromImage(*m_img).scaled(800,600));
    update();
}

void MainWindow::on_pushButton_2_clicked()
{
    QVector <QRect> rectVec= api::getFaceRect(*m_img,0);//获得人脸区域
    QImage *img1=new QImage(*m_img);
    if(rectVec.size()>0)
    {
        for(auto rec:rectVec)
        {
            qDebug()<<rec;

            QImage *img=new QImage(m_img->copy(rec));
            api::QImageD_RunBEEPSHorizontalVertical(img,img);//对人脸区域进行美颜
            QPainter painter;
            painter.begin(img1);
            painter.drawImage(rec.x(),rec.y(), *img);//处理过的人脸图像绘制到原图像
            painter.end();
            delete img;
            img=nullptr;

        }
    }
    //没检测到人脸，全局美颜
    else {
        api::QImageD_RunBEEPSHorizontalVertical(m_img,img1);//全局美颜
    }
    ui->label_2->setPixmap(QPixmap::fromImage(*img1).scaled(800,600));
    update();
}
