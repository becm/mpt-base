/*!*
 * output interface to Qt
 */

#include <limits>
#include <cstdio>

#include <unistd.h>

#include "meta.h"
#include "array.h"
#include "output.h"
#include "client.h"
#include "convert.h"

__MPT_NAMESPACE_BEGIN

static int print_message(const char *fcn, int type, const char *fmt, va_list va)
{
	logger *log;
	if ((log = logger::default_instance())) {
		return log->log(fcn, type | logger::LogFunction, fmt, va);
	}
	FILE *fd = (type & 0xff) ? stderr : stdout;
	if (fcn) {
		std::fputs(fcn, fd);
		std::fputs("(): ", fd);
	}
	if (fmt) {
		std::vfprintf(fd, fmt, va);
	}
	std::fputs(mpt_newline_string(0), fd);
	return 0;
}

// logger interfaces
/*!
 * \ingroup mptMeta
 * \brief log to metatype
 * 
 * Select and use log interface for message.
 * 
 * \param mt   target log metatype
 * \param fcn  originating location
 * \param type message type and flags
 * \param fmt  log arguments format string
 * 
 * \return log operation result
 */
int log(const metatype *mt, const char *fcn, int type, const char *fmt, ...)
{
	va_list va;
	if (fmt) va_start(va, fmt);
	int ret;
	if (mt) {
		ret = mpt_meta_vlog(mt, fcn, type | logger::LogFunction, fmt, va);
	} else {
		ret = print_message(fcn, type, fmt, va);
	}
	if (fmt) va_end(va);
	return ret;
}
int debug(const char *from, const char *fmt, ... )
{
	va_list va;
	if (fmt) va_start(va, fmt);
	int ret = print_message(from, logger::Debug, fmt, va);
	if (fmt) va_end(va);
	return ret;
}
int warning(const char *from, const char *fmt, ... )
{
	va_list va;
	if (fmt) va_start(va, fmt);
	int ret = print_message(from, logger::Warning, fmt, va);
	if (fmt) va_end(va);
	return ret;
}
int error(const char *from, const char *fmt, ... )
{
	va_list va;
	if (fmt) va_start(va, fmt);
	int ret = print_message(from, logger::Error, fmt, va);
	if (fmt) va_end(va);
	return ret;
}
int critical(const char *from, const char *fmt, ... )
{
	va_list va;
	if (fmt) va_start(va, fmt);
	int ret = print_message(from, logger::Critical, fmt, va);
	if (fmt) va_end(va);
	return ret;
}
int println(const char *fmt, ... )
{
	va_list va;
	if (fmt) va_start(va, fmt);
	int ret = print_message(0, 0, fmt, va);
	if (fmt) va_end(va);
	return ret;
}
logger *logger::default_instance()
{
	return mpt_log_default();
}
int logger::message(const char *from, int err, const char *fmt, ...)
{
	va_list va;
	if (fmt) va_start(va, fmt);
	int ret = log(from, err | LogFunction, fmt, va);
	if (fmt) va_end(va);
	return ret;
}

// logging store entry
logger::LogType message_store::entry::type() const
{
	header *h;
	
	if (length() < sizeof(*h)) {
		return logger::Message;
	}
	h = static_cast<header *>(base());
	return (logger::LogType) h->type;
}
const char *message_store::entry::source() const
{
	header *h;
	
	if (length() < (sizeof(*h) + 1)) {
		return 0;
	}
	h = static_cast<header *>(base());
	
	if (!h->from) {
		return 0;
	}
	return (const char *) (h + 1);
}
span<const char> message_store::entry::data(int part) const
{
	header *h = static_cast<header *>(base());
	const char *data;
	size_t skip, len = length();
	
	if ((len < sizeof(*h)) || ((len <= (skip = h->from + sizeof(*h))))) {
		return span<const char>(0, 0);
	}
	data = ((char *) h) + skip;
	len -= skip;
	
	while (part >= 0) {
		const char *end;
		
		if (!(end = (const char *) memchr(data, 0, len))) {
			if (part) {
				return span<const char>(0, 0);
			}
			break;
		}
		skip = end - data;
		if (!part) {
			len = skip;
			break;
		}
		++skip;
		
		data += skip;
		len  -= skip;
	}
	return span<const char>(data, len);
}


