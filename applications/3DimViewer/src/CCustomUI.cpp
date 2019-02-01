///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////


#include <CCustomUI.h>
#include <osg/OSGCanvas.h>
#include <osg/OSGOrtho2DCanvas.h>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QStylePainter>
#include <mainwindow.h>
#include <C3DimApplication.h>
#include <controls/CScaledCursor.h>

#include <coremedi/app/Signals.h>
#include <Signals.h>


///////////////////////////////////////////////////////////////////////////////

bool TabBarMouseFunctionalityEx::eventFilter(QObject *obj, QEvent *event)
{
    if (QEvent::MouseButtonRelease == event->type() && NULL!=MainWindow::getInstance())
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        QTabBar* pBar = qobject_cast<QTabBar*>(obj);
        if (NULL!=pBar && Qt::MidButton == me->button())
        {
            int idxTab = pBar->tabAt(me->pos());
            if (idxTab>=0)
            {
                QList<QDockWidget*> dws = MainWindow::getInstance()->findChildren<QDockWidget*>();
                foreach (QDockWidget * dw, dws)
                {
                    if (pBar->tabText(idxTab)==dw->windowTitle())
                    {
                        dw->hide();
                        return true;
                    }
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }
    return QObject::eventFilter(obj, event);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Custom Dock Widget Title Bar

CCustomDockWidgetTitle::CCustomDockWidgetTitle(int viewType, int flags, QWidget *parent)
    : QWidget(parent)
    , m_viewType(viewType)
{
    m_bMaximized = false;
    m_bIconMaximize = true;
    m_bPinned = false;
    m_bIconPin = true;
    m_closeButton = NULL;
    m_maximizeButton = NULL;
    m_screenshotButton = NULL;
    m_pinButton = NULL;
    m_viewButton = NULL;
    m_slider = NULL;
    m_action3D = NULL;
    m_actionXY = NULL;
    m_actionXZ = NULL;
    m_actionYZ = NULL;
    m_actionARB = NULL;

    double factor = C3DimApplication::getDpiFactor();
    factor = 1 + (factor - 1) / 2; // reduce upscale because our icons aren't that good
    m_dockButtonSize = 18 * factor;
    m_dockTitleHeight = 22 * factor;

    createLayout(flags);

    QObject::connect(this, SIGNAL(saveScreenshotOfCanvas(OSGCanvas*)), MainWindow::getInstance(), SLOT(saveScreenshotOfCanvas(OSGCanvas*)));
    QObject::connect(this, SIGNAL(copyScreenshotToClipboard(OSGCanvas*)), MainWindow::getInstance(), SLOT(copyScreenshotToClipboard(OSGCanvas*)));
    QObject::connect(this, SIGNAL(saveSlice(OSGCanvas*, int)), MainWindow::getInstance(), SLOT(saveSlice(OSGCanvas*, int)));

    onNewDensityData(NULL);
}

void CCustomDockWidgetTitle::createLayout(int flags)
{
    if (DWT_BUTTONS_CLOSE == (flags&DWT_BUTTONS_CLOSE))
    {
        m_closeButton = new QToolButton;
        m_closeButton->setAutoRaise(true);
        m_closeButton->setIcon(QIcon(":/svg/svg/dock_icon_close.svg"));
        m_closeButton->setMinimumSize(8, 8);
        m_closeButton->setMaximumSize(m_dockButtonSize - 2, m_dockButtonSize);
        QObject::connect(m_closeButton, SIGNAL(clicked()), this, SLOT(btnCloseClicked()));
    }
    if (DWT_BUTTONS_MAXIMIZE == (flags&DWT_BUTTONS_MAXIMIZE))
    {
        m_maximizeButton = new QToolButton;
        m_maximizeButton->setAutoRaise(true);
        m_maximizeButton->setIcon(QIcon(":/svg/svg/dock_icon_maximize.svg"));
        m_maximizeButton->setMinimumSize(8, 8);
        m_maximizeButton->setMaximumSize(m_dockButtonSize - 2, m_dockButtonSize);
        QObject::connect(m_maximizeButton, SIGNAL(clicked()), this, SLOT(btnMaximizeClicked()));
    }
    {
        m_screenshotButton = new QToolButton;
        m_screenshotButton->setAutoRaise(true);
        m_screenshotButton->setIcon(QIcon(":/svg/svg/dock_icon_screenshot.svg"));
        m_screenshotButton->setMinimumSize(8, 8);
        m_screenshotButton->setMaximumSize(m_dockButtonSize - 2, m_dockButtonSize);
        QObject::connect(m_screenshotButton, SIGNAL(clicked()), this, SLOT(btnScreenshotClicked()));
    }
    if (DWT_BUTTONS_PIN == (flags&DWT_BUTTONS_PIN))
    {
        m_pinButton = new QToolButton;
        m_pinButton->setAutoRaise(true);
        m_pinButton->setIcon(QIcon(":/svg/svg/dock_icon_unpin.svg"));
        m_pinButton->setMinimumSize(8, 8);
        m_pinButton->setMaximumSize(m_dockButtonSize - 2, m_dockButtonSize);
        QObject::connect(m_pinButton, SIGNAL(clicked()), this, SLOT(btnPinClicked()));
    }
    if (DWT_BUTTONS_VIEW == (flags&DWT_BUTTONS_VIEW))
    {
        m_viewButton = new QToolButton;
        m_viewButton->setAutoRaise(true);
        m_viewButton->setIcon(QIcon(":/svg/svg/dock_icon_visibility.svg"));
        m_viewButton->setMinimumSize(25, 8);
        m_viewButton->setMaximumSize(m_dockButtonSize - 2, m_dockButtonSize);
        m_viewButton->setPopupMode(QToolButton::InstantPopup);
        createMenuForViewButton();
    }
    if (DWT_SLIDER_VR == (flags&DWT_SLIDER_VR))
    {
        m_slider = new CSliderEx(Qt::Horizontal, this);
        m_slider->setProperty("Type", DWT_SLIDER_VR);
        m_slider->setContentsMargins(1, 2, 1, 2);
        m_slider->setRange(-100, 200);
        m_slider->setToolTip(tr("Adjusts VR density threshold"));
        m_slider->show();
        QObject::connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(vrValueChanged(int)));
        m_vrDataRemapChangeConnection = VPL_SIGNAL(SigVRDataRemapChange).connect(this, &CCustomDockWidgetTitle::vrDataRemapChange);
    }
    if (DWT_SLIDER_XY == (flags&DWT_SLIDER_XY))
    {
        m_slider = new CSliderEx(Qt::Horizontal, this);
        m_slider->setProperty("Type", DWT_SLIDER_XY);
        m_slider->setContentsMargins(1, 2, 1, 2);
        m_slider->setRange(0, 100);
        m_slider->setSingleStep(1);
        m_slider->setPageStep(1);
        m_slider->setToolTip(tr("Adjusts slice position"));
        m_slider->show();
        QObject::connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(xySliderChange(int)));
        m_ConnectionData = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &CCustomDockWidgetTitle::onNewDensityData);
        m_ConnectionXY = APP_STORAGE.getEntrySignal(data::Storage::SliceXY::Id).connect(this, &CCustomDockWidgetTitle::onNewSliceXY);
    }
    if (DWT_SLIDER_XZ == (flags&DWT_SLIDER_XZ))
    {
        m_slider = new CSliderEx(Qt::Horizontal, this);
        m_slider->setProperty("Type", DWT_SLIDER_XZ);
        m_slider->setContentsMargins(1, 2, 1, 2);
        m_slider->setRange(0, 100);
        m_slider->setSingleStep(1);
        m_slider->setPageStep(1);
        m_slider->setToolTip(tr("Adjusts slice position"));
        m_slider->show();
        QObject::connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(xzSliderChange(int)));
        m_ConnectionData = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &CCustomDockWidgetTitle::onNewDensityData);
        m_ConnectionXZ = APP_STORAGE.getEntrySignal(data::Storage::SliceXZ::Id).connect(this, &CCustomDockWidgetTitle::onNewSliceXZ);
    }
    if (DWT_SLIDER_YZ == (flags&DWT_SLIDER_YZ))
    {
        m_slider = new CSliderEx(Qt::Horizontal, this);
        m_slider->setProperty("Type", DWT_SLIDER_YZ);
        m_slider->setContentsMargins(1, 2, 1, 2);
        m_slider->setRange(0, 100);
        m_slider->setSingleStep(1);
        m_slider->setPageStep(1);
        m_slider->setToolTip(tr("Adjusts slice position"));
        m_slider->show();
        QObject::connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(yzSliderChange(int)));
        m_ConnectionData = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &CCustomDockWidgetTitle::onNewDensityData);
        m_ConnectionYZ = APP_STORAGE.getEntrySignal(data::Storage::SliceYZ::Id).connect(this, &CCustomDockWidgetTitle::onNewSliceYZ);
    }

    if (DWT_SLIDER_ARB == (flags&DWT_SLIDER_ARB))
    {
        m_slider = new CSliderEx(Qt::Horizontal, this);
        m_slider->setProperty("Type", DWT_SLIDER_ARB);
        m_slider->setContentsMargins(1, 2, 1, 2);
        m_slider->setRange(0, 100);
        m_slider->setSingleStep(1);
        m_slider->setPageStep(1);
        m_slider->setToolTip(tr("Adjusts slice position"));
        m_slider->show();
        QObject::connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(arbSliderChange(int)));
        m_ConnectionData = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &CCustomDockWidgetTitle::onNewDensityData);
        m_ConnectionARB = APP_STORAGE.getEntrySignal(data::Storage::ArbitrarySlice::Id).connect(this, &CCustomDockWidgetTitle::onNewSliceARB);
    }
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->setSpacing(1);
    pLayout->setContentsMargins(2, 2, 2, 2);
    pLayout->addStretch();
