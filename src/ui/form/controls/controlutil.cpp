#include "controlutil.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QWidgetAction>

#include <util/iconcache.h>

QCheckBox *ControlUtil::createCheckBox(
        bool checked, const std::function<void(bool checked)> &onToggled)
{
    auto c = new QCheckBox();
    c->setChecked(checked);

    c->connect(c, &QCheckBox::toggled, onToggled);

    return c;
}

QComboBox *ControlUtil::createComboBox(
        const QStringList &texts, const std::function<void(int index)> &onActivated)
{
    auto c = new QComboBox();
    c->addItems(texts);

    c->connect(c, QOverload<int>::of(&QComboBox::activated), onActivated);

    return c;
}

QPushButton *ControlUtil::createButton(const QString &iconPath, const QString &text)
{
    auto c = new QPushButton(IconCache::icon(iconPath), text);

    return c;
}

QPushButton *ControlUtil::createButton(
        const QString &iconPath, const std::function<void()> &onClicked)
{
    auto c = new QPushButton(IconCache::icon(iconPath), QString());

    c->connect(c, &QPushButton::clicked, onClicked);

    return c;
}

QToolButton *ControlUtil::createToolButton(
        const QString &iconPath, const std::function<void()> &onClicked)
{
    auto c = new QToolButton();
    c->setIcon(IconCache::icon(iconPath));

    c->connect(c, &QToolButton::clicked, onClicked);

    return c;
}

QPushButton *ControlUtil::createLinkButton(
        const QString &iconPath, const QString &linkPath, const QString &toolTip)
{
    auto c = new QPushButton(IconCache::icon(iconPath), QString());
    c->setFlat(true);
    c->setCursor(Qt::PointingHandCursor);
    c->setWindowFilePath(linkPath);
    c->setToolTip(!toolTip.isEmpty() ? toolTip : linkPath);
    return c;
}

QPushButton *ControlUtil::createFlatButton(
        const QString &iconPath, const std::function<void()> &onClicked)
{
    auto c = createButton(iconPath, onClicked);
    c->setFlat(true);
    c->setCursor(Qt::PointingHandCursor);
    c->setFocusPolicy(Qt::NoFocus);
    c->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    return c;
}

QPushButton *ControlUtil::createSplitterButton(
        const QString &iconPath, const std::function<void()> &onClicked)
{
    auto c = createFlatButton(iconPath, onClicked);
    c->setFixedSize(32, 32);
    return c;
}

QLabel *ControlUtil::createLabel(const QString &text)
{
    auto c = new QLabel(text);
    return c;
}

QLineEdit *ControlUtil::createLineLabel()
{
    auto c = new QLineEdit();
    c->setReadOnly(true);
    c->setFrame(false);

    QPalette pal;
    pal.setColor(QPalette::Base, Qt::transparent);
    c->setPalette(pal);

    return c;
}

QLineEdit *ControlUtil::createLineEdit(
        const QString &text, const std::function<void(const QString &text)> &onEdited)
{
    auto c = new QLineEdit(text);

    c->connect(c, &QLineEdit::textChanged, onEdited);

    return c;
}

QMenu *ControlUtil::createMenuByLayout(QBoxLayout *layout, QWidget *parent)
{
    auto menu = new QMenu(parent);

    auto menuWidget = new QWidget();
    menuWidget->setLayout(layout);

    auto wa = new QWidgetAction(menu);
    wa->setDefaultWidget(menuWidget);
    menu->addAction(wa);

    return menu;
}

QBoxLayout *ControlUtil::createLayoutByWidgets(const QList<QWidget *> &widgets, Qt::Orientation o)
{
    auto layout =
            new QBoxLayout(o == Qt::Vertical ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);

    for (auto w : widgets) {
        if (!w) {
            layout->addStretch();
        } else {
            layout->addWidget(w);
        }
    }

    return layout;
}

QFrame *ControlUtil::createSeparator(Qt::Orientation o)
{
    auto c = new QFrame();
    c->setFrameShape(o == Qt::Horizontal ? QFrame::HLine : QFrame::VLine);
    c->setFrameShadow(QFrame::Sunken);
    return c;
}

QLayout *ControlUtil::createRowLayout(QWidget *w1, QWidget *w2, int stretch1)
{
    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(w1, stretch1);
    layout->addWidget(w2);
    return layout;
}

QLayout *ControlUtil::createScrollLayout(QLayout *content, bool isBgTransparent)
{
    auto scrollAreaContent = new QWidget();
    scrollAreaContent->setLayout(content);

    auto scrollArea = wrapToScrollArea(scrollAreaContent, isBgTransparent);

    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scrollArea);

    return layout;
}

QWidget *ControlUtil::wrapToScrollArea(QWidget *content, bool isBgTransparent)
{
    auto c = new QScrollArea();
    c->setContentsMargins(0, 0, 0, 0);
    c->setWidgetResizable(true);
    c->setWidget(content);

    if (isBgTransparent) {
        c->setStyleSheet("QScrollArea { background: transparent; }");
        content->setAutoFillBackground(false);
    }

    return c;
}

QFont ControlUtil::fontDemiBold()
{
    QFont font;
    font.setWeight(QFont::DemiBold);
    return font;
}
