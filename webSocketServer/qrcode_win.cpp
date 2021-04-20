#include "qrcode_win.h"
#include "ui_qrcode_win.h"
#include "qrencode.h"
#include <QNetworkInterface>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>

QRCode_win::QRCode_win(QWidget *parent, int tmp_port, int tmp_userId) :
    QWidget(parent),
    ui(new Ui::QRCode_win)
{
    ui->setupUi(this);
    this->port = tmp_port;
    this->userId = tmp_userId;
    this->pixmap = nullptr;

    networkInterfaces = QNetworkInterface::allInterfaces();

    for(uint8_t i = 0; i < networkInterfaces.length(); ++i){
        if(networkInterfaces[i].isValid() && networkInterfaces[i].flags() & QNetworkInterface::IsUp){
            this->networkInterfaceMap.insert(networkInterfaces[i].name(),
                                             networkInterfaces[i]);
        }
    }

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(onPushButtonClicked()));

    ui->comboBox->addItems(QStringList(networkInterfaceMap.keys()));
    this->onComboBox1Changed(ui->comboBox->currentText());
    connect(ui->comboBox, SIGNAL(currentTextChanged(const QString&)), this, SLOT(onComboBox1Changed(const QString&)));
    connect(ui->comboBox_2, SIGNAL(currentTextChanged(const QString&)), this, SLOT(onComboBox2Changed(const QString&)));
}


void QRCode_win::onPushButtonClicked(){
    if(this->pixmap != nullptr)
        delete this->pixmap;

    QJsonObject tmp_json_object;

    QString tmp_string = "ws://" + ui->comboBox_2->currentText() + ":" + QString::number(this->port);

    tmp_json_object.insert("type", QJsonValue(2));
    tmp_json_object.insert("serverAddress", QJsonValue(tmp_string));
    tmp_json_object.insert("userId", QJsonValue(this->userId));
    QJsonDocument tmp_json_document(tmp_json_object);

    QRcode *tmp_qrcode = QRcode_encodeString8bit(tmp_json_document.toJson().constData(), 0, QR_ECLEVEL_Q);
    if(tmp_qrcode == NULL){
        QMessageBox::warning(this, "警告", "生成二维码失败");
        return;
    }

    float scale = 256.0f / tmp_qrcode->width;
    this->pixmap = new QPixmap(256, 256);
    QPainter painter(this->pixmap);
    QColor backgroundColor(Qt::white);
    QColor foregroundColor(Qt::black);
    QBrush tmp_brush(backgroundColor);
    painter.setBrush(tmp_brush);
    painter.setPen(Qt::NoPen);

    QRect tmp_rect(0, 0, 256, 256);
    painter.drawRect(tmp_rect);

    tmp_brush = QBrush(foregroundColor);
    painter.setBrush(tmp_brush);
    for(int i = 0; i < tmp_qrcode->width; ++i){
        for(int j = 0; j < tmp_qrcode->width; ++j){
            if((char)tmp_qrcode->data[i * tmp_qrcode->width + j] & 0x01){
                QRectF tmp_rectf(j * scale, i * scale, scale, scale);
                painter.drawRect(tmp_rectf);
            }
        }
    }

    ui->label->setPixmap(*this->pixmap);
    QRcode_free(tmp_qrcode);
}

void QRCode_win::onComboBox1Changed(const QString& string){
    if(!networkInterfaceMap.contains(string)){
        QMessageBox::warning(this, "warning", "未找到相应网卡设备");
        return;
    }

    ui->comboBox_2->clear();
    QList<QNetworkAddressEntry> addressEntries = networkInterfaceMap[string].addressEntries();
    QStringList addrList;
    for(QNetworkAddressEntry addressEntry: addressEntries){
        addrList.append(addressEntry.ip().toString());
    }

    ui->comboBox_2->addItems(addrList);
}

void QRCode_win::onComboBox2Changed(const QString &string){

}

QRCode_win::~QRCode_win()
{
    delete ui;

    if(this->pixmap != nullptr)
        delete this->pixmap;
}
