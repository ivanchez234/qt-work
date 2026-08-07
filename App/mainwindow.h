#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTimer>
#include <QMessageBox>
#include <QSqlQueryModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void attemptConnection();

    void on_btnFindFlights_clicked();

    void on_btnShowStats_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QTimer *reconnectTimer;
    QSqlQueryModel *queryModel;
};
#endif // MAINWINDOW_H
