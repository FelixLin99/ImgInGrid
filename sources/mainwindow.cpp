#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QPainter>
#include <QImage>
#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QDebug>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InitMembers();
    InitConnections();
}

MainWindow::~MainWindow()
{
    DeleteSpinMat();
    delete pFormCanvas;

    delete ui;
}

void MainWindow::InitMembers()
{
    m_gridSize = 100;
    m_scale = 100;

    int w = 400, h = 600;
    m_pixmap = QPixmap(w, h);
    pFormCanvas = new FormCanvas();
    pFormCanvas->setFixedWidth(w);
    pFormCanvas->setFixedHeight(h);

    pFormCanvas->setParent(ui->wgtCanvas);
    ui->wgtCanvas->setFixedWidth(w);
    ui->wgtCanvas->setFixedHeight(h);

    GenInputMat();
}

void MainWindow::InitConnections()
{
    connect(ui->pbtnGen, SIGNAL(clicked()), this, SLOT(InitCanvas()));
    connect(ui->pbtnDuck, SIGNAL(clicked()), this, SLOT(InitImage()));
    connect(ui->pbtnSave, SIGNAL(clicked()), this, SLOT(SaveImage()));
    connect(ui->pbtnOpenDir, SIGNAL(clicked()), this, SLOT(OpenImageDir()));

    connect(ui->spinGridSize, SIGNAL(valueChanged(int)), this, SLOT(ChangeGridSize(int)));
    connect(ui->spinScale, SIGNAL(valueChanged(int)), this, SLOT(ChangeScale(int)));

    ConnectSpinMat();
}

void MainWindow::InitCanvas()
{
    m_pixmap.fill(Qt::white);
    pFormCanvas->PaintPixmap(m_pixmap);

    return;
}

void MainWindow::DrawImage(int row, int col, int imgNo)
{
    QStringList imgPath = {
        ":/input/blue_duck.png",
        ":/input/blue_bf.png",
        ":/input/red_bf.png"
    };
    if(imgNo < 0 || imgNo >= imgPath.length())
        return;

    //qDebug() <<"draw " << row <<" "<< col <<" "<< imgNo;

    QImage image;
    image.load(imgPath[imgNo]);

    int size = m_gridSize, imgSize = int(size*m_scale/100);
    QPixmap pixmap = QPixmap::fromImage(image.scaled(imgSize, imgSize, Qt::KeepAspectRatio));

    QPainter painter(&m_pixmap);
    int offset = (size - imgSize)/2;
    painter.drawPixmap(col*size+offset, row*size+offset, pixmap);
    pFormCanvas->PaintPixmap(m_pixmap);

    return;
}

void MainWindow::SaveImage()
{
    QString imgName = "output_image";
    QString imgType = "png";
    QString imgPath = QFileDialog::getSaveFileName(
                this, "保存图片",
                "resources/output/" + imgName,
                "PNG 图片 (*.png)"
                );

    //关闭对话框，则路径为空，退出
    if(imgPath.isEmpty()){
        return;
    }
    qDebug() << imgPath;

    //检查后缀，若不是合法图片后缀，则补上
    int suf_pos = imgPath.lastIndexOf(".");
    QString suffix = imgPath.right(imgPath.length()-suf_pos-1);
    if(suffix != imgType){
        qDebug() << imgPath;
        imgPath += "." + imgType;
    }
    //保存图片：质量因数 80（1-100，数值越大，质量越高）
    m_pixmap.save(imgPath, Q_NULLPTR, 80);

    QFileInfo info(imgPath);
    QString imgDir = info.path();
    ui->ptxtPath->setPlainText(imgDir);

    return;
}

//打开图片文件夹
void MainWindow::OpenImageDir()
{
    QString dirPath = ui->ptxtPath->toPlainText();
    qDebug() << dirPath;
    QDir dir(dirPath);
    if(!dir.exists())
        return;
    QDesktopServices::openUrl(QUrl(dirPath));

    return;
}

void MainWindow::GenInputMat()
{
    m_row = 6;
    m_col = 4;
    ui->tabInput->setRowCount(m_row);
    ui->tabInput->setColumnCount(m_col);

    m_pSpinMat = new QSpinBox**[m_row];
    for(int i = 0; i < m_row; i++){
        m_pSpinMat[i] = new QSpinBox*[m_col];
        for(int j = 0; j < m_col; j++){
            QSpinBox *pSpin = new QSpinBox();
            pSpin->setValue(0);
            m_pSpinMat[i][j] = pSpin;
            ui->tabInput->setCellWidget(i, j, pSpin);
        }
    }
    int width = 80;
    for(int j = 0; j < m_col; j++)
        ui->tabInput->setColumnWidth(j, width);

    return;
}

void MainWindow::ConnectSpinMat()
{
    for(int i = 0; i < m_row; i++){
        for(int j = 0; j < m_col; j++){
            //valueChanged有重载，需加显式类型转换
            connect(m_pSpinMat[i][j], static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](){
                DrawImage(i, j, m_pSpinMat[i][j]->value());
            });
        }
    }
}

void MainWindow::DeleteSpinMat()
{
    for(int i = 0; i < m_col; i++)
        delete []m_pSpinMat[i];
    delete []m_pSpinMat;
}

void MainWindow::InitImage()
{
    for(int i = 0; i < m_row; i++){
        for(int j = 0; j < m_col; j++){
            DrawImage(i, j, m_pSpinMat[i][j]->value());
        }
    }
    return;
}

void MainWindow::ChangeGridSize(int val)
{
    m_gridSize = val;
    int w = m_gridSize*m_col;
    int h = m_gridSize*m_row;
    ui->wgtCanvas->setFixedSize(QSize(w, h));
    pFormCanvas->setFixedSize(QSize(w, h));
    m_pixmap = QPixmap(w, h);
    m_pixmap.fill(Qt::white);
    InitImage();
}

void MainWindow::ChangeScale(int val)
{
    if(val <= 10)
        return;
    m_pixmap.fill(Qt::white);
    m_scale = val;
    InitImage();
}
