#ifndef FIXEDINT_H
#define FIXEDINT_H

#include <boost/cstdint.hpp>

typedef boost::uint64_t u64;
typedef boost::int64_t i64;
typedef boost::int32_t i32;

/*
    helper functions
*/
inline u64 load_3(const unsigned char *in) {
    u64 result;

    result = (u64) in[0];
    result |= ((u64) in[1]) << 8;
    result |= ((u64) in[2]) << 16;

    return result;
}

inline u64 load_4(const unsigned char *in) {
    u64 result;

    result = (u64) in[0];
    result |= ((u64) in[1]) << 8;
    result |= ((u64) in[2]) << 16;
    result |= ((u64) in[3]) << 24;
    
    return result;
}

inline i64 shift_left(i64 v, int s) {
	return i64(u64(v) << s);
}

#endif //FIXEDINT_H