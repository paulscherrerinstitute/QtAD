#include <QtDebug>

#include "pvobject.h"

struct ca_client_context * PvObject::m_cac = NULL;

PvObject::PvObject(QObject *parent):
    QObject(parent)
{
    if (m_cac == NULL)
        init_ca();
    _chid = NULL;
    _evid = NULL;
    _arrayData = NULL; 
    _arrayBytes = 0;
    _arraySize  = 0;
    _getCallbacked = false;
    _mutex = epicsMutexCreate(); 
}

PvObject::~PvObject()
{
    unmonitor();
    disconnect();
}

long PvObject::init(bool monitor)
{
    if (_chid) {
        unmonitor();
        disconnect();
    }

    int status = 0; 
    status = connect(_name.toLatin1());
    if (status != 0)
        return status; 

    if (monitor)
        status = this->monitor(0);
    return  status; 
}

long PvObject::get(int count)
{
    int status = 0;
    if(_chid)
    {
        // Default to be DBR_TIME_XXX
        chtype reqtype = dbf_type_to_DBR_TIME(ca_field_type(_chid));
        _getCallbacked = false;
        ENTER_CA{
            status = ca_array_get_callback(reqtype,
                count,
                _chid,
                getCallback,
                this);
            if(status != ECA_NORMAL)
                return(CA_EXTRACT_MSG_NO(status));
            status = ca_pend_io(1);
            if(status != ECA_NORMAL)
                return(CA_EXTRACT_MSG_NO(status));
        }LEAVE_CA
        while (!_getCallbacked)
            ca_pend_io(1);
    }
    return 0;
}

void PvObject::setValue(const QVariant val)
{
    _value = val;
}

QVariant PvObject::value()
{
    epicsMutexLock(_mutex);
    QVariant v = _value; 
    epicsMutexUnlock(_mutex); 
    return v;
}

void PvObject::setChannel(const QString name)
{
    _name = name;
}

QString PvObject::channel()
{
    return _name;
}

long PvObject::connect(const char* name)
{
    int status;

    ENTER_CA{
        // make connection
        status = ca_create_channel(name, 0, 0, 0, &_chid);
        if(status != ECA_NORMAL)
        {
            _chid = 0;
            return(CA_EXTRACT_MSG_NO(status));
        }

        status = ca_pend_io(1);
        if(status != ECA_NORMAL)
        {
            ca_clear_channel(_chid);
            _chid = 0;
            return(CA_EXTRACT_MSG_NO(status));
        }

    }LEAVE_CA

    return 0;
}

long PvObject::disconnect()
{
    if(_chid)
    {

        ENTER_CA{
            int status = ca_clear_channel(_chid);
            _chid = NULL;
            if(status != ECA_NORMAL)
                return(CA_EXTRACT_MSG_NO(ECA_DISCONN));
            status = ca_pend_io(1);
            if(status != ECA_NORMAL)
                return(CA_EXTRACT_MSG_NO(status));
        }LEAVE_CA
    }
    return 0;
}

long PvObject::monitor(unsigned long count)
{
    int status;

    if(_chid && !_evid)
    {
        // Default to be DBR_TIME_XXX
        chtype reqtype = dbf_type_to_DBR_TIME(ca_field_type(_chid));

        ENTER_CA{
            status = ca_create_subscription(reqtype,
                count,
                _chid,
                DBE_VALUE | DBE_ALARM,
                getCallback,
                this,
                &_evid);

            if(status != ECA_NORMAL)
                return(CA_EXTRACT_MSG_NO(status));

            status = ca_pend_io(1);
            if(status != ECA_NORMAL)
                return(CA_EXTRACT_MSG_NO(status));
        }LEAVE_CA
    }
    return 0;
}

long PvObject::unmonitor()
{
    int status;

    if(_chid && _evid)
    {
        ENTER_CA{
            status = ca_clear_subscription(_evid);
            _evid = 0;
            if(status != ECA_NORMAL)
                return(CA_EXTRACT_MSG_NO(status));
            status = ca_pend_io(1);
            if(status != ECA_NORMAL)
                return(CA_EXTRACT_MSG_NO(status));
        }LEAVE_CA
    }
    return 0;
}

#define ConvertSTS(VP) \
    chan->_status	= VP.status;\
    chan->_severity	= VP.severity;

#define ConvertTime(VP) \
    chan->_sec		= VP.stamp.secPastEpoch;\
    chan->_nsec		= VP.stamp.nsec;\

#define ConvertCtrl(VP) \
    chan->_units		= VP.units;\
    chan->_updisplim = VP.upper_disp_limit;\
    chan->_lodisplim = VP.lower_disp_limit;\
    chan->_upwarnlim = VP.upper_warning_limit;\
    chan->_lowarnlim = VP.lower_warning_limit;\
    chan->_upalrmlim = VP.upper_alarm_limit;\
    chan->_loalrmlim = VP.lower_alarm_limit;\
    chan->_upctrllim = VP.upper_ctrl_limit;\
    chan->_loctrllim = VP.lower_ctrl_limit;


