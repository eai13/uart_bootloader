#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>

#include <QString>
#include <string.h>

#define MYNAME  "HOST"
#define UNKNOWN "FFFF"

#define WRITE       0x01
#define READ        0x02
#define ERASE       0x03
#define PING        0x04
#define BOOT_OK     0x05
#define BOOT_ERROR  0x06
#define JUMP        0x07

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#pragma pack(push, 1)
struct bootloader_header{
    uint8_t device_src[4];
    uint8_t device_dest[4];
    uint8_t cmd;
    uint32_t payload_size;

    QString GetSrc(void){
        QString str;
        str.push_back(this->device_src[0]);
        str.push_back(this->device_src[1]);
        str.push_back(this->device_src[2]);
        str.push_back(this->device_src[3]);
        return str;
    }
    QString GetDest(void){
        QString str;
        str.push_back(this->device_dest[0]);
        str.push_back(this->device_dest[1]);
        str.push_back(this->device_dest[2]);
        str.push_back(this->device_dest[3]);
        return str;
    }
    void SetSrc(std::string str){
        this->device_src[0] = str[0];
        this->device_src[1] = str[1];
        this->device_src[2] = str[2];
        this->device_src[3] = str[3];
    }
    void SetDest(std::string str){
        this->device_dest[0] = str[0];
        this->device_dest[1] = str[1];
        this->device_dest[2] = str[2];
        this->device_dest[3] = str[3];
    }
};
#pragma pack(pop)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    void PrintConsole(QString msg);

    ~MainWindow();

public slots:
    void receive_serial(void);

private slots:
    void on_pushButton_open_clicked();

    void on_pushButton_close_clicked();

    void on_pushButton_browse_clicked();

    void on_pushButton_probe_clicked();

    void on_pushButton_flash_clicked();

    uint8_t on_pushButton_erase_clicked();

    void on_pushButton_jump_clicked();

    void on_pushButton_clearoutput_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
