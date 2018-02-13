///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//


#include "notespluginpanel.h"
#include "ui_notespluginpanel.h"

#include <numeric>
#include <QDesktopWidget>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <QHBoxLayout>
#include <QHeaderView>


#include <QColorDialog>
#include <QMouseEvent>
#include <QMenu>

#include <osg/CSceneManipulator.h>

#include "render/PSVRrenderer.h"
#include <osg/OSGCanvas.h>


const int CNotesPluginPanel::keystrokeSaveTimerInterval = 350;
const float CNotesPluginPanel::rotationSimilarityThreshold = 1.0f - 0.0001f;  // 1.0 is the same so allow some leeway
const float CNotesPluginPanel::translationSimilarityThreshold = 1.0f;     //max allowed distance; again some float leeway


const QString CNotesPluginPanel::default_textedit_text = QT_TRANSLATE_NOOP("CNotesPluginPanel", "Type your note here or start drawing.");
const QString CNotesPluginPanel::default_note_text = QT_TRANSLATE_NOOP("CNotesPluginPanel", "Write your note here  . . .");
const QString CNotesPluginPanel::default_note_name = QT_TRANSLATE_NOOP("CNotesPluginPanel", "Note number ");
int CNotesPluginPanel::note_name_index = 1;



QColor data::CColorGenerator::getQColor() {

    if (index < predefinedColors.size()) {
        index++;
        lastColor = predefinedColors.at(index - 1);
        return CColorToQColor(lastColor);
    }


    float red = ((13 * index) % 256);
    float green = ((14 * (int)red * 255) % 256);
    float blue = ((7 * (int)green * 255) % 256);

    index++;

    lastColor = QColorToCColor(QColor(red, green, blue, 255));

    return QColor(red, green, blue, 255);
}

data::CColor4f data::CColorGenerator::getCColor() {


    if (index < predefinedColors.size()) {
        index++;
        lastColor = predefinedColors.at(index - 1);
        return lastColor;
    }


    float red = ((13 * index) % 256);
    float green = ((14 * (int)red * 255) % 256);
    float blue = ((7 * (int)green * 255) % 256);

    index++;

    lastColor = data::CColor4f(red, green, blue, 1.0f);
    return lastColor;
}

//! Implementation of virtual function to highlight text only when the text is default
void QTextEdit::mouseReleaseEvent(QMouseEvent *event) {
    if (isReadOnly())
        return;

    if (toPlainText().compare(CNotesPluginPanel::tr(CNotesPluginPanel::default_note_text.toStdString().c_str())) == 0 ||
        toPlainText().compare(CNotesPluginPanel::tr(CNotesPluginPanel::default_textedit_text.toStdString().c_str())) == 0)
        selectAll();

}




CNotesPluginPanel::CNotesPluginPanel(CAppBindings *pBindings, QWidget *parent) :
QWidget(parent),
    ui(new Ui::CNotesPluginPanel)
{
    m_visibilityInvalidated = false;
    m_hasFocus = false;
    m_transparent = false;
    m_lastActiveRow = -1;
    m_sizeFactor = 1.0f;
    m_itemSelectionChanged = false;
    m_zoom_level = 0;
    m_restore = false;
    m_cameraManipulator = nullptr;
    m_customBackgroundColor = QColor(211, 241, 247, 255);

    ui->setupUi(this);
    Q_ASSERT(pBindings);
    setAppMode(pBindings->getAppMode());
    setDataStorage(pBindings->getDataStorage());
    setRenderer(pBindings->getRenderer());
    setVPLSignals(pBindings->getVPLSignals());


    this->setFocusPolicy(Qt::ClickFocus);

    ui->noteText->installEventFilter(this);
    ui->noteText->document()->setUndoRedoEnabled(false);

    ui->strokeColorLabel->setVisible(false);
    ui->colorSelectorButton->setVisible(false);

    ui->noteTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->noteTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    connect(&m_keystrokeSaveTimer, SIGNAL(timeout()), this, SLOT(saveText()));


    m_notesDataSignalConnection = PLUGIN_APP_STORAGE.getEntrySignal(data::Storage::NoteData::Id).connect(this, &CNotesPluginPanel::on_notesData_itemChanged);


    QDesktopWidget* pDesktop = QApplication::desktop();
    if (pDesktop != nullptr)
        m_sizeFactor = std::max(1.0f, pDesktop->logicalDpiX() / 96.0f);

    buildNoteTree();

    m_currentColor = colorGenerator.getQColor();
    m_nextUpColor = colorGenerator.repeatQColor();

    //ui->colorSelectorButton->setAutoFillBackground(true);

    // Initialize color box first time
    QString qss = QString(
        "QPushButton {"
        "background-color: red;"
        "border-style: outset;"
        "border-width: 1px;"
        "border-radius: 2px;"
        "border-color: black;}");

    ui->colorSelectorButton->setToolTip(tr("Sets the default color of new note"));
    ui->colorSelectorButton->setStatusTip(tr("Sets the default color of new note"));

    ui->colorSelectorButton->setStyleSheet(qss);
    ui->colorSelectorButton->update();

    QString qss2 = QString(
        "QPushButton:checked {"
            "background-color: rgb(220, 255, 220);"
            "border-style: solid;"
            "border-width: 1px;"
            "border-color: rgb(130, 255, 130);"
            "height: %1px;"
        "}\n"

        "QPushButton:!checked {"
            "background-color: rgb(255, 220, 220);"
            "border-style: solid;"
            "border-width: 1px;"
            "border-color: rgb(255, 130, 130);"
            "height: %1px;"
        "}\n"
    
        "QPushButton:hover {"
            "border-style: solid;"
            "border-width: 1px;"
            "border-color: black;"
            "height: %1px;"
            "}").arg(24 * m_sizeFactor);  //82, 95

    ui->notesActivePushButton->setStyleSheet(qss2);
    ui->notesActivePushButton->update();

    connectDrawingHandler();

    m_drawingMode = data::CDrawingOptions::DRAW_STROKE;

    setDrawingOptions();

    ui->noteText->setText(tr(CNotesPluginPanel::default_textedit_text.toStdString().c_str()));

    qDebug() << "NotePluginPanel constructed !!!!!!";
}


CNotesPluginPanel::~CNotesPluginPanel() {

    PLUGIN_APP_MODE.disconnectAllDrawingHandlers();
    PLUGIN_APP_STORAGE.getEntrySignal(data::Storage::NoteData::Id).disconnect(m_notesDataSignalConnection);


    qDebug() << "NotePluginPanel destructed !!!!!!";

}


