#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QSpinBox>

#include <QString>
#include <QStringList>
#include <QCloseEvent>

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

    QStringList m_imgList, m_signList, m_signNameList;
    QString m_inputDir, m_outputDir;

    int m_row, m_col;
    int m_gridSize, m_gap;
    double m_scale;

    bool m_quit;

protected:
    void closeEvent(QCloseEvent *) override;

private:
    void InitMembers();
    void InitConnections();

    void SetQuit(bool val);

    void GenInputMat();

    void ConnectSpinMat();
    void DeleteSpinMat();

    void ReadImageList();
    void FillImageList();

    void SetOutputDir();

    void GetCanvasSize(int &w, int &h);
    void SetCanvas();

    void GetLastPos(int row, int col, int &last_row, int &last_col, int skip_row, int skip_col);
    void GetLastPos(int row, int col, int &last_row, int &last_col);

private slots:
    void InitCanvas();
    void DrawAllImage();

    void DrawSign(int signNo, QPixmap &canvas, bool clean = 1);
    void DrawSign(int signNo);
    void DrawSign();

    void DrawImage(int row, int col, int imgNo, QPixmap &canvas);
    void DrawImage(int row, int col, int imgNo);

    void ChangeGridSize(int val);
    void ChangeImageScale(int val);
    void ChangeSignScale(int val);

    void SaveImage();
    void OpenImageDir();

    //批量生成
    void DrawBatch1Blue();
    void DrawBatch1Blue1Red();
    void DrawBatchFin();

};
#endif // MAINWINDOW_H
