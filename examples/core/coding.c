/*!
 * test COBS encoding compound
 */

#include <stdio.h>
#include <assert.h>
#include <inttypes.h>

#include <sys/uio.h>

#include <mpt/array.h>
#include <mpt/convert.h>

/* hex dump of data */
static void disp(MPT_STRUCT(array) *arr, size_t len)
{
	uint8_t *base;
	
	base = (void *) (arr->_buf + 1);
	
	while (len) {
		size_t i, line;
		line = len > 16 ? 16 : len;
		for (i = 0; i < line; i++) {
			fprintf(stdout, "%02x ", base[i]);
		}
		fputc('\n',stdout);
		base += line;
		len -= line;
	}
}
static void enc(MPT_STRUCT(array) *arr, struct iovec *data, MPT_TYPE(DataEncoder) encode)
{
	MPT_STRUCT(codestate) info = MPT_CODESTATE_INIT;
	
	mpt_array_push(arr, &info, encode, data);
	mpt_array_push(arr, &info, encode, 0);
	fputc('<',stdout);
	fputc(' ',stdout);
	disp(arr, info.done);
	encode(&info, 0, 0);
}

/* decode and display next message */
static void dec(MPT_STRUCT(array) *arr, MPT_TYPE(DataDecoder) decode)
{
	MPT_STRUCT(codestate) info = MPT_CODESTATE_INIT;
	struct iovec vec;
	ssize_t len;
	
	vec.iov_base = arr->_buf + 1;
	vec.iov_len  = arr->_buf->used;
	/*decode(info, &vec, 0);*/
	while ((len = decode(&info, &vec, 1)) < 0) {
		if (len == MPT_ERROR(MissingBuffer)) {
			assert(mpt_array_insert(arr, info.done + info.scratch, 8));
			vec.iov_base = (void *) (arr->_buf + 1);
			vec.iov_len  = arr->_buf->used;
			info.scratch += 8;
			continue;
		}
		fprintf(stderr, "%s %"__PRIPTR_PREFIX"d\n", "error", len);
		return;
	}
	fputc('>',stdout);
	fputc(' ',stdout);
	disp(arr, len);
	decode(&info, 0, 0);
}

int main(int argc, char * const argv[])
{
	struct {
		uint8_t len, data[255];
	} msg[] = {
	  { 1, { 0x0 } },
	  { 4, { 0x11, 0x22, 0x00, 0x33 } },
	  { 9, { 0x11, 0x00, 0x00, 0x22, 0x00, 0x00, 0x33, 0x00, 0x00 } }
	};
	MPT_STRUCT(codestate) info = MPT_CODESTATE_INIT;
	int type = MPT_ENUM(EncodingCobs);
	MPT_TYPE(DataDecoder) decode;
	MPT_TYPE(DataEncoder) encode;
	
	MPT_STRUCT(array) arr = MPT_ARRAY_INIT;
	struct iovec vec;
	int i, max = sizeof(msg)/sizeof(*msg);
	uint8_t big[255];
	
	if (argc-- < 2) {
		fprintf(stderr, "%s: %s\n", *argv, "{encoding}");
	}
	++argv;
	
	while (argc--) {
		if ((type = mpt_encoding_value(*argv++, -1)) < 0) {
			fprintf(stderr, "%s: %s\n", "bad encoding", argv[-1]);
			continue;
		}
		if (!(decode = mpt_message_decoder(type))
		    || !(encode = mpt_message_encoder(type))) {
			fputs("unsupported encoding\n", stderr);
			return 2;
		}
		puts("separate");
		for (i = 0; i < max; i++) {
			vec.iov_base = msg[i].data;
			vec.iov_len  = msg[i].len;
			enc(&arr, &vec, encode);
			dec(&arr, decode);
			
			arr._buf->used = 0;
		}
		puts("joined");
		for (i = 0; i < max; i++) {
			vec.iov_base = msg[i].data;
			vec.iov_len  = msg[i].len;
			mpt_array_push(&arr, &info, encode, &vec);
		}
		mpt_array_push(&arr, &info, encode, 0);
		fputc('<',stdout);
		fputc(' ',stdout);
		disp(&arr, info.done);
		encode(&info, 0, 0);
		info.done = info.scratch = 0;
		
		dec(&arr, decode);
		
		arr._buf->used = 0;
		
		puts("big msg");
		max = sizeof(big);
		for (i = 0; i < max; i++) big[i] = i+1;
		vec.iov_base = big;
		vec.iov_len  = max;
		enc(&arr, &vec, encode);
		dec(&arr, decode);
		
		arr._buf->used = 0;
	}
	return 0;
}
