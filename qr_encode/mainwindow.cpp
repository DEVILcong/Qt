#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QFileDialog>
#include "qrencode.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(onTextEditInput()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(onClearButtonClicked()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(onAckButtonClicked()));
    connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(onSaveButtonClicked()));

    this->pixmap = nullptr;
    this->image_area_width = 0;
    this->image_area_height = 0;
    this->qrcode_width = 0;
    this->scale = 0;
    this->qrcode_display_size = 0;
}

void MainWindow::onTextEditInput(void){
    int length = ui->textEdit->toPlainText().size();

    statusBar()->showMessage(QString("length: ") + QString::number(length) + QString("/") + QString::number(this->input_max_length));
}

void MainWindow::onClearButtonClicked(void){
    ui->textEdit->clear();
}

void MainWindow::onAckButtonClicked(void){
    this->image_area_width = ui->label->width();
    this->image_area_height = ui->label->height();
    float x = 0, y = 0;
    QString content = ui->textEdit->toPlainText();
    if(content.size() > this->input_max_length){
        QMessageBox::warning(this, "warning", "输入数据太长了");
        return;
    }

    QRcode *code = NULL;
    code = QRcode_encodeString8bit(content.toUtf8().constData(), 0, QR_ECLEVEL_Q);
    if(code == NULL){
        QMessageBox::critical(this, "ERROR", "encode error");
        return;
    }

    this->qrcode_width = code->width;
    //选择显示区域长宽中较小的一个值，作为显示二维码正方形的边长
    this->qrcode_display_size = this->image_area_width >= this->image_area_height ? this->image_area_height : this->image_area_width;
    //根据显示区域的尺寸与要显示二维码宽度确定一个二维码单位占用 多少个像素，即对二维码进行缩放
    this->scale = ((float)this->qrcode_display_size) / this->qrcode_width;

    //调整绘图初始点位置，使二维码显示在中心
    if(this->qrcode_display_size == this->image_area_width){
        x = 0;
        y = (this->image_area_height - this->qrcode_display_size) / 2;
    }else{
        x = (this->image_area_width - this->qrcode_display_size) / 2;
        y = 0;
    }

    //开始绘图
    if(this->pixmap != nullptr)
        delete this->pixmap;
    this->pixmap = new QPixmap(this->image_area_width, this->image_area_height);
    QPainter painter(pixmap);
    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, this->image_area_width, this->image_area_height);
    QColor foreground(Qt::black);
    painter.setBrush(foreground);

    for(int i = 0; i < this->qrcode_width; ++i){
        float tmp_x = x;
        for(int j = 0; j < this->qrcode_width; ++j){
            if((char)(code->data[i * this->qrcode_width + j]) & 0x01){
                QRectF rectf(tmp_x, y, this->scale, this->scale);
                painter.drawRects(&rectf, 1);
            }
            tmp_x += scale;
        }
        y += scale;
    }

    ui->label->setPixmap(*pixmap);
    QRcode_free(code);
}

void MainWindow::onSaveButtonClicked(void){
    if(this->pixmap == nullptr){
        QMessageBox::warning(this, "warning", "未发现要保存的图片");
        return;
    }
    QString dest = QFileDialog::getSaveFileName(this, "Save", "", "Images (*.png *.jpg)");
    bool is_success = this->pixmap->save(dest + QString(".png"));

    if(!is_success){
        QMessageBox::warning(this, "ERROR", "保存失败");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    if(this->pixmap != nullptr){
        delete this->pixmap;
    }
}

