#include "applicationspage.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTimeEdit>
#include <QVBoxLayout>

#include "../../../conf/appgroup.h"
#include "../../../conf/firewallconf.h"
#include "../../../util/net/netutil.h"
#include "../../controls/checkspincombo.h"
#include "../../controls/checktimeperiod.h"
#include "../../controls/controlutil.h"
#include "../../controls/tabbar.h"
#include "../controls/textarea2splitter.h"
#include "../controls/textarea2splitterhandle.h"
#include "../optionscontroller.h"
#include "apps/appscolumn.h"

namespace {

const ValuesList speedLimitValues = {
    10, 0, 20, 30, 50, 75, 100, 150, 200, 300, 500, 900,
    1024, qRound(1.5 * 1024), 2 * 1024, 3 * 1024, 5 * 1024, qRound(7.5 * 1024),
    10 * 1024, 15 * 1024, 20 * 1024, 30 * 1024, 50 * 1024
};

}

ApplicationsPage::ApplicationsPage(OptionsController *ctrl,
                                   QWidget *parent) :
    BasePage(ctrl, parent)
{
    setupUi();
}

void ApplicationsPage::setAppGroup(AppGroup *v)
{
    if (m_appGroup != v) {
        m_appGroup = v;
        emit appGroupChanged();
    }
}

void ApplicationsPage::onRetranslateUi()
{
    m_editGroupName->setPlaceholderText(tr("Group Name"));
    m_btAddGroup->setText(tr("Add Group"));
    m_btRenameGroup->setText(tr("Rename Group"));

    m_cbBlockAll->setText(tr("Block All"));
    m_cbAllowAll->setText(tr("Allow All"));

    m_btGroupOptions->setText(tr("Options"));
    m_cscLimitIn->checkBox()->setText(tr("Download speed limit:"));
    m_cscLimitOut->checkBox()->setText(tr("Upload speed limit:"));
    retranslateGroupLimits();
    m_cbFragmentPacket->setText(tr("Fragment first TCP packet"));

    m_cbGroupEnabled->setText(tr("Enabled"));
    m_ctpGroupPeriod->checkBox()->setText(tr("time period:"));

    m_blockApps->labelTitle()->setText(tr("Block"));
    m_allowApps->labelTitle()->setText(tr("Allow"));

    m_splitter->handle()->btMoveAllFrom1To2()->setToolTip(tr("Move All Lines to 'Allow'"));
    m_splitter->handle()->btMoveAllFrom2To1()->setToolTip(tr("Move All Lines to 'Block'"));
    m_splitter->handle()->btMoveSelectedFrom1To2()->setToolTip(tr("Move Selected Lines to 'Allow'"));
    m_splitter->handle()->btMoveSelectedFrom2To1()->setToolTip(tr("Move Selected Lines to 'Block'"));
    m_splitter->handle()->btSelectFile()->setToolTip(tr("Select File"));

    retranslateAppsPlaceholderText();
}

void ApplicationsPage::setupUi()
{
    auto layout = new QVBoxLayout();

    // Header
    auto header = setupHeader();
    layout->addLayout(header);

    // Tab Bar
    setupTabBar();
    layout->addWidget(m_tabBar);

    // App Group
    auto groupHeader = setupGroupHeader();
    layout->addLayout(groupHeader);

    // App Columns
    setupBlockApps();
    setupAllowApps();

    // Splitter
    setupSplitter();
    layout->addWidget(m_splitter, 1);

    setupAppGroup();

    this->setLayout(layout);
}

QLayout *ApplicationsPage::setupHeader()
{
    auto layout = new QHBoxLayout();

    m_editGroupName = new QLineEdit();
    m_editGroupName->setFixedWidth(200);

    setupAddGroup();
    setupRenameGroup();

    setupBlockAllowAll();

    layout->addWidget(m_editGroupName);
    layout->addWidget(m_btAddGroup);
    layout->addWidget(m_btRenameGroup);
    layout->addStretch();
    layout->addWidget(m_cbBlockAll);
    layout->addWidget(m_cbAllowAll);

    return layout;
}

