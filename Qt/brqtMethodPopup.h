#include <QComboBox.h>
#include <QTextEdit.h>
#include <vector>

class brqtMethodPopup : public QComboBox {
  Q_OBJECT

  public:
    brqtMethodPopup(QWidget *p, const char *name) : QComboBox(p, name) {
      insertItem("Go to method...");
    };
		
    void setTextArea( QTextEdit *e) {
      _textArea = e;
    }

  public slots:
	virtual void go( int );

  private:
    void popup();    
    QTextEdit *_textArea;
	std::vector<int> _lineMap;

};
