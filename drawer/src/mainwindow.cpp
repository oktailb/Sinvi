#include "include/mainwindow.h"
#include "./ui_mainwindow.h"
#include "pCapUtils.h"
#include "variables.h"
#include "configuration.h"
#include <stdint.h>
#include <stdfloat>
#include <QChart>
#include <QLineSeries>
#include <netinet/in.h>

DebugWindow::DebugWindow(std::map<std::string, std::string> &configuration, ThreadSafeQueue &queue, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DebugWindow)
    , configuration(configuration)
    , queue(queue.data())
    , timerId(0)
{
    extractVariablesFromConfiguration(configuration, variables);

    ui->setupUi(this);
    fillVariables(variables);
    timerId = startTimer(20);
}

void DebugWindow::fillVariables(std::map<std::string, varDef_t> &variablesList)
{
    for (auto item : variablesList)
        ui->variables->insertItem(ui->variables->count() + 1, QString(item.first.c_str()), item.second.offset);
    for (auto type : stringDataType)
        ui->type->insertItem(ui->type->count() + 1, QString(type.first.c_str()), type.second);
    for (auto type : stringDataSize)
        ui->size->insertItem(ui->size->count() + 1, QString(type.first.c_str()), type.second);
}

DebugWindow::~DebugWindow()
{
    killTimer(timerId);
    delete ui;
}

void DebugWindow::timerEvent(QTimerEvent *event)
{
    QMainWindow::timerEvent(event);
    ui->centralwidget->repaint();
}

QLineSeries * DebugWindow::SeriesFromOffset(uint32_t offset, uint32_t size, DataType type, bool toHostEndian, uint64_t mask, uint8_t shift, double ratio)
{
    QLineSeries *series = new QLineSeries();
    //series->append(0, 0);
    double timestamp0 = queue[0].ts.tv_sec * 1000 + queue[0].ts.tv_usec / 1000;
    for (uint32_t pkt = 0 ; pkt < queue.size() ; pkt++)
    {
        double timestamp = (queue[pkt].ts.tv_sec * 1000 + queue[pkt].ts.tv_usec / 1000 - timestamp0) / 1000.0;
        uint8_t *mapper = (uint8_t*) queue[pkt].payload.data();

        switch(type)
        {
        case DataType::e_float:
            switch(size)
            {
            case DataSize::e_32: {
                float tmp;
                memcpy(&tmp, mapper + offset, sizeof(float));
                if (toHostEndian) ntoh<float>(tmp);
                series->append(timestamp, tmp / ratio);
            }
            break;
            case DataSize::e_64: {
                double tmp;
                memcpy(&tmp, mapper + offset, sizeof(double));
                if (toHostEndian) ntoh<double>(tmp);
                series->append(timestamp, tmp / ratio);
            }
            break;
            default:
                break;
            }
            break;
        case DataType::e_int:
            switch(size)
            {
            case DataSize::e_8:
                series->append(timestamp, (int8_t)mapper[offset]);
                break;
            case DataSize::e_16: {
                int16_t tmp;
                memcpy(&tmp, mapper + offset, sizeof(int16_t));
                if (toHostEndian) tmp = ntoh<int16_t>(tmp);
                series->append(timestamp, ((tmp & mask) >> shift) / ratio);
            }
            break;
            case DataSize::e_32: {
                int32_t tmp;
                memcpy(&tmp, mapper + offset, sizeof(int32_t));
                if (toHostEndian) tmp = ntoh<int32_t>(tmp);
                series->append(timestamp, ((tmp & mask) >> shift) / ratio);
            }
            break;
            case DataSize::e_64: {
                int64_t tmp;
                memcpy(&tmp, mapper + offset, sizeof(int64_t));
                if (toHostEndian) tmp = ntoh<int64_t>(tmp);
                series->append(timestamp, ((tmp & mask) >> shift) / ratio);
            }
            break;
            default:
                break;
            }
            break;
        case DataType::e_uint:
            switch(size)
            {
            case DataSize::e_8:
                series->append(timestamp, (uint8_t)mapper[offset]);
                break;
            case DataSize::e_16: {
                uint16_t tmp;
                memcpy(&tmp, mapper + offset, sizeof(uint16_t));
                if (toHostEndian) tmp = ntoh<uint16_t>(tmp);
                series->append(timestamp, ((tmp & mask) >> shift) / ratio);
            }
            break;
            case DataSize::e_32: {
                uint32_t tmp;
                memcpy(&tmp, mapper + offset, sizeof(uint32_t));
                if (toHostEndian) tmp = ntoh<int32_t>(tmp);
                series->append(timestamp, ((tmp & mask) >> shift) / ratio);
            }
            break;
            case DataSize::e_64: {
                uint64_t tmp;
                memcpy(&tmp, mapper + offset, sizeof(uint64_t));
                if (toHostEndian) tmp = ntoh<uint64_t>(tmp);
                series->append(timestamp, ((tmp & mask) >> shift) / ratio);
            }
            break;
            default:
                break;
            }
            break;

        case DataType::e_string:
            break;
        default:
            break;
        }
    }
    return series;
}
void DebugWindow::updateChart(std::string name,
                              uint32_t offset,
                              DataSize size,
                              DataType type,
                              bool toHostEndian,
                              uint64_t mask,
                              uint8_t shift,
                              double ratio)
{
    QLineSeries *series = SeriesFromOffset(offset, size, type, toHostEndian, mask, shift, ratio);
    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle(name.c_str());

    ui->chart->setChart(chart);
}

