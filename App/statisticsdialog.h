#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QtCharts>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>

namespace Ui {
class StatisticsDialog;
}

class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(QWidget *parent = nullptr);
    ~StatisticsDialog();
    void setAirportInfo(const QString &airportName, const QString &airportCode);
private slots:
    void on_btnCloseDialog_clicked();
    void on_cbMonth_currentIndexChanged(int index);
private:
    Ui::StatisticsDialog *ui;
    QString m_airportCode;

    QVector<QPair<QDate, int>> m_dailyStats;

    void loadYearlyStatistics();
    void loadMonthlyStatistics();
    void updateMonthlyChart(int monthIndex);
};

#endif // STATISTICSDIALOG_H
