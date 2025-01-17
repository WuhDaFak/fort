#include "programscontroller.h"

#include <appinfo/appinfocache.h>
#include <conf/confmanager.h>
#include <conf/firewallconf.h>
#include <fortmanager.h>
#include <fortsettings.h>
#include <manager/translationmanager.h>
#include <manager/windowmanager.h>
#include <model/applistmodel.h>
#include <util/ioc/ioccontainer.h>

ProgramsController::ProgramsController(QObject *parent) :
    QObject(parent), m_appListModel(new AppListModel(this))
{
    connect(translationManager(), &TranslationManager::languageChanged, this,
            &ProgramsController::retranslateUi);
}

FortManager *ProgramsController::fortManager() const
{
    return IoC<FortManager>();
}

FortSettings *ProgramsController::settings() const
{
    return IoC<FortSettings>();
}

ConfManager *ProgramsController::confManager() const
{
    return IoC<ConfManager>();
}

FirewallConf *ProgramsController::conf() const
{
    return confManager()->conf();
}

IniOptions *ProgramsController::ini() const
{
    return &conf()->ini();
}

IniUser *ProgramsController::iniUser() const
{
    return confManager()->iniUser();
}

TranslationManager *ProgramsController::translationManager() const
{
    return IoC<TranslationManager>();
}

WindowManager *ProgramsController::windowManager() const
{
    return IoC<WindowManager>();
}

AppInfoCache *ProgramsController::appInfoCache() const
{
    return IoC<AppInfoCache>();
}

void ProgramsController::initialize()
{
    appListModel()->initialize();
}
