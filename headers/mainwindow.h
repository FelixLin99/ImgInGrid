#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QSpinBox>

#include <QString>
#include <QStringList>
#include <QCloseEvent>

#include <QVector>

#include "form_canvas.h"

#define SIGN_IDX_NONE   0
#define SIGN_IDX_PLUS   1
#define SIGN_IDX_UP     2
#define SIGN_IDX_DOWN   3

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

    QVector<int> m_imgIndexList;
//    QVector<QPixmap> m_imgPixmap;

    int m_row, m_col;
    int m_gridSize, m_gapUD, m_gapLR;
    int m_quality;
    double m_scale;

    bool m_quit, m_break;

protected:
    void closeEvent(QCloseEvent *) override;

private:
    void InitMembers();
    void InitConnections();

    void SetQuit(bool val);
    void SetBatchStatus(bool val);

    void GenInputMat();

    void ConnectSpinMat();
    void DeleteSpinMat();

    void ReadImageList();
    void FillImageList();
//    void LoadImagePixmap();

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

    int DrawImageFromStr(const QString &str, QPixmap &canvas);

    void ChangeGridSize(int val);
    void ChangeImageScale(int val);
    void ChangeSignScale(int val);
    void ChangeGapLR(int val);
    void ChangeQuality(int val);

    void SaveImage();
    void OpenImageDir();

    //批量生成
    void DrawBatch1Blue();
    void DrawBatch1Blue1Red();
    void DrawBatchFin();

};
#endif // MAINWINDOW_H
