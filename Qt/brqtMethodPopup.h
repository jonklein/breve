#include <qcombobox.h>
#include <qtextedit.h>
#include <vector>

class brqtMethodPopup : public QComboBox {
  Q_OBJECT

  public:
    brqtMethodPopup( QWidget *p ) : QComboBox( p ) {
      insertItem( count(), "Go to method..." );
    };
		
    void setTextArea( QTextEdit *e) {
      _textArea = e;
    }

  public slots:
	virtual void go( unsigned int );

  private:
    void showPopup();    
    QTextEdit *_textArea;
	std::vector<int> _lineMap;

};
