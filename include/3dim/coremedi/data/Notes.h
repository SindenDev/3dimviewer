#ifndef NOTES_H
#define NOTES_H

#define _USE_MATH_DEFINES


#include <VPL/Base/Setup.h>
#include <VPL/Base/Object.h>
#include <VPL/Base/SharedPtr.h>

#include <data/CStorageInterface.h>
#include <data/CObjectHolder.h>
#include <data/storage_ids_core.h>
#include <data/CSerializableData.h>
#include <data/ESnapshotType.h>




#include <3dim/core/data/CColorVector.h>

#include <osg/Array>
#include <deque>


#define NOTES_FORMAT_VERSION 1

namespace data
{

//! Representation of drawn line
    class noteLine {

    public:

        //! must be initialized with coordinates of the first point to ensure that float value of z -> +-0 will
        //! always be inside the BB, so the point is used to fudge Z values of BB around the correct place
        noteLine() {
            m_id = uniqueLineId++;
            m_points = new osg::Vec3Array();
            m_width = default_line_width;
            //qDebug() << "line" << m_id << "constructed";
        }

        ~noteLine() {
            //qDebug() << "line" << m_id << "destructed";       
        }

        //! Id of this line segment. When deleting line this id should help..
        unsigned int m_id;

        //! Points this line consists of.. Will maybe change later to some other representation..
        osg::ref_ptr<osg::Vec3Array> m_points;

        static int default_line_width;

        //! Width of the line
        unsigned int m_width;


        //copying vector of lines in Note should duplicate all points inside individual lines
        noteLine(const noteLine &obj) {

            m_id = obj.m_id;
            m_points = obj.m_points;// osg::clone(obj.m_points.get(), osg::CopyOp::SHALLOW_COPY);
            m_width = obj.m_width;
        }

        //! Also takes care of expanding BB
        void addPoint(osg::Vec3 point) const {

            m_points->push_back(point);
        }

        //! Proximity detection for highlighting and erasing.
        //! point_to_screen is current view*projection matrix which is used with given dimensions to convert
        //! normalized line coordinates back to screen for more straightforward distance testing.
        bool isOver(osg::Vec3 point, const osg::Matrix &point_to_screen, int X, int Y) const;



    private:
        static unsigned int uniqueLineId;

        //! Space beyond actual width of the line where coordinates still report as over.
        static const int detection_area_width = 10;

       // bool isOnTheLeft(OpenMesh::Vec3f normal, OpenMesh::Vec3f vector_from_point);
    };

//! parameter to signal SigNoteLineChange for noteSubtree..
class lineChange {	
public:
    enum /*class*/ action_enum { empty, add, discard, color, highlight } m_action; // note: class works with vs2013 but not with 2010

    //! Discard
    lineChange(action_enum action, int note_index, std::vector<unsigned int> addedIds) 
    {
        m_highlight = false;
        m_note_index = 0;
        m_line_id = 0;
        m_line = nullptr;
        m_action = action_enum::empty;

        if (action != action_enum::discard) {
            m_action = action_enum::empty;
        } else {
            m_action = action;
            m_note_index = note_index;
            m_addedIds = addedIds;
        }
    }

    //! Highlight
    lineChange(action_enum action, int note_index, int line_id, bool highlight) 
    {
        m_highlight = false;
        m_note_index = 0;
        m_line_id = 0;
        m_line = nullptr;
        m_action = action_enum::empty;

        if (action != action_enum::highlight) {
            m_action = action_enum::empty;
        } else {
            m_action = action;
            m_note_index = note_index;
            m_line_id = line_id;
            m_highlight = highlight;
        }
    }

    //! Add
    lineChange(action_enum action, int note_index, noteLine *line) 
    {
        m_highlight = false;
        m_note_index = 0;
        m_line_id = 0;
        m_line = nullptr;
        m_action = action_enum::empty;

        if (action != action_enum::add || line == nullptr) {
            m_action = action_enum::empty;
        } else {
            m_action = action;
            m_note_index = note_index;
            m_line = line;
        }
    }

    //! Color
    lineChange(action_enum action, int note_index, data::CColor4f color) 
    {
        m_highlight = false;
        m_note_index = 0;
        m_line_id = 0;
        m_line = nullptr;
        m_action = action_enum::empty;

        if (action != action_enum::color) {
            m_action = action_enum::empty;
        } else {
            m_action = action;
            m_note_index = note_index;
            m_color = color;
        }
    }


	//! Everytime
    int m_note_index;

    //! When highlighting
    int m_line_id;

    //! When adding
    noteLine *m_line;

    //! When discarding
    std::vector<unsigned int> m_addedIds;

    //! When changing color
    data::CColor4f m_color;

	//! When highlighting
    bool m_highlight;
};

class CNote {


public:
    CNote();
    CNote(std::string note_text);
    explicit CNote(std::string note_text, osg::Matrix sceneTransform, double distance, CColor4f color = CColor4f(1.0f, 0.0f, 0.0f, 1.0f)) : color(color), m_sceneTransform(sceneTransform), m_distance(distance)
    {
        m_name = note_text;
        m_isVisible = false;
        m_note_text = note_text;
        //m_separateName = false;
    }

	//! Also takes care of updating note name.
	void setText(std::string text);

    //! User set name which won't be automatically ovewritten when setting text
    void setName(std::string name);

	std::string getText() const;

	std::string *getTextPtr();

	//! Default note text for determining whether user changed note text
	//static const std::string default_note_text;	

	CColor4f color;	

	//! Shortened text makes notes' name which is used in pluginpanel noteTree
	std::string m_name;				

	//! Drawn lines belonging to note
	std::vector<noteLine> m_lines;