void CNotesPluginPanel::insertNoteToTree(int row, data::CNote *note) {
    
    //http://doc.qt.io/qt-4.8/qt.html#ItemDataRole-enum
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->noteTree);
    

    ui->noteTree->insertTopLevelItem(row, item);


    item->setFlags(item->flags() | Qt::ItemIsEditable /*| Qt::ItemIsUserCheckable*/ | Qt::ItemIsSelectable);

    item->setData(treeColumns::name, Qt::EditRole, QString::fromStdString(note->m_name));
    item->setToolTip(treeColumns::name, tr("Double click to rename."));
    item->setStatusTip(treeColumns::name, tr("Double click to rename."));


    QWidget *color_button_in_widget = new QWidget();
    QPushButton *color_button = new QPushButton();

    spitOutNewColorButton(color_button, color_button_in_widget);
    setButtonColor(color_button, colorGenerator.CColorToQColor(note->color));

    treeElementToItemMap.insert(color_button, item);

    ui->noteTree->setItemWidget(item, treeColumns::color, color_button_in_widget);


    QWidget *visibility_checkBox_in_widget = new QWidget();
    QCheckBox *box = new QCheckBox();

    spitOutNewVisibilityCheckBox(box, visibility_checkBox_in_widget);

    treeElementToItemMap.insert(box, item);

    ui->noteTree->setItemWidget(item, treeColumns::visibility, visibility_checkBox_in_widget);



    QWidget *discard_button_in_widget = new QWidget();
    QPushButton *discard_button = new QPushButton();

    spitOutNewDiscardButton(discard_button, discard_button_in_widget);
    
    treeElementToItemMap.insert(discard_button, item);

    ui->noteTree->setItemWidget(item, treeColumns::discard, discard_button_in_widget);



    treeItemToElementsMap.insert(item, color_button);
    treeItemToElementsMap.insert(item, box);
    treeItemToElementsMap.insert(item, discard_button);

}



void CNotesPluginPanel::buildNoteTree() const {

        
	ui->noteTree->setColumnCount(4);
    ui->noteTree->setUniformRowHeights(true);
    ui->noteTree->setAllColumnsShowFocus(true);



    ui->noteTree->setSelectionMode(QTreeView::SingleSelection);
    ui->noteTree->setSelectionBehavior(QTreeView::SelectRows);
    ui->noteTree->setSortingEnabled(false);

	//prevents editing of field
	//ui->noteTree->setItemDelegateForColumn(1, new NoEditDelegate(this));
    //ui->noteTree->setItemDelegateForColumn(2, new NoEditDelegate(this));

    


    QTreeWidgetItem *headerItem = new QTreeWidgetItem();
    headerItem->setText(treeColumns::name, tr("Name"));
    headerItem->setText(treeColumns::color, tr("  Color"));
    headerItem->setText(treeColumns::visibility, "");
    headerItem->setText(treeColumns::discard, "");

    
    headerItem->setIcon(treeColumns::visibility, QIcon(":/icons/icons/visibility.png"));

    ui->noteTree->setHeaderItem(headerItem);

    ui->noteTree->header()->setStretchLastSection(false);

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))    
	ui->noteTree->header()->setResizeMode(treeColumns::name         , QHeaderView::Stretch);
    ui->noteTree->header()->setResizeMode(treeColumns::color        , QHeaderView::Fixed);
    ui->noteTree->header()->setResizeMode(treeColumns::visibility   , QHeaderView::Fixed);
    ui->noteTree->header()->setResizeMode(treeColumns::discard      , QHeaderView::Fixed);
#else
    ui->noteTree->header()->setSectionResizeMode(treeColumns::name, QHeaderView::Stretch);
    ui->noteTree->header()->setSectionResizeMode(treeColumns::color, QHeaderView::Fixed);
    ui->noteTree->header()->setSectionResizeMode(treeColumns::visibility, QHeaderView::Fixed);
    ui->noteTree->header()->setSectionResizeMode(treeColumns::discard, QHeaderView::Fixed);
#endif


    ui->noteTree->setColumnWidth(treeColumns::name, 200 * m_sizeFactor);
    ui->noteTree->setColumnWidth(treeColumns::color, 50 * m_sizeFactor);
    ui->noteTree->setColumnWidth(treeColumns::visibility, 30 * m_sizeFactor);
    ui->noteTree->setColumnWidth(treeColumns::discard, 32 * m_sizeFactor);

    connect(ui->noteTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(onNoteTreeCurrentItemChanged(QTreeWidgetItem*)));
}


void CNotesPluginPanel::spitOutNewVisibilityCheckBox(QCheckBox *box, QWidget *widget_with_checkbox) const 
{
    QHBoxLayout *pLayout = new QHBoxLayout(widget_with_checkbox);
    pLayout->addWidget(box);
    pLayout->setAlignment(Qt::AlignCenter);
    pLayout->setContentsMargins(0, 0, 0, 0);
    widget_with_checkbox->setLayout(pLayout);

    box->setToolTip(tr("Forces the note to show or hide"));
    box->setStatusTip(tr("Forces the note to show or hide"));

    connect(box, SIGNAL(clicked()), this, SLOT(onCheckBoxClicked()));

}

void CNotesPluginPanel::spitOutNewColorButton(QPushButton *button, QWidget *widget_with_button) const 
{
    button->setFlat(true);
    button->setObjectName("colorButton");
    button->setMaximumWidth(32);
    button->setMaximumHeight(24);

    button->setToolTip(tr("Will change color of this note"));
    button->setStatusTip(tr("Will change color of this note"));

    //gets colored by button style sheet.. weird..

    setButtonColor(button, Qt::red); 

    connect(button, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));

    QHBoxLayout *pLayout = new QHBoxLayout(widget_with_button);
    pLayout->addWidget(button);
    pLayout->setAlignment(Qt::AlignCenter);
    pLayout->setContentsMargins(0, 0, 0, 0);
    widget_with_button->setLayout(pLayout);

}

void CNotesPluginPanel::spitOutNewDiscardButton(QPushButton *button, QWidget *widget_with_button) const 
{   
    button->setFlat(true);
    button->setObjectName("discardButton");
    button->setMaximumWidth(32);
    button->setMaximumHeight(32);
    button->setIcon(QIcon(":/icons/icons/notedeletesmall.png"));
    button->setToolTip(tr("Will delete this note"));
    button->setStatusTip(tr("Will delete this note"));

    connect(button, SIGNAL(clicked()), this, SLOT(onDiscardButtonClicked()));

    QHBoxLayout *pLayout = new QHBoxLayout(widget_with_button);
    pLayout->addWidget(button);
    pLayout->setAlignment(Qt::AlignCenter);
    pLayout->setContentsMargins(0, 0, 0, 0);
    widget_with_button->setLayout(pLayout);
}

