#include <CCustomUI.h>
#include <osg/OSGCanvas.h>
#include <osg/OSGOrtho2DCanvas.h>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QStylePainter>
#include <mainwindow.h>


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

#define DOCKBUTTON_SIZE     18
#define DOCKTITLE_HEIGHT    22

QSize CCustomDockWidgetTitle::minimumSizeHint() const
{
    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    Q_ASSERT(dw != 0);
    QSize result(DOCKTITLE_HEIGHT,DOCKTITLE_HEIGHT);
    if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar)
        result.transpose();
    return result;
}

CCustomDockWidgetTitle::CCustomDockWidgetTitle(int flags, QWidget *parent)
    : QWidget(parent)
{
    m_bMaximized = false;
    m_bIconMaximize = true;
    m_closeButton = NULL;
    m_maximizeButton = NULL;
    m_screenshotButton = NULL;
    m_slider = NULL;

    if (DWT_BUTTONS_CLOSE==(flags&DWT_BUTTONS_CLOSE))
    {
        m_closeButton = new QToolButton;
        m_closeButton->setAutoRaise(true);
        m_closeButton->setIcon(QIcon(":/icons/dock_icon_close.png"));
        m_closeButton->setMinimumSize(8,8);
        m_closeButton->setMaximumSize(DOCKBUTTON_SIZE-2,DOCKBUTTON_SIZE);
        QObject::connect(m_closeButton,SIGNAL(clicked()), this, SLOT(btnCloseClicked()));
    }
    if (DWT_BUTTONS_MAXIMIZE==(flags&DWT_BUTTONS_MAXIMIZE))
    {
        m_maximizeButton = new QToolButton;
        m_maximizeButton->setAutoRaise(true);
        m_maximizeButton->setIcon(QIcon(":/icons/dock_icon_maximize.png"));
        m_maximizeButton->setMinimumSize(8,8);
        m_maximizeButton->setMaximumSize(DOCKBUTTON_SIZE-2,DOCKBUTTON_SIZE);
        QObject::connect(m_maximizeButton,SIGNAL(clicked()), this, SLOT(btnMaximizeClicked()));
    }    
    {
        m_screenshotButton = new QToolButton;
        m_screenshotButton->setAutoRaise(true);
        m_screenshotButton->setIcon(QIcon(":/icons/dock_icon_screenshot.png"));
        m_screenshotButton->setMinimumSize(8,8);
        m_screenshotButton->setMaximumSize(DOCKBUTTON_SIZE-2,DOCKBUTTON_SIZE);
        QObject::connect(m_screenshotButton,SIGNAL(clicked()), this, SLOT(btnScreenshotClicked()));
    }    
    if (DWT_SLIDER_VR == (flags&DWT_SLIDER_VR))
    {        
        m_slider = new CSliderEx(Qt::Horizontal,this);
        m_slider->setContentsMargins(1,2,1,2);
        m_slider->setRange(-100,200);
        m_slider->show();
        QObject::connect(m_slider,SIGNAL(valueChanged(int)),this,SLOT(vrValueChanged(int)));
        m_vrDataRemapChangeConnection = VPL_SIGNAL(SigVRDataRemapChange).connect(this, &CCustomDockWidgetTitle::vrDataRemapChange);
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
    QObject::connect(this,SIGNAL(saveSlice(OSGCanvas*,int)),MainWindow::getInstance(),SLOT(saveSlice(OSGCanvas*,int)));
}

CCustomDockWidgetTitle::~CCustomDockWidgetTitle()
{
    if (NULL!=m_slider)
        VPL_SIGNAL(SigVRDataRemapChange).disconnect(m_vrDataRemapChangeConnection);

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
        QRect rect(0,0,dw->width(),DOCKTITLE_HEIGHT);
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
            QImage img(QImage(iconPath).scaledToHeight(DOCKTITLE_HEIGHT-6,Qt::SmoothTransformation));
            p.drawImage(QRectF((DOCKTITLE_HEIGHT-img.width()+2)/2,(DOCKTITLE_HEIGHT-img.height()+1)/2,img.width(),img.height()),img);
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
        QAction* saveScreenshotA=contextMenu.addAction(tr("Save screenshot"));
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