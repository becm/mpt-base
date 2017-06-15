/*!
 * test COBS encoding compound
 */

#include <stdio.h>
#include <assert.h>
#include <inttypes.h>

#include <sys/uio.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(array.h)
#include MPT_INCLUDE(convert.h)

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
static void enc(MPT_STRUCT(encode_array) *arr, size_t len, const void *data)
{
	mpt_array_push(arr, len, data);
	mpt_array_push(arr, 0, 0);
	fputc('<',stdout);
	fputc(' ',stdout);
	disp(&arr->_d, arr->_state.done);
	arr->_enc(&arr->_state, 0, 0);
}

/* decode and display next message */
static void dec(MPT_STRUCT(array) *arr, MPT_TYPE(DataDecoder) decode)
{
	MPT_STRUCT(decode_state) info = MPT_DECODE_INIT;
	struct iovec vec;
	int len;
	
	vec.iov_base = arr->_buf + 1;
	vec.iov_len  = arr->_buf->_used;
	/*decode(info, &vec, 0);*/
	while ((len = decode(&info, &vec, 1)) < 0) {
		if (len == MPT_ERROR(MissingBuffer)) {
			assert(mpt_array_insert(arr, info.done + info.scratch, 8));
			vec.iov_base = (void *) (arr->_buf + 1);
			vec.iov_len  = arr->_buf->_used;
			info.scratch += 8;
			continue;
		}
		fprintf(stderr, "%s %d\n", "error", len);
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
	int type = MPT_ENUM(EncodingCobs);
	MPT_TYPE(DataDecoder) decode;
	
	MPT_STRUCT(encode_array) arr = MPT_ENCODE_ARRAY_INIT;
	int i, max = sizeof(msg)/sizeof(*msg);
	uint8_t big[255];
	
	if (argc-- < 2) {
		fprintf(stderr, "%s: %s\n", *argv, "{encoding}");
		return 1;
	}
	++argv;
	
	while (argc--) {
		if ((type = mpt_encoding_value(*argv++, -1)) < 0) {
			fprintf(stderr, "%s: %s\n", "bad encoding", argv[-1]);
			continue;
		}
		if (!(decode = mpt_message_decoder(type))
		    || !(arr._enc = mpt_message_encoder(type))) {
			fputs("unsupported encoding\n", stderr);
			return 2;
		}
		puts("separate");
		for (i = 0; i < max; i++) {
			enc(&arr, msg[i].len, msg[i].data);
			dec(&arr._d, decode);
			
			mpt_buffer_cut(arr._d._buf, 0, 0);
		}
		puts("joined");
		for (i = 0; i < max; i++) {
			mpt_array_push(&arr, msg[i].len, msg[i].data);
		}
		mpt_array_push(&arr, 0, 0);
		fputc('<',stdout);
		fputc(' ',stdout);
		disp(&arr._d, arr._state.done);
		
		dec(&arr._d, decode);
		
		arr._enc(&arr._state, 0, 0);
		mpt_buffer_cut(arr._d._buf, 0, 0);
		
		puts("big msg");
		max = sizeof(big);
		for (i = 0; i < max; i++) big[i] = i+1;
		enc(&arr, max, big);
		dec(&arr._d, decode);
		
		arr._state.done = arr._state.scratch = 0;
		mpt_buffer_cut(arr._d._buf, 0, 0);
	}
	return 0;
}
