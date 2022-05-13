#include <string.h>
#include <err.h>

#include "encoding.h"
#include "maru/maru.h"

/* encodes an int */
int encode_long(long value, void *data, long maxlen)
{
    if (maxlen > sizeof(long))
	{
	    memcpy(data, &value, sizeof(long));
	    return sizeof(long);
	} else
	    return -1;
}

/* decodes an long */
 int decode_long(long *value, void *data, long len)
{
    if (len >= sizeof(long)) {
	memcpy(value, data, sizeof(long));
	return sizeof(long);
    } else
	return -1;
}

/* encodes an int */
 int encode_int(int value, void *data, int maxlen)
{
    if (maxlen > sizeof(int))
	{
	    memcpy(data, &value, sizeof(int));
	    return sizeof(int);
	} else
	    return -1;
}

/* decodes an int */
 int decode_int(int *value, void *data, int len)
{
    if (len >= sizeof(int)) {
	memcpy(value, data, sizeof(int));
	return sizeof(int);
    } else
	return -1;
}

/* encodes opaque binary data */
 int encode_raw(void *value, void *data, int maxlen, int datalen)
{
    ENCODE(int, datalen);
    if (maxlen - datalen > 0)
	{
	    memcpy(data, value, datalen);
	    return datalen;
	}
    else
	return -1;
}

/* decodes opaque binary data */
 int decode_raw(void **value, void *data, int len)
{
    int binlen;

    DECODE(int, &binlen);
    if (binlen >= 0 && len - binlen >= 0) {
	*value = data;
	return binlen;
    } else
	return -1;
}

/* encodes a string */
 int encode_string(char *value, void *data, int maxlen)
{
    return encode_raw(value, data, maxlen, strlen(value)+1);
}

/* decodes a string. string has to be zero terminated */
 int decode_string(char **value, void *data, int len)
{
    int slen;

    DECODE(int, &slen);
    if (slen >= 0 && len - slen >= 0) {
	*value = data;
	if ((*value)[slen - 1] != '\0') {
	    warnx("decode error: string is missing terminating " \
		  "null character");
	    return -1;
	}
	return slen;
    } else
	return -1;
}
