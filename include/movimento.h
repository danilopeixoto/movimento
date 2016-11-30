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

#ifndef MOVIMENTO_H
#define MOVIMENTO_H

#include "ui_movimento.h"
#include "data.h"

#include <QMainWindow>
#include <QtCharts/QValueAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QChartView>
#include <QSerialPort>
#include <QGraphicsEffect>
#include <QList>

QT_CHARTS_USE_NAMESPACE

class Movimento : public QMainWindow {
    Q_OBJECT

public:
    Movimento(QWidget * parent = Q_NULLPTR);
    ~Movimento();

private:
    Ui::MovimentoWindow * ui;

    QValueAxis * xAxis, * yAxis;
    QSplineSeries * splineSeries;
    QChart * chart;
    QChartView * chartView;

    QSerialPort * serialPort;
    QByteArray * deviceDataStream;
    QStringList * deviceDataBuffer;

    QList<ForceData *> forceDataBuffer;

    static const quint16 deviceVendorId = 9025;
    static const quint16 deviceProductId = 67;

    QString portName;
    bool isAvailable;

    float maximumForce, coefficient, normalForce;

    void updateSerialPort();

    void showDataViewer();
    void hideDataViewer();
    void setDefaultDataViewer();

    void removeAllData();

    void setDefaultUi();

    void computeForce(float &, float &, float &);

    void updateUi();

    void closeEvent(QCloseEvent *);

private slots:
    void showAboutMessage();
    void setDefaultSettings();
    void setDefaultToolsSettings();

    void updateForceUi();
    void updateStatisticsUi();

    void connectDevice(bool);
    void simulate(bool);

    void handleError(QSerialPort::SerialPortError);

    void readDataStream();
};

#endif
