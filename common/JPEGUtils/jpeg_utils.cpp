#include "jpeg_utils.h"

#include <cassert>

#include "std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/compat/memory.h"

// Hack to make our code compatible with the version of libjpeg-turbo
// distributed alongside WebRTC.
#ifdef _WIN32
#define boolean int
#define HAVE_BOOLEAN
#endif

#include "third_party/libjpeg_turbo/jpeglib.h"
typedef boolean jpeg_boolean;

#ifdef _WIN32
#undef HAVE_BOOLEAN
#undef boolean
#endif

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

//NOTE: see manual by jpeg_turbo https://raw.githubusercontent.com/libjpeg-turbo/libjpeg-turbo/master/libjpeg.txt

static inline void read_RGB_scanlines(uint8_t *out, struct jpeg_decompress_struct &cinfo) noexcept
{
	assert(out);

	/* Here we use the library's state variable cinfo.output_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 */
	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could ask for
		* more than one scanline at a time if that's more convenient.
		*/
		std::uint8_t *rowptr = out + cinfo.output_scanline * cinfo.image_width * cinfo.output_components;
		(void)jpeg_read_scanlines(&cinfo, &rowptr, 1);
	}
}

typedef void (*read_scanlines_exec_ptr)(uint8_t */*out*/, struct jpeg_decompress_struct &/*cinfo*/);

//https://github.com/LuaDist/libjpeg/blob/master/example.c
static bool read_JPEG_file(const char *filename, std::unique_ptr<uint8_t[]> &out, unsigned &width, unsigned &height, int outputComponents, J_COLOR_SPACE outColorSpace, read_scanlines_exec_ptr exec) noexcept
{
	assert(filename);
	assert(exec);

	static_assert(std::is_same<uint8_t, JSAMPLE>::value, "!");

	/* This struct contains the JPEG decompression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 */
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	 * Note that this struct must live as long as the main JPEG parameter
	 * struct, to avoid dangling-pointer problems.
	 */
	 /* More stuff */
	FILE *infile;		/* source file */

	/* In this example we want to open the input file before doing anything else,
	 * so that the setjmp() error recovery below can assume the file is open.
	 * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	 * requires it in order to read binary files.
	 */

	if ((infile = ::fopen(filename, "rb")) == NULL) {
		dstream1 << "can't open the file: " << filename;
		return false;
	}

	/* Step 1: allocate and initialize JPEG decompression object */
	struct jpeg_error_mgr jerr;

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = [](j_common_ptr cinfo)
	{
		throw cinfo;
	};

	VS_SCOPE_EXIT
	{
		::fclose(infile);

	/* Step 8: Release JPEG decompression object */
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);
	};

	try
	{
		/* Now we can initialize the JPEG decompression object. */
		jpeg_create_decompress(&cinfo);

		/* Step 2: specify data source (eg, a file) */

		jpeg_stdio_src(&cinfo, infile);

		/* Step 3: read file parameters with jpeg_read_header() */

		(void)jpeg_read_header(&cinfo, TRUE);
		/* We can ignore the return value from jpeg_read_header since
		 *   (a) suspension is not possible with the stdio data source, and
		 *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
		 * See libjpeg.txt for more info.
		 */

		 /* Step 4: set parameters for decompression */

		 /* In this example, we don't need to change any of the defaults set by
		  * jpeg_read_header(), so we do nothing here.
		 */

		 //https://stackoverflow.com/questions/5619810/libjpeg-decoding-to-bgr
		cinfo.out_color_space = outColorSpace;
		cinfo.num_components = outputComponents;

		//https://chromium.googlesource.com/chromium/chromium/+/master/ui/gfx/codec/jpeg_codec.cc#478
		jpeg_calc_output_dimensions(&cinfo);

		width = cinfo.output_width;
		height = cinfo.output_height;

		out = vs::make_unique<std::uint8_t[]>(width * height * cinfo.output_components);

		/* Step 5: Start decompressor */

		(void)jpeg_start_decompress(&cinfo);
		/* We can ignore the return value since suspension is not possible
		 * with the stdio data source.
		 */

		 /* Step 6: while (scan lines remain to be read) */
		(*exec)(out.get(), cinfo);

		/* Step 7: Finish decompression */
		(void)jpeg_finish_decompress(&cinfo);
		/* We can ignore the return value since suspension is not possible
		 * with the stdio data source.
		 */

	}
	catch (j_common_ptr cinfoErr)
	{
		char jpeg_last_error_msg[JMSG_LENGTH_MAX];
		(*cinfoErr->err->format_message)(cinfoErr, jpeg_last_error_msg);
		dstream1 << "jpeg error: " << jpeg_last_error_msg;
		return false;
	}

	/* And we're done! */
	return true;
}

static inline void write_RGB_scanlines(const uint8_t *in, struct jpeg_compress_struct &cinfo) noexcept
{
	while (cinfo.next_scanline < cinfo.image_height)
	{
		auto rowptr = const_cast<std::uint8_t *>(in + cinfo.next_scanline * cinfo.image_width * cinfo.input_components);
		(void)jpeg_write_scanlines(&cinfo, &rowptr, 1);
	}
}

