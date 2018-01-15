
#include <data/Notes.h>

#include <memory>

using namespace data;

size_t CNote::character_limit = 80;		//vic asi nebude ani potreba.. min by bylo asi malo..


unsigned int data::noteLine::uniqueLineId = 0;

const std::string CNoteData::default_save_file = "notes";

int noteLine::default_line_width = 4;

CNote::CNote(std::string note_text)
{

    color = CColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    m_name = note_text;
    m_sceneTransform.makeIdentity();
    m_isVisible = false;
    m_note_text = note_text;
   // m_separateName = false;
}

CNote::CNote() {

    color = CColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    m_name = "";
    m_sceneTransform.makeIdentity();
    m_isVisible = false;
    m_note_text = "";
   // m_separateName = false;
}

//void CNote::updateName(){

    //has separate name so nothing will happen
    //if (m_separateName == true)
   //     return;

   // m_name = generateName();
   
//}

//std::string CNote::generateName() {
    /*
    std::string tmp;

    //generate name from text
    for (size_t i = 0; i < std::min(m_note_text.size(), character_limit); ++i) {

        if (m_note_text.at(i) == '\n' || m_note_text.at(i) == '\t') {
            tmp.push_back(' ');
            continue;
        }

        tmp.push_back(m_note_text.at(i));
    }

    return tmp;
    */
//}

void CNote::setText(std::string text){
    m_note_text = text;
  
    //updateName();
}

void CNote::setName(std::string name) {
    
    m_name = name;
}

std::string CNote::getText() const
{
    return m_note_text;
}

std::string *CNote::getTextPtr() {

    return &m_note_text;
}



void CNote::removeLines(std::vector<unsigned int> lineIds) {

    for (size_t i = 0; i < lineIds.size(); ++i) {

        unsigned int lineId = lineIds[i];

        auto iterator = std::find_if( m_lines.begin(), m_lines.end(),
            [lineId](const noteLine & line) -> bool { return line.m_id == lineId; } );

        if (iterator != m_lines.end()) 
            m_lines.erase(iterator);
        
    }
}


//////////////////////////////////////////////////////////////
//// Notes

CNoteData::CNoteData() : CObject(), CSerializable(){
    m_loaded = false;
    //qDebug() << "CNoteData constructed !!!!!!";
}

CNoteData::~CNoteData() {
    //qDebug() << "CNoteData destructed !!!!!!";
}
/*
//http://stackoverflow.com/questions/10273816/how-to-check-whether-file-exists-in-qt-in-c
bool CNoteData::fileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return (check_file.exists() && check_file.isFile());
}
*/
void CNoteData::setAllVisibleOff() {

    for (size_t i = 0; i < m_notes.size(); i++) 
        m_notes[i].m_isVisible = false;
    
}

void CNoteData::addToLineChangeQueue(lineChange change) {

    m_lineChanges.push_back(change);

    m_lastAction = noteChange(0, noteChange::line);

}

void CNoteData::addToNoteChangeQueue(noteChange change) {

    m_noteChanges.push_back(change);

}

void CNoteData::init(){


    m_notes.clear();
    m_lastAction = noteChange(0, noteChange::change_enum::init);
    

    //qDebug() << "CNoteData initialized !!!!!!";

}



size_t CNoteData::size() const
{
    return m_notes.size();
}


void CNoteData::setText(unsigned int row, std::string text) {

	m_notes[row].setText(text);

	m_lastAction = noteChange(row, noteChange::change_enum::edit);

}
/*
void CNoteData::setName(unsigned row, std::string name) {
    
    notes[row].name = name;

    lastAction = noteChange(row, noteChange::change_enum::edit);

}
*//*
CNote *CNoteData::addNote(std::string note_text) {

    m_notes.emplace_back(CNote(note_text));
	
	m_lastAction = noteChange(0, noteChange::change_enum::insert);
	
	return &m_notes.back();
}
*/

CNote *CNoteData::insertNote(int position, CNote note) {
	if (position < 0)
		position = 0;
    	
    if(position > m_notes.size())
		position = m_notes.size();


    m_notes.insert(m_notes.begin() + position, note);

	m_lastAction = noteChange(position, noteChange::change_enum::insert);

	return &m_notes[position];
}


/*
void CNoteData::updateNote(unsigned int row, Note editedNote) {
    
    notes[row] = editedNote;
    notes[row].updateName();

	lastAction = noteChange(0, noteChange::change_enum::edit);

}
*/

CNote *CNoteData::getNote(unsigned int row) {

    if (row >= m_notes.size() || row < 0)
        return nullptr;
    else
        return &m_notes.at(row);

}

CNote CNoteData::getNoteCopy(unsigned int row){

    if (m_notes.size() - 1 >= row)
        return m_notes.at(row);
    else 
        return CNote();
}


void CNoteData::discardNote(unsigned int row){

    if (row >= m_notes.size())
        return;

    m_notes.erase(m_notes.begin() + row);
    
	m_lastAction = noteChange(row, noteChange::change_enum::discard);

}


void CNoteData::overwrite(std::vector<CNote> &notes) {

	init();

	for (size_t i = 0; i < notes.size(); i++) 
		insertNote(i, notes[i]);
	

    m_lastAction = noteChange(notes.size(), noteChange::change_enum::init);

}


