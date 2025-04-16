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

private:
    QTableWidget *table_widget;
    bool stopRequested;

public slots:
    void requestStop();  // 请求停止线程
};

#endif // TESTTHREAD_H
