#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <QThread>
#include <QTableWidget>
#include <QString>

class TestThread : public QThread
{
    Q_OBJECT

public:
    TestThread(QTableWidget *tableWidget, QObject *parent = nullptr);
    ~TestThread();

protected:
    void run() override;

signals:
    void updateLog(const QString &message);  // 自定义信号，用于更新日志
    void updateProgress(int row, int value);  // 自定义信号，用于更新进度条
    void updateResult(int row, const QString &result);  // 自定义信号，用于更新结果列

private:
    QTableWidget *table_widget;
    bool stopRequested;

    // 执行cskburn.exe命令烧录固件
    bool runCskBurn(const QString &comPort, int baudRate, const QString &address, const QString &binFile, QString &output);

public slots:
    void requestStop();  // 请求停止线程
};

#endif // TESTTHREAD_H
