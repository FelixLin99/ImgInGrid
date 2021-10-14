#ifndef FORM_CANVAS_H
#define FORM_CANVAS_H

#include <QWidget>
#include <QPixmap>

namespace Ui {
class FormCanvas;
}

class FormCanvas : public QWidget
{
    Q_OBJECT

public:
    QPixmap m_pixmap;

public:
    explicit FormCanvas(QWidget *parent = nullptr);
    ~FormCanvas();

    void PaintPixmap(QPixmap &pixmap);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::FormCanvas *ui;

private:
    void InitMembers();
    void InitConnections();

};

#endif // FORM_CANVAS_H