#ifdef __APPLE__
    if (NULL != m_slider)
        pLayout->addWidget(m_slider, 0, Qt::AlignRight);
#else
    if (NULL != m_slider)
        pLayout->addWidget(m_slider, 0, 0/*Qt::AlignRight*/);
#endif
    if (NULL != m_screenshotButton)
    {
        pLayout->addWidget(m_screenshotButton, 0, Qt::AlignRight);
    }

    if (NULL != m_viewButton)
    {
        pLayout->addWidget(m_viewButton, 0, Qt::AlignRight);
    }

    if (NULL != m_pinButton)
    {
        pLayout->addWidget(m_pinButton, 0, Qt::AlignRight);
    }

    if (NULL != m_maximizeButton)
    {
        pLayout->addWidget(m_maximizeButton, 0, Qt::AlignRight);
    }

    if (NULL != m_closeButton)
    {
        pLayout->addWidget(m_closeButton, 0, Qt::AlignRight);
    }

    setLayout(pLayout);

    QObject::connect(this, SIGNAL(saveScreenshotOfCanvas(OSGCanvas*)), MainWindow::getInstance(), SLOT(saveScreenshotOfCanvas(OSGCanvas*)));
    QObject::connect(this, SIGNAL(copyScreenshotToClipboard(OSGCanvas*)), MainWindow::getInstance(), SLOT(copyScreenshotToClipboard(OSGCanvas*)));
    QObject::connect(this, SIGNAL(saveSlice(OSGCanvas*, int)), MainWindow::getInstance(), SLOT(saveSlice(OSGCanvas*, int)));

    onNewDensityData(NULL);
}

