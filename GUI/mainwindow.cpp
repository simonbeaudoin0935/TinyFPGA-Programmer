#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_uart(new QSerialPort)
{
    ui->setupUi(this);
     ui->progressBar->setValue(0);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connectUart_Button_clicked()
{
    m_uart->setBaudRate(115200);
    m_uart->setPortName(ui->com_name->text());
    if(!m_uart->open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(this,"DAMN","DAMN BOY, thing didn't worked");
    }
    else
    {
        ui->connectUart_Button->setText("CONNECTED");
        ui->connectUart_Button->setEnabled(false);
    }

}





void MainWindow::on_test_connection_button_clicked()
{
    QByteArray in,out;
    in.append('a');
    qint64 available;

    m_uart->write(in);
    do
    {
        if (!m_uart->waitForReadyRead(2000)) return;
        available = m_uart->bytesAvailable();
    }while(available != 3);

    char output_char[4];
    m_uart->read(output_char,3);

    out.append(output_char[0]);
    out.append(output_char[1]);
    out.append(output_char[2]);



//    if(output_char[0] == 0x1F && output_char[1] == 0x85 && output_char[2] == 0x01)
//    {
//        ui->output->append("SPI Flash recognized!");
//        ui->test_connection_button->setText("GOOD");
//        ui->test_connection_button->setEnabled(false);
//    }
//    else
//    {
//        ui->output->append("SPI Flash not recognized, make sure the board is maintained in reset state by pressing the button or connecting rst pad to gnd, and replug the board.");
//    }
//
//
    ui->output->append(QString(out.toHex()));
}

void MainWindow::on_flash_button_clicked()
{
    QByteArray opcode;
    char buff[256];
    qint64 available;
    qint64 bitstream_size;
    qint64 count = 0;


    opcode.append('d');
    m_uart->write(opcode);

    ui->output->append("ERASING CHIP");
    do
    {

        QCoreApplication::processEvents();
        m_uart->waitForReadyRead();
        QByteArray read = m_uart->readAll();
        for(int i = 0; i != read.size(); i++)
        {
            if(!(read.at(i) & 0x01)) goto end;
        }

    }while(1);

end:

    opcode.clear();
    opcode.append('f');

    m_uart->write(opcode);

    QFile file(ui->lineEdit->text());
    bitstream_size = file.size();
    ui->output->append("bitstream size : " + QString::number(bitstream_size));
    file.open(QIODevice::ReadOnly);

    bool last_iteration = false;
    do
    {
        qint64 file_read_bytes = file.read(buff,32);


        if(file_read_bytes != 32)
        {
            qDebug() << "last iteration";
            last_iteration = true;
            for(int i = (int)file_read_bytes; i != 32; i++) buff[i] = 0x00; //zero the remaining bytes
        }

        m_uart->write(buff,32);


        QCoreApplication::processEvents();

        m_uart->waitForBytesWritten(-1);
        count += 32;



        if(!m_uart->waitForReadyRead(2000))
        {
            QMessageBox::critical(this,"SHIT","no response from programmer");
            return;
        }
        char x[32];

        m_uart->read(x,32);
        ui->output->append("COUNT : " + QString::number(count));

        int percent = (int)(((float)((float)count)/((float)bitstream_size))*100.0);
        ui->progressBar->setValue(percent);
    }while(last_iteration == false);
}

void MainWindow::on_read_flash_button_clicked()
{
    QByteArray opcode;
    opcode.append('c');

    m_uart->write(opcode);
    m_uart->waitForBytesWritten();

    qDebug() << "opcode 'f' sent";

    QByteArray read_back;
    read_back.resize(135179);

    int received = 0;

    char x[256];

    do
    {
       m_uart->waitForReadyRead(-1);
       qint64 read_bytes = m_uart->read(x,256);
       for(int i = 0; i != read_bytes; i++)
       {
           read_back.push_back(x[i]);
       }
       received += read_bytes;
        qDebug() << "we now at : " << received;
    }while(received != 135179);


    QFile file(ui->lineEdit->text());
    qint64 bitstream_size = file.size();
    qDebug() << ("bitstream size : " + QString::number(bitstream_size));
    file.open(QIODevice::ReadOnly);

    if(received != bitstream_size) qDebug() << "TABARNAKKKK";

    QByteArray file_read = file.read(received);

    QFile new_file("new_shit.bin");
    new_file.open(QIODevice::ReadWrite);
    new_file.write(file_read);
    new_file.close();

    for(int i = 0; i != received; i++)
    {
        if(file_read.at(i) != read_back.at(i))
        {
            qDebug() << "fuuuck";
        }else
        {
            qDebug() << "ho shit";
        }
    }

}

void MainWindow::on_load_filebutton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,"yo","C:\\Users\\simon\\Desktop\\icecube2_template\\template_Implmnt\\sbt\\outputs\\bitmap","*.bin");
    ui->lineEdit->setText(filename);
}
