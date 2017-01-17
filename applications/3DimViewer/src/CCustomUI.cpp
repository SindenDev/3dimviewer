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

CCustomDockWidgetTitle::CCustomDockWidgetTitle(int flags, QWidget *parent)
    : QWidget(parent)
{
    m_bMaximized = false;
    m_bIconMaximize = true;
    m_closeButton = NULL;
    m_maximizeButton = NULL;
    m_screenshotButton = NULL;
    m_slider = NULL;

    double factor = C3DimApplication::getDpiFactor();
    factor = 1 + (factor - 1)/2; // reduce upscale because our icons aren't that good
    m_dockButtonSize = 18 * factor;
    m_dockTitleHeight = 22 * factor;

    if (DWT_BUTTONS_CLOSE==(flags&DWT_BUTTONS_CLOSE))
    {
        m_closeButton = new QToolButton;
        m_closeButton->setAutoRaise(true);
        m_closeButton->setIcon(QIcon(":/icons/dock_icon_close.png"));
        m_closeButton->setMinimumSize(8,8);
        m_closeButton->setMaximumSize(m_dockButtonSize-2,m_dockButtonSize);
        QObject::connect(m_closeButton,SIGNAL(clicked()), this, SLOT(btnCloseClicked()));
    }
    if (DWT_BUTTONS_MAXIMIZE==(flags&DWT_BUTTONS_MAXIMIZE))
    {
        m_maximizeButton = new QToolButton;
        m_maximizeButton->setAutoRaise(true);
        m_maximizeButton->setIcon(QIcon(":/icons/dock_icon_maximize.png"));
        m_maximizeButton->setMinimumSize(8,8);
        m_maximizeButton->setMaximumSize(m_dockButtonSize-2,m_dockButtonSize);
        QObject::connect(m_maximizeButton,SIGNAL(clicked()), this, SLOT(btnMaximizeClicked()));
    }    
    {
        m_screenshotButton = new QToolButton;
        m_screenshotButton->setAutoRaise(true);
        m_screenshotButton->setIcon(QIcon(":/icons/dock_icon_screenshot.png"));       
        m_screenshotButton->setMinimumSize(8,8);
        m_screenshotButton->setMaximumSize(m_dockButtonSize-2,m_dockButtonSize);
        QObject::connect(m_screenshotButton,SIGNAL(clicked()), this, SLOT(btnScreenshotClicked()));
    }    
    if (DWT_SLIDER_VR == (flags&DWT_SLIDER_VR))
    {        
        m_slider = new CSliderEx(Qt::Horizontal,this);
		m_slider->setProperty("Type",DWT_SLIDER_VR);
        m_slider->setContentsMargins(1,2,1,2);
        m_slider->setRange(-100,200);
		m_slider->setToolTip(tr("Adjusts VR density threshold"));
        m_slider->show();		
        QObject::connect(m_slider,SIGNAL(valueChanged(int)),this,SLOT(vrValueChanged(int)));
        m_vrDataRemapChangeConnection = VPL_SIGNAL(SigVRDataRemapChange).connect(this, &CCustomDockWidgetTitle::vrDataRemapChange);
    }
    if (DWT_SLIDER_XY == (flags&DWT_SLIDER_XY))
    {        
        m_slider = new CSliderEx(Qt::Horizontal,this);
		m_slider->setProperty("Type",DWT_SLIDER_XY);
        m_slider->setContentsMargins(1,2,1,2);
        m_slider->setRange(0,100);
		m_slider->setSingleStep(1);
		m_slider->setPageStep(1);
		m_slider->setToolTip(tr("Adjusts slice position"));
        m_slider->show();		
        QObject::connect(m_slider,SIGNAL(valueChanged(int)),this,SLOT(xySliderChange(int)));
		m_ConnectionData = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &CCustomDockWidgetTitle::onNewDensityData);
		m_ConnectionXY = APP_STORAGE.getEntrySignal(data::Storage::SliceXY::Id).connect(this, &CCustomDockWidgetTitle::onNewSliceXY);
    }	
    if (DWT_SLIDER_XZ == (flags&DWT_SLIDER_XZ))
    {
        m_slider = new CSliderEx(Qt::Horizontal,this);
		m_slider->setProperty("Type",DWT_SLIDER_XZ);
        m_slider->setContentsMargins(1,2,1,2);
        m_slider->setRange(0,100);
		m_slider->setSingleStep(1);
		m_slider->setPageStep(1);
		m_slider->setToolTip(tr("Adjusts slice position"));
        m_slider->show();
        QObject::connect(m_slider,SIGNAL(valueChanged(int)),this,SLOT(xzSliderChange(int)));
		m_ConnectionData = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &CCustomDockWidgetTitle::onNewDensityData);
		m_ConnectionXZ = APP_STORAGE.getEntrySignal(data::Storage::SliceXZ::Id).connect(this, &CCustomDockWidgetTitle::onNewSliceXZ);
    }	
    if (DWT_SLIDER_YZ == (flags&DWT_SLIDER_YZ))
    {
        m_slider = new CSliderEx(Qt::Horizontal,this);
		m_slider->setProperty("Type",DWT_SLIDER_YZ);
        m_slider->setContentsMargins(1,2,1,2);
        m_slider->setRange(0,100);
		m_slider->setSingleStep(1);
		m_slider->setPageStep(1);
		m_slider->setToolTip(tr("Adjusts slice position"));
        m_slider->show();
        QObject::connect(m_slider,SIGNAL(valueChanged(int)),this,SLOT(yzSliderChange(int)));
		m_ConnectionData = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &CCustomDockWidgetTitle::onNewDensityData);
		m_ConnectionYZ = APP_STORAGE.getEntrySignal(data::Storage::SliceYZ::Id).connect(this, &CCustomDockWidgetTitle::onNewSliceYZ);
    }
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->setSpacing(1);
    pLayout->setContentsMargins(2,2,2,2);
    pLayout->addStretch();
