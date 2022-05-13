extern  int encode_long(long value, void *data, long maxlen);
extern  int decode_long(long *value, void *data, long len);
extern  int encode_int(int value, void *data, int maxlen);
extern  int decode_int(int *value, void *data, int len);
extern  int encode_raw(void *value, void *data, int maxlen, int datalen);
extern  int decode_raw(void **value, void *data, int len);
extern  int encode_string(char *value, void *data, int maxlen);
extern  int decode_string(char **value, void *data, int len);

#define ENCODING_OFFSET_int	0
#define ENCODING_OFFSET_long 0
#define ENCODING_OFFSET_string	sizeof(int)

/* macros for encoding */
#define ENCODE(type, value) do { \
    int result = encode_##type(value, data, maxlen); \
    if (result < 0) \
	return -1; \
    result += ENCODING_OFFSET_##type; \
    maxlen -= result; \
    data += result; \
} while(0);

#define ENCODE_RAW(value, dlen) do { \
    int result = encode_raw(value, data, maxlen, dlen); \
    if (result < 0) \
	return -1; \
    result += sizeof(int); \
    maxlen -= result; \
    data += result; \
} while(0);

/* macros for decoding */
#define DECODE(type, value) do { \
    int result = decode_##type(value, data, len); \
    if (result < 0) \
	return -1; \
    result += ENCODING_OFFSET_##type; \
    len -= result; \
    data += result; \
} while(0);

#define DECODE_RAW(value) do { \
    int result = decode_raw(value, data, len); \
    if (result < 0) \
	return -1; \
    result += sizeof(int); \
    len -= result; \
    data += result; \
} while(0);