void ApplicationsPage::setupAddGroup()
{
    m_btAddGroup = ControlUtil::createButton(":/images/application_add.png", [&] {
        const auto text = m_editGroupName->text();
        if (text.isEmpty()) {
            m_editGroupName->setFocus();
            return;
        }

        conf()->addAppGroupByName(text);
        resetGroupName();

        const int tabIndex = m_tabBar->addTab(text);
        m_tabBar->setCurrentIndex(tabIndex);

        ctrl()->setConfEdited(true);
    });

    const auto refreshAddGroup = [&] {
        m_btAddGroup->setEnabled(appGroupsCount() < 16);
    };

    refreshAddGroup();

    connect(conf(), &FirewallConf::appGroupsChanged, this, refreshAddGroup);
}

void ApplicationsPage::setupRenameGroup()
{
    m_btRenameGroup = ControlUtil::createButton(":/images/application_edit.png", [&] {
        const auto text = m_editGroupName->text();
        if (text.isEmpty()) {
            m_editGroupName->setFocus();
            return;
        }

        const int tabIndex = m_tabBar->currentIndex();
        m_tabBar->setTabText(tabIndex, text);

        appGroup()->setName(text);
        resetGroupName();

        ctrl()->setConfEdited(true);
    });
}

void ApplicationsPage::setupBlockAllowAll()
{
    m_cbBlockAll = ControlUtil::createCheckBox(conf()->appBlockAll(), [&](bool checked) {
        conf()->setAppBlockAll(checked);
        ctrl()->setConfFlagsEdited(true);
    });
    m_cbAllowAll = ControlUtil::createCheckBox(conf()->appAllowAll(), [&](bool checked) {
        conf()->setAppAllowAll(checked);
        ctrl()->setConfFlagsEdited(true);
    });

    const auto refreshBlockAllowAllEnabled = [&] {
        const bool blockAll = m_cbBlockAll->isChecked();
        const bool allowAll = m_cbAllowAll->isChecked();

        m_cbBlockAll->setEnabled(blockAll || !allowAll);
        m_cbAllowAll->setEnabled(!blockAll || allowAll);
    };

    refreshBlockAllowAllEnabled();

    connect(m_cbBlockAll, &QCheckBox::toggled, this, refreshBlockAllowAllEnabled);
    connect(m_cbAllowAll, &QCheckBox::toggled, this, refreshBlockAllowAllEnabled);
}

void ApplicationsPage::setupTabBar()
{
    m_tabBar = new TabBar();
    m_tabBar->setTabMinimumWidth(120);
    m_tabBar->setShape(QTabBar::TriangularNorth);
    m_tabBar->setExpanding(false);
    m_tabBar->setTabsClosable(true);
    m_tabBar->setMovable(true);

    for (const auto appGroup : conf()->appGroups()) {
        addTab(appGroup->name());
    }

    connect(m_tabBar, &QTabBar::tabCloseRequested, [&](int index) {
        conf()->removeAppGroup(index, index);

        if (m_tabBar->count() > 1) {
            m_tabBar->removeTab(index);
        } else {
            // Reset alone tab to default one
            setAppGroup(appGroupByIndex(0));
            m_tabBar->setTabText(0, appGroup()->name());
        }

        ctrl()->setConfEdited(true);
    });
    connect(m_tabBar, &QTabBar::tabMoved, [&](int from, int to) {
        conf()->moveAppGroup(from, to);
        ctrl()->setConfEdited(true);
    });
}

int ApplicationsPage::addTab(const QString &text)
{
    const int tabIndex = m_tabBar->addTab(text);
    m_tabBar->setTabIcon(tabIndex, QIcon(":/images/application_double.png"));
    return tabIndex;
}

QLayout *ApplicationsPage::setupGroupHeader()
{
    auto layout = new QHBoxLayout();

    setupGroupOptions();
    setupGroupEnabled();
    setupGroupPeriod();
    setupGroupPeriodEnabled();

    layout->addWidget(m_btGroupOptions);
    layout->addStretch();
    layout->addWidget(m_cbGroupEnabled);
    layout->addWidget(m_ctpGroupPeriod);

    return layout;
}

void ApplicationsPage::setupGroupOptions()
{
    m_btGroupOptions = new QPushButton();
    m_btGroupOptions->setIcon(QIcon(":/images/application_key.png"));

    setupGroupLimitIn();
    setupGroupLimitOut();
    setupGroupFragmentPacket();

    // Menu
    const QList<QWidget *> menuWidgets = {
        m_cscLimitIn, m_cscLimitOut,
        ControlUtil::createHSeparator(),
        m_cbFragmentPacket
    };
    auto menu = ControlUtil::createMenuByWidgets(
                menuWidgets, m_btGroupOptions);

    m_btGroupOptions->setMenu(menu);
}