#define ConvertValue(VP,TYPE)\
    if (count == 1) \
        value = (TYPE)*(&(VP.value));\
    else {\
        if (chan->_arrayBytes < count*sizeof(TYPE)) \
            chan->_arrayData = realloc(chan->_arrayData, count*sizeof(TYPE)); \
        chan->_arrayBytes = count*sizeof(TYPE);  \
        chan->_arraySize  = count; \
        memcpy(chan->_arrayData, &(VP.value),  chan->_arrayBytes);\
    }

typedef dbr_string_t* dbr_string_t_ptr;

void PvObject::getCallback(struct event_handler_args args)
{
    PvObject *chan = (PvObject *)args.usr;
    epicsMutexLock(chan->_mutex); 
    if(args.status == ECA_NORMAL)
    {
        union db_access_val *val = (union db_access_val *)args.dbr;
        chtype type = args.type;
        unsigned long count  = args.count;
        QVariant value;
        switch(type)
        {
        case DBR_CTRL_STRING:
        case DBR_TIME_STRING:
            ConvertSTS(val->tshrtval)
            ConvertTime(val->tshrtval)
            for(unsigned long i=0; i<count; i++)
                value = QString(*(dbr_string_t_ptr)(&(val->tstrval.value) + i));
            break;
        case DBR_TIME_INT:
            ConvertSTS(val->tshrtval)
            ConvertTime(val->tshrtval)
            ConvertValue(val->tshrtval,dbr_int_t)
            break;
        case DBR_TIME_FLOAT:
            ConvertSTS(val->tfltval);
            ConvertTime(val->tfltval);
            ConvertValue(val->tfltval,dbr_float_t)
            break;
        case DBR_TIME_ENUM:
            ConvertSTS(val->tenmval);
            ConvertTime(val->tenmval);
            ConvertValue(val->tenmval,dbr_enum_t)
            break;
        case DBR_TIME_CHAR:
            ConvertSTS(val->tchrval);
            ConvertTime(val->tchrval);
            ConvertValue(val->tchrval,dbr_char_t)
            break;
        case DBR_TIME_LONG:
            ConvertSTS(val->tlngval);
            ConvertTime(val->tlngval);
            ConvertValue(val->tlngval,dbr_long_t)
            break;
        case DBR_TIME_DOUBLE:
            ConvertSTS(val->tdblval);
            ConvertTime(val->tdblval);
            ConvertValue(val->tdblval,dbr_double_t)
            break;
        case DBR_CTRL_INT:
            ConvertSTS(val->cshrtval)
            ConvertCtrl(val->cshrtval)
            ConvertValue(val->cshrtval,dbr_int_t)
            break;
        case DBR_CTRL_FLOAT:
            ConvertSTS(val->cfltval);
            ConvertCtrl(val->cfltval);
            ConvertValue(val->cfltval,dbr_float_t)
            chan->_precision = val->cfltval.precision;
            break;
        case DBR_CTRL_ENUM:
            ConvertSTS(val->cenmval);
            chan->_nostr = val->cenmval.no_str;
            chan->_strs.clear();
            for(int i=0; i<val->cenmval.no_str; i++)
                chan->_strs += val->cenmval.strs[i];
            ConvertValue(val->cenmval,dbr_enum_t)
            break;
        case DBR_CTRL_CHAR:
            ConvertSTS(val->cchrval);
            ConvertCtrl(val->cchrval);
            ConvertValue(val->cchrval,dbr_char_t)
            break;
        case DBR_CTRL_LONG:
            ConvertSTS(val->clngval);
            ConvertCtrl(val->clngval);
            ConvertValue(val->clngval,dbr_long_t)
            break;
        case DBR_CTRL_DOUBLE:
            ConvertSTS(val->cdblval);
            ConvertCtrl(val->cdblval);
            ConvertValue(val->cdblval,dbr_double_t)
            chan->_precision = val->cdblval.precision;
            break;
        }

        // Set to channel
        chan->setValue(value);
    }
    chan->_getCallbacked = true;
    epicsMutexUnlock(chan->_mutex); 
    if (chan->monitored())
        chan->emit valueChanged();
}

long PvObject::ensureConnection()
{
    // Not ever connected
    if (_chid == 0)
        return ECA_BADCHID;

    // Closed connection
    channel_state state = ca_state(_chid);
    if(state == cs_closed)
        return ECA_DISCONN;

    // Ensure connection
    if(state != cs_conn)
    {
        int status = ca_poll();
        if(status != ECA_NORMAL)
            return status;
    }

    // No connection at all
    if(ca_state(_chid) != cs_conn)
        return ECA_DISCONN;

    return ECA_NORMAL;
}