#ifdef __APPLE__
    if (NULL!=m_slider)
        pLayout->addWidget(m_slider,0,Qt::AlignRight);
#else
    if (NULL!=m_slider)
        pLayout->addWidget(m_slider,0,0/*Qt::AlignRight*/);
#endif
    if (NULL!=m_screenshotButton)
        pLayout->addWidget(m_screenshotButton,0,Qt::AlignRight);
    if (NULL!=m_maximizeButton)
        pLayout->addWidget(m_maximizeButton,0,Qt::AlignRight);
    if (NULL!=m_closeButton)
        pLayout->addWidget(m_closeButton,0,Qt::AlignRight);
    setLayout(pLayout);

    QObject::connect(this,SIGNAL(saveScreenshot(OSGCanvas*)),MainWindow::getInstance(),SLOT(saveScreenshot(OSGCanvas*)));
	QObject::connect(this,SIGNAL(copyScreenshotToClipboard(OSGCanvas*)),MainWindow::getInstance(),SLOT(copyScreenshotToClipboard(OSGCanvas*)));
    QObject::connect(this,SIGNAL(saveSlice(OSGCanvas*,int)),MainWindow::getInstance(),SLOT(saveSlice(OSGCanvas*,int)));

	onNewDensityData(NULL);
}

CCustomDockWidgetTitle::~CCustomDockWidgetTitle()
{
	if (NULL!=m_slider && DWT_SLIDER_VR==m_slider->property("Type").toInt())
        VPL_SIGNAL(SigVRDataRemapChange).disconnect(m_vrDataRemapChangeConnection);
	if (NULL!=m_ConnectionData.getSignalPtr())
		APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).disconnect(m_ConnectionData);
	if (NULL!=m_ConnectionXY.getSignalPtr())
		APP_STORAGE.getEntrySignal(data::Storage::SliceXY::Id).disconnect(m_ConnectionXY);
	if (NULL!=m_ConnectionXZ.getSignalPtr())
		APP_STORAGE.getEntrySignal(data::Storage::SliceXZ::Id).disconnect(m_ConnectionXZ);
	if (NULL!=m_ConnectionYZ.getSignalPtr())
		APP_STORAGE.getEntrySignal(data::Storage::SliceYZ::Id).disconnect(m_ConnectionYZ);
}

