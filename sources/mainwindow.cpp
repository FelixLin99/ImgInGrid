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
#include <QTimer>
#include <QTime>
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
    SetQuit(0);
    m_gridSize = 100;
    m_gap = 100;
    m_scale = 100;

    m_row = 6;
    m_col = 4;

    m_inputDir = "resources/input/";
    m_outputDir = "resources/output/";
    SetOutputDir();

    m_signNameList = QStringList {
        "",
        "plus",
        "up",
        "down"
    };
    m_signList =  QStringList {
        "",
        "plus.png",
        "up.png",
        "down.png"
    };

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

    connect(ui->pbtn1Blue, SIGNAL(clicked()), this, SLOT(DrawBatch1Blue()));
    connect(ui->pbtn1Blue1Red, SIGNAL(clicked()), this, SLOT(DrawBatch1Blue1Red()));
    connect(ui->pbtnFin, SIGNAL(clicked()), this, SLOT(DrawBatchFin()));

    ConnectSpinMat();
}

void MainWindow::SetQuit(bool val)
{
    m_quit = val;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    SetQuit(1);
}

void MainWindow::InitCanvas()
{
    m_pixmap.fill(Qt::white);
    pFormCanvas->PaintPixmap(m_pixmap);

    return;
}

void MainWindow::DrawSign(int signNo, QPixmap &canvas, bool clean)
{
    if(signNo < 0 || signNo >= m_signList.length())
        return;
    //清空原位置
    QPainter painter(&canvas);
    QPixmap pixmap;
    int x, y;
    if(clean){
        pixmap = QPixmap(m_gap, m_gap);
        pixmap.fill(Qt::white);

        x = ((m_col+2)*m_gridSize - m_gap)>>1;
        y = ((m_row+3)*m_gridSize - m_gap)>>1;
        painter.drawPixmap(x, y, pixmap);
        pFormCanvas->PaintPixmap(canvas);
    }
    if(signNo == 0)     //无连接符号
        return;

    //有连接符号
    QImage image;
    image.load(m_inputDir+m_signList[signNo]);
    if(image.isNull())      //图片不存在
        return;

    int scale = ui->spinSignScale->value();
    int size = int(m_gap*scale/100);
    pixmap = QPixmap::fromImage(image.scaled(size, size, Qt::KeepAspectRatio));

    x = ((m_col+2)*m_gridSize - size)>>1;
    y = ((m_row+3)*m_gridSize - size)>>1;
    painter.drawPixmap(x, y, pixmap);
    return;
}

void MainWindow::DrawSign(int signNo)
{
    if(signNo < 0 || signNo >= m_signList.length())
        return;
    DrawSign(signNo, m_pixmap);
    pFormCanvas->PaintPixmap(m_pixmap);

    return;
}

void MainWindow::DrawSign()
{
    int index = ui->combSign->currentIndex();
    DrawSign(index);
}

void MainWindow::DrawImage(int row, int col, int imgNo, QPixmap &canvas)
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

    QPainter painter(&canvas);
    int offset = (size - imgSize)/2;
    int gap = (row >= m_row/2) ? size : 0;
    painter.drawPixmap((col+1)*size+offset, gap+(row+1)*size+offset, pixmap);

    return;
}