int message_store::entry::set(const char *from, int type, const char *fmt, va_list arg)
{
	header *h;
	array d;
	size_t len;
	
	if (!(h = static_cast<header *>(d.append(sizeof(*h))))) {
		return -1;
	}
	h->args = 0;
	h->from = 0;
	h->type = type;
	h->_cmd = 0;
	
	static const size_t maxlen = std::numeric_limits<__decltype(h->from)>::max();
	if (!from) {
		len = 0;
	}
	else if ((len = strlen(from)) >= maxlen) {
		static const char end = 0;
		if (!(d.append(maxlen, from)) || !(d.append(1, &end))) {
			return -1;
		}
		len = maxlen;
	} else {
		if (!(d.append(++len, from))) {
			return -1;
		}
	}
	h = static_cast<header *>(d.base());
	h->from = len;
	
	if (!fmt) {
		h->args = 0;
	}
	else if (mpt_vprintf(&d, fmt, arg) < 0) {
		return -1;
	} else {
		h = static_cast<header *>(d.base());
		h->args = 1;
	}
	
	mpt_array_clone(this, &d);
	
	return length();
}

message_store::message_store(metatype *next) : reference<metatype>(next), _act(0), _flags(FlowNormal), _ignore(Debug), _level(0)
{ }
message_store::~message_store()
{ }
static int log_meta(const metatype *mt, const char *from, int type, const char *fmt, va_list arg)
{
	logger *l;
	if (!mt) {
		if (!(l = mpt_log_default())) {
			return 0;
		}
		return l->log(from, type, fmt, arg);
	}
	if ((l = mt->cast<logger>())) {
		return l->log(from, type, fmt, arg);
	}
	output *o;
	if ((o = mt->cast<output>())) {
		return mpt_output_vlog(o, from, type, fmt, arg);
	}
	return BadType;
}
int message_store::log(const char *from, int type, const char *fmt, va_list arg)
{
	int save = 0, pass = 0, code = 0x7f & type;
	
	if (!code) {
		save |= _flags & SaveMessage;
		pass |= _flags & PassMessage;
	}
	else if (code >= _ignore) {
		save = (type & File) && (_flags & SaveLogAll);
		pass |= _flags & PassUnsaved;
	} else {
		save = 1;
		pass |= _flags & PassSaved;
	}
	if (type & File) {
		pass |= _flags & PassFile;
	}
	metatype *mt = instance();
	// fast-track without argument list copy
	if (!save) {
		if (mt && pass) {
			return log_meta(mt, from, type, fmt, arg);
		}
		return 0;
	}
	int ret;
	if (mt && pass) {
		va_list tmp;
		// use copy to keep data for storage operation
		va_copy(tmp, arg);
		ret = log_meta(mt, from, type, fmt, arg);
		va_end(tmp);
	}
	entry m;
	
	if (code && code < _level) _level = code;
	
	if ((ret = m.set(from, type, fmt, arg)) < 0) {
		return ret;
	}
	if (!_msg.insert(_msg.length(), m)) {
		return -1;
	}
	return ret;
}

const message_store::entry *message_store::next()
{
	entry *e;
	if ((e = _msg.get(_act))) {
		++_act;
	}
	return e;
}

void message_store::clear()
{
	_msg.resize(0);
	_act = 0;
	_level = 0;
}

bool message_store::set_ignore_level(int val)
{
	if (val < 0) {
		val = Debug;
	}
	else if (val >= File) {
		return false;
	}
	_ignore = val;
	return true;
}
bool message_store::set_flow_flags(int val)
{
	_flags = val;
	return true;
}

__MPT_NAMESPACE_END