void CNotesPluginPanel::onDiscardButtonClicked() {
    
    snap();

    QTreeWidgetItem* item = treeElementToItemMap.value(sender());

    int index = ui->noteTree->indexOfTopLevelItem(item);

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));

    noteData->discardNote(index);
    
    m_action = data::noteChange(index, data::noteChange::change_enum::discard);

    noteData->m_noteChanges.push_back(m_action);

    m_visibilityInvalidated = true;
    PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

    //current row changes after this and triggers change in NoteSubtree so the signal from
    //invalidate must get there first..
    // discardNoteFromList(index);
     discardNoteFromTree(index);
}


void CNotesPluginPanel::onColorButtonClicked() 
{
    QTreeWidgetItem* item = treeElementToItemMap.value(sender());

    ui->noteTree->setCurrentItem(item);
    int index = ui->noteTree->indexOfTopLevelItem(item);

    QColor oldColor = Qt::red;

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));
   
    auto currentNote = noteData->getNote(index);

    if (currentNote != nullptr)
        oldColor = colorGenerator.CColorToQColor(currentNote->color);
    

    QColor newColor;
    newColor = QColorDialog::getColor(oldColor);

    if (newColor.isValid()) {
        
        snap();

        setButtonColor((QPushButton *)sender(), newColor);

        if (currentNote != nullptr)
            currentNote->color = colorGenerator.QColorToCColor(newColor);

        setDrawingOptions();
        
        data::lineChange change(data::lineChange::action_enum::color, index, data::CColor4f(newColor.redF(), newColor.greenF(), newColor.blueF(), 1.0f));
        noteData->addToLineChangeQueue(change);

        m_action = data::noteChange(0, data::noteChange::change_enum::line);

        PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

    }


    qDebug() << ui->noteTree->currentIndex().row();
}

void CNotesPluginPanel::onCheckBoxClicked() {

    //only user click
    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));


    QTreeWidgetItem* item = treeElementToItemMap.value(sender());

    int index = ui->noteTree->indexOfTopLevelItem(item);

    int indexOfCurrentItem = getIndexOfCurrentItem();

    //deselecting current item causes all notes with similar view have their view deselected too..
    if (ui->showSimilarCheckBox->isChecked() && index == indexOfCurrentItem) {

        uncheckAll();
        noteData->setAllVisibleOff();

        for (size_t i = 0; i < noteData->size(); ++i)
            setRowBackgroundColor(i, Qt::white);

        ui->noteTree->setCurrentItem(nullptr);
        m_visibilityInvalidated = true;
        PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

        getRenderer()->redraw();

        return;
    }


    if (((QCheckBox *)sender())->isChecked()) 
        noteData->getNote(index)->m_isVisible = true;

    else
        noteData->getNote(index)->m_isVisible = false;

    m_visibilityInvalidated = true;
    PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());


    //if none is checked deselect current item
    if (getNumberOfChecked() == 0)
        ui->noteTree->setCurrentItem(nullptr);
   

    getRenderer()->redraw();
}


void CNotesPluginPanel::on_noteTree_itemDoubleClicked(QTreeWidgetItem *item, int column) {
    
    if (column != name)
        return;

    m_double_click_name_backup = item->data(column, Qt::EditRole).toString();

}

void CNotesPluginPanel::on_noteTree_itemChanged(QTreeWidgetItem *item, int column) {
    
    if (column != name)
        return;

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));

    int index = ui->noteTree->indexOfTopLevelItem(item);

    auto currentNote = noteData->getNote(index);
    
    ////
    if (currentNote == nullptr || m_double_click_name_backup.size() == 0 || m_double_click_name_backup.compare(item->data(column, Qt::EditRole).toString()) == 0)
        return;
     

    //now i'm confident that it was user originated change of name
    m_double_click_name_backup.clear();

    currentNote->setName(item->data(column, Qt::EditRole).toString().toStdString());

    

}

void CNotesPluginPanel::onNoteTreeCurrentItemChanged(QTreeWidgetItem* current) {

    if (m_restore)
        return;

    int index = ui->noteTree->indexOfTopLevelItem(current);
    //qDebug() << "current item changed" << index;

    if (index < 0) {

        data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));

        uncheckAll();

        for (size_t i = 0; i < noteData->size(); ++i)
            setRowBackgroundColor(i, Qt::white);

        displayNote(nullptr);
        setDrawingOptions();

        noteData->setAllVisibleOff();

        m_visibilityInvalidated = true;
        PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

        getRenderer()->redraw();

        return;
    }

    
    //so as to prevent of turning off highlighting with wrong row
    turnOffAllHighlighted(m_lastActiveRow);

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));
    data::CNote *currentNote = noteData->getNote(index);

    if (currentNote == nullptr)
        return;

    if (getIndexOfCurrentItem() != -1) {
        m_lastActiveRow = getIndexOfCurrentItem();

        m_itemSelectionChanged = true;
        m_zoom_level = 0;
    }

    uncheckAll();
    noteData->setAllVisibleOff();

    for (size_t i = 0; i < ui->noteTree->topLevelItemCount(); ++i)
        setRowBackgroundColor(i, Qt::white);

    check(current, Qt::Checked);
    setRowBackgroundColor(index, m_customBackgroundColor);

    displayNote(currentNote);

    setDrawingOptions();
    connectDrawingHandler();

   
    if (not ui->showSimilarCheckBox->isChecked())
        currentNote->m_isVisible = true;
    else
        currentNote->m_isVisible = false;

    if (ui->showSimilarCheckBox->isChecked())
        on_showSimilarCheckBox_toggled(true);
    

    m_visibilityInvalidated = true;
    PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

    getRenderer()->redraw();

}

void CNotesPluginPanel::on_noteTree_itemSelectionChanged() {

    //happnes always bewore call to itemClicked
    //qDebug() << "selection changed" << getIndexOfCurrentItem();

    m_itemSelectionChanged = true;

    if (m_lastActiveRow != -1 && m_lastActiveRow < ui->noteTree->topLevelItemCount() && not ui->showSimilarCheckBox->isChecked())
        setRowBackgroundColor(m_lastActiveRow, Qt::white);

    if (getIndexOfCurrentItem() != -1)
        m_lastActiveRow = getIndexOfCurrentItem();

    m_zoom_level = 0;
}

