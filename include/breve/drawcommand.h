#include "config.h"
#include "glIncludes.h"
#include "camera.h"

#ifndef _DRAWCOMMAND_H
#define _DRAWCOMMAND_H

class slDrawCommand;
class slWorld;

#include <list>

class slDrawCommandList {
	public:
		slDrawCommandList(slWorld *w);

		~slDrawCommandList();

		void setCommandLimit(int n) { _limit = n; };
		int getCommandCount() { return _commands.size(); }

		void addCommand(slDrawCommand *command);
		void draw(slCamera *c);
		void clear();

	protected:
		std::list<slDrawCommand*> _commands;
		bool _drawingPolygon;
		unsigned int _limit;

		slVector _origin;
		double _rotation[3][3];

		friend class slDrawEndPolygon;
		friend class slDrawCommandPoint;
		friend class slDrawCommandLine;
};

class slDrawCommand {
	public:
		virtual ~slDrawCommand() { };

		virtual void execute(slDrawCommandList &list) = 0;
};

class slDrawCommandPoint : slDrawCommand {
	public:
		slDrawCommandPoint(slVector *p) : _point(*p) {}

		void execute(slDrawCommandList &list) {
			#ifndef OPENGLES
			if(!list._drawingPolygon) {
				glBegin(GL_POLYGON);
				list._drawingPolygon = 1;
			}

			glVertex3f(_point.x, _point.y, _point.z);
			#endif
		}

	private:
		slVector _point;
};

class slDrawSetLineStyle : slDrawCommand {
	public:
		slDrawSetLineStyle(unsigned int s) : _style(s) {}

		void execute(slDrawCommandList &list) {
			#ifndef OPENGLES
			glLineStipple(1, _style);
			#endif
		}

	private:
		unsigned int _style;
};

class slDrawSetLineWidth : slDrawCommand {
	public:
		slDrawSetLineWidth(double w) : _width(w) {}

		void execute(slDrawCommandList &list) {
			#ifndef OPENGLES
			glLineWidth(_width);
			#endif
		}

	private:
		double _width;
};

class slDrawEndPolygon : slDrawCommand {
	public:
		void execute(slDrawCommandList &list) {
			if(list._drawingPolygon) {
				#ifndef OPENGLES
				glEnd();
				list._drawingPolygon = 0;
				#endif
			}

		}
};

class slDrawCommandLine : slDrawCommand {
	private:
		slVector _start, _end;

	public:
		slDrawCommandLine(slVector *s, slVector *e) : _start(*s), _end(*e) {}

		void execute(slDrawCommandList &list) {
			#ifndef OPENGLES
			if(list._drawingPolygon) {
				list._drawingPolygon = 0;
				glEnd();
			}

			glBegin(GL_LINES);
			glVertex3f(_start.x, _start.y, _start.z);
			glVertex3f(_end.x, _end.y, _end.z);
			glEnd();
			#endif
		}
};

class slDrawCommandColor : slDrawCommand {
	public:
		slDrawCommandColor(slVector *c, double a) : _color(*c), _alpha(a) {}

		void execute(slDrawCommandList &list) {
			glColor4f(_color.x, _color.y, _color.z, _alpha);
		}

	private:
		slVector _color;
		double _alpha;
};

#endif /* _DRAWCOMMAND_H */
