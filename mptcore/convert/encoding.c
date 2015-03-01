/*!
 * association of encoding name and type
 */

#include <stdint.h>
#include <string.h>
#include <strings.h>

#include "convert.h"

static const struct {
	const char name[31];
	uint8_t    id;
} _encodings[] = {
	{ "",         0 },
	{ "none",     0 },
	
	{ "command",     MPT_ENUM(EncodingCommand) },
	
	{ "cobs",        MPT_ENUM(EncodingCobs) },
	{ "cobs/r",      MPT_ENUM(EncodingCobsInline) },
	
	{ "cobs/zpe",    MPT_ENUM(EncodingCompress) },
	{ "cobs/c",      MPT_ENUM(EncodingCompress) },
	
	{ "cobs/zpe+r",  MPT_ENUM(EncodingCompress) | MPT_ENUM(EncodingCobsInline) }
};

/*!
 * \ingroup mptConvert
 * \brief type of encoding
 * 
 * Associated type of encoding.
 * 
 * \param name target output descriptor
 * \param len  name length
 */
extern int mpt_encoding_value(const char *name, int len)
{
	static const int max = sizeof(_encodings)/sizeof(*_encodings);
	int i;
	
	if (!name) {
		return 0;
	}
	if (len < 0) {
		len = strlen(name);
	}
	for (i = 0; i < max; ++i) {
		if (strlen(_encodings[i].name) == (size_t) len
		    && !strncasecmp(name, _encodings[i].name, len)) {
			return _encodings[i].id;
		}
	}
	return -2;
}
/*!
 * \ingroup mptConvert
 * \brief name for encoding
 * 
 * Associated name for encoding type.
 * 
 * \param type encoding type
 * 
 * \return name of encoding
 */
extern const char *mpt_encoding_type(int type)
{
	static const int max = sizeof(_encodings)/sizeof(*_encodings);
	int i;
	
	for (i = 0; i < max; ++i) {
		if (((int) _encodings[i].id) == type) {
			return _encodings[i].name;
		}
	}
	return 0;
}
