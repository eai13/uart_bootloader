#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QStringList>
#include <QFileDialog>
#include <QList>
#include <QByteArray>
#include <QFile>
#include <QtTest/QTest>

QSerialPort * serial;

QByteArray GlobalSerialBuffer;

void MainWindow::receive_serial(void){
    GlobalSerialBuffer.append(serial->readAll());
    qDebug() << GlobalSerialBuffer;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow){

    ui->setupUi(this);

    QList<QSerialPortInfo> serial_info = QSerialPortInfo::availablePorts();
    for (auto iter = serial_info.begin(); iter != serial_info.end(); iter++)
        ui->comboBox_serial->addItem(iter->portName());
}

MainWindow::~MainWindow(){
    if (serial->isOpen()) serial->close();
    delete ui;
}


void MainWindow::on_pushButton_open_clicked(){
    serial = new QSerialPort();
    serial->setPortName(ui->comboBox_serial->currentText());
    serial->setBaudRate(ui->comboBox_baud->currentText().toInt());
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite)){
        connect(serial, SIGNAL(readyRead()), this, SLOT(receive_serial()));
        this->PrintConsole("Serial Port at " + ui->comboBox_serial->currentText() + " " + ui->comboBox_baud->currentText() + " NONE, 1, NONE");
        ui->comboBox_baud->setEnabled(false);
        ui->comboBox_serial->setEnabled(false);
        ui->pushButton_close->setEnabled(true);
        ui->pushButton_open->setEnabled(false);
        ui->groupBox_device->setEnabled(true);
    }
    else this->PrintConsole("Failed to Open Serial at " + ui->comboBox_serial->currentText());
}

void MainWindow::on_pushButton_close_clicked(){
    if (serial->isOpen())
        serial->close();

    this->PrintConsole("Serial closed");

    ui->comboBox_baud->setEnabled(true);
    ui->comboBox_serial->setEnabled(true);
    ui->pushButton_open->setEnabled(true);
    ui->pushButton_close->setEnabled(false);
    ui->groupBox_device->setEnabled(false);
}

void MainWindow::on_pushButton_browse_clicked(){
    QString filename = QFileDialog::getOpenFileName(this, "Firmware Browse...", "/", "Binary Files (*.bin)");
    ui->lineEdit_filename->setText(filename);
}

void MainWindow::PrintConsole(QString msg){
    ui->listWidget_console->addItem(msg);
}

void MainWindow::on_pushButton_probe_clicked(){
    GlobalSerialBuffer.clear();

    bootloader_header boot;
    boot.cmd = PING;
    boot.SetSrc(MYNAME);
    boot.SetDest(UNKNOWN);
    boot.payload_size = 0;

    serial->write(reinterpret_cast<char *>(&boot), sizeof(bootloader_header));
    while(GlobalSerialBuffer.size() < sizeof(bootloader_header)){
        if (serial->waitForReadyRead(60000) == false){
            PrintConsole("  Timeout occured");
            return;
        }
    }

    bootloader_header * recv;
    recv = reinterpret_cast<bootloader_header *>(GlobalSerialBuffer.data());
    PrintConsole("Ping from device [" + recv->GetSrc() + "] " + ((recv->cmd == BOOT_OK) ? ("OK") : ("ERROR")));
    GlobalSerialBuffer.remove(0, sizeof(bootloader_header));
}

void MainWindow::on_pushButton_flash_clicked(){
    PrintConsole("FLASH Start");
    bootloader_header send_header;
    bootloader_header * recv_header;

    uint8_t buffer[512];

    send_header.cmd = WRITE;
    send_header.SetSrc(MYNAME);
    send_header.SetDest(UNKNOWN);

    QFile file(ui->lineEdit_filename->text());
    if (!(file.exists())){
        PrintConsole("No File at [" + ui->lineEdit_filename->text() + "] found");
        return;
    }

    file.open(QIODevice::ReadOnly);

    if (this->on_pushButton_erase_clicked())
        return;

    while(1){
        GlobalSerialBuffer.clear();

        send_header.payload_size = file.read(reinterpret_cast<char *>(buffer), 256);
        if (send_header.payload_size == 0) break;
        std::cout << send_header.payload_size << std::endl;
        serial->write(reinterpret_cast<char *>(&send_header), sizeof(bootloader_header));
        QTest::qSleep(50);
        serial->write(reinterpret_cast<char *>(buffer), send_header.payload_size);

        while(GlobalSerialBuffer.size() < sizeof(bootloader_header)){
            if (serial->waitForReadyRead(60000) == false){
                PrintConsole("  Timeout occured");
                return;
            }
        }

        recv_header = reinterpret_cast<bootloader_header *>(GlobalSerialBuffer.data());
        if (recv_header->cmd == BOOT_ERROR){
            PrintConsole("  Error");
            GlobalSerialBuffer.remove(0, sizeof(bootloader_header));
            return;
        }
        else{
            PrintConsole("  Chunk of 256 bytes written to flash " + recv_header->GetSrc());
            GlobalSerialBuffer.remove(0, sizeof(bootloader_header));
        }
    }

    PrintConsole("FLASH Done! Jolly Good!");

}

uint8_t MainWindow::on_pushButton_erase_clicked(){
    GlobalSerialBuffer.clear();
    bootloader_header boot;
    boot.cmd = ERASE;
    boot.SetSrc(MYNAME);
    boot.SetDest(UNKNOWN);
    boot.payload_size = 0;

    PrintConsole("Erasing the Device Firmware...");

    serial->write(reinterpret_cast<char *>(&boot), sizeof(bootloader_header));

    while(GlobalSerialBuffer.size() < sizeof(bootloader_header)){
        if (serial->waitForReadyRead(60000) == false){
            PrintConsole("  Timeout occured");
            return 1;
        }
    }

    bootloader_header * recv;
    recv = reinterpret_cast<bootloader_header *>(GlobalSerialBuffer.data());
    PrintConsole("Erase of device [" + recv->GetSrc() + "]" + ((recv->cmd == BOOT_OK) ? ("OK") : ("ERROR")));
    uint8_t status = recv->cmd;
    GlobalSerialBuffer.remove(0, sizeof(bootloader_header));
    if (status == BOOT_OK) return 0;
    else return 1;
}

void MainWindow::on_pushButton_jump_clicked(){
    GlobalSerialBuffer.clear();
    bootloader_header send_header;
    bootloader_header * recv_header;

    send_header.cmd = JUMP;
    send_header.SetSrc(MYNAME);
    send_header.SetDest(UNKNOWN);
    send_header.payload_size = 4;

    serial->write(reinterpret_cast<char *>(&send_header), sizeof(bootloader_header));

    while(GlobalSerialBuffer.size() < sizeof(bootloader_header)){
        if (serial->waitForReadyRead(60000) == false){
            PrintConsole(" Timeout occured");
            return;
        }
    }

    recv_header = reinterpret_cast<bootloader_header *>(GlobalSerialBuffer.data());
    PrintConsole("Jump Done [" + recv_header->GetSrc() + "]" + ((recv_header->cmd = BOOT_OK) ? ("OK") : ("ERROR")));
    GlobalSerialBuffer.remove(0, sizeof(bootloader_header));
}

void MainWindow::on_pushButton_clearoutput_clicked(){
    ui->listWidget_console->clear();
}
