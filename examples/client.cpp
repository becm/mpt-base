/*!
 * instance of MPT client
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(client.h)
#include MPT_INCLUDE(output.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

class MyClient : public mpt::client
{
public:
    MyClient();
    virtual ~MyClient() { }
    
    void unref();
    int step(mpt::metatype *);
    int log(const char *, const char *, ...);
    
protected:
    mpt::Reference<mpt::logger> _log;
};
MyClient::MyClient()
{
    mpt::output *o = mpt::mpt_output_new();

    o->set("history", "/dev/stdout", 0);
    o->set("level", "info", 0);
    
    _log.setPointer(mpt::mpt_output_logger(o));
}
int MyClient::log(const char *fcn, const char *fmt, ...)
{
    mpt::logger *l;
    if (!(l = _log.pointer())) {
        return -1;
    }
    else {
        va_list ap;
        int ret;
        va_start(ap, fmt);
        ret = l->log(fcn, LogLevel | mpt::LogFile, fmt, ap);
        va_end(ap);
        return ret;
    }
}

int MyClient::step(mpt::metatype *)
{ return 0; }

void MyClient::unref()
{ delete this; }

int main()
{
    mtrace();
    MyClient *c = new MyClient;

    c->log(__func__, "%s = %i", "value", 5);

    c->init();
    c->unref();
}
