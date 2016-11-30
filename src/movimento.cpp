// Copyright (c) 2016 Danilo Peixoto. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "movimento.h"
#include "common.h"

#include <QSerialPortInfo>
#include <QMessageBox>
#include <cmath>
#include <vector>

using namespace std;

Movimento::Movimento(QWidget * parent) : QMainWindow(parent) {
    ui = new Ui::MovimentoWindow;
    ui->setupUi(this);

    QBrush curveColor("#3b72ab");
    QBrush gridColor("#5e5e5e");
    QBrush textColor("#bbbbbb");

    splineSeries = new QSplineSeries;
    splineSeries->setPen(QPen(curveColor, 2));

    xAxis = new QValueAxis;
    xAxis->setTitleText("Angle of Inclination (º)");
    xAxis->setLabelFormat("%i");
    xAxis->setTitleBrush(textColor);
    xAxis->setLabelsBrush(textColor);
    xAxis->setGridLinePen(QPen(gridColor, 1));
    xAxis->setRange(0, 90.0);
    xAxis->setTickCount(10);

    yAxis = new QValueAxis;
    yAxis->setTitleText("Friction Force (N)");
    yAxis->setLabelFormat("%.2f");
    yAxis->setTitleBrush(textColor);
    yAxis->setLabelsBrush(textColor);
    yAxis->setGridLinePen(QPen(gridColor, 1));
    yAxis->setRange(0, 10.0);
    yAxis->setTickCount(5);

    chart = new QChart;
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins());
    chart->addSeries(splineSeries);
    chart->setAxisX(xAxis, splineSeries);
    chart->setAxisY(yAxis, splineSeries);

    chartView = new QChartView(chart);
    chartView->setInteractive(false);
    chartView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    ui->staticFrictionGroupLayout->addWidget(chartView);

    serialPort = new QSerialPort;
    deviceDataStream = new QByteArray;
    deviceDataBuffer = new QStringList;

    isAvailable = false;

    maximumForce = 0;
    coefficient = 0;
    normalForce = 0;

    connect(ui->actionAboutMovimento, SIGNAL(triggered()), this, SLOT(showAboutMessage()));
    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(setDefaultSettings()));
    connect(ui->resetToolsButton, SIGNAL(clicked()), this, SLOT(setDefaultToolsSettings()));
    connect(ui->massToolsField, SIGNAL(valueChanged(double)), this, SLOT(updateForceUi()));
    connect(ui->gravitationalAccelerationToolsField, SIGNAL(valueChanged(double)), this, SLOT(updateForceUi()));
    connect(ui->frictionAngleToolsField, SIGNAL(valueChanged(double)), this, SLOT(updateForceUi()));
    connect(ui->measuredDataToolsField, SIGNAL(textChanged(const QString &)), this, SLOT(updateStatisticsUi()));
    connect(ui->connectButton, SIGNAL(toggled(bool)), this, SLOT(connectDevice(bool)));
    connect(ui->startButton, SIGNAL(toggled(bool)), this, SLOT(simulate(bool)));

    hideDataViewer();

    updateSerialPort();
}

Movimento::~Movimento() {
    delete ui;

    delete xAxis, yAxis;
    delete splineSeries;
    delete chart;
    delete chartView;

    if (serialPort->isOpen()) {
        serialPort->clear();
        serialPort->close();
    }

    delete serialPort;
    delete deviceDataStream;
    delete deviceDataBuffer;

    foreach (ForceData * forceData, forceDataBuffer) {
        delete forceData;
    }

    forceDataBuffer.clear();
}

void Movimento::updateSerialPort() {
    isAvailable = false;

    foreach (const QSerialPortInfo & serialPortInfo, QSerialPortInfo::availablePorts()) {
        if (serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()) {
            if (serialPortInfo.vendorIdentifier() == deviceVendorId &&
                    serialPortInfo.productIdentifier() == deviceProductId) {
                portName = serialPortInfo.portName();
                isAvailable = true;
            }
        }
    }
}

void Movimento::showDataViewer() {
    ui->dataRequestWarning->setVisible(false);

    ui->coefficientLabel->setVisible(true);
    ui->coefficientField->setVisible(true);

    ui->maximumForceLabel->setVisible(true);
    ui->maximumForceField->setVisible(true);

    ui->normalForceLabel->setVisible(true);
    ui->normalForceField->setVisible(true);

    chartView->setVisible(true);
}

void Movimento::hideDataViewer() {
    ui->dataRequestWarning->setVisible(true);

    ui->coefficientLabel->setVisible(false);
    ui->coefficientField->setVisible(false);

    ui->maximumForceLabel->setVisible(false);
    ui->maximumForceField->setVisible(false);

    ui->normalForceLabel->setVisible(false);
    ui->normalForceField->setVisible(false);

    chartView->setVisible(false);

    ui->staticFrictionGroup->setMinimumHeight(0);
}

