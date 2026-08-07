#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "statisticsdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    queryModel = new QSqlQueryModel(this);
    ui->tvSchedule->setModel(queryModel);
    ui->lblDbStatus->setText("Отключено");
    ui->lblDbStatus->setStyleSheet("color: red;");

    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("981757-ca08998.tmweb.ru");
    db.setPort(5432);
    db.setDatabaseName("demo");
    db.setUserName("netology_usr_cpp");
    db.setPassword("CppNeto3");

    reconnectTimer = new QTimer(this);
    reconnectTimer->setInterval(5000);
    connect(reconnectTimer, &QTimer::timeout, this, &MainWindow::attemptConnection);

    attemptConnection();
}

MainWindow::~MainWindow()
{
    if(db.isOpen()) {
        db.close();
    }
    delete ui;
}

void MainWindow::attemptConnection()
{
    if (db.isOpen()) {
        return;
    }

    if (db.open()) {
        reconnectTimer->stop();

        ui->lblDbStatus->setText("Подключено");
        ui->lblDbStatus->setStyleSheet("color: green;");

        QSqlQuery query(db);
        query.prepare("SELECT airport_name->>'ru' as \"airportName\", airport_code FROM bookings.airports_data");

        if (query.exec()) {
            ui->cbAirports->clear();

            while (query.next()) {
                QString airportName = query.value(0).toString();
                QString airportCode = query.value(1).toString();

                ui->cbAirports->addItem(airportName, QVariant(airportCode));
            }

            ui->btnFindFlights->setEnabled(true);
            ui->btnShowStats->setEnabled(true);
        } else {
            QMessageBox::critical(this, "Ошибка БД",
                                  "Не удалось загрузить список аэропортов:\n" + query.lastError().text());
        }

    }
    else {
        ui->lblDbStatus->setText("Отключено");
        ui->lblDbStatus->setStyleSheet("color: red;");

        QMessageBox::critical(this, "Ошибка подключения",
                              "Не удалось подключиться к базе данных:\n" + db.lastError().text());

        if (!reconnectTimer->isActive()) {
            reconnectTimer->start();
        }
    }
}
void MainWindow::on_btnFindFlights_clicked()
{

    QString airportCode = ui->cbAirports->currentData().toString();

    QString dateStr = ui->deFlightDate->date().toString("yyyy-MM-dd");

    QSqlQuery query(db);

    if (ui->rbArrivals->isChecked()) {

        query.prepare("SELECT flight_no, scheduled_arrival, ad.airport_name->>'ru' as \"Name\" "
                      "FROM bookings.flights f "
                      "JOIN bookings.airports_data ad ON ad.airport_code = f.departure_airport "
                      "WHERE f.arrival_airport = :airportCode AND f.scheduled_arrival::date = :date");

        query.bindValue(":airportCode", airportCode);
        query.bindValue(":date", dateStr);
        query.exec();

        queryModel->setQuery(query);
        queryModel->setHeaderData(0, Qt::Horizontal, "Номер рейса");
        queryModel->setHeaderData(1, Qt::Horizontal, "Время прилета");
        queryModel->setHeaderData(2, Qt::Horizontal, "Аэропорт отправления");

    } else if (ui->rbDepartures->isChecked()) {

        query.prepare("SELECT flight_no, scheduled_departure, ad.airport_name->>'ru' as \"Name\" "
                      "FROM bookings.flights f "
                      "JOIN bookings.airports_data ad ON ad.airport_code = f.arrival_airport "
                      "WHERE f.departure_airport = :airportCode AND f.scheduled_departure::date = :date");

        query.bindValue(":airportCode", airportCode);
        query.bindValue(":date", dateStr);
        query.exec();

        queryModel->setQuery(query);
        queryModel->setHeaderData(0, Qt::Horizontal, "Номер рейса");
        queryModel->setHeaderData(1, Qt::Horizontal, "Время вылета");
        queryModel->setHeaderData(2, Qt::Horizontal, "Аэропорт назначения");
    }

    ui->tvSchedule->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvSchedule->horizontalHeader()->setStretchLastSection(true);
}


void MainWindow::on_btnShowStats_clicked()
{
    QString airportName = ui->cbAirports->currentText();
    QString airportCode = ui->cbAirports->currentData().toString();

    StatisticsDialog statsDialog(this);

    statsDialog.setAirportInfo(airportName, airportCode);

    statsDialog.exec();
}