CCustomDockWidgetTitle::~CCustomDockWidgetTitle()
{
    if (NULL != m_slider && DWT_SLIDER_VR == m_slider->property("Type").toInt())
        VPL_SIGNAL(SigVRDataRemapChange).disconnect(m_vrDataRemapChangeConnection);
    if (NULL != m_ConnectionData.getSignalPtr())
        APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).disconnect(m_ConnectionData);
    if (NULL != m_ConnectionXY.getSignalPtr())
        APP_STORAGE.getEntrySignal(data::Storage::SliceXY::Id).disconnect(m_ConnectionXY);
    if (NULL != m_ConnectionXZ.getSignalPtr())
        APP_STORAGE.getEntrySignal(data::Storage::SliceXZ::Id).disconnect(m_ConnectionXZ);
    if (NULL != m_ConnectionYZ.getSignalPtr())
        APP_STORAGE.getEntrySignal(data::Storage::SliceYZ::Id).disconnect(m_ConnectionYZ);
    if (NULL != m_ConnectionARB.getSignalPtr())
        APP_STORAGE.getEntrySignal(data::Storage::ArbitrarySlice::Id).disconnect(m_ConnectionARB);
}

QSize CCustomDockWidgetTitle::minimumSizeHint() const
{
    QWidget *dw = qobject_cast<QWidget*>(parentWidget());
    Q_ASSERT(dw != 0);
    QSize result(m_dockTitleHeight, m_dockTitleHeight);
    return result;
}