void Movimento::setDefaultDataViewer() {
    ui->progressBar->setValue(0);
    ui->startButton->setText("Start");
    ui->startButton->setChecked(false);

    hideDataViewer();

    disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(readDataStream()));
}

void Movimento::removeAllData() {
    deviceDataStream->clear();
    deviceDataBuffer->clear();

    foreach (ForceData * forceData, forceDataBuffer) {
        delete forceData;
    }

    forceDataBuffer.clear();
}

void Movimento::setDefaultUi() {
    ui->inspectorTab->setEnabled(false);
    ui->objectGroup->setEnabled(false);
    chartView->setEnabled(false);
    ui->connectButton->setText("Connect");
    ui->connectButton->setChecked(false);

    setDefaultDataViewer();
    removeAllData();

    if (serialPort->isOpen()) {
        serialPort->clear();
        serialPort->close();
    }
}

void Movimento::computeForce(float & maximumForce, float & coefficient, float & normalForce) {
    float weightForce = ui->massField->value() * ui->gravitationalAccelerationField->value();

    QStringList::iterator deviceDataIt = deviceDataBuffer->begin();

    for (deviceDataIt; deviceDataIt != (deviceDataBuffer->end() - 1); deviceDataIt += 2) {
        ForceData * forceData = new ForceData;

        forceData->angle = (*(deviceDataIt + 1)).toFloat();
        forceData->force = weightForce * sin(radians(forceData->angle));

        forceDataBuffer.push_back(forceData);
    }

    maximumForce = forceDataBuffer.back()->force;
    coefficient = tan(radians(forceDataBuffer.back()->angle));

    if (coefficient != 0) {
        normalForce = maximumForce / coefficient;
    }
}

void Movimento::updateUi() {
    maximumForce = 0;
    coefficient = 0;
    normalForce = 0;

    computeForce(maximumForce, coefficient, normalForce);

    ui->maximumForceField->setValue(maximumForce);
    ui->coefficientField->setValue(coefficient);
    ui->normalForceField->setValue(normalForce);
    ui->staticFrictionGroup->setMinimumHeight(400);

    yAxis->setRange(0, (maximumForce != 0) ? maximumForce * 1.25 : 10.0);
    yAxis->setTickCount(5 + maximumForce * 0.05);

    splineSeries->clear();

    deviceDataStream->clear();
    deviceDataBuffer->clear();

    foreach (ForceData * forceData, forceDataBuffer) {
        splineSeries->append(forceData->angle, forceData->force);

        delete forceData;
    }

    forceDataBuffer.clear();
}

void Movimento::closeEvent(QCloseEvent * event) {
    if (serialPort->isOpen()) {
        serialPort->write("stop");
        serialPort->flush();

        removeAllData();
    }

    event->accept();
}

void Movimento::showAboutMessage() {
    QString aboutText = "<span style=\"font-size:14px;font-weight:bold\">Movimento 1.0.0</span><br><br>"
                        "Copyright \302\251 2016 Danilo Peixoto.<br>"
                        "All rights reserved.";

    QPixmap movimentoIcon(":/movimento/res/icons/movimento.png");

    QMessageBox aboutMessage(this);
    aboutMessage.setWindowTitle("About Movimento");
    aboutMessage.setText(aboutText);
    aboutMessage.setIconPixmap(movimentoIcon);
    aboutMessage.setStandardButtons(QMessageBox::Close);
    aboutMessage.exec();
}

void Movimento::setDefaultSettings() {
    ui->massField->setValue(1.0);
    ui->gravitationalAccelerationField->setValue(9.8);
}

void Movimento::setDefaultToolsSettings() {
    ui->massToolsField->setValue(1.0);
    ui->gravitationalAccelerationToolsField->setValue(9.8);
    ui->frictionAngleToolsField->setValue(0);
    ui->measuredDataToolsField->setText("");
}

void Movimento::updateForceUi() {
    float toolsFrictionForce, toolsCoefficient, toolsNormalForce, frictionAngle;

    toolsFrictionForce = 0;
    toolsCoefficient = 0;
    toolsNormalForce = 0;
    frictionAngle = ui->frictionAngleToolsField->value();

    if (frictionAngle > 0 && frictionAngle < 90) {
        float frictionAngleRadians = radians(frictionAngle);

        toolsFrictionForce = ui->massToolsField->value() *
                ui->gravitationalAccelerationToolsField->value() * sin(frictionAngleRadians);
        toolsCoefficient = tan(frictionAngleRadians);
        toolsNormalForce = toolsFrictionForce / toolsCoefficient;
    }

    ui->maximumForceToolsField->setValue(toolsFrictionForce);
    ui->coefficientToolsField->setValue(toolsCoefficient);
    ui->normalForceToolsField->setValue(toolsNormalForce);
}

