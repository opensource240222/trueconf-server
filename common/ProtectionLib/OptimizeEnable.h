// NOTE: No include guard. This file can be included multiple time.

#if defined(ENABLE_ARMADILLO_BUILD)
// Disabling optimizations is only required for Armadillo
#if defined(_MSC_VER)
#pragma optimize("", on)
#pragma warning(pop)
#endif
#endif