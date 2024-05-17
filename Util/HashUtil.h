#ifndef _AT_UTIL_HASH_UTIL_H_
#define _AT_UTIL_HASH_UTIL_H_

namespace AT::Util::Hash {
	template <class T> inline void hash_combine(std::size_t& s, const T& v)
	{
		std::hash<T> h;
		s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
	}
};
#endif