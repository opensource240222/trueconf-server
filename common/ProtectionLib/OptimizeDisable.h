// NOTE: No include guard. This file can be included multiple time.

#if defined(ENABLE_ARMADILLO_BUILD)
// Disabling optimizations is only required for Armadillo
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4748)
#pragma optimize("", off)
#endif
#endif