void CCustomDockWidgetTitle::paintEvent(QPaintEvent*)
{
    QWidget *dw = qobject_cast<QWidget*>(parentWidget());
    Q_ASSERT(dw);
    if (!dw) return;
    {
        QString iconPath = dw->property("Icon").toString();

        QStylePainter p(this);
        p.setBrush((QBrush(Qt::lightGray)));
        QStyleOptionDockWidgetV2 titleOpt;
        QRect rect(0, 0, dw->width(), m_dockTitleHeight);
        titleOpt.rect = rect;
        titleOpt.floatable = false;
        titleOpt.movable = false;
        titleOpt.closable = false;
#ifdef WIN32
        if (!iconPath.isEmpty())
            titleOpt.title = "       " + dw->windowTitle();
        else
#endif
            titleOpt.title = dw->windowTitle();
        p.drawControl(QStyle::CE_DockWidgetTitle, titleOpt);

        if (!iconPath.isEmpty())
        {
            QImage img(QImage(iconPath).scaledToHeight(m_dockTitleHeight - 6, Qt::SmoothTransformation));
            p.drawImage(QRectF((m_dockTitleHeight - img.width() + 2) / 2, (m_dockTitleHeight - img.height() + 1) / 2, img.width(), img.height()), img);
        }

        if (m_maximizeButton)
        {
            if (m_bMaximized)
            {
                if (m_bIconMaximize)
                    m_maximizeButton->setIcon(QIcon(":/svg/svg/dock_icon_restore.svg"));
                m_bIconMaximize = false;
            }
            else
            {
                if (!m_bIconMaximize)
                    m_maximizeButton->setIcon(QIcon(":/svg/svg/dock_icon_maximize.svg"));
                m_bIconMaximize = true;
            }
        }

        if (m_pinButton)
        {
            if (m_bPinned)
            {
                //if (m_bIconPin)
                    m_pinButton->setIcon(QIcon(":/svg/svg/dock_icon_unpin.svg"));
                //m_bIconPin = false;
            }
            else
            {
                //if (!m_bIconPin)
                    m_pinButton->setIcon(QIcon(":/svg/svg/dock_icon_pin.svg"));
                //m_bIconPin = true;
            }
        }
    }
}

void CCustomDockWidgetTitle::vrValueChanged(int value)
{
    MainWindow* pMain = MainWindow::getInstance();
    if (NULL != pMain)
    {
        PSVR::PSVolumeRendering* pRenderer = pMain->getRenderer();
        if (NULL != pRenderer)
        {
            float expand, shift;
            pRenderer->getDataRemap(expand, shift);
            shift = (float)value / 500.0f;
            pRenderer->setDataRemap(expand, shift);
            pRenderer->redraw();
        }
    }
}

#define myround(x) (x<0?ceil((x)-0.5):floor((x)+0.5))
void    CCustomDockWidgetTitle::vrDataRemapChange(float expand, float offset)
{
    m_slider->blockSignals(true);
    m_slider->setValue(myround(offset * 500));
    m_slider->blockSignals(false);
}


