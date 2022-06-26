#include "std-generic/cpplib/VS_Container.h"

#include "VS_UtilsLib.h"
#include "VS_PublicKeyCrypt.h"
#include "VS_Certificate.h"
#include "VS_SecureConstants.h"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

char * VS_Base64EncodeAlloc(const void *in_buf, const uint32_t in_sz, uint32_t &out_sz)
{
	BIO *bmem(0), *b64(0);
	BUF_MEM *bptr(0);
	if(!(b64 = BIO_new(BIO_f_base64())))
		return 0;
	if(!(bmem = BIO_new(BIO_s_mem())))
	{
		BIO_free_all(b64);
		return 0;
	}
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, in_buf, in_sz);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);
	char *buff = static_cast<char *>(malloc(bptr->length));
    memcpy(buff, bptr->data, bptr->length);
	out_sz = bptr->length;
	//buff[bptr->length-1] = 0;
	BIO_free_all(b64);
	return buff;
}

void * VS_Base64DecodeAlloc(void *in_buf, const uint32_t in_sz, uint32_t &out_sz)
{
	BIO *b64(0), *bmem(0);

	char *buffer = static_cast<char *>(malloc(in_sz));
	memset(buffer, 0, in_sz);
	if(!(b64 = BIO_new(BIO_f_base64())))
	{
		free(buffer);
		return 0;
	}
	bmem = BIO_new_mem_buf(in_buf, in_sz);
	if(!bmem)
	{
		BIO_free_all(b64);
		free(buffer);
		return 0;
	}
	bmem = BIO_push(b64, bmem);
	int res = BIO_read(bmem, buffer, in_sz);
	//BIO_free_all(bmem);
	BIO_free_all(b64);
	if(res<=0)
	{
		free(buffer);
		buffer =0;
		out_sz = 0;
	}
	else
		out_sz = res;

	return buffer;
}

