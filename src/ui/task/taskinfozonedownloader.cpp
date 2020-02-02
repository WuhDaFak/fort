#include "taskinfozonedownloader.h"

#include "../conf/addressgroup.h"
#include "../conf/firewallconf.h"
#include "../fortmanager.h"
#include "taskzonedownloader.h"

TaskInfoZoneDownloader::TaskInfoZoneDownloader(QObject *parent) :
    TaskInfo(ZoneDownloader, parent)
{
}

TaskZoneDownloader *TaskInfoZoneDownloader::zoneDownloader() const
{
    return static_cast<TaskZoneDownloader *>(taskWorker());
}

bool TaskInfoZoneDownloader::processResult(FortManager *fortManager, bool success)
{
    if (!success)
        return false;

    const auto worker = zoneDownloader();

    FirewallConf *conf = fortManager->conf();
    AddressGroup *inetGroup = conf->inetAddressGroup();

    if (inetGroup->excludeText() == worker->rangeText())
        return false;

    inetGroup->setExcludeText(worker->rangeText());

    return fortManager->saveOriginConf(tr("Zone Addresses Updated!"));
}

void TaskInfoZoneDownloader::setupTaskWorker()
{
    const auto worker = zoneDownloader();

}