void CNotesPluginPanel::on_noteTree_itemClicked(QTreeWidgetItem *item, int column) {
    
   //qDebug() << "Item clicked";

    int index = ui->noteTree->indexOfTopLevelItem(item);

    //item was clicked again
    if (not m_itemSelectionChanged) {

        uncheckAll();

        for (size_t i = 0; i < ui->noteTree->topLevelItemCount(); ++i)
            setRowBackgroundColor(i, Qt::white);

        ui->noteTree->setCurrentItem(nullptr);
        setDrawingOptions();
        return;
    }

    m_itemSelectionChanged = false;

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));
    data::CNote *currentNote = noteData->getNote(index);

    if (currentNote == nullptr)
        return;

    setRowBackgroundColor(index, m_customBackgroundColor);

    displayNote(currentNote);
    setDrawingOptions();
    connectDrawingHandler();

    if (ui->showSimilarCheckBox->isChecked())
        on_showSimilarCheckBox_toggled(true);

    currentNote->m_isVisible = true;
    
    m_visibilityInvalidated = true;
    PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

}

void CNotesPluginPanel::on_notesActivePushButton_toggled(bool checked) {
    
    if (checked) {
        setDrawingOptions();
        connectDrawingHandler();

        ui->noteTree->setCurrentItem(ui->noteTree->topLevelItem(m_lastActiveRow));

        ui->notesActivePushButton->setText(tr("Notes active"));

    } else {
        PLUGIN_APP_MODE.disconnectAllDrawingHandlers();
        ui->noteTree->setCurrentItem(nullptr);

        ui->notesActivePushButton->setText(tr("Notes inactive"));

    }

}

void CNotesPluginPanel::on_showSimilarCheckBox_toggled(bool checked) {
   
    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));


    //check all with similar matrix
    if (checked) {

        data::CNote *currentNote = noteData->getNote(m_lastActiveRow);

        if (currentNote == nullptr || getIndexOfCurrentItem() < 0)
            return;

        for (size_t i = 0; i < noteData->size(); ++i) {

            if (isTheMatrixSimilar(noteData->getNote(i)->m_sceneTransform, currentNote->m_sceneTransform)) {

                check(ui->noteTree->topLevelItem(i), Qt::Checked);
                noteData->getNote(i)->m_isVisible = true;

                setRowBackgroundColor(i, m_customBackgroundColor);

            } else {
                check(ui->noteTree->topLevelItem(i), Qt::Unchecked);
                noteData->getNote(i)->m_isVisible = false;

                setRowBackgroundColor(i, Qt::white);

            }
        }

        //uncheck all except last active item
    } else {

        for (int i = 0; i < (int)noteData->size(); ++i) {

            check(ui->noteTree->topLevelItem(i), Qt::Unchecked);
            noteData->getNote(i)->m_isVisible = false;

            setRowBackgroundColor(i, Qt::white);

        }

        if (m_lastActiveRow != -1 && noteData->size() != 0) {
            check(ui->noteTree->topLevelItem(m_lastActiveRow), Qt::Checked);
            noteData->getNote(m_lastActiveRow)->m_isVisible = true;
            setRowBackgroundColor(m_lastActiveRow, m_customBackgroundColor);
        }
    }

    m_visibilityInvalidated = true;
    PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

    getRenderer()->redraw();

}

void CNotesPluginPanel::setRowBackgroundColor(int index, QColor color) {

    for (int j = 0; j < ui->noteTree->columnCount(); ++j) {
        ui->noteTree->topLevelItem(index)->setBackgroundColor(j, color);
    }
}

void CNotesPluginPanel::on_noteTree_clicked() {
    
   //qDebug() << "Tree clicked";


}




void CNotesPluginPanel::removeReferencesFromMaps(int index) {
    
    auto item = ui->noteTree->topLevelItem(index);

    //get elements of item
    auto elements = treeItemToElementsMap.values(item);

    //remove backward references
    for (size_t j = 0; j < elements.size(); j++) {
        treeElementToItemMap.remove(elements.at(j));
    }

    //remove forward reference
    treeItemToElementsMap.remove(item);
    

}


int CNotesPluginPanel::getIndexOfCurrentItem() const {

    return ui->noteTree->indexOfTopLevelItem(ui->noteTree->currentItem());
}

int CNotesPluginPanel::getNumberOfChecked() const {
    
    int count = 0;

    for (size_t i = 0; i < ui->noteTree->topLevelItemCount(); i++) {
        auto elements = treeItemToElementsMap.values(ui->noteTree->topLevelItem(i));

        // color, visibility, discard
        if (((QCheckBox *)elements.at(1))->checkState() == Qt::Checked)
            count++;
    }

    return count;
}

void CNotesPluginPanel::uncheckAll() {

    for (size_t i = 0; i < ui->noteTree->topLevelItemCount(); i++)
        check(ui->noteTree->topLevelItem(i), Qt::Unchecked);

}

void CNotesPluginPanel::check(QTreeWidgetItem *item, Qt::CheckState state) {

    auto elements = treeItemToElementsMap.values(item);
    ((QCheckBox *)elements.at(1))->setCheckState(state);

}

void CNotesPluginPanel::forceItemChanged(int row) const {
    
    if (row >= 0) {
        ui->noteTree->setCurrentItem(nullptr);

        ui->noteTree->setCurrentItem(ui->noteTree->topLevelItem(row));
    }
}

void CNotesPluginPanel::turnOffAllHighlighted(int index) {
    

    if (index == -1)
        index = m_lastActiveRow;

    if (m_highlightedLineIds.size() == 0)
        return;

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));

    for (size_t i = 0; i < m_highlightedLineIds.size(); ++i) {
        data::lineChange change(data::lineChange::action_enum::highlight, index, m_highlightedLineIds[i], false);
        noteData->addToLineChangeQueue(change);
    }

    m_action = data::noteChange(0, data::noteChange::change_enum::line);

    m_highlightedLineIds.clear();

    PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());
}

void CNotesPluginPanel::showContextMenu(const QPoint &pos) {






    QPoint global_position = ui->noteTree->mapToGlobal(pos);

    QMenu submenu;
    QAction *createAction = submenu.addAction(tr("Create new note"));

    QAction *duplicateAction = nullptr;
    QAction *deleteAction = nullptr;

    if (ui->noteTree->selectedItems().size() != 0){
        duplicateAction = submenu.addAction(tr("Duplicate this note"));
        deleteAction = submenu.addAction(tr("Delete this note"));
    }

	QAction* selectedAction = submenu.exec(global_position);


    int index = getIndexOfCurrentItem();

	if (selectedAction == createAction) {
       
	    on_addNewNoteButton_clicked();

    } else if (selectedAction == deleteAction && index != -1) {
		
        on_discardNoteButton_clicked();

    } else if (selectedAction == duplicateAction && index != -1) {

		snap();

        insertDuplicateNote();
		
	}

}

