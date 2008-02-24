#ifndef _BREVEINTERFACE_H_
#define _BREVEINTERFACE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BreveInterface.cpp"
#endif

#include <string>

class brInstance;
class brEngine;
class stSteveData;

class BreveInterface
{
    public:
	BreveInterface(char * simfile, wxString simdir, char * text);
	~BreveInterface();

	bool Initialize();
	void Iterate();
	void Render();
	void Abort();
	void Pause(int pause = 2);
	void ResizeView(int x, int y);
	void reportError();
	void RunMenu(int id, brInstance*);
	void ExecuteCommand(wxString str);
	void RunCommand(char * str);
	void UpdateLog();

	char * getLoadname();
	char * getSavename();
	int dialogCallback(char *title, char *message, char *b1, char *b2);
	void menuCallback( brInstance *i );
	void messageCallback( const char *text );

	bool Paused() {
	    return paused;
	}

	bool Initialized() {
	    return _engine != NULL;
	}

	int GetX() {
	    return x;
	}

	int GetY() {
	    return y;
	}

	BreveInterface *GetNext() {
	    return next;
	}

	BreveInterface * Rechain(BreveInterface * n) {
	    next = n;
	}

	wxMenu * GetMenu() {
	    return _simmenu;
	}

	bool IsValid() {
	    return valid;
	}

	void UpdateCode( char * c ) {
	    if ( Initialized() ) {
		free(c);
		return;
	    }

	    free(text);
	    text = c;
	}

	brEngine* GetEngine() {
	    return _engine;
	}

    private:
	void			GenerateEngine();

	int 			x, y;
	int 			valid;
	int 			paused;

	char 			*text;
	char 			*simulationfile;

	BreveInterface 		*next;
	brEngine*		_engine;
	stSteveData*		_steveData;
	wxMenu*			_simmenu;

	wxString		_simdir;

	std::string 		mQueuedMessage;
};

char * getLoadname();
char * getSavename();
int soundCallback();
int pauseCallback();
void *newWindowCallback(char *name, void *graph);
void freeWindowCallback(void *w);
void renderWindowCallback(void *w);
void graphDisplay();
void menuCallback(brInstance *i); // brGlutMenuUpdate
int dialogCallback(char *title, char *message, char *b1, char *b2);
char *interfaceVersionCallback();
void messageCallback( const char *text );

#endif
