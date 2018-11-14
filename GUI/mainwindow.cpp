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


void MainWindow::on_button_open_serial_port_clicked()
{
    /*
     * We are going to try to open the serial port
     */

    m_uart->setBaudRate(115200);                            //Set the right baudrate

    m_uart->setPortName(ui->com_name->text());              //Open the com port provided in the input text field

    if(!m_uart->open(QIODevice::ReadWrite))                 //Try to open the serial port
    {                                                       //Opening failed...
        ui->button_open_serial_port->setText("RETRY");
        QMessageBox::warning(this,"UART error","Inpossible to connect to the serial port.");
    }
    else                                                    //Opening succeeded
    {
        ui->button_open_serial_port->setText("CONNECTED");
        ui->button_open_serial_port->setEnabled(false);
        ui->button_test_spi_connection->setEnabled(true);

    }
}

void MainWindow::on_button_test_spi_connection_clicked()
{
    QByteArray  command;                                     //The data to be sent to the arduino
    QByteArray  received_data;                               //The data read back from the arduino
    qint64      available;                                   //Available bytes to read
    QByteArray  expected_id;                                 //The expected ID from the spi flash
    QByteArray  received_id;
    QString output_string;
    //********************************************************************************************

    ui->button_test_spi_connection->setEnabled(false);
    ui->button_test_spi_connection->setText("testing");

    command.append('a');                                    //The opcode to read the SPI ID is 'a'

    m_uart->flush();                                        //Flush anything in the uart buffer

    m_uart->write(command);                                 //Send the command

    if(!m_uart->waitForBytesWritten(2000))                  //Wait for command to be sent
    {
        QMessageBox::critical(this,
                              "ERROR",
                              "uart write timeout");
        output_string = "Make sure arduino is properly programmed";
        ui->output->append(output_string);
        ui->button_test_spi_connection->setEnabled(true);
        ui->button_test_spi_connection->setText("retry");

    }

    do
    {
        if (!m_uart->waitForReadyRead(2000)){
            QMessageBox::critical(this,
                                  "ERROR",
                                  "Programmer timeout");
            return;
        }

        available = m_uart->bytesAvailable();
    }while(available != 3);                                 //The response from the programmer is 3 bytes long

    char received_chars[3];
    m_uart->read(received_chars,3);


    expected_id.append(0x1f).append(0x85).append(0x01);
    received_id.append(received_chars[0]).append(received_chars[1]).append(received_chars[2]);


    if(expected_id != received_id)
    {
        output_string = "Wrong chip ID : '" + received_id.toHex() + " ' expected : '" + expected_id.toHex() + "'";
        ui->output->append(output_string);
        ui->button_test_spi_connection->setEnabled(true);
        ui->button_test_spi_connection->setText("retry");
        QMessageBox::critical(this,
                              "ERROR",
                              QString("Impossible to communicate with the SPI Flash, make sure the reset ") +
                              "button is pressed DURING ALL THE OPERATION other otherwise the FPGA" +
                              " is going to try to occupy the SPI port.");
        return;
    }

    else
    {
        output_string = "Good connection with the SPI Flash!";
        ui->output->append(output_string);
        ui->button_test_spi_connection->setText("Good Connection");
        ui->lineEdit_bin_file_name->setEnabled(true);
        ui->pushButton_load_bin_file->setEnabled(true);
    }
}



void MainWindow::on_pushButton_load_bin_file_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,"yo","C:\\Users\\simon\\Desktop\\icecube2_template\\template_Implmnt\\sbt\\outputs\\bitmap","*.bin");
    ui->lineEdit_bin_file_name->setText(filename);
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

    QFile file(ui->lineEdit_bin_file_name->text());
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
//    QByteArray opcode;
//    opcode.append('c');

//    m_uart->write(opcode);
//    m_uart->waitForBytesWritten();

//    qDebug() << "opcode 'f' sent";

//    QByteArray read_back;
//    read_back.resize(135179);

//    int received = 0;

//    char x[256];

//    do
//    {
//       m_uart->waitForReadyRead(-1);
//       qint64 read_bytes = m_uart->read(x,256);
//       for(int i = 0; i != read_bytes; i++)
//       {
//           read_back.push_back(x[i]);
//       }
//       received += read_bytes;
//        qDebug() << "we now at : " << received;
//    }while(received != 135179);


//    QFile file(ui->lineEdit->text());
//    qint64 bitstream_size = file.size();
//    qDebug() << ("bitstream size : " + QString::number(bitstream_size));
//    file.open(QIODevice::ReadOnly);

//    if(received != bitstream_size) qDebug() << "TABARNAKKKK";

//    QByteArray file_read = file.read(received);

//    QFile new_file("new_shit.bin");
//    new_file.open(QIODevice::ReadWrite);
//    new_file.write(file_read);
//    new_file.close();

//    for(int i = 0; i != received; i++)
//    {
//        if(file_read.at(i) != read_back.at(i))
//        {
//            qDebug() << "fuuuck";
//        }else
//        {
//            qDebug() << "ho shit";
//        }
//    }

}



