#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QSpinBox>

#include <QString>
#include <QStringList>

#include "form_canvas.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;    
    FormCanvas *pFormCanvas;

    QSpinBox ***m_pSpinMat;
    QPixmap m_pixmap;

    QStringList m_imgList;
    QString m_inputDir, m_outputDir;

    int m_row, m_col;
    int m_gridSize, m_gap;
    double m_scale;

private:
    void InitMembers();
    void InitConnections();

    void GenInputMat();

    void ConnectSpinMat();
    void DeleteSpinMat();

    void ReadImageList();
    void FillImageList();

    void SetOutputDir();
    void SetCanvas();

private slots:
    void InitCanvas();
    void DrawAllImage();

    void DrawSign(int signNo);
    void DrawSign();
    void DrawImage(int row, int col, int imgNo);

    void ChangeGridSize(int val);
    void ChangeImageScale(int val);
    void ChangeSignScale(int val);

    void SaveImage();
    void OpenImageDir();

};
#endif // MAINWINDOW_H