void CCustomDockWidgetTitle::mousePressEvent(QMouseEvent *event)
{
    /*QPoint pos = event->pos();

    QRect rect = this->rect();

    QWidget *dw = qobject_cast<QWidget*>(parentWidget());
    Q_ASSERT(dw != 0);

    if (Qt::RightButton == event->button())
    {
        pos = this->mapToGlobal(pos);
        OSGCanvas* pCanvas = qobject_cast<OSGCanvas*>(dw->widget());
        QMenu contextMenu;
        QAction* copyScreenshot2ClipboardA = contextMenu.addAction(tr("Copy Screenshot to Clipboard"));
        QAction* saveScreenshotA = contextMenu.addAction(tr("Save Screenshot"));
        QAction* saveSliceA = NULL;
#ifdef _DEBUG
        if (NULL != dynamic_cast<OSGOrtho2DCanvas*>(pCanvas))
            saveSliceA = contextMenu.addAction(tr("Save slice"));
#endif
        QAction* win = contextMenu.exec(pos);
        if (NULL != win)
        {
            if (win == saveScreenshotA)
            {
                if (NULL != pCanvas)
                    emit saveScreenshotOfCanvas(pCanvas);
            }
            if (win == copyScreenshot2ClipboardA)
            {
                if (NULL != pCanvas)
                    emit copyScreenshotToClipboard(pCanvas);
            }
            if (win == saveSliceA)
            {
                if (NULL != pCanvas)
                    emit saveSlice(pCanvas, 0);
            }
        }
    }
    */
    event->ignore();
}

void CCustomDockWidgetTitle::mouseDoubleClickEvent(QMouseEvent * event)
{
    if (Qt::LeftButton == event->buttons())
    {
        QWidget *dw = qobject_cast<QWidget*>(parentWidget());
        Q_ASSERT(dw != 0);
        event->accept();
        m_bMaximized = !m_bMaximized;
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

// switches visibility state of all sub-widgets that are not pDWIgnore
static void XSplitterShowHide(QSplitter* pHighestParent, QWidget* pDWIgnore)
{
    Q_ASSERT(NULL != pHighestParent && NULL != pDWIgnore);
    QSplitter* pS = qobject_cast<QSplitter*>(pHighestParent);
    Q_ASSERT(NULL != pS);
    for (int i = 0; i<pS->count(); i++)
    {
        QWidget* pWidget = pS->widget(i);
        if (NULL != pWidget && pWidget != pDWIgnore)
        {
            QSplitter* pSS = qobject_cast<QSplitter*>(pWidget);
            if (NULL != pSS)
            {
                XSplitterShowHide(pSS, pDWIgnore);
            }
            else
            {
                if (pWidget->isVisible())
                    pWidget->hide();
                else
                    pWidget->show();
            }
        }
    }
}

// finds highest QSplitter parent and calls the method above
static void XSplitterShowHide(QWidget* pDW)
{
    if (NULL == pDW) return;
    QWidget* pSplitter = pDW;
    QWidget* pHighestParent = NULL;
    do {
        pHighestParent = pSplitter;
        pSplitter = qobject_cast<QSplitter*>(pSplitter->parentWidget());
    } while (NULL != pSplitter);
    if (pHighestParent != pDW)
        XSplitterShowHide(qobject_cast<QSplitter*>(pHighestParent), pDW);
}

void CCustomDockWidgetTitle::btnMaximizeClicked()
{
    QWidget *dw = qobject_cast<QWidget*>(parentWidget());
    Q_ASSERT(dw != 0);
    m_bMaximized = !m_bMaximized;
    XSplitterShowHide(dw);
}

void CCustomDockWidgetTitle::btnCloseClicked()
{
    QWidget *dw = qobject_cast<QWidget*>(parentWidget());
    Q_ASSERT(dw != 0);
    dw->close();
}

void CCustomDockWidgetTitle::btnPinClicked()
{
    CCustomDockWidget *dw = qobject_cast<CCustomDockWidget*>(parentWidget());
    Q_ASSERT(dw != 0);
    m_bPinned = !m_bPinned;

    dw->setFloating(!m_bPinned);
}

void CCustomDockWidgetTitle::setPinned(bool pinned)
{
    m_bPinned = pinned;
}

void CCustomDockWidgetTitle::btnScreenshotClicked()
{
    VPL_SIGNAL(SigSaveScreenshot).invoke(m_viewType);
}

void CCustomDockWidgetTitle::onNewDensityData(data::CStorageEntry *pEntry)
{
    if (NULL == m_slider)
        return;

    data::CObjectPtr<data::CActiveDataSet> spDataSet(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(spDataSet->getId()));

    const vpl::tSize xSize = spVolume->getXSize();
    const vpl::tSize ySize = spVolume->getYSize();
    const vpl::tSize zSize = spVolume->getZSize();

    m_slider->blockSignals(true);
    if (DWT_SLIDER_XY == m_slider->property("Type").toInt())
    {
        m_slider->setMaximum(zSize - 1);
        data::CObjectPtr<data::COrthoSliceXY> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXY::Id));
        m_slider->setValue(spSlice->getPosition());
    }
    if (DWT_SLIDER_XZ == m_slider->property("Type").toInt())
    {
        m_slider->setMaximum(ySize - 1);
        data::CObjectPtr<data::COrthoSliceXZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXZ::Id));
        m_slider->setValue(spSlice->getPosition());
    }
    if (DWT_SLIDER_YZ == m_slider->property("Type").toInt())
    {
        m_slider->setMaximum(xSize - 1);
        data::CObjectPtr<data::COrthoSliceYZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceYZ::Id));
        m_slider->setValue(spSlice->getPosition());
    }
    if (DWT_SLIDER_ARB == m_slider->property("Type").toInt())
    {
        data::CObjectPtr<data::CArbitrarySlice> spSlice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
        m_slider->setMinimum(spSlice->getPositionMin());
        m_slider->setMaximum(spSlice->getPositionMax());
        m_slider->setValue(spSlice->getPosition());
    }
    m_slider->blockSignals(false);
}

