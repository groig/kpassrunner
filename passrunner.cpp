#include "passrunner.h"
#include <KRunner/RunnerContext>
#include <KRunner/QueryMatch>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QDir>
#include <QDebug>
#include <KLocalizedString>
#include <KNotification>

PassRunner::PassRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList & /*args*/)
    : KRunner::AbstractRunner(parent, metaData)
{
    setObjectName(QStringLiteral("passrunner"));
    addSyntax(KRunner::RunnerSyntax(QStringLiteral("pass :q:"),
                                   i18n("Search entries in password-store")));
    m_entries = getEntries();
}

QStringList PassRunner::getEntries() const
{
    // First, try to get the password store directory from pass
    QProcess passProc;
    passProc.start(QStringLiteral("pass"), QStringList() << QStringLiteral("git") << QStringLiteral("rev-parse") << QStringLiteral("--show-toplevel"));
    passProc.waitForFinished();

    QString passwordStoreDir;
    if (passProc.exitCode() == 0) {
        passwordStoreDir = QString::fromUtf8(passProc.readAllStandardOutput()).trimmed();
    } else {
        // Fallback to standard location
        passwordStoreDir = QDir::homePath() + QStringLiteral("/.password-store");
    }

    QProcess proc;
    proc.start(QStringLiteral("find"), QStringList()
               << passwordStoreDir
               << QStringLiteral("-name")
               << QStringLiteral("*.gpg")
               << QStringLiteral("-type")
               << QStringLiteral("f"));
    proc.waitForFinished();

    if (proc.exitCode() != 0) {
        // Fallback to pass ls if find fails
        proc.start(QStringLiteral("pass"), QStringList() << QStringLiteral("ls"));
        proc.waitForFinished();
        QString output = QString::fromUtf8(proc.readAllStandardOutput());
        QStringList lines = output.split(QStringLiteral("\n"), Qt::SkipEmptyParts);
        for (QString &line : lines) {
            line.remove(QRegularExpression(QStringLiteral("[│├└─ ]"))); // strip tree symbols
            line = line.trimmed();
        }
        return lines;
    }

    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    QStringList lines = output.split(QStringLiteral("\n"), Qt::SkipEmptyParts);
    QStringList entries;
    QString storeDirWithSlash = passwordStoreDir + QStringLiteral("/");

    for (const QString &line : lines) {
        QString entry = line.trimmed();
        // Remove the password store directory path and .gpg extension
        if (entry.startsWith(storeDirWithSlash)) {
            entry = entry.mid(storeDirWithSlash.length());
        }
        if (entry.endsWith(QStringLiteral(".gpg"))) {
            entry.chop(4); // Remove .gpg extension
        }
        if (!entry.isEmpty()) {
            entries << entry;
        }
    }

    return entries;
}

void PassRunner::match(KRunner::RunnerContext &context)
{
    const QString query = context.query().trimmed();
    if (query.isEmpty())
        return;

    for (const QString &entry : m_entries) {
        if (entry.contains(query, Qt::CaseInsensitive)) {
            KRunner::QueryMatch match(this);
            // Make sure we store clean entry names without quotes
            QString cleanEntry = entry;
            if (cleanEntry.startsWith(QStringLiteral("\"")) && cleanEntry.endsWith(QStringLiteral("\""))) {
                cleanEntry = cleanEntry.mid(1, cleanEntry.length() - 2);
            }
            match.setId(cleanEntry);
            match.setText(cleanEntry);
            match.setSubtext(i18n("Password entry from pass"));
            match.setRelevance(1.0);
            context.addMatch(match);
        }
    }
}

void PassRunner::run(const KRunner::RunnerContext &, const KRunner::QueryMatch &match)
{
    QString entry = match.id().trimmed();

    // Remove any surrounding quotes that might have been added
    if (entry.startsWith(QStringLiteral("\"")) && entry.endsWith(QStringLiteral("\""))) {
        entry = entry.mid(1, entry.length() - 2);
    }
    if (entry.startsWith(QStringLiteral("'")) && entry.endsWith(QStringLiteral("'"))) {
        entry = entry.mid(1, entry.length() - 2);
    }

    entry.replace(QStringLiteral("passrunner_"), QStringLiteral(""));

    // Simplified approach - just run the copy command
    QProcess *copyProcess = new QProcess();

    QObject::connect(copyProcess,
                     QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [this, entry](int exitCode, QProcess::ExitStatus /*exitStatus*/) {

        if (exitCode == 0) {

            // Use KNotification::event() with your notifyrc
            QMetaObject::invokeMethod(this, [entry]() {
                KNotification::event(QStringLiteral("password_copied"),
                                     i18n("Password for '%1' copied to clipboard", entry),
                                     QStringLiteral("krunner"));
            });
        }    });

    QObject::connect(copyProcess, &QProcess::errorOccurred, [entry](QProcess::ProcessError error) {
    });

    // Clean up when finished
    QObject::connect(copyProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     copyProcess, &QProcess::deleteLater);

    // Start the copy process
    copyProcess->start(QStringLiteral("pass"), QStringList() << QStringLiteral("-c") << entry);
}

K_PLUGIN_CLASS_WITH_JSON(PassRunner, "metadata.json")
#include "passrunner.moc"
