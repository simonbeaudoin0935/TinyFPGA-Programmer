#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_button_open_serial_port_clicked();

    void on_pushButton_load_bin_file_clicked();

    void on_button_test_spi_connection_clicked();





    void on_flash_button_clicked();

    void on_read_flash_button_clicked();






private:
    Ui::MainWindow *ui;
    QSerialPort* m_uart;
};

#endif // MAINWINDOW_H