	//! Is used to set the camera view to given position
	osg::Matrix m_sceneTransform;

    double m_distance;

	//! Removes lines from note with given Ids
	void removeLines(std::vector<unsigned int> lineIds);

   // bool hasSeparateName() const { return m_separateName; }

    //! This helps coordination between notespluginpanel and notesubtree.
    //! PluginPanel sets this in store an upon invalidate NoteSubtree updates visibilities from this.
    bool m_isVisible;


protected:

	std::string m_note_text;

	//! Takes note_text and takes up to character_limit of it to form note name.
	//void updateName();
    
    //! Creates name from note_text
    //std::string generateName();


	//! Character limit for name. 
	static size_t character_limit;

    //! Flag telling whether this note has user defined name or generated one.
    //bool m_separateName;


	//! Because of updateName and note_text in deserialize and updateNote
	friend class CNoteData;
};


//must keep track of recent change because it can come from outside of notesPlugin
//and it has to be able to recognize that
//position in init type says how many items are there... - undo/redo cause reinit and more in sequence would cause trouble otherwise
class noteChange {

public:
	enum /*class*/  change_enum { insert, edit, discard, init, line, empty } type;
	
	noteChange() 
    {
        type = change_enum::init;
        position = 0;
    }
	noteChange(int position, change_enum type) : type(type), position(position)
    {

    }

	unsigned int position;

	bool operator ==(const noteChange &other) const {
		return (this->type == other.type && this->position == other.position);
	};

	bool operator !=(const noteChange &other) const {
		return !(*this == other);
	};


	std::string typeToString() {
	
	
		switch (type) {
		
			case insert: return "insert";
			case edit: return "edit";
			case discard: return "discard";
			case init: return "init";
            case line: return "line";

			case empty:
			default: return "empty";
		
		}
	
	}
};






class CNoteData : public vpl::base::CObject, public vpl::mod::CSerializable {

    public:
        //! So this can be pointed at from places
        VPL_SHAREDPTR(CNoteData);

        //! Default compression method.
        VPL_ENTITY_COMPRESSION(vpl::mod::CC_GZIP);
        //VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

        //! Default class name.
        VPL_ENTITY_NAME("NoteData");

        //! The owning class lets Notes to manage this part of ui
        CNoteData();

        ~CNoteData();

		//static bool fileExists(QString path);

        //! Reports the size of note vector.
        size_t size() const;

        CNote *getNote(unsigned int row);

		CNote getNoteCopy(unsigned int row);

        //! Adds blank, brand new note to the end of the list
        //CNote *addNote(std::string note_text);

        //! Used by undo. Allows for inserting previously discarded note back.
		CNote *insertNote(int position, CNote note);

		//! During undo/redo this just simply replaces all the data.
		void overwrite(std::vector<CNote> &notes);

       
        //! Removes note from currently active row.
        void discardNote(unsigned int row);

		//! Sets note text which updates its name too...
		void setText(unsigned int row, std::string text);


      //  void setName(unsigned int row, std::string name);

        //! Regenerates the object state according to any changes in the data storage. UNUSED
		static void update(const CChangedEntries& Changes) {}

        //! Sets all visibility flags in notes to false
        void setAllVisibleOff();

        //! pushes back lineChange and updates lastAction
        void addToLineChangeQueue(lineChange change);

        //! pushes back noteChange
        void addToNoteChangeQueue(noteChange change);

        //! Clears all notes. Sets change to init. Can be repopulated again by loadNotes.
        void init();

        //! Does object contain relevant data?
        //! - Returns always true.
        virtual bool hasData() { return true; }  //??

        //! Serialize the content of notes vector. Names of notes are not saved because they are derived from note_text.
        void serialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Writer);

        //! Deserialize 
        void deserialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Reader);

        void deserialize_version_1(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Reader);

        //! Returns true if changes of a given parent entry may affect this object.
		static bool checkDependency(CStorageEntry *pParent) { return false; }

        //! list of changes for noteSubtre to process
        std::deque<lineChange> m_lineChanges;

        //! list of changes for noteSubtre to process
        std::deque<noteChange> m_noteChanges;

        //! Note of lastAction for pluginPanel to filter out actions it initiated and not do them for a second time
        noteChange m_lastAction;

		//! The stuff around is here because of this.
		std::vector<CNote> m_notes;

		//! set after deserialization. Prevents repeated loading.
		bool m_loaded;
protected:

    static const std::string default_save_file;


        
};

DECLARE_SERIALIZATION_WRAPPER(CNoteData);


namespace Storage
{
    //! Drawing options
    DECLARE_OBJECT(NoteData, CNoteData, CORE_STORAGE_NOTE_PLUGIN_ID);
}

class CNoteSnapshot : public CSnapshot {

public:
    CNoteSnapshot(std::vector<CNote> &notes, CUndoProvider *provider = nullptr) : CSnapshot(data::UNDO_NOTES, provider) {

        for (size_t i = 0; i < notes.size(); i++) {
            this->m_notes.push_back(notes[i]);
        }
    }


    //! duplicate of notes
    std::vector<CNote> m_notes;


    //! This should cover most of the important bits.. it's NOT exact.
    long getDataSize() override {

        unsigned int size = m_notes.size() * sizeof(CNote);

        for (size_t i = 0; i < m_notes.size(); i++) {

            size += m_notes[i].getTextPtr()->size() * sizeof(char);

            for (auto line = m_notes[i].m_lines.begin(); line != m_notes[i].m_lines.end(); line++)
            {
                size += line->m_points->size() * sizeof(osg::Vec3);
                size += sizeof(noteLine);
            }
        }

        return size;
    }


};

}	//namespace data

#endif  //NOTES_H