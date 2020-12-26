/*!
 * instance of MPT client
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include <iostream>

#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(client.h)
#include MPT_INCLUDE(message.h)
#include MPT_INCLUDE(object.h)

#include MPT_INCLUDE(notify.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

class client : public mpt::client
{
public:
	client();
	virtual ~client() { }
	
	int convert(int , void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	client *clone() const __MPT_OVERRIDE;
	
	int process(uintptr_t , mpt::iterator *) __MPT_OVERRIDE;
protected:
	mpt::reference<mpt::metatype> _mt;
};
client::client()
{
	mpt::metatype *mt = mpt::mpt_output_remote();
	mpt::property pr;
	mpt::mpt_convertable_info(mt, &pr);
	mpt::debug(__func__, "%s: %s", pr.name, pr.desc);
	mpt::object *o;
	if (mt && (o = mpt::typecast<mpt::object>(*mt))) {
		o->set(0, "w:client.out");
	}
	_mt.set_instance(mt);
}
int client::convert(int type, void *ptr)
{
	metatype *mt;
	int ret;
	if ((mt = _mt.instance()) && (ret = mt->convert(type, ptr)) > 0) {
		return mpt::type_properties<mpt::client *>::id();
	}
	return client::convert(type, ptr);
}
void client::unref()
{
	delete this;
}
client *client::clone() const
{
	return new client();
}
int client::process(uintptr_t , mpt::iterator *)
{
	return mpt::event::Terminate;
}

static int do_command(void *ptr, mpt::event *ev)
{
	mpt::client *c = static_cast<mpt::client *>(ptr);
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
	if (mpt::mpt_notify_config(&n, 0) < 0) {
		return 2;
	}
	client c;
	mpt::dispatch d;
	d.set_handler(mpt::msgtype::Command, do_command, &c);
	n.set_handler(&d);
	mpt::mpt_dispatch_param(&d);
	
	n.loop();
	
	mpt::log(&c, __func__, mpt::logger::Debug, "%s = %i", "value", 5);
	
	mpt::convertable *val;
	if ((val = mpt::typecast<mpt::config>(*mpt::config::global())->get("mpt.args"))) {
		mpt::iterator *it = mpt::typecast<mpt::iterator>(*val);
		const char *arg;
		while (it->consume(arg)) {
			std::cerr << arg << std::endl;
		}
	}
}