void ApplicationsPage::setupGroupLimitIn()
{
    m_cscLimitIn = createGroupLimit();

    connect(m_cscLimitIn->checkBox(), &QCheckBox::toggled, [&](bool checked) {
        if (appGroup()->limitInEnabled() == checked)
            return;

        appGroup()->setLimitInEnabled(checked);

        ctrl()->setConfEdited(true);
    });
    connect(m_cscLimitIn->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), [&](int value) {
        const auto speedLimit = quint32(value);

        if (appGroup()->speedLimitIn() == speedLimit)
            return;

        appGroup()->setSpeedLimitIn(speedLimit);

        ctrl()->setConfEdited(true);
    });
}

void ApplicationsPage::setupGroupLimitOut()
{
    m_cscLimitOut = createGroupLimit();

    connect(m_cscLimitOut->checkBox(), &QCheckBox::toggled, [&](bool checked) {
        if (appGroup()->limitOutEnabled() == checked)
            return;

        appGroup()->setLimitOutEnabled(checked);

        ctrl()->setConfEdited(true);
    });
    connect(m_cscLimitOut->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), [&](int value) {
        const auto speedLimit = quint32(value);

        if (appGroup()->speedLimitOut() == speedLimit)
            return;

        appGroup()->setSpeedLimitOut(speedLimit);

        ctrl()->setConfEdited(true);
    });
}

CheckSpinCombo *ApplicationsPage::createGroupLimit()
{
    auto c = new CheckSpinCombo();
    c->spinBox()->setRange(0, 99999);
    c->spinBox()->setSuffix(" KiB/s");
    c->setValues(speedLimitValues);
    return c;
}

void ApplicationsPage::setupGroupFragmentPacket()
{
    m_cbFragmentPacket = ControlUtil::createCheckBox(false, [&](bool checked) {
        if (appGroup()->fragmentPacket() == checked)
            return;

        appGroup()->setFragmentPacket(checked);

        ctrl()->setConfEdited(true);
    });
}

void ApplicationsPage::setupGroupOptionsEnabled()
{
    const auto refreshOptionsEnabled = [&] {
        const bool logStat = conf()->logStat();

        m_cscLimitIn->setEnabled(logStat);
        m_cscLimitOut->setEnabled(logStat);
        m_cbFragmentPacket->setEnabled(logStat);
    };

    refreshOptionsEnabled();

    connect(conf(), &FirewallConf::logStatChanged, this, refreshOptionsEnabled);
}

void ApplicationsPage::retranslateGroupLimits()
{
    QStringList list;

    list.append(tr("Custom"));
    list.append(tr("Disabled"));

    int index = 0;
    for (const int v : speedLimitValues) {
        if (++index > 2) {
            list.append(formatSpeed(v));
        }
    }

    m_cscLimitIn->setNames(list);
    m_cscLimitOut->setNames(list);
}

void ApplicationsPage::setupGroupEnabled()
{
    m_cbGroupEnabled = ControlUtil::createCheckBox(false, [&](bool checked) {
        if (appGroup()->enabled() == checked)
            return;

        appGroup()->setEnabled(checked);

        ctrl()->setConfFlagsEdited(true);
    });
}

void ApplicationsPage::setupGroupPeriod()
{
    m_ctpGroupPeriod = new CheckTimePeriod();

    connect(m_ctpGroupPeriod->checkBox(), &QCheckBox::toggled, [&](bool checked) {
        if (appGroup()->periodEnabled() == checked)
            return;

        appGroup()->setPeriodEnabled(checked);

        ctrl()->setConfEdited(true);
    });
    connect(m_ctpGroupPeriod->timeEdit1(), &QTimeEdit::userTimeChanged, [&](const QTime &time) {
        const auto timeStr = CheckTimePeriod::fromTime(time);

        if (appGroup()->periodFrom() == timeStr)
            return;

        appGroup()->setPeriodFrom(timeStr);

        ctrl()->setConfEdited(true);
    });
    connect(m_ctpGroupPeriod->timeEdit2(), &QTimeEdit::userTimeChanged, [&](const QTime &time) {
        const auto timeStr = CheckTimePeriod::fromTime(time);

        if (appGroup()->periodTo() == timeStr)
            return;

        appGroup()->setPeriodTo(timeStr);

        ctrl()->setConfEdited(true);
    });
}

