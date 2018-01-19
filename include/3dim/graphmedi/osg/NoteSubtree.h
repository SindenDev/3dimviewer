#pragma once


#include <ciso646>

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Switch>
#include <osg/CGeneralObjectObserverOSG.h>

#include <3dim/core/data/CColorVector.h>
#include <render/CVolumeRenderer.h>

#include <data/Notes.h>

//When subclassing from Referenced (directly or indirectly), always protect the destructor so the object cannot be allocated on the stack.


/*

	   NoteDrawerNode										//note switcher
	  /				\
NoteLineSwitch		NoteLineSwitch							//one per note, switches on/off lines - important for editing
	|					|		  \
NoteGeode			NoteGeode	   NoteGeode				//one per line in note
	|					|			|		
NoteGeometry		NoteGeometry   NoteGeometry				//references note points of one line




NoteTransformNode handles construction of NoteModelViewMatrix and NoteDrawerNode
NoteLineSwitch handles construction of NoteGeode and NoteGeometry

NoteTransformNode is created in mainwindow.

*/



namespace osg {

//holds data to draw, under NoteDrawables
class  CNoteGeometry : public Geometry {
	public:

		CNoteGeometry() {
            m_highlighted = false;
			Geometry::setUseVertexBufferObjects(true);
		}

		//! Sets up data for drawing.
		void setData(data::noteLine *line, data::CColor4f& color);

		//! Resets the color array
		void setColor(data::CColor4f color);

		//! Sets color to yellow or reaches out to NoteLineSwitch parent to retrieve original color.
		void setHighlight(bool highlight);

	protected:
		~CNoteGeometry() {}

		bool m_highlighted;

		//osg::Vec4 colorArray;

		//osg::Vec4 highlightColorArray;

		//or do own shaders and change uniform and set different colors, instead of transparent one, at edges
};

//! one NoteGeode per line
class CNoteGeode : public Geode {
	public:
		CNoteGeode() {}
	protected:
		~CNoteGeode() {}
};


//! the line switcher, represents one whole note
class CNoteLineSwitch : public Switch {
	
public:
	CNoteLineSwitch(data::CNote *note);

	//! Creates line subtree -> NoteGeode + NoteGeometry
	void addLine(data::noteLine *line, data::CColor4f color);

	//! Removes line identified by its id from drawables
	void removeLine(int id);

    //! (re)sets color array of all children
	void changeColor(data::CColor4f newColor);

    //! Tells select line to highlight - change color array to yellow
	void setHighlightNoteLine(int line_id, bool highlight);

	data::CColor4f m_color;	//red;

	
protected:
	~CNoteLineSwitch() {}

	//! Keeps mapping of line Ids to their indices
	//! Removing line with given Id takes finding its id in this and removing child at that index.
	std::vector<int> m_lineIds;

};




//! the big switcher, switches note visibility on/off
class  CNoteDrawerNode : public Switch, public scene::CGeneralObjectObserverOSG<CNoteDrawerNode>
{
public:

    //! Constructor
    CNoteDrawerNode();

    //! Destructor
    ~CNoteDrawerNode();

    //! Creates NoteGeometry in Note drawable which is added as child and refreshes first note
    void addNote(data::CNote* note);

    //! Removes child at given index
    void discardNote(unsigned int index);

    //! unused
    void replaceNote(int index, data::CNote* note);

    //! Passes the task to NoteLineSwitch
    void setHighlightNoteLine(int note_index, int line_id, bool highlight);

    //! Does clear first and then creates children from note data store.
    void rebuild();

    //! Reads visibility flags of notes in dataStore and sets its children accordingly
    void refreshVisibility();

    //! Removes all children.
    void clear();

    const CVolumeRenderer* getRenderer() const { return m_renderer; }

    //! Observer function over note data store to avoid using VPL signals,
    //! which may come at an inconvenient time and possibly crash the application.
    void updateFromStorage() override;

protected:


    //! Used to force scene redraw, else the screen updates only when it has mouse over it.
    CVolumeRenderer* m_renderer;


    //! Called whenever there is called invalidate on note data store.
    //! calls noteChangeHandler and lineChangeHandler on their respective lists of changes in note data store.
    void on_notesData_itemChanged(data::CStorageEntry* whoknowswhat);

    //! Takes care of changes to notes. adding/discarding/rebuilding
    void noteChangeHandler(data::noteChange change);

    //! Takes care of changes to lines - adding/discarding/highlighting/color change
    void lineChangeHandler(data::lineChange change);
};


}  // namespace osg