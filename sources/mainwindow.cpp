#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColor>
#include <QPoint>
#include <QPainter>
#include <QImage>

#include <QTableWidgetItem>

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>

#include <QDesktopServices>
#include <QDebug>

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
    m_gap = 100;
    m_scale = 100;

    m_row = 6;
    m_col = 4;

    m_inputDir = "resources/input/";
    m_outputDir = "resources/output/";
    SetOutputDir();

    pFormCanvas = new FormCanvas();
    pFormCanvas->setParent(ui->wgtCanvas);
    SetCanvas();

    ReadImageList();
    GenInputMat();      //数值框要设置最大值，需先读取图像数量

    //图像列表不可编辑
    ui->tabImgList->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::InitConnections()
{
    connect(ui->pbtnGen, SIGNAL(clicked()), this, SLOT(InitCanvas()));
    connect(ui->pbtnDuck, SIGNAL(clicked()), this, SLOT(DrawAllImage()));
    connect(ui->pbtnSave, SIGNAL(clicked()), this, SLOT(SaveImage()));
    connect(ui->pbtnOpenDir, SIGNAL(clicked()), this, SLOT(OpenImageDir()));

    connect(ui->spinGridSize, SIGNAL(valueChanged(int)), this, SLOT(ChangeGridSize(int)));
    connect(ui->spinImgScale, SIGNAL(valueChanged(int)), this, SLOT(ChangeImageScale(int)));

    connect(ui->spinSignScale, SIGNAL(valueChanged(int)), this, SLOT(ChangeSignScale(int)));
    connect(ui->combSign, SIGNAL(currentIndexChanged(int)), this, SLOT(DrawSign(int)));

    ConnectSpinMat();
}

void MainWindow::InitCanvas()
{
    m_pixmap.fill(Qt::white);
    pFormCanvas->PaintPixmap(m_pixmap);

    return;
}

void MainWindow::DrawSign(int signNo)
{
    QStringList signList = {
        "",
        "plus.png",
        "up.png",
        "down.png"
    };
    QPixmap pixmap;
    if(signNo < 0 || signNo >= signList.length())
        return;
    //清空原位置
    pixmap = QPixmap(m_gap, m_gap);
    pixmap.fill(Qt::white);

    QPainter painter(&m_pixmap);
    int x = ((m_col+2)*m_gridSize - m_gap)>>1;
    int y = ((m_row+3)*m_gridSize - m_gap)>>1;
    painter.drawPixmap(x, y, pixmap);
    pFormCanvas->PaintPixmap(m_pixmap);

    if(signNo == 0)     //无连接符号
        return;

    //有连接符号
    QImage image;
    image.load(m_inputDir+signList[signNo]);
    if(image.isNull())      //图片不存在
        return;

    int scale = ui->spinSignScale->value();
    int size = int(m_gap*scale/100);
    pixmap = QPixmap::fromImage(image.scaled(size, size, Qt::KeepAspectRatio));

    x = ((m_col+2)*m_gridSize - size)>>1;
    y = ((m_row+3)*m_gridSize - size)>>1;
    painter.drawPixmap(x, y, pixmap);
    pFormCanvas->PaintPixmap(m_pixmap);

    return;
}

void MainWindow::DrawSign()
{
    int index = ui->combSign->currentIndex();
    DrawSign(index);
}

void MainWindow::DrawImage(int row, int col, int imgNo)
{
    /*
    QStringList imgList = {
        "resources/input/duck_blue.png",
        "resources/input/bf_blue.png",
        "resources/input/bf_red.png"
    };*/
    ReadImageList();
    if(imgNo < 0 || imgNo >= m_imgList.length())
        return;

    //qDebug() <<"draw " << row <<" "<< col <<" "<< imgNo;

    QImage image;
    image.load(m_imgList[imgNo]);
    if(image.isNull())      //图片不存在
        return;

    int size = m_gridSize, imgSize = int(size*m_scale/100);
    QPixmap pixmap = QPixmap::fromImage(image.scaled(imgSize, imgSize, Qt::KeepAspectRatio));

    QPainter painter(&m_pixmap);
    int offset = (size - imgSize)/2;
    int gap = (row >= m_row/2) ? size : 0;
    painter.drawPixmap((col+1)*size+offset, gap+(row+1)*size+offset, pixmap);
    pFormCanvas->PaintPixmap(m_pixmap);

    return;
}

void MainWindow::SaveImage()
{
    QString imgName = "output_image";
    QString imgType = "png";
    QString imgPath = QFileDialog::getSaveFileName(
                this, "保存图片",
                m_outputDir + imgName,
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
    m_outputDir = imgDir;

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
    //打开文件夹
    QDesktopServices::openUrl(QUrl(dirPath));

    return;
}

void MainWindow::ReadImageList()
{
    QString filePath = m_inputDir+"image_list.txt";
    QFile file(filePath);
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "failed to open " + filePath;
        return;
    }

    QString imgDir = m_inputDir;
    m_imgList.clear();
    QString imgPath;
    //读取文件中的图片路径
    QTextStream txtin(&file);
    while(!txtin.atEnd()){
        imgPath = txtin.readLine();
        if(imgPath != "")
            m_imgList.push_back(imgDir+imgPath);
        //qDebug() << imgDir + imgPath;
    }
    file.close();

    FillImageList();
    return;
}

void MainWindow::FillImageList()
{
    int n = m_imgList.length();
    ui->tabImgList->setRowCount(n);
    //行表头
    QStringList VHeaders;
    VHeaders.reserve(n);
    for(int i = 0; i < n; i++)
        VHeaders.push_back(QString::number(i));
    ui->tabImgList->setVerticalHeaderLabels(VHeaders);
    //表格内容
    for(int i = 0; i < n; i++){
        ui->tabImgList->setItem(0, i, new QTableWidgetItem(m_imgList[i]));
    }
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
            pSpin->setRange(0, m_imgList.length()-1);
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

void MainWindow::DrawAllImage()
{
    for(int i = 0; i < m_row; i++){
        for(int j = 0; j < m_col; j++){
            DrawImage(i, j, m_pSpinMat[i][j]->value());
        }
    }
    return;
}

void MainWindow::SetOutputDir()
{
    QDir dir(m_outputDir);
    if(!dir.exists())
        return;
    ui->ptxtPath->setPlainText(dir.absolutePath());
    return;
}

void MainWindow::SetCanvas()
{
    int w = m_gridSize*(m_col+2);
    int h = m_gridSize*(m_row+2) + m_gap;

    ui->wgtCanvas->setFixedSize(QSize(w, h));
    pFormCanvas->setFixedSize(QSize(w, h));
    m_pixmap = QPixmap(w, h);
    return;
}

void MainWindow::ChangeGridSize(int val)
{
    m_gridSize = val;
    m_gap = m_gridSize;

    SetCanvas();

    m_pixmap.fill(Qt::white);   //清空画布
    DrawAllImage();     //重画图像
    DrawSign();         //重画连接符号
    return;
}

void MainWindow::ChangeImageScale(int val)
{
    if(val <= 10)
        return;
    m_scale = val;

    m_pixmap.fill(Qt::white);   //清空画布
    DrawAllImage();     //重画图像
    DrawSign();         //重画连接符号
    return;
}

void MainWindow::ChangeSignScale(int val)
{
    if(val <= 10)
        return;
    DrawSign();
}
