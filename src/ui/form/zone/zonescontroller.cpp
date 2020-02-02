#include "zonescontroller.h"

#include "../../conf/confmanager.h"
#include "../../fortmanager.h"
#include "../../translationmanager.h"

ZonesController::ZonesController(FortManager *fortManager,
                                 QObject *parent) :
    QObject(parent),
    m_fortManager(fortManager)
{
    connect(translationManager(), &TranslationManager::languageChanged,
            this, &ZonesController::retranslateUi);
}

ConfManager *ZonesController::confManager() const
{
    return fortManager()->confManager();
}

ZoneListModel *ZonesController::zoneListModel() const
{
    return fortManager()->zoneListModel();
}

TranslationManager *ZonesController::translationManager() const
{
    return TranslationManager::instance();
}

void ZonesController::closeWindow()
{
    fortManager()->closeZonesWindow();
}