#include "form_canvas.h"
#include "ui_form_canvas.h"

#include <QPainter>

FormCanvas::FormCanvas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormCanvas)
{
    ui->setupUi(this);

    InitMembers();
    InitConnections();
}

FormCanvas::~FormCanvas()
{
    delete ui;
}

void FormCanvas::InitMembers()
{

}

void FormCanvas::InitConnections()
{

}

void FormCanvas::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_pixmap);
    return;
}

void FormCanvas::PaintPixmap(QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}
