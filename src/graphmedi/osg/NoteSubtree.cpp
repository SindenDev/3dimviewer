#include <osg/NoteSubtree.h>
#include <osg/LineWidth>

#include <qdebug.h>

void osg::CNoteGeometry::setData(data::noteLine *line, data::CColor4f& color) {

	osg::Vec3Array * vertex_colors = new osg::Vec3Array();
	vertex_colors->push_back(osg::Vec3(color.getR(), color.getG(), color.getB()));

    //TODO conversion to triangles here
	setVertexArray(line->m_points);

	setColorArray(vertex_colors, osg::Array::BIND_OVERALL);
	
	auto primitiveSetList = this->getPrimitiveSetList();
	
	//but should only run at most once
	while (primitiveSetList.size() > 0) 
		removePrimitiveSet(0);

	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, line->m_points->size()));

	osg::LineWidth* linewidth = new osg::LineWidth();
	linewidth->setWidth(line->m_width);
	getOrCreateStateSet()->setAttributeAndModes(linewidth, osg::StateAttribute::ON);

	//This variant does not work!!!! I suspect it misses element list
	//addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS));
	//addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP));

}


void osg::CNoteGeometry::setColor(data::CColor4f color) {

	osg::Vec3Array * vertex_colors = new osg::Vec3Array();
	vertex_colors->push_back(osg::Vec3(color.getR(), color.getG(), color.getB()));

    setColorArray(vertex_colors, osg::Array::BIND_OVERALL);
}


void osg::CNoteGeometry::setHighlight(bool highlight) {



	if (m_highlighted == highlight)
		return;

   // qDebug() << "highlight" << highlight;

	m_highlighted = highlight;

    CNoteLineSwitch *parent = (CNoteLineSwitch *)getParent(0)->getParent(0);

	if (m_highlighted) {
		//setColor(data::CColor4f(1.0f, 1.0f, 0.0f, 1.0f));
	
        setColor(data::CColor4f((parent->m_color.getB() + 1.0f) / 2.0f, (parent->m_color.getR() + 1.0f) / 2.0f, (parent->m_color.getG() + 1.0f) / 2.0f, 1.0f));
	} else {
	
		CNoteLineSwitch *parent = (CNoteLineSwitch *)getParent(0)->getParent(0);
		setColor(parent->m_color);
	}
	//change color arrays and whtwr

}


////////////////////////////////////////////


osg::CNoteDrawerNode::CNoteDrawerNode() 
{
    m_renderer = nullptr;
	osg::StateSet* stateSet = new osg::StateSet();
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

    //makes osg to treat this render bin as if it was on the same level with others above it - so the bin number has any effect
    stateSet->setNestRenderBins(false);

    //should be higher than everything defined in CSceneOSG.h
    stateSet->setRenderBinDetails(300, "RenderBin");

	setStateSet(stateSet);
	

	// Connect the observer
    scene::CGeneralObjectObserverOSG<CNoteDrawerNode>::connect(APP_STORAGE.getEntry(data::Storage::NoteData::Id).get());

	this->setupObserver(this);



	m_renderer = VPL_SIGNAL(SigGetRenderer).invoke2();

	qDebug() << "NoteDrawerNode constructed !!!!!!";

}

osg::CNoteDrawerNode::~CNoteDrawerNode() 
{
    freeObserver(this);
    scene::CGeneralObjectObserverOSG<CNoteDrawerNode>::disconnect(APP_STORAGE.getEntry(data::Storage::NoteData::Id).get());
}

//! Called whenever there is called invalidate on this.
void osg::CNoteDrawerNode::on_notesData_itemChanged(data::CStorageEntry *whoknowswhat) {


	data::CObjectPtr<data::CNoteData> noteData(APP_STORAGE.getEntry(data::Storage::NoteData::Id));
	


    while (not noteData->m_noteChanges.empty()) {
        auto noteChange = noteData->m_noteChanges.front();

       
        noteChangeHandler(noteChange);
        
        noteData->m_noteChanges.pop_front();

    }


    while (not noteData->m_lineChanges.empty()) {
        auto lineChange = noteData->m_lineChanges.front();

        lineChangeHandler(lineChange);
        
        noteData->m_lineChanges.pop_front();

    }

    refreshVisibility();
    
	m_renderer->redraw();
}

void osg::CNoteDrawerNode::noteChangeHandler(data::noteChange change) {
    
    data::CObjectPtr<data::CNoteData> noteData(APP_STORAGE.getEntry(data::Storage::NoteData::Id));


 

    if (change.type == data::noteChange::change_enum::init) {

        //let's not play a hero here.. just rebuild it everytime..
        rebuild();
   
    } else if (change.type == data::noteChange::change_enum::edit) {


        //don't react to edit changes.. only to adding/deleting notes

    ///////dead branch
    //replaceNote(change.position, noteData->getNote(change.position));

    } else if (change.type == data::noteChange::change_enum::insert) {

        addNote(noteData->getNote(change.position));

    } else if (change.type == data::noteChange::change_enum::discard) {


        discardNote(change.position);

    }

}