static inline void write_I420_scanlines(const uint8_t *in, struct jpeg_compress_struct &cinfo) noexcept
{
	static_assert(std::is_same<JSAMPLE, uint8_t>::value);

	const auto w = cinfo.image_width;
	const auto h = cinfo.image_height;

	std::vector<JSAMPLE> tmpbuf(w * 3);
	const size_t total_pixels = w * h;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		JSAMPROW tmp = &tmpbuf[0];
		JSAMPROW row_pointer = tmp;
		const size_t iy = cinfo.next_scanline;
		for (size_t ix = 0; ix < w; ix++) // unpack I420
		{
			// Formulae: https://en.wikipedia.org/wiki/YUV#Y%E2%80%B2UV420p_(and_Y%E2%80%B2V12_or_YV12)_to_RGB888_conversion
			const JSAMPLE Y = in[iy * w + ix];
			const JSAMPLE U = in[(iy / 2) * (w / 2) + (ix / 2) + total_pixels];
			const JSAMPLE V = in[(iy / 2) * (w / 2) + (ix / 2) + total_pixels + (total_pixels / 4)];

			const size_t row_offset = ix * 3;
			tmp[row_offset + 0] = Y;
			tmp[row_offset + 1] = U;
			tmp[row_offset + 2] = V;
		}
		(void)jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}
}

typedef void(*write_scanlines_exec_ptr)(const uint8_t * /*in*/, struct jpeg_compress_struct & /*cinfo*/);

static bool write_JPEG_mem(const uint8_t *in, std::vector<uint8_t> &out, unsigned width, unsigned height, int inputComponents, int quality, J_COLOR_SPACE inColorSpace, write_scanlines_exec_ptr exec) noexcept
{
	assert(in != NULL);
	assert(exec);

	static_assert(std::is_same<uint8_t, JSAMPLE>::value, "!");

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	VS_SCOPE_EXIT
	{
		jpeg_destroy_compress(&cinfo);
	};

	cinfo.err = jpeg_std_error(&jerr);

	// By default libjpeg(-turbo) calls exit(). We should avoid it.
	jerr.error_exit = [](j_common_ptr cinfo)
	{
		throw cinfo;
	};

	try
	{
		jpeg_create_compress(&cinfo);

		if (cinfo.dest == NULL) {
			cinfo.dest = reinterpret_cast<jpeg_destination_mgr *>(
				(*cinfo.mem->alloc_small)(
					reinterpret_cast<j_common_ptr>(&cinfo),
					JPOOL_PERMANENT,
					sizeof(jpeg_destination_mgr)));
		}

		assert(cinfo.dest != NULL);

		cinfo.client_data = &out;

		cinfo.dest->init_destination = [](j_compress_ptr cinfo) -> void
		{
			std::vector<uint8_t> &vec = *reinterpret_cast<std::vector<uint8_t> *>(cinfo->client_data);
			vec.resize(65536);
			cinfo->dest->next_output_byte = &vec[0];
			cinfo->dest->free_in_buffer = vec.size();
		};

		cinfo.dest->empty_output_buffer = [](j_compress_ptr cinfo) -> jpeg_boolean
		{
			std::vector<uint8_t> &vec = *reinterpret_cast<std::vector<uint8_t> *>(cinfo->client_data);
			size_t oldsize = vec.size();
			vec.resize(oldsize + 65536);
			cinfo->dest->next_output_byte = &vec[oldsize];
			cinfo->dest->free_in_buffer = vec.size() - oldsize;

			return TRUE;
		};

		cinfo.dest->term_destination = [](j_compress_ptr cinfo) -> void
		{
			std::vector<uint8_t> &vec = *reinterpret_cast<std::vector<uint8_t> *>(cinfo->client_data);
			vec.resize(vec.size() - cinfo->dest->free_in_buffer);
		};

		/* First we supply a description of the input image.
		 * Four fields of the cinfo struct must be filled in:
		 */
		cinfo.image_width = width; 	/* image width and height, in pixels */
		cinfo.image_height = height;
		cinfo.input_components = inputComponents;	 /* # of color components per pixel */
		cinfo.in_color_space = inColorSpace; 	/* colorspace of input image */
		/* Now use the library's routine to set default compression parameters.
		 * (You must set at least cinfo.in_color_space before calling this,
		 * since the defaults depend on the source color space.)
		 */

		jpeg_set_defaults(&cinfo);

		if (quality > 0)
		{
			int q;
			if (quality > 100)
			{
				q = 100;
			}
			else
			{
				q = static_cast<int>(quality);
			}

			jpeg_set_quality(&cinfo, q, TRUE);
		}

		jpeg_start_compress(&cinfo, TRUE);

		(*exec)(in, cinfo);

		jpeg_finish_compress(&cinfo);
	}
	catch (j_common_ptr cinfoErr)
	{
		char jpeg_last_error_msg[JMSG_LENGTH_MAX];
		(*cinfoErr->err->format_message)(cinfoErr, jpeg_last_error_msg);
		dstream1 << "jpeg error: " << jpeg_last_error_msg;
		return false;
	}

	return true;
}


bool jpeg::read_RGB24_file(const char *filename, std::unique_ptr<uint8_t[]> &out, unsigned &width, unsigned &height) noexcept
{
	return read_JPEG_file(filename, out, width, height, 3, JCS_RGB, &read_RGB_scanlines);
}

bool jpeg::write_RGB24_mem(const uint8_t *in, std::vector<uint8_t> &out, unsigned width, unsigned height, int quality) noexcept
{
	return write_JPEG_mem(in, out, width, height, 3, quality, JCS_RGB, &write_RGB_scanlines);
}

bool jpeg::write_I420_mem(const uint8_t *in, std::vector<uint8_t> &out, unsigned width, unsigned height, int quality) noexcept
{
	return write_JPEG_mem(in, out, width, height, 3, quality, JCS_YCbCr, &write_I420_scanlines);
}

#undef DEBUG_CURRENT_MODULE