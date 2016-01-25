#ifndef __GALILEO_BASE_MISC_H__
#define __GALILEO_BASE_MISC_H__

template<typename T>
T swap_bytes(T x) {
  T y;
  for (size_t i = 0, j = sizeof(T) - 1; j >= 0; ++i, --j) {
    reinterpret_cast<uint8_t *>(&y)[i] = reinterpret_cast<uint8_t *>(&x)[j];
  }
  return y;
}

#endif // !__GALILEO_BASE_MISC_H__
