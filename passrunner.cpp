#include "passrunner.h"
#include <KRunner/RunnerContext>
#include <KRunner/QueryMatch>
#include <QProcess>
#include <QDir>
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
    QString passwordStoreDir = QDir::homePath() + QStringLiteral("/.password-store");

    const QString customDir = qEnvironmentVariable("PASSWORD_STORE_DIR");
    if (!customDir.isEmpty()) {
        passwordStoreDir = customDir;
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
        return QStringList();
    }

    const QString output = QString::fromUtf8(proc.readAllStandardOutput());
    const QStringList lines = output.split(QStringLiteral("\n"), Qt::SkipEmptyParts);
    QStringList entries;
    const QString storeDirWithSlash = passwordStoreDir + QStringLiteral("/");

    for (const QString &line : lines) {
        QString entry = line.trimmed();
        if (entry.startsWith(storeDirWithSlash)) {
            entry = entry.mid(storeDirWithSlash.length());
        }
        if (entry.endsWith(QStringLiteral(".gpg"))) {
            entry.chop(4);
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
    if (query.isEmpty()) {
        return;
    }

    for (const QString &entry : m_entries) {
        if (entry.contains(query, Qt::CaseInsensitive)) {
            KRunner::QueryMatch match(this);
            match.setText(entry);
            match.setSubtext(i18n("Password entry from pass"));
            match.setRelevance(entry.startsWith(query, Qt::CaseInsensitive) ? 1.0 : 0.8);
            context.addMatch(match);
        }
    }
}

void PassRunner::run(const KRunner::RunnerContext &, const KRunner::QueryMatch &match)
{
    const QString entry = match.text();

    QProcess *copyProcess = new QProcess(this);

    connect(copyProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    [entry, copyProcess](int exitCode, QProcess::ExitStatus) {
        if (exitCode == 0) {
            KNotification *notification = new KNotification(QStringLiteral("passwordCopied"),
                KNotification::CloseOnTimeout);
            notification->setComponentName(QStringLiteral("passrunner"));
            notification->setText(i18n("Password for '%1' copied to clipboard", entry));
            notification->sendEvent();
        }
        copyProcess->deleteLater();
    });

    copyProcess->start(QStringLiteral("pass"), QStringList() << QStringLiteral("-c") << entry);
}

K_PLUGIN_CLASS_WITH_JSON(PassRunner, "metadata.json")
#include "passrunner.moc"
