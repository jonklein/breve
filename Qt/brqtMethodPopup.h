#include <qcombobox.h>
#include <qtextedit.h>
#include <vector>

class brqtMethodPopup : public QComboBox {
  Q_OBJECT

  public:
	brqtMethodPopup( QWidget *p ) : QComboBox( p ) {
		connect( this, SIGNAL( highlighted( int ) ), this, SLOT( go( int ) ) );

		insertItem( count(), "Go to method..." );

		_updating = false;
	}
		
	void setTextArea( QTextEdit *e ) {
		_textArea = e;
	}

  public slots:
	virtual void go( int );

  private:
	void 			showPopup();	
	QTextEdit*		_textArea;
	std::vector<int> 	_lineMap;

	bool			_updating;

};
