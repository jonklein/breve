#include <qthread.h>

class MyThread : public QThread {
    public:

    brEngine *_engine;

    brqtThread(brEngine *e) {
        _engine = e;
    }

    void run() {
        brEngineIterate( _engine);
    }
};


