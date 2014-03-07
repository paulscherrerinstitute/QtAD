#ifndef PVOBJECT_H
#define PVOBJECT_H

#include <QObject>
#include <QVariant>
#include <QStringList>

#include <cadef.h>

#define ENTER_CA \
    if(m_cac)\
    {ca_attach_context(m_cac);{

#define LEAVE_CA \
    }}

#include <QThread>
class SleeperThread : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};

class PvObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString  channel  READ channel  WRITE setChannel)
    Q_PROPERTY(int      severity READ severity)
    Q_PROPERTY(int      status   READ status)
    Q_PROPERTY(QString  units    READ units)
    Q_PROPERTY(int      prec     READ prec)
    Q_PROPERTY(int      nostr   READ nostr)
    Q_PROPERTY(QStringList strs     READ strs)
    Q_PROPERTY(QVariant upctrllim READ upctrllim)
    Q_PROPERTY(QVariant lostrllim READ loctrllim)

public:
    explicit PvObject(QObject *parent = 0);
    ~PvObject();

    long init(bool monitor=true);
    long get(int count);

    /* context */
    static long init_ca()
    {
        int status;

        status = ca_context_create(ca_enable_preemptive_callback);
        if(status != ECA_NORMAL)
            return(CA_EXTRACT_MSG_NO(status));

        m_cac = ca_current_context();

        return 0;
    }
    static long exit_ca()
    {
        ENTER_CA{
            ca_context_destroy();
            m_cac = NULL;
        }LEAVE_CA

        return 0;
    }


    long connect(const char *name);
    long disconnect();
    long ensureConnection();


    long monitor(unsigned long);
    long unmonitor();
    bool monitored() { return _evid != 0; }
    static void getCallback(struct event_handler_args);

    void* arrayData() {return _arrayData;}
    unsigned int arraySize() { return _arraySize;}
    unsigned int arrayBytes() {return _arrayBytes;}

    void setValue(const QVariant val);
    QVariant value();

    void setChannel(const QString name);
    QString channel();

    long dataType()   {return _chid != NULL?ca_field_type(_chid):-1; }
    int severity() {return _severity;}
    int status()   {return _status;}
    QString units()    {return _units;}
    int prec()     {return _precision;}
    int nostr()   {return _nostr;}
    QStringList strs()     {return _strs;}
    QVariant upctrllim() {return _upctrllim;}
    QVariant loctrllim() {return _loctrllim;}

signals:
    void valueChanged();

public slots:


private:
    QVariant _value;
    QString _name;
    void * _arrayData; 
    unsigned int _arrayBytes; 
    unsigned int _arraySize;

    // pv alarm info
    int _status;			// alarm status
    int _severity;			// alarm severity
    int _sec;				// time stamp - seconds since Midnight Jan.1, 1990
    int _nsec;			// time stamp - nano seconds within second

    // pv control info
    QString _units; // units

    int _nostr;		// no. of state strings
    QStringList _strs;     // state strings separated by ;

    int _precision;	// precision for float and double type

    QVariant _updisplim;  // upper display limit
    QVariant _lodisplim;  // lower display limit

    QVariant _upalrmlim;  // upper alarm   limit
    QVariant _loalrmlim;  // lower alarm   limit

    QVariant _upwarnlim;  // upper warn    limit
    QVariant _lowarnlim;	 // lower warn    limit

    QVariant _upctrllim;  // upper control limit
    QVariant _loctrllim;  // lower control limit

    
    epicsMutexId _mutex; 
    chid _chid;
    evid _evid;
    bool _getCallbacked;
    static struct ca_client_context *m_cac;
};

#endif // PVOBJECT_H
