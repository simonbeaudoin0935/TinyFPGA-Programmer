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
    void on_connectUart_Button_clicked();





    void on_test_connection_button_clicked();

    void on_flash_button_clicked();

    void on_read_flash_button_clicked();

    void on_load_filebutton_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort* m_uart;
};

#endif // MAINWINDOW_H