/*
void CNotesPluginPanel::focusInEvent(QFocusEvent* event) {
	
	//qDebug() << "Focus event IN";

}


void CNotesPluginPanel::focusOutEvent(QFocusEvent* event) {
	
	//qDebug() << "Focus event OUT";


}

void CNotesPluginPanel::enterEvent(QEvent* event) {
	
	//qDebug() << "Enter event IN";


}
void CNotesPluginPanel::leaveEvent(QEvent* event) {
	
	//qDebug() << "Leave event OUT";

}
*/
void CNotesPluginPanel::hideEvent(QHideEvent* event) {
	
	//qDebug() << "Hide event";

    //Panel was closed so stop drawing 


    //save what was visible beforehand?
    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));
    
    noteData->setAllVisibleOff();
    
    m_visibilityInvalidated = true;
    PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

    ui->notesActivePushButton->setChecked(false);


    getRenderer()->redraw();
}

void CNotesPluginPanel::showEvent(QShowEvent* event) {

    //qDebug() << "Show event";
    
   
    //Panel was opened again 
    int index = getIndexOfCurrentItem();

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));

    if (index != -1)
        noteData->getNote(index)->m_isVisible = true;


    if (ui->showSimilarCheckBox->isChecked())
        on_showSimilarCheckBox_toggled(true);

    m_visibilityInvalidated = true;
    PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

    ui->notesActivePushButton->setChecked(true);

    getRenderer()->redraw();

}

bool CNotesPluginPanel::eventFilter(QObject *object, QEvent *event) {
   
    if (object == ui->noteText) {

		if (event->type() == QKeyEvent::KeyRelease) {

            //reset save timer
            m_keystrokeSaveTimer.start(keystrokeSaveTimerInterval);
            m_keystrokeSaveTimer.setSingleShot(true);

			ui->noteText->document()->setModified(true);
        } else 

        //it could be possible to change curent item before the timer runs out..
        //probably should not rely on that it won't happen
        if (event->type() == QEvent::FocusOut && ui->noteText->document()->isModified()) {
            saveText();	
        }


    //it can only be volume renderer window
    } else {
    
		//mouse move
        if (event->type() == QMouseEvent::MouseMove) {
            QMouseEvent *ev = (QMouseEvent *)event;
            
            highlight_lines(ev->x(), ev->y());

            //if dragging view,	check for big enough difference to stop drawing note
            if (((QApplication::mouseButtons() & Qt::LeftButton) == Qt::LeftButton) ||
                ((QApplication::mouseButtons() & Qt::RightButton) == Qt::RightButton))
                    checkMatrices();

        } else if (event->type() == QMouseEvent::Wheel) {    
            
            QWheelEvent *ev = (QWheelEvent *)event;
            if(ev->delta() > 0)
                m_zoom_level++;
            else
                m_zoom_level--;


            if (colorGenerator.repeatQColor() != m_nextUpColor)
                m_nextUpColor = colorGenerator.getQColor();

            m_currentColor = m_nextUpColor;

            setDrawingOptions();
        }
    
    }
        

    return QObject::eventFilter(object, event);
}
    

void CNotesPluginPanel::saveText() {

    //in case that this was called in reaction to focusOut and not keyRelease
    m_keystrokeSaveTimer.stop();


    if (not ui->noteText->document()->isModified()) // or if already saved...
        return;

    int position = ui->noteText->textCursor().position();
    int index = getIndexOfCurrentItem();

    QTextCursor cursor = ui->noteText->textCursor();
    cursor.setPosition(position);

	ui->noteText->document()->setModified(false);
    

	snap();
    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));


    //enables creation of new note by attempting to save with no item selected
    if (ui->noteTree->selectedItems().isEmpty()) {

        //new note is immediately displayed in noteText so backup the typed string
        std::string tmp = ui->noteText->toPlainText().toStdString();

        data::CNote* note = addNewNote();
        index = getIndexOfCurrentItem();

        noteData->setText(index, tmp);

        ui->noteTree->currentItem()->setData(treeColumns::name, Qt::EditRole, QString::fromStdString(note->m_name));

        m_action = data::noteChange(index, data::noteChange::change_enum::edit);

        //current item changed by creation and now it needs to be called again to display change in text
        onNoteTreeCurrentItemChanged(ui->noteTree->currentItem());

    } else {
        
        //potentialy changes note name if it has no separateName
        noteData->setText(index, ui->noteText->toPlainText().toStdString());

        auto note = noteData->getNote(index);

        //refresh note name display
        ui->noteTree->currentItem()->setData(treeColumns::name, Qt::EditRole, QString::fromStdString(note->m_name));

        m_action = data::noteChange(index, data::noteChange::change_enum::edit);

    }

    ui->noteText->setTextCursor(cursor);

}


bool CNotesPluginPanel::isTheMatrixSimilar(osg::Matrix &note_matrix, osg::Matrix &other) const {
	
	using namespace osg;


	//vector representation of rotation quaternions
	Vec4d q1, q2;

    Vec3 current_translation, other_translation;

	{
        //dont care about 'some', but I do care aout current and other rotation
        Vec3 some_scale;
        Quat current_rotation, other_rotation, some_scale_orientation;

        note_matrix.decompose(other_translation, other_rotation, some_scale, some_scale_orientation);
        other.decompose(current_translation, current_rotation, some_scale, some_scale_orientation);
         
		q1 = current_rotation.asVec4();
		q2 = other_rotation.asVec4();

	}

    
    //if the translation difference is close to zero, camera hasn't moved
    double translation_result = (current_translation - other_translation).length2();

    if (translation_result > translationSimilarityThreshold)
        return false;


    //if the quaternions are similar (close to 1) camera hasn't even rotated so we can say that View hasn't shifted at all
	double rotation_result = std::inner_product(q1.ptr(), q1.ptr() + 4, q2.ptr(), 0.0);
	
    if (abs(rotation_result) > rotationSimilarityThreshold)
		return true;
	 else 
		return false;

}

void CNotesPluginPanel::checkMatrices() {
    
    int row = getIndexOfCurrentItem();
    

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));

    if (row < 0 || row >= noteData->size())
        return;

    data::CNote *note = noteData->getNote(row);

    osg::Matrix current_matrix = getRenderer()->getViewMatrix();

    
        if (not isTheMatrixSimilar(note->m_sceneTransform, current_matrix)) {

            displayNote(nullptr);

            uncheckAll();

            for (size_t i = 0; i < noteData->size(); ++i)
                setRowBackgroundColor(i, Qt::white);

            ui->noteTree->clearSelection();
            ui->noteTree->setCurrentItem(nullptr);


            setDrawingOptions();

            noteData->setAllVisibleOff();

            m_visibilityInvalidated = true;
            PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

        }

}

