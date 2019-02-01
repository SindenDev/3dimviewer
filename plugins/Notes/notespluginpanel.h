///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//

#ifndef CNOTESPLUGINPANEL_H
#define CNOTESPLUGINPANEL_H


#include <ciso646>

#include <qtplugin/PluginInterface.h>

#include <Signals.h>
#include <coremedi/app/Signals.h>

#include <data/CSnapshot.h>
#include "data/CRegionData.h"
#include "data/COrthoSlice.h"

#include <data/Notes.h>

#include <QDebug>
#include <QMap>
#include <QMultiMap>
#include <QDesktopWidget>
#include <QWidget>
#include <Qtimer>
#include <QTreeWidget>
#include <Qcheckbox>

namespace osg {
    class CSceneManipulator;
}
////////file notesUtil or something..
inline void printMatrix(osg::Matrix matrix) {

    QDebug debug = qDebug();

    for (size_t row = 0; row < 4; row++) {
        for (size_t column = 0; column < 4; column++) {
            debug << matrix(row, column);
        }
        debug << "\n";
    }

}

//! Mostly for notes. At the beggining it goes through predefined array of soft colors.
//! After that it generates some pseudo random colors with potentially unwanted properties (e.g. dark blue)
namespace data {
    class CColorGenerator {

    public:
        CColorGenerator() {
            index = 0;
            predefinedColors.push_back(data::CColor4f(141 / 255.0f, 211 / 255.0f, 199 / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(255 / 255.0f, 255 / 255.0f, 179 / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(190 / 255.0f, 186 / 255.0f, 218 / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(251 / 255.0f, 128 / 255.0f, 114 / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(128 / 255.0f, 177 / 255.0f, 211 / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(253 / 255.0f, 180 / 255.0f, 98  / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(179 / 255.0f, 222 / 255.0f, 105 / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(252 / 255.0f, 205 / 255.0f, 229 / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(217 / 255.0f, 217 / 255.0f, 217 / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(188 / 255.0f, 128 / 255.0f, 189 / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(204 / 255.0f, 235 / 255.0f, 197 / 255.0f, 1.0f));
            predefinedColors.push_back(data::CColor4f(255 / 255.0f, 237 / 255.0f, 111 / 255.0f, 1.0f));
            
            lastColor = data::CColor4f(141 / 255.0f, 211 / 255.0f, 199 / 255.0f, 1.0f);

        }

        data::CColor4f getCColor();

        QColor getQColor();

        QColor CColorToQColor(data::CColor4f color) {

            return QColor(color.getR() * 255, color.getG() * 255, color.getB() * 255, 255);
        }

        data::CColor4f QColorToCColor(QColor color) {

            return data::CColor4f(color.redF(), color.greenF(), color.blueF(), 1.0f);
        }

        QColor repeatQColor() {
            return CColorToQColor(lastColor);
        }

        data::CColor4f repeatCColor() {
            return lastColor;
        }

        void reset() {
            index = 0;
            lastColor = data::CColor4f(141 / 255.0f, 211 / 255.0f, 199 / 255.0f, 1.0f);
        }

    protected:
        int index;

        data::CColor4f lastColor;

        std::vector<data::CColor4f> predefinedColors;

    };

}


class CNotesPlugin;

namespace Ui {
    class CNotesPluginPanel;
}

class CNotesPluginPanel : public QWidget, public CAppBindings, public data::CUndoProvider {
    Q_OBJECT

public:
    explicit CNotesPluginPanel(CAppBindings* pBindings, QWidget *parent = nullptr);
    ~CNotesPluginPanel();

    //! Calls loadNotes (at most once) of the CNoteData class in data store and puts their representations into noteList.
    void fillNoteList();

    //! Defines columns, sizes, headers/icons
	void buildNoteTree() const;

    //! Creates snapshot of the note data store
	data::CSnapshot *getSnapshot(data::CSnapshot *  snapshot)  override;

    //! Uses snapshot info to overwrite the current state
	void restore(data::CSnapshot * snapshot)  override;

    //! Wraps creation of snapshot
	void snap() {
        PLUGIN_VPL_SIGNAL(SigUndoSnapshot).invoke(getSnapshot(nullptr));
    }

    static const QString default_textedit_text;

    static const QString default_note_text;

    static const QString default_note_name;

    static int note_name_index;

    data::CColorGenerator colorGenerator;
protected:


    enum treeColumns {
        name, color, visibility, discard,
            empty
    };

    //! Need a way to associate color button, checkbox and discard button with item
    QMap<QObject*, QTreeWidgetItem *> treeElementToItemMap;

    //! Item to it's elements - color, visibility, discard elements
    QMultiMap<QTreeWidgetItem*, QObject*> treeItemToElementsMap;

    //! Removes references from the two maps above
    void removeReferencesFromMaps(int index);

    osg::CSceneManipulator *m_cameraManipulator;
    
    //! Using store invalidate mechanism for updating visibility of notes in NoteSubtree 
    //! must have a way to filter out the calls on_notesData_itemChanged on this side.
    bool m_visibilityInvalidated;


	//! Currently selected color in defaultColorButton
    QColor m_currentColor;

    QColor m_nextUpColor;

    QColor m_customBackgroundColor;

    //! Name edit is possible only after double clicking the field. So to recognize name change there is backup made
    //! and used to identify name change by user and allow setting of separateName in Note.
    QString m_double_click_name_backup;

    //! Returns index of current item in noteTree
    int getIndexOfCurrentItem() const;

    //! Return number of notes which have visibility checked
    int getNumberOfChecked() const;

    //! Iterates through all columns of a row at given index and sets their color.
    void setRowBackgroundColor(int index, QColor color);

    //! Goes through all items in noteTree and unchecks them
    void uncheckAll();

    void check(QTreeWidgetItem *item, Qt::CheckState state);

    //! Sets current item to nothing and then to the requested row
    void forceItemChanged(int row) const;

    //! Sets all currently highlighted lines as not highlighted
    void turnOffAllHighlighted(int index = -1);

    //! Adds new empty note to the end of the noteTree and to the dataStore + snapshot
	data::CNote *addNewNote();

    //! Duplicates current item and inserts it to the end of the noteTree and dataStore
	data::CNote *insertDuplicateNote();

    //! Calls insertNoteToTree with last row as parameter.
    void addNoteToTree(data::CNote *note);

    //! Inserts note into noteTree at the specified row and creates icon for it from its color.
    void insertNoteToTree(int row, data::CNote* note);

    //! Sets text of note to noteText
    void displayNote(data::CNote *note);

    //! This function keeps button color and currentColor the same.
    void setDefaultColorButtonColor(QColor newColor);

    //! Updates button with stylesheet with updated color
    static void setButtonColor(QPushButton* button, QColor color);
    
    //! Sets parameters of existing button, stylesheet and places it inside widget inside centered layout
    void spitOutNewColorButton(QPushButton *button, QWidget *widget_with_button) const;

    //! Sets parameters of existing button, icon and places it inside widget inside centered layout
    void spitOutNewDiscardButton(QPushButton* button, QWidget* widget_with_button) const;

    //! Inserts into centered layout
    void spitOutNewVisibilityCheckBox(QCheckBox* button, QWidget* widget_with_checkbox) const;

    //! Removes note from given row.
    void discardNoteFromTree(int row);


    //! Sets function handle drawing as a callback for receiving drawn points.
	//! Only one function can be connected at a time so hasFocus keeps track of the current state.
	//! Upon invoking the connect signal the new/curent drawing handler is notified with FOCUS_ON/LOST handler type
	void connectDrawingHandler();

    //! Does few checks and saves points as line of current note. + Receives notification of lost/gained focus.
    void handleDrawing(const osg::Vec3Array * points, const int handlerType, const int mouseButton);

    //! Sets drawing mode, color and stroke width into internal drawer
    void setDrawingOptions();

	//! Drawing mode for internal drawer. Here  basically only DRAW_STROKE or DRAW_NOTHING
	data::CDrawingOptions::EDrawingMode m_drawingMode;

    //! Flag keeping track whether our drawing handle is still valid (has focus) or not (has not focus). 
    bool m_hasFocus;

    float m_sizeFactor;

    //! Prevent drawing on screen
    bool m_transparent;
    
    //! Flag telling that undo/redo was called
    bool m_restore;

    //! helps determining if click at item changed selection or not..
    bool m_itemSelectionChanged;

    int m_zoom_level;
private slots:

    //! Tied up with the keystrokeSaveTimer which calls this. Syncs the content of
    //! note in store with notetext
    void saveText();

    //! Context menu of treeWidget
	void showContextMenu(const QPoint &pos);

    //treeWidget item elements slots
    void onDiscardButtonClicked();

    void onColorButtonClicked();

    void onCheckBoxClicked();

    //ui elements slots
    void on_noteTree_itemDoubleClicked(QTreeWidgetItem* item, int column);

    void on_noteTree_itemChanged(QTreeWidgetItem* item, int column);

    void onNoteTreeCurrentItemChanged(QTreeWidgetItem *current);
    
    void on_noteTree_itemSelectionChanged();

    void on_noteTree_itemClicked(QTreeWidgetItem* item, int column);

    void on_notesActivePushButton_toggled(bool checked);

    void on_showSimilarCheckBox_toggled(bool checked);

    void on_addNewNoteButton_clicked();

    void on_noteTree_clicked();

    void on_discardNoteButton_clicked();

    void on_strokeWidthSpinner_valueChanged(int value);

    void on_colorSelectorButton_clicked();

    void on_drawPushButton_toggled();

    void on_drawArrowPushButton_toggled();

    void on_deleteLinePushButton_toggled();

private:
    std::unique_ptr<Ui::CNotesPluginPanel> ui;

	//! Timer to automatically save the typed text after change
    QTimer m_keystrokeSaveTimer;

	//! The default interval for the timer
    static const int keystrokeSaveTimerInterval;

    //! defines small value where the rotation is still considered the same
	static const float rotationSimilarityThreshold;

    //! defines small value where the translation is still tolerated
    static const float translationSimilarityThreshold;

    //! gets set at each action to recognize in itemChanged that it is unnecesary to take that action again and to ignore lineChanges
    data::noteChange m_action;

	//! After undo/redo remember this and in fillNoteList try to set this back
	int m_lastActiveRow;

    //! Set of line indices over which is mouse currently located when the deleteRadioButton is checked
    std::vector<unsigned int> m_highlightedLineIds;
	
    /*
	void focusInEvent(QFocusEvent * event) override;

	void focusOutEvent(QFocusEvent * event)override;

	void enterEvent(QEvent * event)override;

	void leaveEvent(QEvent * event)override;
*/
	void hideEvent(QHideEvent * event) override;

	void showEvent(QShowEvent * event) override;


	//! Interceps events of notetext and initializes text saving to store
	bool eventFilter(QObject *object, QEvent *event) override;

    //! Highlights all lines of current note which are close to mouse cursor
	void highlight_lines(int X, int Y);

    //! Decomposes given matrices and checks if the should be considered close/same
    bool isTheMatrixSimilar(osg::Matrix &note_matrix, osg::Matrix &other) const;

    //! Checks the current matrix with matrix of current note and hides note if they are not similar
    void checkMatrices();

	//! Called whenever there is called invalidate on this.
	void on_notesData_itemChanged(data::CStorageEntry *whoknowswhat);




    //! Holds the signal connection to notesData
    vpl::mod::tSignalConnection m_notesDataSignalConnection;
	
	//! Filled in connectDrawingHandler. Holds connection of drawing signal.
	vpl::mod::tSignalConnection m_conDrawingDone;


};

/*
//http://stackoverflow.com/questions/2801959/making-only-one-column-of-a-qtreewidgetitem-editable
class NoEditDelegate : public QStyledItemDelegate {
public:
	NoEditDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
		return nullptr;
	}
};
*/

//}
#endif // 
