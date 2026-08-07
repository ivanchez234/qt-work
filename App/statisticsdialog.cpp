#include "statisticsdialog.h"
#include "ui_statisticsdialog.h"
#include <QVBoxLayout>
#include <QMessageBox>

StatisticsDialog::StatisticsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StatisticsDialog)
{
    ui->setupUi(this);

    ui->cbMonth->setCurrentIndex(0);
}

StatisticsDialog::~StatisticsDialog()
{
    delete ui;
}
void StatisticsDialog::setAirportInfo(const QString &airportName, const QString &airportCode)
{
    m_airportCode = airportCode;
    ui->lblAirportName->setText(QString("Загруженность аэропорта: %1").arg(airportName));

    loadYearlyStatistics();
    loadMonthlyStatistics();
}
void StatisticsDialog::on_btnCloseDialog_clicked()
{
    accept();
}

void StatisticsDialog::loadYearlyStatistics()
{
    QSqlQuery query;
    query.prepare("SELECT count(flight_no), date_trunc('month', scheduled_departure) as \"Month\" "
                  "FROM bookings.flights f "
                  "WHERE (scheduled_departure::date > date('2016-08-31') AND scheduled_departure::date <= date('2017-08-31')) "
                  "AND (departure_airport = :code OR arrival_airport = :code) "
                  "GROUP BY \"Month\" ORDER BY \"Month\"");
    query.bindValue(":code", m_airportCode);

    if (!query.exec()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить годовую статистику");
        return;
    }

    QBarSet *set = new QBarSet("Кол-во рейсов");
    QStringList categories;

    while (query.next()) {
        int count = query.value(0).toInt();
        QDate date = query.value(1).toDate();

        *set << count;
        categories << date.toString("MMM yy");
    }

    QBarSeries *series = new QBarSeries();
    series->append(set);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Загруженность за год по месяцам");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->hide();

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    if (!ui->wgtYearChart->layout()) {
        new QVBoxLayout(ui->wgtYearChart);
    }

    QLayoutItem *child;
    while ((child = ui->wgtYearChart->layout()->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    ui->wgtYearChart->layout()->addWidget(chartView);
}

void StatisticsDialog::loadMonthlyStatistics()
{
    m_dailyStats.clear();

    QSqlQuery query;
    query.prepare("SELECT count(flight_no), date_trunc('day', scheduled_departure) as \"Day\" "
                  "FROM bookings.flights f "
                  "WHERE (scheduled_departure::date > date('2016-08-31') AND scheduled_departure::date <= date('2017-08-31')) "
                  "AND (departure_airport = :code OR arrival_airport = :code) "
                  "GROUP BY \"Day\" ORDER BY \"Day\"");
    query.bindValue(":code", m_airportCode);

    if (query.exec()) {
        while (query.next()) {
            int count = query.value(0).toInt();
            QDate date = query.value(1).toDate();
            m_dailyStats.append(qMakePair(date, count));
        }
    }

    updateMonthlyChart(ui->cbMonth->currentIndex());
}

void StatisticsDialog::on_cbMonth_currentIndexChanged(int index)
{
    updateMonthlyChart(index);
}

void StatisticsDialog::updateMonthlyChart(int monthIndex)
{
    int targetMonth = monthIndex + 1;

    QLineSeries *series = new QLineSeries();

    for (const auto &stat : m_dailyStats) {
        if (stat.first.month() == targetMonth) {
            series->append(stat.first.day(), stat.second);
        }
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString("Загруженность за %1").arg(ui->cbMonth->itemText(monthIndex)));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->hide();

    QValueAxis *axisX = new QValueAxis();
    axisX->setLabelFormat("%i");
    axisX->setTitleText("Число месяца");
    axisX->setTickCount(series->count() > 0 ? series->count() : 1);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%i");
    axisY->setTitleText("Количество рейсов");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    if (!ui->wgtMonthChart->layout()) {
        new QVBoxLayout(ui->wgtMonthChart);
    }

    QLayoutItem *child;
    while ((child = ui->wgtMonthChart->layout()->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    ui->wgtMonthChart->layout()->addWidget(chartView);
}