void MainWindow::DrawImage(int row, int col, int imgNo)
{
    DrawImage(row, col, imgNo, m_pixmap);
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

void MainWindow::GetCanvasSize(int &w, int &h)
{
    w = m_gridSize*(m_col+2);
    h = m_gridSize*(m_row+2) + m_gap;
    return;
}

void MainWindow::SetCanvas()
{
    int w, h;
    GetCanvasSize(w, h);
    m_pixmap = QPixmap(w, h);

    ui->wgtCanvas->setFixedSize(QSize(w, h));
    pFormCanvas->setFixedSize(QSize(w, h));
    return;
}

inline static int GetIndex(int total_col, int row, int col)
{
    return row*total_col + col;
}

void MainWindow::GetLastPos(int row, int col, int &last_row, int &last_col, int skip_row, int skip_col)
{
    if(row == 0 && col == 0){
        last_row = -1;
        last_col = -1;
        return;
    }

    int last_index = GetIndex(m_col, row, col) - 1;
    //跳过特定位置
    if(last_index == GetIndex(m_col, skip_row, skip_col))
        last_index--;

    last_row = last_index/m_col;
    last_col = last_index%m_col;
    return;
}

void MainWindow::GetLastPos(int row, int col, int &last_row, int &last_col)
{
    GetLastPos(row, col, last_row, last_col, 0, -1);
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

//暂停防卡死
static void TimerPause(QTime &time_1)
{
    //每运行一秒以上，暂停一段时间，防止窗口卡死
    QTime time_2 = QTime::currentTime();
    if(time_2.second() - time_1.second() != 0){
        time_1 = time_2;
        int msec = 100;     //100ms
        QEventLoop loop;
        QTimer::singleShot(msec, &loop, SLOT(quit()));
        loop.exec();
    }
    return;
}

void MainWindow::DrawBatch1Blue()
{
    int w, h;
    GetCanvasSize(w, h);
    QPixmap canvas = QPixmap(w, h);
    canvas.fill(Qt::white);

    //创建输出目录
    QString imgName, imgPath;
    QString outputDir = "1Blue/";
    QDir dir(m_outputDir);
    dir.mkpath(outputDir);
    outputDir = m_outputDir + outputDir;
    qDebug() << outputDir;
    //初始图片：蓝(0,0)
    for(int i = 0; i < m_row; i++){
        for(int j = 0; j < m_col; j++){
            if(i == 0 && j == 0)
                DrawImage(i, j, 1, canvas);
            else
                DrawImage(i, j, 0, canvas);
        }
    }
    //连接符号
    int index = ui->combSign->currentIndex();
    DrawSign(index, canvas, 0);

    //所有图片
    QTime time_1 = QTime::currentTime();    //计时
    for(int i = 0; i < m_row; i++){
        for(int j = 0; j < m_col; j++){
            //程序中止
            if(m_quit)
                return;

            QString signSuffix = "";
            if(index > 0)
                signSuffix = "_" + m_signNameList[index];

            //不是首张图片，先覆盖上一位置（蓝蝴蝶->蓝鸭子），再画新的位置（蓝蝴蝶）
            if(i != 0 || j != 0){
                int last_i, last_j;
                GetLastPos(i, j, last_i, last_j);
                DrawImage(last_i, last_j, 0, canvas);
                DrawImage(i, j, 1, canvas);
            }
            //保存图片
            imgName = "b_" + QString::number(i) + "_" + QString::number(j)
                    + signSuffix + ".png";
            imgPath = outputDir + imgName;
            qDebug() << imgPath;
            canvas.save(imgPath);

            //每隔一段时间，暂停防卡死
            TimerPause(time_1);
        }
    }
    return;
}

void MainWindow::DrawBatch1Blue1Red()
{
    int w, h;
    GetCanvasSize(w, h);
    QPixmap canvas = QPixmap(w, h);
    canvas.fill(Qt::white);

    //创建输出目录
    QString imgName, imgPath;
    QString outputDir = "1Blue1Red/";
    QDir dir(m_outputDir);
    dir.mkpath(outputDir);
    outputDir = m_outputDir + outputDir;
    qDebug() << outputDir;
    //初始图片：蓝(0,0)
    for(int i = 0; i < m_row; i++){
        for(int j = 0; j < m_col; j++){
            if(i == 0 && j == 0)
                DrawImage(i, j, 1, canvas);
            else
                DrawImage(i, j, 0, canvas);
        }
    }
    //连接符号
    int index = ui->combSign->currentIndex();
    DrawSign(index, canvas, 0);
    QString signSuffix = "";
    if(index > 0)
        signSuffix = "_" + m_signNameList[index];

    //所有图片
    QTime time_1 = QTime::currentTime();    //计时
    for(int i = 0; i < m_row; i++){
        for(int j = 0; j < m_col; j++){
            //固定蓝蝴蝶位置
            //不是首张图片，先覆盖上一位置，再画新的位置
            if(i != 0 || j != 0){
                int last_i, last_j;
                GetLastPos(i, j, last_i, last_j);
                DrawImage(last_i, last_j, 0, canvas);
                DrawImage(i, j, 1, canvas);
            }
            //改变红蝴蝶位置
            for(int i1 = 0; i1 < m_row; i1++){
                for(int j1 = 0; j1 < m_col; j1++){
                    //程序中止
                    if(m_quit)
                        return;
                    //忽略蓝蝴蝶位置
                    if(i1 == i && j1 == j)
                        continue;

                    //蓝(0,0)
                    if(i == 0 && j == 0) {
                        //初始：红(0,1)
                        if(i1 == 0 && j1 == 1){
                            DrawImage(i1, j1, 2, canvas);
                        }
                        //清空原位置，画新位置
                        else {
                            int last_i1, last_j1;
                            GetLastPos(i1, j1, last_i1, last_j1);
                            DrawImage(last_i1, last_j1, 0, canvas);
                            DrawImage(i1, j1, 2, canvas);
                        }
                    }
                    //蓝不是(0,0)
                    else {
                        //初始：红(0,0)
                        if(i1 == 0 && j1 == 0){
                            DrawImage(i1, j1, 2, canvas);
                        }
                        //清空原位置，画新位置
                        else {
                            int last_i1, last_j1;
                            //计算下标，跳过蓝蝴蝶位置
                            GetLastPos(i1, j1, last_i1, last_j1, i, j);
                            DrawImage(last_i1, last_j1, 0, canvas);
                            DrawImage(i1, j1, 2, canvas);
                        }
                    }
                    //保存图片
                    imgName = "b_" + QString::number(i) + "_" + QString::number(j) + "_"
                            + "r_" + QString::number(i1) + "_" + QString::number(j1)
                            + signSuffix + ".png";
                    imgPath = outputDir + imgName;
                    qDebug() << imgPath;
                    canvas.save(imgPath);

                    //每隔一段时间，暂停防卡死
                    TimerPause(time_1);
                }
            }
        }
    }
    return;
}

void MainWindow::DrawBatchFin()
{
    int w, h;
    GetCanvasSize(w, h);
    QPixmap canvas = QPixmap(w, h);
    canvas.fill(Qt::white);

    return;
}
