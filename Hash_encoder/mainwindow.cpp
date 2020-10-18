#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(OnTextInput()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnButtonClicked(){
    QString tmp_string = ui->textEdit->toPlainText();
    QByteArray tmp_byte_arrry = tmp_string.toUtf8();
    QByteArray tmp_byte_array2;

    if(ui->comboBox->currentIndex() == 0)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Md4);
    else if(ui->comboBox->currentIndex() == 1)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Md5);
    else if(ui->comboBox->currentIndex() == 2)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Sha1);
    else if(ui->comboBox->currentIndex() == 3)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Sha224);
    else if(ui->comboBox->currentIndex() == 4)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Sha256);
    else if(ui->comboBox->currentIndex() == 5)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Sha384);
    else if(ui->comboBox->currentIndex() == 6)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Sha512);
    else if(ui->comboBox->currentIndex() == 7)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Sha3_224);
    else if(ui->comboBox->currentIndex() == 8)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Sha3_256);
    else if(ui->comboBox->currentIndex() == 9)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Sha3_384);
    else if(ui->comboBox->currentIndex() == 10)
        tmp_byte_array2 = QCryptographicHash::hash(tmp_byte_arrry, QCryptographicHash::Sha3_512);

    if(ui->checkBox->isChecked())
        tmp_string = tmp_byte_array2.toBase64();
    else
        tmp_string = tmp_byte_array2.toHex();

    ui->textEdit_2->setPlainText(tmp_string);
    ui->lineEdit_2->setText(QString::number(tmp_string.size()));
}

void MainWindow::OnTextInput(){
    ui->lineEdit->setText(QString::number(ui->textEdit->toPlainText().size()));
}

