#include "drawcommand.h"

slDrawCommandList::~slDrawCommandList() {
	clear();
}

void slDrawCommandList::clear() {
	std::list<slDrawCommand*>::iterator di;

	for( di = _commands.begin(); di != _commands.end(); di++ ) {
		delete *di;
	}

	_commands.clear();
}


void slDrawCommandList::addCommand(slDrawCommand *command) {
	if( _limit) {
		if( _commands.size() >= _limit) _commands.pop_front();
	}

	_commands.push_back(command);
}

void slDrawCommandList::draw(slCamera *c) {
	std::list<slDrawCommand*>::iterator di;

	glEnable(GL_BLEND);
	glLineWidth(1.2);
	glDisable(GL_CULL_FACE);
	glColor4f(0.0, 0.0, 0.0, 0.5);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xffff);

	_drawingPolygon = 0;

	for( di = _commands.begin(); di != _commands.end(); di++ ) {
		(*di)->execute( *this);
	}

	if( _drawingPolygon) glEnd();
}
