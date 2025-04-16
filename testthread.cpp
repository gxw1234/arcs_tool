#include "testthread.h"
#include <QDebug>
#include <QLineEdit>
#include <QComboBox>

TestThread::TestThread(QTableWidget *tableWidget, QObject *parent)
    : QThread(parent), table_widget(tableWidget), stopRequested(false)
{
}

TestThread::~TestThread()
{
    requestStop();
    wait(); // 等待线程结束
}

void TestThread::requestStop()
{
    stopRequested = true;
}

void TestThread::run()
{
    // 发送开始测试的日志信号
    emit updateLog("自动化测试开始");
    
    // 遍历表格的每一行
    for (int row = 0; row < table_widget->rowCount(); ++row) {
        // 检查是否请求停止线程
        if (stopRequested) {
            emit updateLog("测试被用户终止");
            break;
        }
        
        // 获取当前行的测试项目名称
        if (table_widget->item(row, 0)) {
            QString testName = table_widget->item(row, 0)->text().trimmed();
            emit updateLog("已完成行 " + QString::number(row) + ": " + testName);
            
            // 模拟实际测试活动
            QThread::msleep(500); // 简单延时
        }
    }
    
    emit updateLog("所有测试已完成");
}

