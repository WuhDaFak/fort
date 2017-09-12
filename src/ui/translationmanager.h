#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QLocale>
#include <QObject>
#include <QStringList>
#include <QVector>

class QTranslator;

class TranslationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool dummyBool READ dummyBool NOTIFY dummyBoolChanged)
    Q_PROPERTY(int language READ language WRITE switchLanguage NOTIFY languageChanged)
    Q_PROPERTY(QStringList naturalLabels READ naturalLabels CONSTANT)

protected:
    explicit TranslationManager(QObject *parent = 0);
    virtual ~TranslationManager();

public:
    static TranslationManager *instance();

    bool dummyBool() const { return true; }

    int language() const { return m_language; }
    QString localeName() const { return m_locale.name(); }

    QStringList naturalLabels() const;

    int getLanguageByName(const QString &localeName) const;

signals:
    void dummyBoolChanged();
    void languageChanged(int language);

public slots:
    bool switchLanguage(int language = 0);
    bool switchLanguageByName(const QString &localeName);

    void refreshTranslations();

private:
    void setupTranslation();

    void uninstallAllTranslators();
    void uninstallTranslator(int language);

    void installTranslator(int language, const QLocale &locale);

    QTranslator *loadTranslator(int language, const QLocale &locale);

    static QString i18nDir();

private:
    int m_language;

    QLocale m_locale;

    QList<QLocale> m_locales;
    QVector<QTranslator *> m_translators;
};

#endif // TRANSLATIONMANAGER_H