void CCustomDockWidgetTitle::onNewSliceXY(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::COrthoSliceXY> spSlice(pEntry);
    m_slider->setValue(spSlice->getPosition());
}

void CCustomDockWidgetTitle::onNewSliceXZ(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::COrthoSliceXZ> spSlice(pEntry);
    m_slider->setValue(spSlice->getPosition());
}

void CCustomDockWidgetTitle::onNewSliceYZ(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::COrthoSliceYZ> spSlice(pEntry);
    m_slider->setValue(spSlice->getPosition());
}

void CCustomDockWidgetTitle::onNewSliceARB(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::CArbitrarySlice> spSlice(pEntry);
    m_slider->blockSignals(true);
    m_slider->setMinimum(spSlice->getPositionMin());
    m_slider->setMaximum(spSlice->getPositionMax());
    m_slider->blockSignals(false);
    m_slider->setValue(spSlice->getPosition());
}

void CCustomDockWidgetTitle::xySliderChange(int value)
{
    VPL_SIGNAL(SigSetSliceXY).invoke(value);
}

void CCustomDockWidgetTitle::xzSliderChange(int value)
{
    VPL_SIGNAL(SigSetSliceXZ).invoke(value);
}

void CCustomDockWidgetTitle::yzSliderChange(int value)
{
    VPL_SIGNAL(SigSetSliceYZ).invoke(value);
}

void CCustomDockWidgetTitle::arbSliderChange(int value)
{
    VPL_SIGNAL(SigSetSliceARB).invoke(value);
}