void ApplicationsPage::setupGroupPeriodEnabled()
{
    const auto refreshPeriodEnabled = [&] {
        m_ctpGroupPeriod->setEnabled(m_cbGroupEnabled->isChecked());
    };

    refreshPeriodEnabled();

    connect(m_cbGroupEnabled, &QCheckBox::toggled, this, refreshPeriodEnabled);
}

void ApplicationsPage::setupBlockApps()
{
    m_blockApps = new AppsColumn();

    connect(m_blockApps->editText(), &QPlainTextEdit::textChanged, [&] {
        const auto text = m_blockApps->editText()->toPlainText();

        if (appGroup()->blockText() == text)
            return;

        appGroup()->setBlockText(text);

        ctrl()->setConfEdited(true);
    });
}

void ApplicationsPage::setupAllowApps()
{
    m_allowApps = new AppsColumn();

    connect(m_allowApps->editText(), &QPlainTextEdit::textChanged, [&] {
        const auto text = m_allowApps->editText()->toPlainText();

        if (appGroup()->allowText() == text)
            return;

        appGroup()->setAllowText(text);

        ctrl()->setConfEdited(true);
    });
}

void ApplicationsPage::retranslateAppsPlaceholderText()
{
    const auto placeholderText = tr("# Examples:") + '\n'
            + QLatin1String(
                "System\n"
                "C:\\Program Files (x86)\\Microsoft\\Skype for Desktop\\Skype.exe\n")
            + '\n' + tr("# All programs in the sub-path:")
            + QLatin1String("\nC:\\Git\\**");

    m_allowApps->editText()->setPlaceholderText(placeholderText);
}

void ApplicationsPage::setupSplitter()
{
    m_splitter = new TextArea2Splitter(ctrl());

    m_splitter->setSelectFileEnabled(true);
    m_splitter->setSettingsPropName("optWindowAppsSplit");

    Q_ASSERT(!m_splitter->handle());

    m_splitter->addWidget(m_blockApps);
    m_splitter->addWidget(m_allowApps);

    Q_ASSERT(m_splitter->handle());

    m_splitter->handle()->setTextArea1(m_blockApps->editText());
    m_splitter->handle()->setTextArea2(m_allowApps->editText());
}

void ApplicationsPage::refreshGroup()
{
    m_cscLimitIn->checkBox()->setChecked(appGroup()->limitInEnabled());
    m_cscLimitIn->spinBox()->setValue(int(appGroup()->speedLimitIn()));

    m_cscLimitOut->checkBox()->setChecked(appGroup()->limitOutEnabled());
    m_cscLimitOut->spinBox()->setValue(int(appGroup()->speedLimitOut()));

    m_cbFragmentPacket->setChecked(appGroup()->fragmentPacket());

    m_cbGroupEnabled->setChecked(appGroup()->enabled());

    m_ctpGroupPeriod->checkBox()->setChecked(appGroup()->periodEnabled());
    m_ctpGroupPeriod->timeEdit1()->setTime(CheckTimePeriod::toTime(
                                               appGroup()->periodFrom()));
    m_ctpGroupPeriod->timeEdit2()->setTime(CheckTimePeriod::toTime(
                                               appGroup()->periodTo()));

    m_blockApps->editText()->setPlainText(appGroup()->blockText());
    m_allowApps->editText()->setPlainText(appGroup()->allowText());
}

void ApplicationsPage::setupAppGroup()
{
    connect(this, &ApplicationsPage::appGroupChanged, this, &ApplicationsPage::refreshGroup);

    const auto refreshAppGroup = [&] {
        const int tabIndex = m_tabBar->currentIndex();
        setAppGroup(appGroupByIndex(tabIndex));
    };

    refreshAppGroup();

    connect(m_tabBar, &QTabBar::currentChanged, this, refreshAppGroup);
}

int ApplicationsPage::appGroupsCount() const
{
    return conf()->appGroups().size();
}

AppGroup *ApplicationsPage::appGroupByIndex(int index) const
{
    return conf()->appGroups().at(index);
}

void ApplicationsPage::resetGroupName()
{
    m_editGroupName->setText(QString());
    m_editGroupName->setFocus();
}

QString ApplicationsPage::formatSpeed(int kbytes)
{
    return NetUtil::formatSpeed(quint32(kbytes * 1024));
}