void DebugWindow::on_variables_currentIndexChanged(int index)
{
    std::string name = ui->variables->currentText().toStdString();
    DataSize size = variables[name].size;
    uint32_t offset = variables[name].offset;
    DataType type = variables[name].type;
    Qt::CheckState endian = (variables[name].endian == DataEndian::e_host)?Qt::CheckState::Checked:Qt::CheckState::Unchecked;
    bool toHostEndian = (variables[name].endian == DataEndian::e_host)?true:false;
    uint64_t mask = variables[name].mask;
    uint8_t shift = variables[name].shift;
    double ratio = variables[name].ratio;

    ui->offset->setValue(offset);
    ui->size->setCurrentText(QString(intDataSize.at(size).c_str()));
    ui->type->setCurrentText(QString(intDataType.at(type).c_str()));
    ui->endian->setCheckState(endian);
    ui->mask->setText(QString::number(mask, 16));
    ui->shift->setValue(shift);
    ui->ratio->setValue(ratio);

    updateChart(name, offset, size, type, toHostEndian, mask, shift, ratio);
}

void DebugWindow::on_offset_valueChanged(int offset)
{
    std::string name = ui->variables->currentText().toStdString();
    DataSize size = variables[name].size;
    variables[name].offset = offset;
    DataType type = variables[name].type;
    Qt::CheckState endian = (variables[name].endian == DataEndian::e_host)?Qt::CheckState::Checked:Qt::CheckState::Unchecked;
    bool toHostEndian = (variables[name].endian == DataEndian::e_host)?true:false;
    uint64_t mask = variables[name].mask;
    uint8_t shift = variables[name].shift;
    double ratio = variables[name].ratio;

    updateChart(name, offset, size, type, toHostEndian, mask, shift, ratio);
}

void DebugWindow::on_type_currentTextChanged(const QString &typeStr)
{
    std::string name = ui->variables->currentText().toStdString();
    DataSize size = variables[name].size;
    uint32_t offset = variables[name].offset;
    variables[name].type = stringDataType.at(typeStr.toStdString());
    Qt::CheckState endian = (variables[name].endian == DataEndian::e_host)?Qt::CheckState::Checked:Qt::CheckState::Unchecked;
    bool toHostEndian = (variables[name].endian == DataEndian::e_host)?true:false;
    uint64_t mask = variables[name].mask;
    uint8_t shift = variables[name].shift;
    double ratio = variables[name].ratio;

    updateChart(name, offset, size, stringDataType.at(typeStr.toStdString()), toHostEndian, mask, shift, ratio);
}


void DebugWindow::on_size_currentTextChanged(const QString &sizeStr)
{
    std::string name = ui->variables->currentText().toStdString();
    variables[name].size = stringDataSize.at(sizeStr.toStdString());
    uint32_t offset = variables[name].offset;
    DataType type= variables[name].type;
    Qt::CheckState endian = (variables[name].endian == DataEndian::e_host)?Qt::CheckState::Checked:Qt::CheckState::Unchecked;
    bool toHostEndian = (variables[name].endian == DataEndian::e_host)?true:false;
    uint64_t mask = variables[name].mask;
    uint8_t shift = variables[name].shift;
    double ratio = variables[name].ratio;

    updateChart(name, offset, stringDataSize.at(sizeStr.toStdString()), type, toHostEndian, mask, shift, ratio);
}


