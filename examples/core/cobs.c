/*!
 * test COBS encoding compound
 */

#include <stdio.h>

#include <sys/uio.h>

#include <mpt/array.h>
#include <mpt/convert.h>

#define ENCODE mpt_encode_cobs
#define DECODE mpt_decode_cobs

/* hex dump of data */
static void disp(MPT_STRUCT(array) *arr, size_t len)
{
	uint8_t *base;
	size_t i;
	
	base = (void *) (arr->_buf + 1);
	
	for (i = 0; i < len; i++) {
		fprintf(stdout, "%02x ", base[i]);
	}
	fputc('\n',stdout);
}
/* decode and display next message */
static void dec(MPT_STRUCT(array) *arr, MPT_STRUCT(codestate) *info)
{
	struct iovec vec;
	ssize_t len;
	
	vec.iov_base = arr->_buf + 1;
	vec.iov_len  = arr->_buf->used;
	DECODE(info, &vec, 0);
	len = DECODE(info, &vec, 1);
	fputc('>',stdout);
	fputc(' ',stdout);
	disp(arr, len);
	DECODE(info, 0, 0);
}

int main()
{
	struct {
		uint8_t len, data[255];
	} msg[] = {
	  { 1, { 0x0 } },
	  { 4, { 0x11, 0x22, 0x00, 0x33 } },
	  { 4, { 0x11, 0x00, 0x00, 0x00 } }
	};
	uint8_t big[255];
	MPT_STRUCT(codestate) info = MPT_CODESTATE_INIT;
	MPT_STRUCT(array) arr = MPT_ARRAY_INIT;
	struct iovec vec;
	int i, max = sizeof(msg)/sizeof(*msg);
	
	puts("separate");
	for (i = 0; i < max; i++) {
		vec.iov_base = msg[i].data;
		vec.iov_len  = msg[i].len;
		mpt_array_push(&arr, &info, ENCODE, &vec);
		mpt_array_push(&arr, &info, ENCODE, 0);
		fputc('<',stdout);
		fputc(' ',stdout);
		disp(&arr, info.done);
		ENCODE(&info, 0, 0);
		
		dec(&arr, &info);
		
		arr._buf->used = 0;
	}
	puts("joined");
	for (i = 0; i < max; i++) {
		vec.iov_base = msg[i].data;
		vec.iov_len  = msg[i].len;
		mpt_array_push(&arr, &info, ENCODE, &vec);
	}
	mpt_array_push(&arr, &info, ENCODE, 0);
	fputc('<',stdout);
	fputc(' ',stdout);
	disp(&arr, info.done);
	ENCODE(&info, 0, 0);
	
	dec(&arr, &info);
	
	arr._buf->used = 0;
	
	puts("big msg");
	max = sizeof(big);
	for (i = 0; i < max; i++) big[i] = i+1;
	vec.iov_base = big;
	vec.iov_len  = max;
	mpt_array_push(&arr, &info, ENCODE, &vec);
	mpt_array_push(&arr, &info, ENCODE, 0);
	fputc('<',stdout);
	fputc(' ',stdout);
	disp(&arr, info.done);
	ENCODE(&info, 0, 0);
	
	dec(&arr, &info);
	
	arr._buf->used = 0;
	
	return 0;
}
