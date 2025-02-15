#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "variables.h"
#include "ThreadSafeQueue.h"
#include "callout.h"
#include "videoDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class DebugWindow;
}
QT_END_NAMESPACE

class DebugWindow : public QMainWindow
{
    Q_OBJECT

public:
    DebugWindow(std::map<std::string, std::string> &configuration, ThreadSafeQueue &queue, QWidget *parent = nullptr);
    ~DebugWindow();

protected:
    void timerEvent(QTimerEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void closeEvent(QCloseEvent *e) override;

private slots:
    void on_variables_currentIndexChanged(int index);
    void on_variables_globalCheckStateChanged(int index);
    void on_offset_valueChanged(int offset);
    void on_type_currentTextChanged(const QString &arg1);
    void on_size_currentTextChanged(const QString &sizeStr);
    void on_mask_textChanged(const QString &mask);
    void on_shift_valueChanged(int shift);
    void on_ratio_valueChanged(double ratio);
    void on_applyButton_clicked();
    void on_endian_toggled(bool checked);
    void on_autoSearch_clicked();
    void on_len_valueChanged(int arg1);
    void on_actionFix_chart_range_triggered();
    void on_actionSave_Configuration_triggered();
    void on_actionExport_triggered();
    void on_sliderFrom_valueChanged(int value);
    void on_sliderTo_valueChanged(int value);
    void displayPlotValue(const QPointF &point, bool state);
    void on_actionhexdump_triggered(bool checked);
    void on_actiondata_listing_triggered(bool checked);
    void on_actionLicence_triggered();
    void on_Play_clicked();
    void on_Pause_clicked();
    void on_actionOpenPcap_triggered();
    void on_actionSpy_Device_triggered();
    void on_actionOpen_UDP_Socket_triggered();
    void on_actionStart_triggered();
    void on_actionStream_to_external_triggered(bool checked);
    void on_actionServer_mode_triggered(bool checked);

private:
    Ui::DebugWindow *ui;
    void fillVariables(std::map<std::string, varDef_t> &variables);
    bool SeriesFromOffset(uint32_t offset, uint32_t size, uint32_t len, DataType type, bool toHostEndian, uint64_t mask, uint8_t shift, double ratio);
    bool computeChartByCriteria(uint32_t offset,
                                DataSize size, uint32_t len,
                                DataType type,
                                bool toHostEndian,
                                uint64_t mask,
                                uint8_t shift,
                                double ratio, double diversityMin, double minVal, double maxVal, double amplitudeMin, double amplitudeMax);
    void updateChart(std::string name);
    void saveFGFSGenericProtocol(const std::map<std::string, std::string>& keyValuePairs, const std::string& filename);
    void showHexdump();
    void showDataAsText(std::vector<double> values);

    // Data related
    std::map<std::string, varDef_t>     variables;
    ThreadSafeQueue &                   queue;
    std::deque<PacketData*>             queueData;
    uint32_t                            packetSize;
    std::map<uint32_t, QLineSeries *>   series;
    std::map<uint32_t, uint32_t>        pktIndex;
    double                              timestamp;

    // UI related
    int                                 timerId;
    bool                                ready;

    QChart *                            chart;
    Callout *                           m_tooltip;
    QList<Callout *>                    m_callouts;
    QPointF                             currentPoint;

    videoDialog *                       videoDialogWindow;
    double                              timeWindowSize;
    bool                                offsetChange;
    bool                                formatChange;

    // Configuration related
    bool                                localFileMode;
    QString                             pcapFile;

    bool                                spyMode;
    QString                             netDevice;
    QString                             netFilter;

    bool                                socketMode;
    QString                             socketAddr;
    uint16_t                            socketPort;

    bool                                otherClient;
    bool                                relay;
    bool                                serverMode;
    std::map<std::string, std::string>  configuration;
};
#endif // MAINWINDOW_H