void CNotesPluginPanel::highlight_lines(int X, int Y) {
	

    using namespace osg;

    int index = getIndexOfCurrentItem();
    



	//We want to highlight lines only when deleting.. and shift must be pressed
    if (not ui->deleteLinePushButton->isChecked() || (ui->deleteLinePushButton->isChecked() && (QApplication::keyboardModifiers() & Qt::ShiftModifier) != Qt::ShiftModifier)) {


        if (m_highlightedLineIds.size() != 0) {
            //insurance against lines remaining highlighted
            turnOffAllHighlighted();
        }
        return;

    }

	data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));

    data::CNote *note = noteData->getNote(index);


	//at startup
	if (note == nullptr)
		return;

	std::vector<unsigned int> highlightOff;
	std::vector<unsigned int> currentHighlightedLines;

	//mouse coordinates come with Y zero at the top!
	QSize size = getRenderer()->getWindowSize();
	Vec3f mouse_coordinates = Vec3f(X, size.height() - Y, 0.0f);


    osg::Matrix view = getRenderer()->getViewMatrix();
    osg::Matrix projection = getRenderer()->getProjectionMatrix();


    osg::Matrix point_to_screen = view * projection;
    

	//collect line ids for highlighting
	for (size_t i = 0; i < note->m_lines.size(); ++i) {

		
		//Perform hit detection over the saved data
        if (note->m_lines[i].isOver(mouse_coordinates, point_to_screen, size.width(), size.height()))
			currentHighlightedLines.push_back(note->m_lines[i].m_id);

	}


	//get changes from the previous state
	std::set_difference(m_highlightedLineIds.begin(), m_highlightedLineIds.end(),
		currentHighlightedLines.begin(), currentHighlightedLines.end(),
		std::inserter(highlightOff, highlightOff.begin()));



	//notify tree of lines off
	for (size_t i = 0; i < highlightOff.size(); ++i) {
        data::lineChange change(data::lineChange::action_enum::highlight, index, highlightOff[i], false);
        noteData->addToLineChangeQueue(change);
	}

	//notify tree of lines on
	for (size_t i = 0; i < currentHighlightedLines.size(); ++i) {
        data::lineChange change(data::lineChange::action_enum::highlight, index, currentHighlightedLines[i], true);
        noteData->addToLineChangeQueue(change);
	}


	//keep the list for use later again
	m_highlightedLineIds = currentHighlightedLines;

    if (highlightOff.size() != 0 || currentHighlightedLines.size() != 0) {
        m_action = data::noteChange(0, data::noteChange::change_enum::line);

        PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());
    }

}



// triggered by calling invalidate
void CNotesPluginPanel::on_notesData_itemChanged(data::CStorageEntry *whoknowswhat) {

   // qDebug() << "NotePLuginPanel note data changed";

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));


    if (m_visibilityInvalidated) {
        m_visibilityInvalidated = false;
        return;
    }



    if (noteData->m_lastAction.type == data::noteChange::change_enum::init && noteData->m_lastAction != m_action) {

        fillNoteList();

        m_restore = false;

        if (ui->noteTree->topLevelItemCount() > 0) {


            //want to keep previously selected position if possible
            if (m_lastActiveRow < ui->noteTree->topLevelItemCount() + 1 && m_lastActiveRow > 0)
                forceItemChanged(std::min(m_lastActiveRow, ui->noteTree->topLevelItemCount() - 1));
            else
                forceItemChanged(0);

            ui->noteText->document()->setModified(false);


        } else if (ui->noteTree->topLevelItemCount() == 0) {
            ui->noteTree->setCurrentItem(nullptr);


            ui->noteText->setText(tr(CNotesPluginPanel::default_textedit_text.toStdString().c_str()));
        }


        if (not isHidden())
            setDrawingOptions();


        m_action = noteData->m_lastAction;
        PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

        return;
    }


    

    //stuff below is probably never used and would not work properly..

    //purpose of this is to ensure consistency even when changes dont originate from this ui - undo
	//Only takes target of last action from noteData and mirrors it in ui.. No data changes..
	if (noteData->m_lastAction != m_action) {
    
        if (noteData->m_lastAction.type == data::noteChange::change_enum::insert) {

            auto position = noteData->m_lastAction.position;

            insertNoteToTree(position, noteData->getNote(position));

        } else if (noteData->m_lastAction.type == data::noteChange::change_enum::edit) {
			
			int position = noteData->m_lastAction.position;

            discardNoteFromTree(getIndexOfCurrentItem());

            insertNoteToTree(position, noteData->getNote(position));
			displayNote(noteData->getNote(position));



        } else if (noteData->m_lastAction.type == data::noteChange::change_enum::discard) {
            
            auto position = noteData->m_lastAction.position;

            discardNoteFromTree(position);

        }

		m_action = noteData->m_lastAction;
    }


	//m_pRenderer->redraw();
}


void CNotesPluginPanel::fillNoteList(){

    qDebug() << "noteTree filling started";

    //ensure the slate is clean.. notes could be reinitialized and this was called for the second time
    if (ui->noteTree->topLevelItemCount() > 0) 
        ui->noteTree->clear();

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));


	//to prevent repeated signals on current row changed 
    ui->noteTree->blockSignals(true);

		//insert notes from noteData to noteList or display
		for (size_t i = 0; i < noteData->size(); ++i)
            addNoteToTree(noteData->getNote(i));
    
    ui->noteTree->blockSignals(false);

    m_action = data::noteChange(noteData->size(), data::noteChange::change_enum::init);

    noteData->m_noteChanges.push_back(m_action);


    //derive default note naming index from note names.. compare translated prefixes first
    //and then check if the rest is a number
    std::string translatedString = CNotesPluginPanel::tr(CNotesPluginPanel::default_note_name.toStdString().c_str()).toStdString();
    
    
    
    note_name_index = 0;
    colorGenerator.reset();

    for (size_t i = 0; i < noteData->size(); ++i) {

        auto note = noteData->getNote(i);
        int lowest_comparison_index = std::min(note->m_name.size(), translatedString.size());

        if (note->m_name.compare(0, lowest_comparison_index, translatedString) == 0) {  //have the same prefix as default name


            std::string number_part = note->m_name.substr(lowest_comparison_index, note->m_name.size() - 1);

            bool isNumber = true;

            for (size_t j = 0; j < number_part.size(); ++j)
                isNumber &= (isdigit(number_part[j]) > 0);

            if (isNumber) {
                note_name_index = std::max(note_name_index, std::atoi(number_part.c_str()) + 1);  // number_part
                colorGenerator.getQColor();     //try to avoid repeating colors..
            }

        }
    }

    m_currentColor = colorGenerator.getQColor();
    m_nextUpColor = colorGenerator.repeatQColor();

    qDebug() << "noteTree filled";

}