void DebugWindow::on_endian_checkStateChanged(const Qt::CheckState &state)
{
    std::string name = ui->variables->currentText().toStdString();
    DataSize size = variables[name].size;
    uint32_t offset = variables[name].offset;
    DataType type= variables[name].type;
    bool toHostEndian = (ui->endian->checkState() == Qt::CheckState::Checked);
    variables[name].endian = (ui->endian->checkState() == Qt::CheckState::Checked)?DataEndian::e_host:DataEndian::e_network;
    uint64_t mask = variables[name].mask;
    uint8_t shift = variables[name].shift;
    double ratio = variables[name].ratio;

    updateChart(name, offset, size, type, toHostEndian, mask, shift, ratio);
}

void DebugWindow::on_mask_textChanged(const QString &mask)
{
    std::string name = ui->variables->currentText().toStdString();
    DataSize size = variables[name].size;
    uint32_t offset = variables[name].offset;
    DataType type= variables[name].type;
    Qt::CheckState endian = (variables[name].endian == DataEndian::e_host)?Qt::CheckState::Checked:Qt::CheckState::Unchecked;
    bool toHostEndian = (variables[name].endian == DataEndian::e_host)?true:false;
    uint64_t newMask = ui->mask->text().toULong(nullptr, 16);
    variables[name].mask = newMask;
    uint8_t shift = variables[name].shift;
    double ratio = variables[name].ratio;

    updateChart(name, offset, size, type, toHostEndian, newMask, shift, ratio);
}

void DebugWindow::on_shift_valueChanged(int shift)
{
    std::string name = ui->variables->currentText().toStdString();
    DataSize size = variables[name].size;
    uint32_t offset = variables[name].offset;
    DataType type= variables[name].type;
    Qt::CheckState endian = (variables[name].endian == DataEndian::e_host)?Qt::CheckState::Checked:Qt::CheckState::Unchecked;
    bool toHostEndian = (variables[name].endian == DataEndian::e_host)?true:false;
    uint64_t mask = variables[name].mask;
    variables[name].shift = shift;
    double ratio = variables[name].ratio;

    updateChart(name, offset, size, type, toHostEndian, mask, shift, ratio);
}

void DebugWindow::on_ratio_valueChanged(double ratio)
{
    std::string name = ui->variables->currentText().toStdString();
    DataSize size = variables[name].size;
    uint32_t offset = variables[name].offset;
    DataType type= variables[name].type;
    Qt::CheckState endian = (variables[name].endian == DataEndian::e_host)?Qt::CheckState::Checked:Qt::CheckState::Unchecked;
    bool toHostEndian = (variables[name].endian == DataEndian::e_host)?true:false;
    uint64_t mask = variables[name].mask;
    uint8_t shift = variables[name].shift;
    variables[name].ratio = ratio;

    updateChart(name, offset, size, type, toHostEndian, mask, shift, ratio);
}


void DebugWindow::on_applyButton_clicked()
{
    std::string name = ui->variables->currentText().toStdString();
    DataSize size = variables[name].size;
    uint32_t offset = variables[name].offset;
    DataType type = variables[name].type;
    Qt::CheckState endian = (variables[name].endian == DataEndian::e_host)?Qt::CheckState::Checked:Qt::CheckState::Unchecked;
    bool toHostEndian = (variables[name].endian == DataEndian::e_host)?true:false;
    uint64_t mask = variables[name].mask;
    uint8_t shift = variables[name].shift;
    double ratio = variables[name].ratio;

    QString out = "";

    out += "0x" + QString::number(offset, 16) + ",";
    out += intDataType.at(type) + ",";
    out += intDataSize.at(size) + ",";
    if (type != DataType::e_string)
    {
        out += QString(toHostEndian?"host":"network") + ",";
        out += "0x" + QString::number(mask, 16) + ",";
        out += QString::number(shift) + ",";
        out += QString::number(ratio) + ",";
    }
    configuration["Vars/" + name] = out.toStdString();
    saveConfiguration(configuration, "config.ini");
}