/*
Order:
version
no. of notes

note size
note name
note text
lines size
    points size
    points x y
    width
color
matrix
distance
*/
void CNoteData::serialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> & Writer) {

    Writer.beginWrite(*this);

    Writer.write((vpl::sys::tUInt32) NOTES_FORMAT_VERSION);
    

    //write number of notes which will be saved
    Writer.write((vpl::sys::tUInt32)m_notes.size());


    for (size_t i = 0; i < m_notes.size(); ++i) {
        CNote *note = &m_notes[i];

        //writes number of chars automatically
        Writer.write(note->m_name);

        Writer.write(note->m_note_text);
        
        Writer.write((vpl::sys::tUInt32)note->m_lines.size());

        for (size_t line_index = 0; line_index < note->m_lines.size(); ++line_index) {
            auto line = note->m_lines.at(line_index);



            //write number of points which will be saved
            Writer.write((vpl::sys::tUInt32)line.m_points->size());

            for (size_t j = 0; j <line.m_points->size(); ++j) {
                Writer.write(line.m_points->at(j).x());
                Writer.write(line.m_points->at(j).y());
                Writer.write(line.m_points->at(j).z());
            }

            //line width
            Writer.write((vpl::sys::tInt32)line.m_width);

        }

        note->color.serialize(Writer);
        
        for (size_t row = 0; row < 4; ++row) 
        for (size_t col = 0; col < 4; ++col)
            Writer.write(note->m_sceneTransform(row, col));
        
        
        Writer.write(note->m_distance);

    }


    Writer.endWrite(*this);

}

/*
pri (de)serializaci stringu neni potreba uvadet delku zapisuje/cte si ji to samo

Order:
version
no. of notes

note size
note name
note text
lines size
    points size
    points x y
    width
color
matrix
distance
*/
void CNoteData::deserialize_version_1(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Reader) {

    vpl::sys::tUInt32 count;
    Reader.read(count);

    for (size_t i = 0; i < count; ++i) {

        m_notes.push_back(CNote());


        vpl::sys::tUInt32 no_of_points, no_of_lines;

        //remove default text
        m_notes.back().m_note_text.clear();

        Reader.read(m_notes.back().m_name);

        Reader.read(m_notes.back().m_note_text);

        Reader.read(no_of_lines);


        for (size_t line_index = 0; line_index < no_of_lines; ++line_index) {


            Reader.read(no_of_points);


            for (size_t j = 0; j < no_of_points; ++j) {

                osg::Vec3 newPoint;

                Reader.read(newPoint[0]);
                Reader.read(newPoint[1]);
                Reader.read(newPoint[2]);

                if (j == 0)
                    m_notes.back().m_lines.push_back(noteLine());

                m_notes.back().m_lines.at(line_index).addPoint(newPoint);

            }


            //line width
            Reader.read(m_notes.back().m_lines.at(line_index).m_width);

        }


        m_notes.back().color.deserialize(Reader);

        for (size_t row = 0; row < 4; ++row)
            for (size_t col = 0; col < 4; ++col)
                Reader.read(m_notes.back().m_sceneTransform(row, col));


        Reader.read(m_notes.back().m_distance);

        //check if read name is the same as generated one
       // if (m_notes.back().generateName().compare(m_notes.back().m_name) != 0)
       //     m_notes.back().m_separateName = true;

        // notes.back().updateName();
    }

    Reader.endRead(*this);


}

void CNoteData::deserialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Reader) {
    
    Reader.beginRead(*this);
    
    vpl::sys::tUInt32 version;
    
    Reader.read(version);


    switch (version) {
    
        case 1: deserialize_version_1(Reader); break;
    
        //case 2: deserialize_version_2(Reader); break;

        //etc..

        default: 	
                m_loaded = false;
                return;    //this is an old version and cannot open any newer format
    }


	m_loaded = true;
   
    m_lastAction = noteChange(m_notes.size(), noteChange::change_enum::init);

}
 
//this is mostly duplication of code found in CApproximation but can't be used due to undesirable
//dependencies... :(
bool noteLine::isOver(osg::Vec3 point, const osg::Matrix &point_to_screen, int X, int Y) const {

    //Every point is projected back to screenspace and denormalized
    //tested point is already there
    //consecutive segments share one point so redundant operations are avoided

    float width_factor = X / 2.0f;
    float height_factor = Y / 2.0f;

    osg::Vec3f p1 = m_points->front() * point_to_screen;
    osg::Vec3f p2;

    p1.set(p1.x() * width_factor + width_factor, p1.y() * height_factor + height_factor, 0.0f);

    osg::Vec3 tested_point(point[0], point[1], point[2]);
    osg::Vec3 segment_point1(p1[0], p1[1], p1[2]);
    osg::Vec3 segment_point2;

    for (size_t i = 0; i < m_points->size() - 1; ++i) {

        //projecting back to screenspace
        p2 = m_points->at(i + 1) * point_to_screen;
        p2.set(p2.x() * width_factor + width_factor, p2.y() * height_factor + height_factor, 0.0f);

        segment_point2 = osg::Vec3(p2[0], p2[1], p2[2]);


        osg::Vec3 first_vector = segment_point2 - segment_point1;
        osg::Vec3 second_vector = point - segment_point1;

        //this is dot product
        float result1 = second_vector * first_vector;

        double distance = 100.0;

        // it's outside of segment, return distance from edge point
        if (result1 <= 0) {
            distance = (point - segment_point1).length();
        } else {
            float result2 = first_vector * first_vector;

            //it's outside the other way
            if (result2 <= result1) {
                distance = (point - segment_point2).length();
            } else {
                //it's inside segment
                float parameter = result1 / result2;

                //get the point
                osg::Vec3 projected_point = segment_point1 + first_vector * parameter;

                distance = (point - projected_point).length();
            }
        }

        if (distance < (float)detection_area_width)
            return true;


        segment_point1 = segment_point2;
    }

    return false;
}