void CNotesPluginPanel::addNoteToTree(data::CNote* note) {
    

    insertNoteToTree(ui->noteTree->topLevelItemCount(), note);

}

void CNotesPluginPanel::discardNoteFromTree(int row) {
    

    int next_index = row - 1;

    if (next_index == -1 && ui->noteTree->topLevelItemCount() > 1)
        next_index = 0;



    //forcing currentItemChanged
    ui->noteTree->setCurrentItem(nullptr);

    removeReferencesFromMaps(row);
    delete ui->noteTree->takeTopLevelItem(row);

    auto next_current_item = ui->noteTree->topLevelItem(next_index);

    ui->noteTree->setCurrentItem(next_current_item);


    if (ui->noteTree->topLevelItemCount() == 0) {

        ui->noteText->setText(CNotesPluginPanel::tr(CNotesPluginPanel::default_textedit_text.toStdString().c_str()));

        ui->drawPushButton->setChecked(true);
    }


}

void CNotesPluginPanel::on_addNewNoteButton_clicked(){
    
	snap();

	addNewNote();

}

data::CNote *CNotesPluginPanel::addNewNote() {

	data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));

    int index = ui->noteTree->topLevelItemCount();

    PSVR::PSVolumeRendering *renderer = dynamic_cast<PSVR::PSVolumeRendering *>(getRenderer());

    if (renderer != nullptr)
        m_cameraManipulator = dynamic_cast<osg::CSceneManipulator *>(renderer->getCanvas()->getView()->getCameraManipulator());


    if (m_cameraManipulator == nullptr)
        return nullptr;

    //prevents adding of note with no name if new note was added while editing another's name
    m_double_click_name_backup.clear();

    m_currentColor = m_nextUpColor;

    data::CNote *note = noteData->insertNote(index, data::CNote(CNotesPluginPanel::tr(CNotesPluginPanel::default_note_text.toStdString().c_str()).toStdString(), getRenderer()->getViewMatrix(), m_cameraManipulator->getDistance(), colorGenerator.QColorToCColor(m_currentColor)));
    
    std::stringstream ss;
    ss << note_name_index++;
    
    note->setName(CNotesPluginPanel::tr(CNotesPluginPanel::default_note_name.toStdString().c_str()).toStdString() + ss.str());

	m_action = data::noteChange(index, data::noteChange::change_enum::insert);
    noteData->m_noteChanges.push_back(m_action);
	//makes Note subtree add this new note..
	PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

    addNoteToTree(note);

    ui->noteTree->setCurrentItem(ui->noteTree->topLevelItem(index));



    //the fact that the program is here means that nextUp color was used..
    m_nextUpColor = colorGenerator.getQColor();


    setDrawingOptions();

    return note;
}

data::CNote* CNotesPluginPanel::insertDuplicateNote() {
	
	data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));

    int index = ui->noteTree->topLevelItemCount();

    data::CNote *note = noteData->insertNote(index, noteData->getNoteCopy(getIndexOfCurrentItem()));

    m_action = data::noteChange(index, data::noteChange::change_enum::insert);
    noteData->m_noteChanges.push_back(m_action);

    insertNoteToTree(index, note);

    ui->noteTree->setCurrentItem(ui->noteTree->topLevelItem(index));

	//makes Note subtree add this new note..
	PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

	return note;
}

void CNotesPluginPanel::on_discardNoteButton_clicked(){

	snap();

	data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));

	int row = getIndexOfCurrentItem();

	m_action = data::noteChange(row, data::noteChange::change_enum::discard);
    noteData->m_noteChanges.push_back(m_action);


	noteData->discardNote(row);

	PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

	//current row changes after this and triggers change of display in NoteSubtree so the signal from
	//invalidate must get there first..
    //So change data first and then display and everything will be ok.
    discardNoteFromTree(row);

}


void CNotesPluginPanel::on_colorSelectorButton_clicked()
{
    QColor newColor;
    newColor = QColorDialog::getColor(m_currentColor);

    if(newColor.isValid())
        setDefaultColorButtonColor(newColor);
}


void CNotesPluginPanel::on_strokeWidthSpinner_valueChanged(int value){

    //For handler.. the lines save this value after they are drawn
    setDrawingOptions();

}


////RADIO BUTTONS
void CNotesPluginPanel::on_drawPushButton_toggled(){

    if (not ui->notesActivePushButton->isChecked()) {
        ui->notesActivePushButton->setChecked(true);
        ui->notesActivePushButton->setText(tr("Notes active"));
    }

    if (ui->drawPushButton->isChecked()) {
        m_drawingMode = data::CDrawingOptions::DRAW_STROKE;
        m_transparent = false;
        setDrawingOptions();
    }
}

void CNotesPluginPanel::on_drawArrowPushButton_toggled() {

    if (not ui->notesActivePushButton->isChecked()) {
        ui->notesActivePushButton->setChecked(true);
        ui->notesActivePushButton->setText(tr("Notes active"));
    }

    if (ui->drawArrowPushButton->isChecked()) {
        m_drawingMode = data::CDrawingOptions::DRAW_ARROW;
        m_transparent = false;
        setDrawingOptions();
    }
}

void CNotesPluginPanel::on_deleteLinePushButton_toggled() {

    if (not ui->notesActivePushButton->isChecked()) {
        ui->notesActivePushButton->setChecked(true);
        ui->notesActivePushButton->setText(tr("Notes active"));
    }


    if (ui->deleteLinePushButton->isChecked()) {
        m_drawingMode = data::CDrawingOptions::DRAW_STROKE;
        m_transparent = true;
        setDrawingOptions();
    }
}



void CNotesPluginPanel::setButtonColor(QPushButton *button, QColor color) {
    

    QString qss = QString(
        "QPushButton {"
        "background-color: %1;"
        "border-style: outset;"
        "border-width: 1px;"
        "border-radius: 2px;"
        "border-color: black;}").arg(color.name());

    button->setStyleSheet(qss);
    button->update();

}



/////////rename or rewrite or both..
void CNotesPluginPanel::setDefaultColorButtonColor(QColor newColor) {

	m_currentColor = newColor;
	setDrawingOptions();

	QString qss = QString(
        "QPushButton {"
        "background-color: %1;"
        "border-style: outset;"
        "border-width: 1px;"
        "border-radius: 2px;"
        "border-color: black;}").arg(newColor.name());

	ui->colorSelectorButton->setStyleSheet(qss);
	ui->colorSelectorButton->update();


}