void Movimento::updateStatisticsUi() {
    vector<float> measuredDataFloat;
    QStringList measuredDataString, measuredDataFloatString;
    float measuredDataValue;
    bool status;

    QString measuredRawData = ui->measuredDataToolsField->text();

    measuredDataString = measuredRawData.split(QRegExp("\\s+"), QString::SkipEmptyParts);

    if (measuredDataString.size() != 0) {
        foreach (const QString & data, measuredDataString) {
            measuredDataValue = data.toFloat(&status);

            if (status) {
                measuredDataFloat.push_back(measuredDataValue);
                measuredDataFloatString.append(data);
            }
            else {
                ui->measuredDataToolsField->setText(measuredDataFloatString.join(' '));

                QMessageBox::information(this, "Invalid Data", "Could not compute mean and standard error.\n"
                                                                "Please enter numeric data separated by whitespace.");

                break;
            }
        }

        if (measuredDataFloat.size() != 0) {
            ui->meanToolsField->setValue(mean(measuredDataFloat));
            ui->errorToolsField->setValue(standardDeviation(measuredDataFloat) / sqrt(measuredDataFloat.size()));
        }
    }
    else {
        ui->meanToolsField->setValue(0);
        ui->errorToolsField->setValue(0);
    }
}

void Movimento::connectDevice(bool isChecked) {
    updateSerialPort();

    if (isChecked) {
        if (isAvailable) {
            ui->inspectorTab->setEnabled(true);
            ui->objectGroup->setEnabled(true);
            chartView->setEnabled(true);
            ui->connectButton->setText("Disconnect");

            serialPort->setPortName(portName);
            serialPort->open(QSerialPort::ReadWrite);
            serialPort->setBaudRate(QSerialPort::Baud9600);
            serialPort->setDataBits(QSerialPort::Data8);
            serialPort->setParity(QSerialPort::NoParity);
            serialPort->setStopBits(QSerialPort::OneStop);
            serialPort->setFlowControl(QSerialPort::NoFlowControl);

            connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
                    this, SLOT(handleError(QSerialPort::SerialPortError)));
        }
        else {
            ui->connectButton->setChecked(false);

            QMessageBox::information(this, "Connection Information", "Could not connect with device.\n"
                                                                     "Please connect your device and try again.");
        }
    }
    else {
        disconnect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
                   this, SLOT(handleError(QSerialPort::SerialPortError)));

        setDefaultUi();
    }
}

void Movimento::simulate(bool isChecked) {
    if (isChecked) {
        updateSerialPort();

        if (serialPort->isOpen() && isAvailable) {
            ui->startButton->setText("Stop");
            ui->settingsTab->setEnabled(false);

            removeAllData();
            serialPort->clear();

            connect(serialPort, SIGNAL(readyRead()), this, SLOT(readDataStream()));

            serialPort->write("start");
            serialPort->flush();
        }
        else {
            setDefaultUi();

            QMessageBox::critical(this, "Connection Error", "Could not communicate with device.\n"
                                                            "Try reconnecting or restarting your device.");
        }
    }
    else {
        setDefaultDataViewer();

        serialPort->write("stop");
        serialPort->flush();

        ui->settingsTab->setEnabled(true);
    }
}

void Movimento::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        disconnect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
                   this, SLOT(handleError(QSerialPort::SerialPortError)));

        setDefaultUi();

        QMessageBox::critical(this, "Connection Error", "Could not communicate with device.\n"
                                                        "Try reconnecting or restarting your device.");
    }
}

void Movimento::readDataStream() {
    deviceDataStream->append(serialPort->readAll());
    serialPort->flush();

    QString deviceDataString = QString::fromStdString(deviceDataStream->toStdString());

    deviceDataBuffer->clear();
    deviceDataBuffer->append(deviceDataString.split(QRegExp("\\s+"), QString::SkipEmptyParts));

    if (!deviceDataBuffer->isEmpty()) {
        ui->progressBar->setValue(deviceDataBuffer->size() / 1.82);

        if (deviceDataBuffer->back() == "stop") {
            ui->progressBar->setValue(100);
            ui->startButton->setText("Start");
            ui->settingsTab->setEnabled(true);
            ui->startButton->setChecked(false);

            if (deviceDataBuffer->size() != 1 && (*(deviceDataBuffer->end() - 2)).toFloat() != 90.0) {
                updateUi();
                showDataViewer();
            }
            else {
                removeAllData();
                hideDataViewer();
            }

            disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(readDataStream()));
        }
    }
}
