#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
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
    void loadAirports();
    void setControlsEnabled(bool enabled);
    void setStatus(const QString &text, const QString &color, const QString &details = QString());
    void handleQueryError(const QSqlQuery &query, const QString &what);
    void startReconnecting(const QString &text, const QString &details);

    Ui::MainWindow *ui;
    QSqlDatabase db;
    QTimer *reconnectTimer;
    QSqlQueryModel *queryModel;
};
#endif // MAINWINDOW_H