void CNotesPluginPanel::displayNote(data::CNote *note) {  

    if (note == nullptr) {
        ui->noteText->setText(CNotesPluginPanel::tr(CNotesPluginPanel::default_textedit_text.toStdString().c_str()));
   
        m_currentColor = m_nextUpColor;

    } else {
        ui->noteText->setText(QString::fromStdString(note->getText()));

        m_currentColor = colorGenerator.CColorToQColor(note->color);

        VPL_SIGNAL(SigNewTransformMatrixFromNote).invoke(note->m_sceneTransform, note->m_distance);

    }
}



void CNotesPluginPanel::connectDrawingHandler() {
    
	if (not m_hasFocus) {
		m_conDrawingDone = PLUGIN_APP_MODE.connectDrawingHandler(this, &CNotesPluginPanel::handleDrawing);
        qDebug() << "connection estabilished";
	}
}


data::CSnapshot *CNotesPluginPanel::getSnapshot(data::CSnapshot *snapshot) {

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));
	
	return new data::CNoteSnapshot(noteData->m_notes, this);
}


void CNotesPluginPanel::restore(data::CSnapshot *snapshot) {

	if (not snapshot->isType(data::UNDO_NOTES))
        return;

    m_restore = true;

	data::CNoteSnapshot *note_snapshot = (data::CNoteSnapshot *)snapshot;

	data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));
	
		noteData->overwrite(note_snapshot->m_notes);
        noteData->m_noteChanges.push_back(data::noteChange(note_snapshot->m_notes.size(), data::noteChange::change_enum::init));

	PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

    //force renderer to react immediately
    getRenderer()->redraw();
}



void CNotesPluginPanel::handleDrawing(const osg::Vec3Array *points, const int handlerType, const int mouseButton) {
	
    using namespace osg;


    //Drawing handler was disconnected
    if (handlerType == data::CDrawingOptions::FOCUS_LOST) {

        // not drawing
        m_drawingMode = data::CDrawingOptions::DRAW_NOTHING;
        
        //reflect the change in ui
        ui->notesActivePushButton->blockSignals(true);
            ui->notesActivePushButton->setChecked(false);
            ui->notesActivePushButton->setText(tr("Notes inactive"));
        ui->notesActivePushButton->blockSignals(false);

        m_hasFocus = false;

        return;
    } 
	
    //Drawing handler was connected
	if (handlerType == data::CDrawingOptions::FOCUS_ON) {

		m_drawingMode = data::CDrawingOptions::DRAW_STROKE;
        
        //reflect the change in ui
	    ui->notesActivePushButton->blockSignals(true);
            ui->notesActivePushButton->setChecked(true);
            ui->notesActivePushButton->setText(tr("Notes active"));
        ui->notesActivePushButton->blockSignals(false);

        m_hasFocus = true;

        setDrawingOptions();

        return;
    }



    //Don't react without focus
    if (not m_hasFocus || points == nullptr || (m_drawingMode != data::CDrawingOptions::DRAW_STROKE && m_drawingMode != data::CDrawingOptions::DRAW_ARROW) || handlerType != data::CDrawingOptions::HANDLER_WINDOW)
        return;
    

    bool createdNew = false;
    
    int index = getIndexOfCurrentItem();

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));
    auto currentNote = noteData->getNote(index);

    snap();

    if (ui->drawPushButton->isChecked() || ui->drawArrowPushButton->isChecked()) {

        osg::Matrix current_matrix = getRenderer()->getViewMatrix();

        if (ui->noteTree->topLevelItemCount() == 0 || currentNote == nullptr || not isTheMatrixSimilar(currentNote->m_sceneTransform, current_matrix)) {
			currentNote = addNewNote();

            index = ui->noteTree->topLevelItemCount() - 1;
            
            ui->noteTree->setCurrentItem(ui->noteTree->topLevelItem(index));

            createdNew = true;
		}

        QSize size = getRenderer()->getWindowSize();

        osg::Matrix view = getRenderer()->getViewMatrix();
        osg::Matrix proj = getRenderer()->getProjectionMatrix();


         
        auto view2 = osg::Matrix::inverse(view);
        auto proj2 = osg::Matrix::inverse(proj);

     
		currentNote->m_lines.push_back(data::noteLine());

		data::noteLine *newLine = &currentNote->m_lines.back();  

        for (size_t i = 0; i < points->size(); ++i) {
            Vec3 point(points->at(i).x(), points->at(i).y(), 0.0f);


            //normalize to (-1, 1)
            float x = (point.x() - size.width() / 2.0f) / (size.width() / 2.0f);
            float y = (point.y() - size.height() / 2.0f) / (size.height() / 2.0f);
            point.set(x, y, 0.0f);

            //transform to world space
            point = (point * proj2) * view2;

            newLine->addPoint(point);

        }

        if (not createdNew) {
            //note already contains the line so it is added together with the note in noteChangeQueue
            data::lineChange change(data::lineChange::action_enum::add, index, &currentNote->m_lines.back());
            noteData->addToLineChangeQueue(change);

        }

    } else if (ui->deleteLinePushButton->isChecked()) {
    
        /*
        for arbitrary delete..
        mouse event highlighting over everything -> noteId + lineIds
        + deleting from that
        */

		
        data::lineChange change(data::lineChange::action_enum::discard, index, m_highlightedLineIds);
        noteData->addToLineChangeQueue(change);
              
        m_action = data::noteChange(0, data::noteChange::change_enum::line);

        currentNote->removeLines(m_highlightedLineIds);
        
        m_highlightedLineIds.clear();

    } 
         
    PLUGIN_APP_STORAGE.invalidate(noteData.getEntryPtr());

}


void CNotesPluginPanel::setDrawingOptions() {

    // Get drawing options from data storage
    data::CObjectPtr<data::CDrawingOptions> spOptions(PLUGIN_APP_STORAGE.getEntry(data::Storage::DrawingOptions::Id));

    float alpha = 1.0f;

    if (m_transparent)
        alpha = 0.0f;
            
    
    // Setup the drawing mode
    spOptions->setDrawingMode(m_drawingMode);

    data::CObjectPtr<data::CNoteData> noteData(PLUGIN_APP_STORAGE.getEntry(data::Storage::NoteData::Id));
    auto currentNote = noteData->getNote(getIndexOfCurrentItem());

    if (ui->noteTree->selectedItems().empty() || (abs(m_zoom_level) > 0)) 
        spOptions->setColor(osg::Vec4(m_nextUpColor.redF(), m_nextUpColor.greenF(), m_nextUpColor.blueF(), alpha));
     else 
        spOptions->setColor(osg::Vec4(currentNote->color.getR(), currentNote->color.getG(), currentNote->color.getB(), alpha));
    
    
    spOptions->setWidth(data::noteLine::default_line_width);		

    // Invalidate the modified object
    PLUGIN_APP_STORAGE.invalidate(spOptions.getEntryPtr());

}