QSize CCustomDockWidgetTitle::minimumSizeHint() const
{
    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    Q_ASSERT(dw != 0);
    QSize result(m_dockTitleHeight,m_dockTitleHeight);
    if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar)
        result.transpose();
    return result;
}

void CCustomDockWidgetTitle::paintEvent(QPaintEvent*)
{
    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    Q_ASSERT(dw);
    if (!dw) return;
    {
        QString iconPath = dw->property("Icon").toString();

        QStylePainter p(this);
        p.setBrush((QBrush(Qt::lightGray)));
        QStyleOptionDockWidgetV2 titleOpt;
        QRect rect(0,0,dw->width(),m_dockTitleHeight);
        titleOpt.rect=rect;
        titleOpt.floatable=0!=(dw->features()&QDockWidget::DockWidgetFloatable);
        titleOpt.movable=0!=(dw->features()&QDockWidget::DockWidgetMovable);
        titleOpt.closable=0!=(dw->features()&QDockWidget::DockWidgetClosable);
#ifdef WIN32
        if (!iconPath.isEmpty())
            titleOpt.title = "       " + dw->windowTitle();
        else
#endif
            titleOpt.title = dw->windowTitle();
        p.drawControl(QStyle::CE_DockWidgetTitle, titleOpt);
        
        if (!iconPath.isEmpty())            
        {
            QImage img(QImage(iconPath).scaledToHeight(m_dockTitleHeight-6,Qt::SmoothTransformation));
            p.drawImage(QRectF((m_dockTitleHeight-img.width()+2)/2,(m_dockTitleHeight-img.height()+1)/2,img.width(),img.height()),img);
        }

        if (m_maximizeButton)
        {
            if (m_bMaximized)
            {
                if (m_bIconMaximize)
                    m_maximizeButton->setIcon(QIcon(":/icons/dock_icon_restore.png"));
                m_bIconMaximize=false;
            }
            else
            {
                if (!m_bIconMaximize)
                    m_maximizeButton->setIcon(QIcon(":/icons/dock_icon_maximize.png"));
                m_bIconMaximize=true;
            }
        }
    }
}

