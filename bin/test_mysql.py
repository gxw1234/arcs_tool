import pymysql
import argparse
import json


def connect_to_database():
    """连接到 MySQL 数据库并返回连接对象"""
    return pymysql.connect(
        host='192.168.32.130',  # 替换为您的树莓派的 IP 地址
        user='arcs',
        password='PI',
        database='mydatabase',
        cursorclass=pymysql.cursors.DictCursor
    )


def insert_data(connection, values):
    """插入数据到 arcs_scan_02 表"""
    with connection.cursor() as cursor:
        insert_query = """
        INSERT INTO arcs_scan_02 (SN, Site, result, chipid, log1, log2, log3)
        VALUES (%s, %s, %s, %s, %s, %s, %s)
        """
        cursor.execute(insert_query, values)
        connection.commit()
        print(f"成功插入数据: {values}")


def query_by_sn(connection, sn):
    """根据 SN 查询 arcs_scan_02 表中的数据，并以格式化形式打印结果"""
    with connection.cursor() as cursor:
        select_query = "SELECT * FROM arcs_scan_02 WHERE SN = %s"
        cursor.execute(select_query, (sn,))
        results = cursor.fetchall()
        if results:
            # for row in results:
            #     print(row)
            #     # 只输出 SN, Site, result 和 chipid 字段
            #     formatted_output = (
            #         f"SN: {row['SN']}, Site: {row['Site']}, result: {row['result']}, "
            #         f"chipid: {row['chipid']}"
            #     )
            # 初始化变量以存储最高的 Site 值
            highest_site = None
            # 遍历结果集
            for row in results:
                # 确保结果为 'PASS'
                if row['result'] == 'PASS':
                    # 如果还没有记录，或者找到更高的 Site 值
                    if highest_site is None or int(row['Site']) > int(highest_site):
                        highest_site = row['Site']
            # 输出找到的最高 Site 值
            if highest_site is not None:
                print("Site:", highest_site)
            else:
                print("Site:", 0)
        else:
            print(f"未找到 SN = {sn} 的数据")
def main():
    # 设置命令行参数解析
    parser = argparse.ArgumentParser(description='查询或写入 MySQL 数据库中的数据')
    subparsers = parser.add_subparsers(dest='command')  # 添加子解析器
    # 添加查询命令的解析器
    query_parser = subparsers.add_parser('query', help='查询数据')
    query_parser.add_argument('sn', type=str, help='要查询的 SN 号')
    # 添加写入命令的解析器
    insert_parser = subparsers.add_parser('insert', help='写入数据')
    insert_parser.add_argument('sn', type=str, help='SN 号')
    insert_parser.add_argument('site', type=int, help='Site 值')
    insert_parser.add_argument('result', type=str, help='结果 (PASS/FAIL)')
    insert_parser.add_argument('chipid', type=str, help='Chip ID')
    insert_parser.add_argument('logs', type=str, help='包含日志的 JSON 字符串')
    args = parser.parse_args()
    try:
        connection = connect_to_database()
        print("成功连接到 MySQL 数据库")

        if args.command == 'query':
            # 查询数据
            query_by_sn(connection, args.sn)
        elif args.command == 'insert':
            # 将日志 JSON 字符串解析为字典
            logs = json.loads(args.logs)
            print("接收到的 logs 参数:", repr(args.logs))
            try:
                logs = json.loads(args.logs)
            except json.JSONDecodeError as e:
                print("JSON 解析失败:", e)
                return
            # 提取 log1, log2 和 log3
            log1 = logs.get('log1', '')
            log2 = logs.get('log2', '')
            log3 = logs.get('log3', '')
            # 准备插入数据
            values = (args.sn, args.site, args.result, args.chipid, log1, log2, log3)
            insert_data(connection, values)

    except pymysql.MySQLError as e:
        print("连接或操作失败:", e)
    except json.JSONDecodeError as e:
        print("日志 JSON 解析失败:", e)
    finally:
        # 确保连接关闭
        if 'connection' in locals() and connection.open:
            connection.close()
            print("MySQL 连接已关闭")


if __name__ == "__main__":
    main()
