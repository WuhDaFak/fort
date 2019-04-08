#include "sqlitedb.h"

#include <QDebug>
#include <QDir>

#include <sqlite3.h>

SqliteDb::SqliteDb() :
    m_db(nullptr)
{
    sqlite3_initialize();
}

SqliteDb::~SqliteDb()
{
    close();

    sqlite3_shutdown();
}

bool SqliteDb::open(const QString &filePath)
{
    return sqlite3_open16(filePath.utf16(), &m_db) == SQLITE_OK;
}

void SqliteDb::close()
{
    if (m_db != nullptr) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool SqliteDb::execute(const char *sql)
{
    return sqlite3_exec(m_db, sql, nullptr, nullptr, nullptr) == SQLITE_OK;
}

bool SqliteDb::execute16(const ushort *sql)
{
    sqlite3_stmt *stmt = nullptr;
    int res = sqlite3_prepare16_v2(db(), sql, -1, &stmt, nullptr);
    if (stmt != nullptr) {
        res = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    return res;
}

bool SqliteDb::executeStr(const QString &sql)
{
    return execute16(sql.utf16());
}

QVariant SqliteDb::executeEx(const char *sql,
                              const QVariantList &vars)
{
    QVariant res;
    QStringList bindTexts;
    sqlite3_stmt *stmt = nullptr;

    sqlite3_prepare_v2(db(), sql, -1, &stmt, nullptr);
    if (stmt != nullptr) {
        // Bind variables
        if (!vars.isEmpty()) {
            int index = 0;
            for (const QVariant &v : vars) {
                ++index;
                switch (v.type()) {
                case QVariant::Int:
                    sqlite3_bind_int(stmt, index, v.toInt());
                    break;
                case QVariant::Double:
                    sqlite3_bind_double(stmt, index, v.toDouble());
                    break;
                case QVariant::String: {
                    const QString text = v.toString();
                    const int bytesCount = text.size() * int(sizeof(wchar_t));
                    bindTexts.append(text);

                    sqlite3_bind_text16(stmt, index, text.utf16(),
                                        bytesCount, SQLITE_STATIC);
                    break;
                }
                default:
                    Q_UNREACHABLE();
                }
            }
        }

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            switch (sqlite3_column_type(stmt, 0)) {
            case SQLITE_INTEGER:
                res = sqlite3_column_int64(stmt, 0);
                break;
            case SQLITE_FLOAT:
                res = sqlite3_column_double(stmt, 0);
                break;
            case SQLITE_TEXT:
                res = QString::fromUtf8(reinterpret_cast<const char *>(
                                            sqlite3_column_text(stmt, 0)));
                break;
            default:
                Q_UNREACHABLE();
            }
        }
        sqlite3_finalize(stmt);
    }

    return res;
}

qint64 SqliteDb::lastInsertRowid() const
{
    return sqlite3_last_insert_rowid(m_db);
}

int SqliteDb::changes() const
{
    return sqlite3_changes(m_db);
}

bool SqliteDb::beginTransaction()
{
    return execute("BEGIN;");
}

bool SqliteDb::commitTransaction()
{
    return execute("COMMIT;");
}

bool SqliteDb::rollbackTransaction()
{
    return execute("ROLLBACK;");
}

bool SqliteDb::beginSavepoint(const char *name)
{
    return (name == nullptr)
            ? execute("SAVEPOINT _;")
            : executeStr(QString("SAVEPOINT %1;").arg(name));
}

bool SqliteDb::releaseSavepoint(const char *name)
{
    return (name == nullptr)
            ? execute("RELEASE _;")
            : executeStr(QString("RELEASE %1;").arg(name));
}

bool SqliteDb::rollbackSavepoint(const char *name)
{
    return (name == nullptr)
            ? execute("ROLLBACK TO _;")
            : executeStr(QString("ROLLBACK TO %1;").arg(name));
}

QString SqliteDb::errorMessage() const
{
    const char *text = sqlite3_errmsg(m_db);

    return QString::fromUtf8(text);
}

int SqliteDb::userVersion()
{
    return executeEx("PRAGMA user_version;").toInt();
}

bool SqliteDb::migrate(const QString &sqlDir, int version,
                       SQLITEDB_MIGRATE_FUNC migrateFunc,
                       void *migrateContext)
{
    // Check version
    const int userVersion = this->userVersion();
    if (userVersion == version)
        return true;

    if (userVersion > version) {
        qWarning() << "SQLite: Cannot open new DB" << userVersion
                   << "from old code" << version;
        return false;
    }

    // Run migration SQL scripts
    QDir dir(sqlDir);
    bool res = true;

    beginTransaction();
    for (int i = userVersion + 1; i <= version; ++i) {
        const QString filePath = dir.filePath(QString::number(i) + ".sql");

        QFile file(filePath);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            qWarning() << "SQLite: Cannot open migration file" << filePath;
            res = false;
            break;
        }

        const QByteArray data = file.readAll();
        if (data.isEmpty()) {
            qWarning() << "SQLite: Migration file is empty" << filePath;
            res = false;
            break;
        }

        beginSavepoint();
        if (!execute(data.constData())
                || !(migrateFunc == nullptr
                     || migrateFunc(this, i, migrateContext))) {
            qWarning() << "SQLite: Migration error:" << filePath << errorMessage();
            res = false;
            rollbackSavepoint();
            break;
        }
        releaseSavepoint();
    }
    commitTransaction();

    return res;
}