#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QVector>

QT_FORWARD_DECLARE_CLASS(SqliteDb)
QT_FORWARD_DECLARE_CLASS(SqliteStmt)

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(const QString &filePath,
                             QObject *parent = nullptr);
    virtual ~DatabaseManager();

    bool initialize();

    SqliteDb *sqliteDb() const { return m_sqliteDb; }

    void logProcNew(const QString &appPath, bool &isNew);
    void logStatTraf(quint16 procCount, const quint8 *procBits,
                     const quint32 *trafBytes);
    void logClear();

    void getAppList(QStringList &list, QVector<qint64> &appIds);

    qint64 getAppId(const QString &appPath);

    qint32 getTrafficTime(const char *sql, qint64 appId = 0);

    void getTraffic(const char *sql, qint32 trafTime,
                    qint64 &inBytes, qint64 &outBytes,
                    qint64 appId = 0);

signals:

public slots:

private:
    typedef QList<SqliteStmt *> QStmtList;

    bool createTables();

    qint64 createAppId(const QString &appPath);

    void updateTrafficList(const QStmtList &insertStmtList,
                           const QStmtList &updateStmtList,
                           quint32 inBytes, quint32 outBytes,
                           qint64 appId = 0);

    bool updateTraffic(SqliteStmt *stmt, quint32 inBytes,
                       quint32 outBytes, qint64 appId = 0);

    SqliteStmt *getTrafficStmt(const char *sql, qint32 trafTime);

    SqliteStmt *getSqliteStmt(const char *sql);

private:
    qint32 m_lastTrafHour;
    qint32 m_lastTrafDay;
    qint32 m_lastTrafMonth;

    QString m_filePath;

    SqliteDb *m_sqliteDb;

    QHash<const char *, SqliteStmt *> m_sqliteStmts;

    QStringList m_appPaths;
    QVector<qint64> m_appIds;
};

#endif // DATABASEMANAGER_H