void CCustomDockWidgetTitle::createMenuForViewButton()
{
    if (NULL == m_viewButton)
    {
        return;
    }

    m_action3D = new QAction(QIcon(":/svg/svg/icon3d.svg"), "3D");
    m_actionXY = new QAction(QIcon(":/svg/svg/iconxy.svg"), "Axial / XY");
    m_actionXZ = new QAction(QIcon(":/svg/svg/iconxz.svg"), "Coronal / XZ");
    m_actionYZ = new QAction(QIcon(":/svg/svg/iconyz.svg"), "Sagittal / YZ");
    m_actionARB = new QAction(QIcon(":/svg/svg/iconarb.svg"), "Arbitrary");

    m_actionList.clear();
    m_actionList.push_back(m_action3D);
    m_actionList.push_back(m_actionXY);
    m_actionList.push_back(m_actionXZ);
    m_actionList.push_back(m_actionYZ);
    m_actionList.push_back(m_actionARB);

    QObject::connect(m_action3D, SIGNAL(triggered()), this, SLOT(show3DView()));
    QObject::connect(m_actionXY, SIGNAL(triggered()), this, SLOT(showXYView()));
    QObject::connect(m_actionXZ, SIGNAL(triggered()), this, SLOT(showXZView()));
    QObject::connect(m_actionYZ, SIGNAL(triggered()), this, SLOT(showYZView()));
    QObject::connect(m_actionARB, SIGNAL(triggered()), this, SLOT(showARBView()));

    std::vector<QAction*> actions;
    actions.push_back(m_action3D);
    actions.push_back(m_actionXY);
    actions.push_back(m_actionXZ);
    actions.push_back(m_actionYZ);
    actions.push_back(m_actionARB);

    QMenu* menu = new QMenu();

    for (int i = 0; i < actions.size(); ++i)
    {
        //if (i != m_viewType)
        {
            menu->addAction(actions[i]);
        }
    }

    m_viewButton->setMenu(menu);
}

void CCustomDockWidgetTitle::removeActionFromViewButton(int actionIndex)
{
    if (actionIndex < 0 || actionIndex >= m_actionList.size())
    {
        return;
    }

    m_viewButton->menu()->removeAction(m_actionList[actionIndex]);
}

void CCustomDockWidgetTitle::show3DView()
{
    VPL_SIGNAL(SigShowViewInWindow).invoke(0, m_viewType);
}

void CCustomDockWidgetTitle::showXYView()
{
    VPL_SIGNAL(SigShowViewInWindow).invoke(1, m_viewType);
}

void CCustomDockWidgetTitle::showXZView()
{
    VPL_SIGNAL(SigShowViewInWindow).invoke(2, m_viewType);
}

void CCustomDockWidgetTitle::showYZView()
{
    VPL_SIGNAL(SigShowViewInWindow).invoke(3, m_viewType);
}

void CCustomDockWidgetTitle::showARBView()
{
    VPL_SIGNAL(SigShowViewInWindow).invoke(4, m_viewType);
}


CCustomDockWidget::CCustomDockWidget(int viewType, const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags)
    , m_viewType(viewType)
{

}

CCustomDockWidget::CCustomDockWidget(int viewType, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
    , m_viewType(viewType)
{

}

CCustomDockWidget::~CCustomDockWidget()
{

}

void CCustomDockWidget::setFloating(bool floating)
{
    VPL_SIGNAL(SigMakeWindowFloating).invoke(m_viewType, floating, true);
}


CFloatingMainWindow::CFloatingMainWindow(int windowType, bool has2DCanvas, QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_windowType(windowType)
{
    //this->setFocusPolicy(Qt::StrongFocus);
    //this->setWindowFlags(flags | Qt::WindowStaysOnTopHint);
}

CFloatingMainWindow::~CFloatingMainWindow()
{

}

void CFloatingMainWindow::closeEvent(QCloseEvent* event)
{
    if (!this->property("StayFloating").toBool())
    {
        VPL_SIGNAL(SigMakeWindowFloating).invoke(m_windowType, false, true);
    }

    event->accept();
}

void CFloatingMainWindow::enterEvent(QEvent* event)
{
    QApplication::setActiveWindow(this);

    /*QPoint posBkp = QApplication::desktop()->cursor().pos();
    QPoint point = this->pos();

    setCursor(Qt::BlankCursor);

    QApplication::desktop()->cursor().setPos(point.x() + 150, point.y() + 100);
    mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 1, 1, 0, 0);
    QApplication::desktop()->cursor().setPos(posBkp);

    setCursor(Qt::ArrowCursor);*/

    //QApplication::processEvents();

    //event->accept();
}
