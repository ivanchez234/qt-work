#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "statisticsdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <utility>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    queryModel = new QSqlQueryModel(this);
    ui->tvSchedule->setModel(queryModel);
    setStatus("Подключение…", "gray");

    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("981757-ca08998.tmweb.ru");
    db.setPort(5432);
    db.setDatabaseName("demo");
    db.setUserName("netology_usr_cpp");
    db.setPassword("CppNeto3");

    db.setConnectOptions("connect_timeout=3");

    reconnectTimer = new QTimer(this);
    reconnectTimer->setInterval(5000);
    connect(reconnectTimer, &QTimer::timeout, this, &MainWindow::attemptConnection);

    QTimer::singleShot(100, this, &MainWindow::attemptConnection);
}

MainWindow::~MainWindow()
{
    if(db.isOpen()) {
        db.close();
    }
    delete ui;
}

void MainWindow::setStatus(const QString &text, const QString &color, const QString &details)
{
    ui->lblDbStatus->setText(text);
    ui->lblDbStatus->setStyleSheet("color: " + color + ";");
    ui->lblDbStatus->setToolTip(details);
}

void MainWindow::setControlsEnabled(bool enabled)
{
    ui->btnFindFlights->setEnabled(enabled);
    ui->btnShowStats->setEnabled(enabled);
}

void MainWindow::startReconnecting(const QString &text, const QString &details)
{
    setControlsEnabled(false);
    setStatus(text, "red", details);
    if (!reconnectTimer->isActive()) {
        reconnectTimer->start();
    }
}

void MainWindow::attemptConnection()
{
    if (db.isOpen()) {
        reconnectTimer->stop();
        return;
    }

    setStatus("Подключение…", "gray");
    ui->lblDbStatus->repaint(); // open() блокирует цикл событий — обновляем надпись сразу

    if (!db.open()) {
        startReconnecting("Отключено, повтор через 5 с", db.lastError().text());
        return;
    }

    reconnectTimer->stop();
    setStatus("Подключено", "green");
    loadAirports();
}

void MainWindow::loadAirports()
{
    QSqlQuery query(db);
    query.prepare("SELECT airport_name->>'ru' as \"airportName\", airport_code FROM bookings.airports_data");

    if (!query.exec()) {
        handleQueryError(query, "Не удалось загрузить список аэропортов");
        return;
    }

    // После переподключения сохраняем выбранный аэропорт
    const QString selectedCode = ui->cbAirports->currentData().toString();
    ui->cbAirports->clear();

    while (query.next()) {
        QString airportName = query.value(0).toString();
        QString airportCode = query.value(1).toString();

        ui->cbAirports->addItem(airportName, QVariant(airportCode));
    }

    const int index = ui->cbAirports->findData(selectedCode);
    if (index >= 0) {
        ui->cbAirports->setCurrentIndex(index);
    }

    setControlsEnabled(true);
}

void MainWindow::handleQueryError(const QSqlQuery &query, const QString &what)
{
    const QString error = query.lastError().text();

    QSqlQuery ping(db);
    if (!db.isOpen() || !ping.exec("SELECT 1")) {
        db.close();
        startReconnecting("Связь потеряна, повтор через 5 с", error);
        return;
    }

    QMessageBox::warning(this, "Ошибка запроса", what + ":\n" + error);
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
        if (!query.exec()) {
            handleQueryError(query, "Не удалось получить расписание");
            return;
        }

        queryModel->setQuery(std::move(query));
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
        if (!query.exec()) {
            handleQueryError(query, "Не удалось получить расписание");
            return;
        }

        queryModel->setQuery(std::move(query));
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
