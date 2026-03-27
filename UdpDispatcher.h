#ifndef UDPDISPATCHER_H
#define UDPDISPATCHER_H

#include <QObject>
#include <QHostAddress>
#include <QHash>
#include <QByteArray>
#include <functional>

// 定义哈希键结构
struct RouteKey {
    QHostAddress ip;
    quint16 port;

    bool operator==(const RouteKey &other) const {
        return (port == other.port) && (ip == other.ip);
    }
};

// 哈希函数实现
inline uint qHash(const RouteKey &key, uint seed = 0) {
    return qHash(key.ip.toString(), seed) ^ qHash(key.port, seed);
}


// 转发规则结构：必须支持 2 个参数 (const QByteArray&, double)
struct Rule {
    std::function<void(const QByteArray&, double)> callback; // 修改这里
    QObject* receiver;
};

class UdpDispatcher : public QObject {
    Q_OBJECT
public:
    explicit UdpDispatcher(QObject *parent = nullptr);
    virtual ~UdpDispatcher() = default;

    /**
     * @brief 添加分流规则 (模板实现必须放在头文件)
     * @param T 目标类类型
     * @param method 成员函数指针，例如 &RtpParser::inputPacket
     */
    /**
         * @brief 重载1：支持 2 个参数的成员函数 (data, timestamp)
         */
        template <typename T>
        void addRule(const QHostAddress &ip, quint16 port, T* parser,
                     void (T::*method)(const QByteArray&, double)) {
            RouteKey key{ip, port};
            Rule rule;
            rule.receiver = parser;

            // 绑定带两个参数的函数
            rule.callback = [parser, method](const QByteArray &payload, double ts) {
                (parser->*method)(payload, ts);
            };

            routeTable.insert(key, rule);

            connect(parser, &QObject::destroyed, this, [this, key](){
                routeTable.remove(key);
            });
        }

        /**
         * @brief 重载2：兼容 1 个参数的旧成员函数 (data)
         */
        template <typename T>
        void addRule(const QHostAddress &ip, quint16 port, T* parser,
                     void (T::*method)(const QByteArray&)) {
            RouteKey key{ip, port};
            Rule rule;
            rule.receiver = parser;

            // 包装：忽略传入的 timestamp，只调用单参数函数
            rule.callback = [parser, method](const QByteArray &payload, double /*ts*/) {
                (parser->*method)(payload);
            };

            routeTable.insert(key, rule);

            connect(parser, &QObject::destroyed, this, [this, key](){
                routeTable.remove(key);
            });
        }

public slots:
    //void dispatch(const QHostAddress &ip, quint16 port, const QByteArray &payload);
    void dispatch(const QHostAddress &ip, quint16 port, const QByteArray &payload, double timestamp);

private:
    QHash<RouteKey, Rule> routeTable;
};

#endif // UDPDISPATCHER_H