void CCustomDockWidgetTitle::vrValueChanged(int value)
{
    MainWindow* pMain = MainWindow::getInstance();
    if (NULL!=pMain)
    {
        PSVR::PSVolumeRendering* pRenderer = pMain->getRenderer();
        if (NULL!=pRenderer)
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
    m_slider->setValue(myround(offset*500));
    m_slider->blockSignals(false);
}


// switches visibility state of all sub-widgets that are not pDWIgnore
static void XSplitterShowHide(QSplitter* pHighestParent, QDockWidget* pDWIgnore)
{
    Q_ASSERT(NULL!=pHighestParent && NULL!=pDWIgnore);
    QSplitter* pS = qobject_cast<QSplitter*>(pHighestParent);
    Q_ASSERT(NULL!=pS);
    for(int i=0;i<pS->count();i++)
    {
        QWidget* pWidget=pS->widget(i);
        if (NULL!=pWidget && pWidget!=pDWIgnore)
        {
            QSplitter* pSS=qobject_cast<QSplitter*>(pWidget);
            if (NULL!=pSS)
            {
                XSplitterShowHide(pSS,pDWIgnore);
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
static void XSplitterShowHide(QDockWidget* pDW)
{
    if (NULL==pDW) return;
    QWidget* pSplitter=pDW;
    QWidget* pHighestParent=NULL;
    do {
        pHighestParent=pSplitter;
        pSplitter = qobject_cast<QSplitter*>(pSplitter->parentWidget());
    }
    while(NULL!=pSplitter);
    if (pHighestParent!=pDW)
        XSplitterShowHide(qobject_cast<QSplitter*>(pHighestParent),pDW);
}

void CCustomDockWidgetTitle::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    QRect rect = this->rect();

    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    Q_ASSERT(dw != 0);

    if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar) {
        QPoint p = pos;
        pos.setX(rect.left() + rect.bottom() - p.y());
        pos.setY(rect.top() + p.x() - rect.left());

        QSize s = rect.size();
        s.transpose();
        rect.setSize(s);
    }

    if (Qt::RightButton == event->button())
    {
        pos=this->mapToGlobal(pos);
        OSGCanvas* pCanvas = qobject_cast<OSGCanvas*>(dw->widget());
        QMenu contextMenu;
		QAction* copyScreenshot2ClipboardA=contextMenu.addAction(tr("Copy Screenshot to Clipboard"));
        QAction* saveScreenshotA=contextMenu.addAction(tr("Save Screenshot"));
        QAction* saveSliceA=NULL;
#ifdef _DEBUG
        if (NULL!=dynamic_cast<OSGOrtho2DCanvas*>(pCanvas))
            saveSliceA = contextMenu.addAction(tr("Save slice"));
#endif
        QAction* win=contextMenu.exec(pos);
        if (NULL!=win)
        {
            if (win==saveScreenshotA)
            {                
                if (NULL!=pCanvas)
                    emit saveScreenshot(pCanvas);
            }
			if (win==copyScreenshot2ClipboardA)
            {                
                if (NULL!=pCanvas)
                    emit copyScreenshotToClipboard(pCanvas);
            }			
            if (win==saveSliceA)
            {
                if (NULL!=pCanvas)
                    emit saveSlice(pCanvas,0);
            }            
        }
    }

    event->ignore();
}

void CCustomDockWidgetTitle::mouseDoubleClickEvent ( QMouseEvent * event )
{
    if (Qt::LeftButton==event->buttons())
    {
        QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
        Q_ASSERT(dw != 0);
        event->accept();
        m_bMaximized=!m_bMaximized;
        XSplitterShowHide(dw);
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void CCustomDockWidgetTitle::btnMaximizeClicked()
{
    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    Q_ASSERT(dw != 0);
    m_bMaximized=!m_bMaximized;
    XSplitterShowHide(dw);
}

void CCustomDockWidgetTitle::btnCloseClicked()
{
    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    Q_ASSERT(dw != 0);
    dw->close();
}

void CCustomDockWidgetTitle::btnScreenshotClicked()
{
    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    OSGCanvas* pCanvas = qobject_cast<OSGCanvas*>(dw->widget());
    if (NULL!=pCanvas)
        emit saveScreenshot(pCanvas);
}

void CCustomDockWidgetTitle::onNewDensityData(data::CStorageEntry *pEntry)
{
	if (NULL==m_slider) 
		return;

    data::CObjectPtr<data::CActiveDataSet> spDataSet( APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id) );
    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(spDataSet->getId()) );

	const vpl::tSize xSize=spVolume->getXSize();
	const vpl::tSize ySize=spVolume->getYSize();
	const vpl::tSize zSize=spVolume->getZSize();
	m_slider->blockSignals(true);
	if (DWT_SLIDER_XY==m_slider->property("Type").toInt())
	{
		m_slider->setMaximum(zSize-1);
		data::CObjectPtr<data::COrthoSliceXY> spSlice( APP_STORAGE.getEntry(data::Storage::SliceXY::Id) );
		m_slider->setValue(spSlice->getPosition());
	}
	if (DWT_SLIDER_XZ==m_slider->property("Type").toInt())
	{
		m_slider->setMaximum(ySize-1);
		data::CObjectPtr<data::COrthoSliceXZ> spSlice( APP_STORAGE.getEntry(data::Storage::SliceXZ::Id) );
		m_slider->setValue(spSlice->getPosition());
	}
	if (DWT_SLIDER_YZ==m_slider->property("Type").toInt())
	{
		m_slider->setMaximum(xSize-1);
		data::CObjectPtr<data::COrthoSliceYZ> spSlice( APP_STORAGE.getEntry(data::Storage::SliceYZ::Id) );
		m_slider->setValue(spSlice->getPosition());
	}
	m_slider->blockSignals(false);
}



void CCustomDockWidgetTitle::onNewSliceXY(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::COrthoSliceXY> spSlice( pEntry );
	m_slider->setValue(spSlice->getPosition());
}

void CCustomDockWidgetTitle::onNewSliceXZ(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::COrthoSliceXZ> spSlice( pEntry );
    m_slider->setValue(spSlice->getPosition());
}

void CCustomDockWidgetTitle::onNewSliceYZ(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::COrthoSliceYZ> spSlice( pEntry );
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