void osg::CNoteDrawerNode::lineChangeHandler(data::lineChange change) {


    if (change.m_action == data::lineChange::action_enum::empty || (uint)change.m_note_index >= getNumChildren())
		return;

	
	//Got no notes..
	if (getNumChildren() == 0)
		return;


    CNoteLineSwitch *note = (CNoteLineSwitch *)getChild(change.m_note_index);

    if (change.m_action == data::lineChange::action_enum::add) {

        note->addLine(change.m_line, note->m_color);
      //  qDebug() << "NoteSubtree line with id" << change.line->id << "added";

    } else if (change.m_action == data::lineChange::action_enum::discard) {
	
        for (size_t i = 0; i < change.m_addedIds.size(); ++i)
            note->removeLine(change.m_addedIds[i]);

      //  qDebug() << "NoteSubtree" << change.addedIds.size() << "lines discarded";

		//note->setAllChildrenOn();
    } else if (change.m_action == data::lineChange::action_enum::color) {
	
		note->changeColor(change.m_color);
	
    } else if (change.m_action == data::lineChange::action_enum::highlight) {
		
        setHighlightNoteLine(change.m_note_index, change.m_line_id, change.m_highlight);
       // qDebug() << "NoteSubtree note number" << change.note_index + 1 << "of" << getNumChildren() << "highlighted line" << change.line_id << change.highlight << getChild(change.note_index)->asSwitch()->getNumChildren() << "lines total";

	}

	//show all note lines
	//note->setAllChildrenOn();

}


void osg::CNoteDrawerNode::addNote(data::CNote *note) {
	
    if (note != nullptr) {
        addChild(new CNoteLineSwitch(note));

      //  qDebug() << "NoteSubtree note added";
    }
	//drawNote(getNumChildren() - 1, false);
}

void osg::CNoteDrawerNode::discardNote(unsigned int index) {
    if (index < getNumChildren()) {
        removeChild(getChild(index));

     //   qDebug() << "NoteSubtree note discarded at" << index;

    }
}


void osg::CNoteDrawerNode::replaceNote(int index, data::CNote *note) {

	discardNote(index);

	if (note == nullptr)
		return;

	insertChild(index, new CNoteLineSwitch(note));

}



void osg::CNoteDrawerNode::setHighlightNoteLine(int note_index, int line_id, bool highlight) {


	auto lineSwitch = (CNoteLineSwitch *)getChild(note_index)->asSwitch();


	lineSwitch->setHighlightNoteLine(line_id, highlight);
}

void osg::CNoteDrawerNode::rebuild() {


	this->clear();

	data::CObjectPtr<data::CNoteData> noteData(APP_STORAGE.getEntry(data::Storage::NoteData::Id));

    setNewChildDefaultValue(false);

	for (size_t i = 0; i < noteData->size(); ++i)
		addNote(noteData->getNote(i));
	 
	 
    qDebug() << "noteSubtree rebuilt with" << getNumChildren() << "children";


}

void osg::CNoteDrawerNode::refreshVisibility() {

    data::CObjectPtr<data::CNoteData> noteData(APP_STORAGE.getEntry(data::Storage::NoteData::Id));

    if (getNumChildren() == 0 || noteData->size() != getNumChildren())
        return;

    for (size_t i = 0; i < noteData->size(); i++)
        setChildValue(getChild(i), noteData->getNote(i)->m_isVisible);
    
}

void osg::CNoteDrawerNode::clear() {

	removeChildren(0, getNumChildren());

}

void osg::CNoteDrawerNode::updateFromStorage() {
	

	on_notesData_itemChanged(nullptr);

}


////////////////////////////////////////////////////////////////////////////


osg::CNoteLineSwitch::CNoteLineSwitch(data::CNote *note) 
{
    //m_color = data::CColor4f(1.0f, 0.0f, 0.0f, 1.0f)
	m_color = note->color;

	for (size_t line_index = 0; line_index < note->m_lines.size(); ++line_index)
		addLine(&note->m_lines.at(line_index), note->color);
}


void osg::CNoteLineSwitch::addLine(data::noteLine *line, data::CColor4f color) {

	//create line branch
	osg::ref_ptr<CNoteGeometry> geometry = new CNoteGeometry;
	osg::ref_ptr<CNoteGeode> geode = new CNoteGeode;

    addChild(geode);
	geode->addDrawable(geometry);

	geometry->setData(line, color);
			
	m_lineIds.push_back(line->m_id);

}


void osg::CNoteLineSwitch::removeLine(int id) {
	auto iterator = std::find(m_lineIds.begin(), m_lineIds.end(), id);

	if (iterator != m_lineIds.end()) {

		removeChild(getChild(iterator - m_lineIds.begin()));
		m_lineIds.erase(iterator);
	}

}

void osg::CNoteLineSwitch::changeColor(data::CColor4f newColor) {

	m_color = newColor;

	
	for (size_t i = 0; i < getNumChildren(); ++i) 
		((CNoteGeometry *)getChild(i)->asGeode()->getDrawable(0))->setColor(newColor);	

}


void  osg::CNoteLineSwitch::setHighlightNoteLine(int line_id, bool highlight) {

	auto iterator = std::find(m_lineIds.begin(), m_lineIds.end(), line_id);
	int line_index = iterator - m_lineIds.begin();

	CNoteGeometry *geometry = (CNoteGeometry *)getChild(line_index)->asGeode()->getDrawable(0);
	geometry->setHighlight(highlight);

}