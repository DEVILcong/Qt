#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTextEditInput(void);
    void onClearButtonClicked(void);
    void onAckButtonClicked(void);
    void onSaveButtonClicked(void);

private:
    Ui::MainWindow *ui;

    int input_max_length = 500;  //最大输入长度
    QPixmap *pixmap;
    int image_area_width;
    int image_area_height; //qt图片显示区域长宽，此处为一个QLabel
    int qrcode_width;      //二维码尺寸，二维码是一个正方形
    float scale;           //二维码根据图片显示区域的尺寸进行缩放，即一个二维码像素单位 占多少个Qt图片显示区域的像素
    int qrcode_display_size;  //二维码实际显示尺寸，此处调整为图片显示区域长宽中小的一个， 为了方便显示正方形，上面的缩放比例根据这个计算得出，具体看代码
};
#endif // MAINWINDOW_H
