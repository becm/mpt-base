/*!
 * instance of MPT client
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include <iostream>

#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(client.h)

#include MPT_INCLUDE(stream.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

class MyClient : public mpt::client, public mpt::proxy
{
public:
    MyClient();
    virtual ~MyClient() { }
    
    void unref() __MPT_OVERRIDE;
    int process(uintptr_t , mpt::iterator *) __MPT_OVERRIDE;
    
    template<typename T>
    T *cast()
    {
        return proxy::cast<T>();
    }
};
MyClient::MyClient()
{
    _ref = mpt::mpt_output_remote();

    mpt::object *o;
    if ((o = proxy::cast<mpt::object>())) {
        o->set(0, "w:client.out");
    }
}
int MyClient::process(uintptr_t , mpt::iterator *)
{
    return mpt::event::Terminate;
}
void MyClient::unref()
{
    delete this;
}

static int doCommand(void *ptr, mpt::event *ev)
{
    auto *c = static_cast<mpt::client *>(ptr);
    if (!ev) {
        return 0;
    }
    if (!ev->msg) {
        return c->dispatch(ev);
    }
    std::cout << ev->msg->length() << std::endl;
    return 0;
}
int main(int argc, char * const argv[])
{
    mtrace();


    int pos;
    if ((pos = mpt::mpt_init(argc, argv)) < 0) {
        perror("mpt init");
        return 1;
    }
    mpt::notify n;
    mpt::dispatch d;
    
    MyClient c;
    d.set(mpt::msgtype::Command, doCommand, &c);
    
    const mpt::metatype *mt = mpt::mpt_config_get(0, "mpt.input", '.', 0);
    mpt::input *in;
    if (!mt) {
        d.setDefault(mpt::msgtype::Command);
    }
    else if (!(in = mt->cast<mpt::input>()) || !in->addref()) {
        std::cerr << "input reference" << std::endl;
        return 1;
    }
    else if (!n.add(in)) {
        perror("notify add");
        return 2;
    }
    n.setDispatch(&d);
    n.loop();
    
    c.log(__func__, mpt::logger::Debug, "%s = %i", "value", 5);
    
    if ((mt = mpt::mpt_config_get(0, "mpt.args", '.', 0))) {
        mpt::consumable v(*mt);
        const char *arg;
        while (v.consume(arg)) {
            std::cerr << arg << std::endl;
        }
    }
}
