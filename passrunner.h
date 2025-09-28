#pragma once
#include <KRunner/AbstractRunner>
#include <QStringList>

class PassRunner : public KRunner::AbstractRunner
{
    Q_OBJECT
public:
    PassRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~PassRunner() override = default;
    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;
private:
    QStringList getEntries() const;
    QStringList m_entries;
};
