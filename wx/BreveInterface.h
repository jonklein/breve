#ifndef _BREVEINTERFACE_H_
#define _BREVEINTERFACE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BreveInterface.cpp"
#endif

struct breveFrontend;
struct brInstance;

class BreveInterface
{
    public:
	BreveInterface(char * simfile, char * text);
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

	char * getLoadname();
	char * getSavename();
	int dialogCallback(char *title, char *message, char *b1, char *b2);
	void menuCallback(brInstance *i);
	void messageCallback(char *text);

	bool Paused()
	{
	    return paused;
	}

	bool Initialized()
	{
	    return initialized;
	}

	int GetX()
	{
	    return x;
	}

	int GetY()
	{
	    return y;
	}

	void SetX(int x)
	{
	    this->x = x;
	}
	void SetY(int y)
	{
	    this->y = y;
	}

	BreveInterface * GetNext()
	{
	    return next;
	}

	BreveInterface * Rechain(BreveInterface * n)
	{
	    next = n;
	}

	wxMenu * GetMenu()
	{
	    return simmenu;
	}

	bool IsValid()
	{
	    return valid;
	}

	void UpdateCode(char * c)
	{
	    if (initialized)
	    {
		free(c);
		return;
	    }

	    free(text);
	    text = c;
	}

	breveFrontend * GetFrontend()
	{
	    return frontend;
	}

    private:
	int x, y;
	int valid;
	int paused;

	bool initialized;

	char * text;
	char * simulationfile;

	BreveInterface * next;
	breveFrontend *frontend;
	wxMenu * simmenu;
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
void messageCallback(char *text);

#endif
