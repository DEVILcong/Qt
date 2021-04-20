#ifndef QRCODE_WIN_H
#define QRCODE_WIN_H

#include <QWidget>
#include <QPixmap>
#include <QNetworkInterface>

namespace Ui {
class QRCode_win;
}

class QRCode_win : public QWidget
{
    Q_OBJECT

public:
    explicit QRCode_win(QWidget *parent = nullptr, int tmp_port = 0, int tmp_userId = 0);
    ~QRCode_win();

private slots:
    void onPushButtonClicked();
    void onComboBox1Changed(const QString& text);
    void onComboBox2Changed(const QString& text);

private:
    Ui::QRCode_win *ui;

    int port = 0;
    int userId = 0;
    QMap<QString, QNetworkInterface> networkInterfaceMap;
    QList<QNetworkInterface> networkInterfaces;
    QPixmap *pixmap;
};

#endif // QRCODE_WIN_H
