#ifndef NN_H
#define NN_H

#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <string.h>

float * load_mnist_bmp(const char * filename, ...);
void save_mnist_bmp(const float * x, const char * filename, ...);

#ifndef __APPLE__
#include <malloc.h>
uint32_t ntohl(uint32_t const net) {
      // https://codereview.stackexchange.com/questions/149717/implementation-of-c-standard-library-function-ntohl
      uint8_t data[4] = {};
      memcpy(&data, &net, sizeof(data));

      return ((uint32_t) data[3] << 0)
           | ((uint32_t) data[2] << 8)
           | ((uint32_t) data[1] << 16)
           | ((uint32_t) data[0] << 24);
}
#endif

#ifndef PATH_MAX
#define PATH_MAX 260
#endif

#ifdef __MINGW32__
int posix_memalign(void **memptr, size_t alignment, size_t size) {
      *memptr = malloc(size); //__mingw_aligned_malloc(size, alignment);
      if(NULL == *memptr) {
            return 1;
      }
      return 0;
}
#endif

// MNISTの画像を読み込む
float * load_mnist_image(const char * filename, int * width, int * height, int * count) {
  FILE * fp = fopen(filename, "rb");
  int32_t header[4];
  int r = fread(header, sizeof(header), 1, fp);
  assert(r == 1);

  int magic = ntohl(header[0]);
  assert(magic == 2051);

  int ni = ntohl(header[1]);
  int nr = ntohl(header[2]);
  int nc = ntohl(header[3]);
  
  assert(nr == 28);
  assert(nc == 28);

  const int N = nr*nc*ni;
  
  float * buf = malloc(N*sizeof(float));
  assert(NULL != buf);

  unsigned char * img = malloc(N);
  assert(NULL != img);

  r = fread(img, N, 1, fp);
  assert(r == 1);
  fclose(fp);

  int i;
  for(i=0 ; i<N ; i++) {
    buf[i] = ((float)(img[i])) / 255.0f;
  }
  free(img);
  
  *width = nc;
  *height = nr;
  *count = ni;
  
  return buf;
}

// MNISTのラベルを読み込む
unsigned char * load_mnist_label(const char * filename, int * count) {
  FILE * fp = fopen(filename, "rb");
  int32_t header[2];
  int r = fread(header, sizeof(header), 1, fp);
  assert(r==1);

  int magic = ntohl(header[0]);
  assert(magic == 2049);

  int ni = ntohl(header[1]);

  unsigned char * buf = malloc(ni);
  assert(NULL != buf);
  
  r = fread(buf, ni, 1, fp);
  assert(r==1);

  fclose(fp);

  *count = ni;
  
  return buf;
}

// MNISTのファイルを読み込む
void load_mnist(float ** train_x, unsigned char ** train_y, int * train_count,
                float ** test_x, unsigned char ** test_y, int * test_count,
                int * width, int * height) {
  assert(train_x != NULL);
  *train_x = load_mnist_image("train-images-idx3-ubyte", width, height, train_count);
  assert(*train_x != NULL);
  assert(*width == 28);
  assert(*height == 28);
  assert(*train_count == 60000);

  assert(train_y != NULL);
  *train_y = load_mnist_label("train-labels-idx1-ubyte", train_count);
  assert(*train_y != NULL);
  assert(*train_count == 60000);

  assert(test_x != NULL);
  *test_x = load_mnist_image("t10k-images-idx3-ubyte", width, height, test_count);
  assert(*test_x != NULL);
  assert(*width == 28);
  assert(*height == 28);
  assert(*test_count == 10000);

  assert(test_y != NULL);
  *test_y = load_mnist_label("t10k-labels-idx1-ubyte", test_count);
  assert(*test_y != NULL);
  assert(*test_count == 10000);
}

/*

  以下の内容はパブリックドメインライブラリ

    stb: single-file public domain libraries for C/C++
    https://github.com/nothings/stb

  より．

*/

#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC


//#define STB_IMAGE_STATIC
//#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

/* stb_image - v2.14 - public domain image loader - http://nothings.org/stb_image.h
                                     no warranty implied; use at your own risk

   Do this:
      #define STB_IMAGE_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define STB_IMAGE_IMPLEMENTATION
   #include "stb_image.h"

   You can #define STBI_ASSERT(x) before the #include to avoid using assert.h.
   And #define STBI_MALLOC, STBI_REALLOC, and STBI_FREE to avoid using malloc,realloc,free


   QUICK NOTES:
      Primarily of interest to game developers and other people who can
          avoid problematic images and only need the trivial interface

      JPEG baseline & progressive (12 bpc/arithmetic not supported, same as stock IJG lib)
      PNG 1/2/4/8-bit-per-channel (16 bpc not supported)

      TGA (not sure what subset, if a subset)
      BMP non-1bpp, non-RLE
      PSD (composited view only, no extra channels, 8/16 bit-per-channel)

      GIF (*comp always reports as 4-channel)
      HDR (radiance rgbE format)
      PIC (Softimage PIC)
      PNM (PPM and PGM binary only)

      Animated GIF still needs a proper API, but here's one way to do it:
          http://gist.github.com/urraka/685d9a6340b26b830d49

      - decode from memory or through FILE (define STBI_NO_STDIO to remove code)
      - decode from arbitrary I/O callbacks
      - SIMD acceleration on x86/x64 (SSE2) and ARM (NEON)

   Full documentation under "DOCUMENTATION" below.


   Revision 2.00 release notes:

      - Progressive JPEG is now supported.

      - PPM and PGM binary formats are now supported, thanks to Ken Miller.

      - x86 platforms now make use of SSE2 SIMD instructions for
        JPEG decoding, and ARM platforms can use NEON SIMD if requested.
        This work was done by Fabian "ryg" Giesen. SSE2 is used by
        default, but NEON must be enabled explicitly; see docs.

        With other JPEG optimizations included in this version, we see
        2x speedup on a JPEG on an x86 machine, and a 1.5x speedup
        on a JPEG on an ARM machine, relative to previous versions of this
        library. The same results will not obtain for all JPGs and for all
        x86/ARM machines. (Note that progressive JPEGs are significantly
        slower to decode than regular JPEGs.) This doesn't mean that this
        is the fastest JPEG decoder in the land; rather, it brings it
        closer to parity with standard libraries. If you want the fastest
        decode, look elsewhere. (See "Philosophy" section of docs below.)

        See final bullet items below for more info on SIMD.

      - Added STBI_MALLOC, STBI_REALLOC, and STBI_FREE macros for replacing
        the memory allocator. Unlike other STBI libraries, these macros don't
        support a context parameter, so if you need to pass a context in to
        the allocator, you'll have to store it in a global or a thread-local
        variable.

      - Split existing STBI_NO_HDR flag into two flags, STBI_NO_HDR and
        STBI_NO_LINEAR.
            STBI_NO_HDR:     suppress implementation of .hdr reader format
            STBI_NO_LINEAR:  suppress high-dynamic-range light-linear float API

      - You can suppress implementation of any of the decoders to reduce
        your code footprint by #defining one or more of the following
        symbols before creating the implementation.

            STBI_NO_JPEG
            STBI_NO_PNG
            STBI_NO_BMP
            STBI_NO_PSD
            STBI_NO_TGA
            STBI_NO_GIF
            STBI_NO_HDR
            STBI_NO_PIC
            STBI_NO_PNM   (.ppm and .pgm)

      - You can request *only* certain decoders and suppress all other ones
        (this will be more forward-compatible, as addition of new decoders
        doesn't require you to disable them explicitly):

            STBI_ONLY_JPEG
            STBI_ONLY_PNG
            STBI_ONLY_BMP
            STBI_ONLY_PSD
            STBI_ONLY_TGA
            STBI_ONLY_GIF
            STBI_ONLY_HDR
            STBI_ONLY_PIC
            STBI_ONLY_PNM   (.ppm and .pgm)

         Note that you can define multiples of these, and you will get all
         of them ("only x" and "only y" is interpreted to mean "only x&y").

       - If you use STBI_NO_PNG (or _ONLY_ without PNG), and you still
         want the zlib decoder to be available, #define STBI_SUPPORT_ZLIB

      - Compilation of all SIMD code can be suppressed with
            #define STBI_NO_SIMD
        It should not be necessary to disable SIMD unless you have issues
        compiling (e.g. using an x86 compiler which doesn't support SSE
        intrinsics or that doesn't support the method used to detect
        SSE2 support at run-time), and even those can be reported as
        bugs so I can refine the built-in compile-time checking to be
        smarter.

      - The old STBI_SIMD system which allowed installing a user-defined
        IDCT etc. has been removed. If you need this, don't upgrade. My
        assumption is that almost nobody was doing this, and those who
        were will find the built-in SIMD more satisfactory anyway.

      - RGB values computed for JPEG images are slightly different from
        previous versions of stb_image. (This is due to using less
        integer precision in SIMD.) The C code has been adjusted so
        that the same RGB values will be computed regardless of whether
        SIMD support is available, so your app should always produce
        consistent results. But these results are slightly different from
        previous versions. (Specifically, about 3% of available YCbCr values
        will compute different RGB results from pre-1.49 versions by +-1;
        most of the deviating values are one smaller in the G channel.)

      - If you must produce consistent results with previous versions of
        stb_image, #define STBI_JPEG_OLD and you will get the same results
        you used to; however, you will not get the SIMD speedups for
        the YCbCr-to-RGB conversion step (although you should still see
        significant JPEG speedup from the other changes).

        Please note that STBI_JPEG_OLD is a temporary feature; it will be
        removed in future versions of the library. It is only intended for
        near-term back-compatibility use.


   Latest revision history:
      2.13  (2016-12-04) experimental 16-bit API, only for PNG so far; fixes
      2.12  (2016-04-02) fix typo in 2.11 PSD fix that caused crashes
      2.11  (2016-04-02) 16-bit PNGS; enable SSE2 in non-gcc x64
                         RGB-format JPEG; remove white matting in PSD;
                         allocate large structures on the stack; 
                         correct channel count for PNG & BMP
      2.10  (2016-01-22) avoid warning introduced in 2.09
      2.09  (2016-01-16) 16-bit TGA; comments in PNM files; STBI_REALLOC_SIZED
      2.08  (2015-09-13) fix to 2.07 cleanup, reading RGB PSD as RGBA
      2.07  (2015-09-13) partial animated GIF support
                         limited 16-bit PSD support
                         minor bugs, code cleanup, and compiler warnings

   See end of file for full revision history.


 ============================    Contributors    =========================

 Image formats                          Extensions, features
    Sean Barrett (jpeg, png, bmp)          Jetro Lauha (stbi_info)
    Nicolas Schulz (hdr, psd)              Martin "SpartanJ" Golini (stbi_info)
    Jonathan Dummer (tga)                  James "moose2000" Brown (iPhone PNG)
    Jean-Marc Lienher (gif)                Ben "Disch" Wenger (io callbacks)
    Tom Seddon (pic)                       Omar Cornut (1/2/4-bit PNG)
    Thatcher Ulrich (psd)                  Nicolas Guillemot (vertical flip)
    Ken Miller (pgm, ppm)                  Richard Mitton (16-bit PSD)
    github:urraka (animated gif)           Junggon Kim (PNM comments)
                                           Daniel Gibson (16-bit TGA)
                                           socks-the-fox (16-bit TGA)
 Optimizations & bugfixes
    Fabian "ryg" Giesen
    Arseny Kapoulkine

 Bug & warning fixes
    Marc LeBlanc            David Woo          Guillaume George   Martins Mozeiko
    Christpher Lloyd        Martin Golini      Jerry Jansson      Joseph Thomson
    Dave Moore              Roy Eltham         Hayaki Saito       Phil Jordan
    Won Chun                Luke Graham        Johan Duparc       Nathan Reed
    the Horde3D community   Thomas Ruf         Ronny Chevalier    Nick Verigakis
    Janez Zemva             John Bartholomew   Michal Cichon      github:svdijk
    Jonathan Blow           Ken Hamada         Tero Hanninen      Baldur Karlsson
    Laurent Gomila          Cort Stratton      Sergio Gonzalez    github:romigrou
    Aruelien Pocheville     Thibault Reuille   Cass Everitt       Matthew Gregan
    Ryamond Barbiero        Paul Du Bois       Engin Manap        github:snagar
    Michaelangel007@github  Oriol Ferrer Mesia Dale Weiler        github:Zelex
    Philipp Wiesemann       Josh Tobin         github:rlyeh       github:grim210@github
    Blazej Dariusz Roszkowski                  github:sammyhw


LICENSE

This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.

*/

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

// DOCUMENTATION
//
// Limitations:
//    - no 16-bit-per-channel PNG
//    - no 12-bit-per-channel JPEG
//    - no JPEGs with arithmetic coding
//    - no 1-bit BMP
//    - GIF always returns *comp=4
//
// Basic usage (see HDR discussion below for HDR usage):
//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ...
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data)
//
// Standard parameters:
//    int *x                 -- outputs image width in pixels
//    int *y                 -- outputs image height in pixels
//    int *channels_in_file  -- outputs # of image components in image file
//    int desired_channels   -- if non-zero, # of image components requested in result
//
// The return value from an image loader is an 'unsigned char *' which points
// to the pixel data, or NULL on an allocation failure or if the image is
// corrupt or invalid. The pixel data consists of *y scanlines of *x pixels,
// with each pixel consisting of N interleaved 8-bit components; the first
// pixel pointed to is top-left-most in the image. There is no padding between
// image scanlines or between pixels, regardless of format. The number of
// components N is 'req_comp' if req_comp is non-zero, or *comp otherwise.
// If req_comp is non-zero, *comp has the number of components that _would_
// have been output otherwise. E.g. if you set req_comp to 4, you will always
// get RGBA output, but you can check *comp to see if it's trivially opaque
// because e.g. there were only 3 channels in the source image.
//
// An output image with N components has the following components interleaved
// in this order in each pixel:
//
//     N=#comp     components
//       1           grey
//       2           grey, alpha
//       3           red, green, blue
//       4           red, green, blue, alpha
//
// If image loading fails for any reason, the return value will be NULL,
// and *x, *y, *comp will be unchanged. The function stbi_failure_reason()
// can be queried for an extremely brief, end-user unfriendly explanation
// of why the load failed. Define STBI_NO_FAILURE_STRINGS to avoid
// compiling these strings at all, and STBI_FAILURE_USERMSG to get slightly
// more user-friendly ones.
//
// Paletted PNG, BMP, GIF, and PIC images are automatically depalettized.
//
// ===========================================================================
//
// Philosophy
//
// stb libraries are designed with the following priorities:
//
//    1. easy to use
//    2. easy to maintain
//    3. good performance
//
// Sometimes I let "good performance" creep up in priority over "easy to maintain",
// and for best performance I may provide less-easy-to-use APIs that give higher
// performance, in addition to the easy to use ones. Nevertheless, it's important
// to keep in mind that from the standpoint of you, a client of this library,
// all you care about is #1 and #3, and stb libraries do not emphasize #3 above all.
//
// Some secondary priorities arise directly from the first two, some of which
// make more explicit reasons why performance can't be emphasized.
//
//    - Portable ("ease of use")
//    - Small footprint ("easy to maintain")
//    - No dependencies ("ease of use")
//
// ===========================================================================
//
// I/O callbacks
//
// I/O callbacks allow you to read from arbitrary sources, like packaged
// files or some other source. Data read from callbacks are processed
// through a small internal buffer (currently 128 bytes) to try to reduce
// overhead.
//
// The three functions you must define are "read" (reads some bytes of data),
// "skip" (skips some bytes of data), "eof" (reports if the stream is at the end).
//
// ===========================================================================
//
// SIMD support
//
// The JPEG decoder will try to automatically use SIMD kernels on x86 when
// supported by the compiler. For ARM Neon support, you must explicitly
// request it.
//
// (The old do-it-yourself SIMD API is no longer supported in the current
// code.)
//
// On x86, SSE2 will automatically be used when available based on a run-time
// test; if not, the generic C versions are used as a fall-back. On ARM targets,
// the typical path is to have separate builds for NEON and non-NEON devices
// (at least this is true for iOS and Android). Therefore, the NEON support is
// toggled by a build flag: define STBI_NEON to get NEON loops.
//
// The output of the JPEG decoder is slightly different from versions where
// SIMD support was introduced (that is, for versions before 1.49). The
// difference is only +-1 in the 8-bit RGB channels, and only on a small
// fraction of pixels. You can force the pre-1.49 behavior by defining
// STBI_JPEG_OLD, but this will disable some of the SIMD decoding path
// and hence cost some performance.
//
// If for some reason you do not want to use any of SIMD code, or if
// you have issues compiling it, you can disable it entirely by
// defining STBI_NO_SIMD.
//
// ===========================================================================
//
// HDR image support   (disable by defining STBI_NO_HDR)
//
// stb_image now supports loading HDR images in general, and currently
// the Radiance .HDR file format, although the support is provided
// generically. You can still load any file through the existing interface;
// if you attempt to load an HDR file, it will be automatically remapped to
// LDR, assuming gamma 2.2 and an arbitrary scale factor defaulting to 1;
// both of these constants can be reconfigured through this interface:
//
//     stbi_hdr_to_ldr_gamma(2.2f);
//     stbi_hdr_to_ldr_scale(1.0f);
//
// (note, do not use _inverse_ constants; stbi_image will invert them
// appropriately).
//
// Additionally, there is a new, parallel interface for loading files as
// (linear) floats to preserve the full dynamic range:
//
//    float *data = stbi_loadf(filename, &x, &y, &n, 0);
//
// If you load LDR images through this interface, those images will
// be promoted to floating point values, run through the inverse of
// constants corresponding to the above:
//
//     stbi_ldr_to_hdr_scale(1.0f);
//     stbi_ldr_to_hdr_gamma(2.2f);
//
// Finally, given a filename (or an open file or memory block--see header
// file for details) containing image data, you can query for the "most
// appropriate" interface to use (that is, whether the image is HDR or
// not), using:
//
//     stbi_is_hdr(char *filename);
//
// ===========================================================================
//
// iPhone PNG support:
//
// By default we convert iphone-formatted PNGs back to RGB, even though
// they are internally encoded differently. You can disable this conversion
// by by calling stbi_convert_iphone_png_to_rgb(0), in which case
// you will always just get the native iphone "format" through (which
// is BGR stored in RGB).
//
// Call stbi_set_unpremultiply_on_load(1) as well to force a divide per
// pixel to remove any premultiplied alpha *only* if the image file explicitly
// says there's premultiplied data (currently only happens in iPhone images,
// and only if iPhone convert-to-rgb processing is on).
//


#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif // STBI_NO_STDIO

#define STBI_VERSION 1

enum
{
   STBI_default = 0, // only used for req_comp

   STBI_grey       = 1,
   STBI_grey_alpha = 2,
   STBI_rgb        = 3,
   STBI_rgb_alpha  = 4
};

typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef STB_IMAGE_STATIC
#define STBIDEF static
#else
#define STBIDEF extern
#endif

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API - works on images of any type
//

//
// load image by filename, open file, or memory buffer
//

typedef struct
{
   int      (*read)  (void *user,char *data,int size);   // fill 'data' with 'size' bytes.  return number of bytes actually read
   void     (*skip)  (void *user,int n);                 // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
   int      (*eof)   (void *user);                       // returns nonzero if we are at end of file/data
} stbi_io_callbacks;

////////////////////////////////////
//
// 8-bits-per-channel interface
//

STBIDEF stbi_uc *stbi_load               (char              const *filename,           int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF stbi_uc *stbi_load_from_memory   (stbi_uc           const *buffer, int len   , int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF stbi_uc *stbi_load_from_callbacks(stbi_io_callbacks const *clbk  , void *user, int *x, int *y, int *channels_in_file, int desired_channels);

#ifndef STBI_NO_STDIO
STBIDEF stbi_uc *stbi_load_from_file   (FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
// for stbi_load_from_file, file pointer is left pointing immediately after image
#endif

////////////////////////////////////
//
// 16-bits-per-channel interface
//

STBIDEF stbi_us *stbi_load_16(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
#ifndef STBI_NO_STDIO
STBIDEF stbi_us *stbi_load_from_file_16(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
#endif
// @TODO the other variants

////////////////////////////////////
//
// float-per-channel interface
//
#ifndef STBI_NO_LINEAR
   STBIDEF float *stbi_loadf                 (char const *filename,           int *x, int *y, int *channels_in_file, int desired_channels);
   STBIDEF float *stbi_loadf_from_memory     (stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
   STBIDEF float *stbi_loadf_from_callbacks  (stbi_io_callbacks const *clbk, void *user, int *x, int *y,  int *channels_in_file, int desired_channels);

   #ifndef STBI_NO_STDIO
   STBIDEF float *stbi_loadf_from_file  (FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
   #endif
#endif

#ifndef STBI_NO_HDR
   STBIDEF void   stbi_hdr_to_ldr_gamma(float gamma);
   STBIDEF void   stbi_hdr_to_ldr_scale(float scale);
#endif // STBI_NO_HDR

#ifndef STBI_NO_LINEAR
   STBIDEF void   stbi_ldr_to_hdr_gamma(float gamma);
   STBIDEF void   stbi_ldr_to_hdr_scale(float scale);
#endif // STBI_NO_LINEAR

// stbi_is_hdr is always defined, but always returns false if STBI_NO_HDR
STBIDEF int    stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user);
STBIDEF int    stbi_is_hdr_from_memory(stbi_uc const *buffer, int len);
#ifndef STBI_NO_STDIO
STBIDEF int      stbi_is_hdr          (char const *filename);
STBIDEF int      stbi_is_hdr_from_file(FILE *f);
#endif // STBI_NO_STDIO


// get a VERY brief reason for failure
// NOT THREADSAFE
STBIDEF const char *stbi_failure_reason  (void);

// free the loaded image -- this is just free()
STBIDEF void     stbi_image_free      (void *retval_from_stbi_load);

// get image dimensions & components without fully decoding
STBIDEF int      stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);
STBIDEF int      stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp);

#ifndef STBI_NO_STDIO
STBIDEF int      stbi_info            (char const *filename,     int *x, int *y, int *comp);
STBIDEF int      stbi_info_from_file  (FILE *f,                  int *x, int *y, int *comp);

#endif



// for image formats that explicitly notate that they have premultiplied alpha,
// we just return the colors as stored in the file. set this flag to force
// unpremultiplication. results are undefined if the unpremultiply overflow.
STBIDEF void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply);

// indicate whether we should process iphone images back to canonical format,
// or just pass them through "as-is"
STBIDEF void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert);

// flip the image vertically, so the first pixel in the output array is the bottom left
STBIDEF void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);

// ZLIB client - used by PNG, available for other purposes

STBIDEF char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
STBIDEF char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header);
STBIDEF char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);
STBIDEF int   stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

STBIDEF char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
STBIDEF int   stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);


#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // STBI_INCLUDE_STB_IMAGE_H

#ifdef STB_IMAGE_IMPLEMENTATION

#if defined(STBI_ONLY_JPEG) || defined(STBI_ONLY_PNG) || defined(STBI_ONLY_BMP) \
  || defined(STBI_ONLY_TGA) || defined(STBI_ONLY_GIF) || defined(STBI_ONLY_PSD) \
  || defined(STBI_ONLY_HDR) || defined(STBI_ONLY_PIC) || defined(STBI_ONLY_PNM) \
  || defined(STBI_ONLY_ZLIB)
   #ifndef STBI_ONLY_JPEG
   #define STBI_NO_JPEG
   #endif
   #ifndef STBI_ONLY_PNG
   #define STBI_NO_PNG
   #endif
   #ifndef STBI_ONLY_BMP
   #define STBI_NO_BMP
   #endif
   #ifndef STBI_ONLY_PSD
   #define STBI_NO_PSD
   #endif
   #ifndef STBI_ONLY_TGA
   #define STBI_NO_TGA
   #endif
   #ifndef STBI_ONLY_GIF
   #define STBI_NO_GIF
   #endif
   #ifndef STBI_ONLY_HDR
   #define STBI_NO_HDR
   #endif
   #ifndef STBI_ONLY_PIC
   #define STBI_NO_PIC
   #endif
   #ifndef STBI_ONLY_PNM
   #define STBI_NO_PNM
   #endif
#endif

#if defined(STBI_NO_PNG) && !defined(STBI_SUPPORT_ZLIB) && !defined(STBI_NO_ZLIB)
#define STBI_NO_ZLIB
#endif


#include <stdarg.h>
#include <stddef.h> // ptrdiff_t on osx
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if !defined(STBI_NO_LINEAR) || !defined(STBI_NO_HDR)
#include <math.h>  // ldexp
#endif

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif

#ifndef STBI_ASSERT
#include <assert.h>
#define STBI_ASSERT(x) assert(x)
#endif


#ifndef _MSC_VER
   #ifdef __cplusplus
   #define stbi_inline inline
   #else
   #define stbi_inline
   #endif
#else
   #define stbi_inline __forceinline
#endif


#ifdef _MSC_VER
typedef unsigned short stbi__uint16;
typedef   signed short stbi__int16;
typedef unsigned int   stbi__uint32;
typedef   signed int   stbi__int32;
#else
#include <stdint.h>
typedef uint16_t stbi__uint16;
typedef int16_t  stbi__int16;
typedef uint32_t stbi__uint32;
typedef int32_t  stbi__int32;
#endif

// should produce compiler error if size is wrong
typedef unsigned char validate_uint32[sizeof(stbi__uint32)==4 ? 1 : -1];

#ifdef _MSC_VER
#define STBI_NOTUSED(v)  (void)(v)
#else
#define STBI_NOTUSED(v)  (void)sizeof(v)
#endif

#ifdef _MSC_VER
#define STBI_HAS_LROTL
#endif

#ifdef STBI_HAS_LROTL
   #define stbi_lrot(x,y)  _lrotl(x,y)
#else
   #define stbi_lrot(x,y)  (((x) << (y)) | ((x) >> (32 - (y))))
#endif

#if defined(STBI_MALLOC) && defined(STBI_FREE) && (defined(STBI_REALLOC) || defined(STBI_REALLOC_SIZED))
// ok
#elif !defined(STBI_MALLOC) && !defined(STBI_FREE) && !defined(STBI_REALLOC) && !defined(STBI_REALLOC_SIZED)
// ok
#else
#error "Must define all or none of STBI_MALLOC, STBI_FREE, and STBI_REALLOC (or STBI_REALLOC_SIZED)."
#endif

#ifndef STBI_MALLOC
#define STBI_MALLOC(sz)           malloc(sz)
#define STBI_REALLOC(p,newsz)     realloc(p,newsz)
#define STBI_FREE(p)              free(p)
#endif

#ifndef STBI_REALLOC_SIZED
#define STBI_REALLOC_SIZED(p,oldsz,newsz) STBI_REALLOC(p,newsz)
#endif

// x86/x64 detection
#if defined(__x86_64__) || defined(_M_X64)
#define STBI__X64_TARGET
#elif defined(__i386) || defined(_M_IX86)
#define STBI__X86_TARGET
#endif

#if defined(__GNUC__) && (defined(STBI__X86_TARGET) || defined(STBI__X64_TARGET)) && !defined(__SSE2__) && !defined(STBI_NO_SIMD)
// NOTE: not clear do we actually need this for the 64-bit path?
// gcc doesn't support sse2 intrinsics unless you compile with -msse2,
// (but compiling with -msse2 allows the compiler to use SSE2 everywhere;
// this is just broken and gcc are jerks for not fixing it properly
// http://www.virtualdub.org/blog/pivot/entry.php?id=363 )
#define STBI_NO_SIMD
#endif

#if defined(__MINGW32__) && defined(STBI__X86_TARGET) && !defined(STBI_MINGW_ENABLE_SSE2) && !defined(STBI_NO_SIMD)
// Note that __MINGW32__ doesn't actually mean 32-bit, so we have to avoid STBI__X64_TARGET
//
// 32-bit MinGW wants ESP to be 16-byte aligned, but this is not in the
// Windows ABI and VC++ as well as Windows DLLs don't maintain that invariant.
// As a result, enabling SSE2 on 32-bit MinGW is dangerous when not
// simultaneously enabling "-mstackrealign".
//
// See https://github.com/nothings/stb/issues/81 for more information.
//
// So default to no SSE2 on 32-bit MinGW. If you've read this far and added
// -mstackrealign to your build settings, feel free to #define STBI_MINGW_ENABLE_SSE2.
#define STBI_NO_SIMD
#endif

#if !defined(STBI_NO_SIMD) && (defined(STBI__X86_TARGET) || defined(STBI__X64_TARGET))
#define STBI_SSE2
#include <emmintrin.h>

#ifdef _MSC_VER

#if _MSC_VER >= 1400  // not VC6
#include <intrin.h> // __cpuid
static int stbi__cpuid3(void)
{
   int info[4];
   __cpuid(info,1);
   return info[3];
}
#else
static int stbi__cpuid3(void)
{
   int res;
   __asm {
      mov  eax,1
      cpuid
      mov  res,edx
   }
   return res;
}
#endif

#define STBI_SIMD_ALIGN(type, name) __declspec(align(16)) type name

static int stbi__sse2_available()
{
   int info3 = stbi__cpuid3();
   return ((info3 >> 26) & 1) != 0;
}
#else // assume GCC-style if not VC++
#define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))

static int stbi__sse2_available()
{
#if defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__) >= 408 // GCC 4.8 or later
   // GCC 4.8+ has a nice way to do this
   return __builtin_cpu_supports("sse2");
#else
   // portable way to do this, preferably without using GCC inline ASM?
   // just bail for now.
   return 0;
#endif
}
#endif
#endif

// ARM NEON
#if defined(STBI_NO_SIMD) && defined(STBI_NEON)
#undef STBI_NEON
#endif

#ifdef STBI_NEON
#include <arm_neon.h>
// assume GCC or Clang on ARM targets
#define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))
#endif

#ifndef STBI_SIMD_ALIGN
#define STBI_SIMD_ALIGN(type, name) type name
#endif

///////////////////////////////////////////////
//
//  stbi__context struct and start_xxx functions

// stbi__context structure is our basic context used by all images, so it
// contains all the IO context, plus some basic image information
typedef struct
{
   stbi__uint32 img_x, img_y;
   int img_n, img_out_n;

   stbi_io_callbacks io;
   void *io_user_data;

   int read_from_callbacks;
   int buflen;
   stbi_uc buffer_start[128];

   stbi_uc *img_buffer, *img_buffer_end;
   stbi_uc *img_buffer_original, *img_buffer_original_end;
} stbi__context;


static void stbi__refill_buffer(stbi__context *s);

// initialize a memory-decode context
static void stbi__start_mem(stbi__context *s, stbi_uc const *buffer, int len)
{
   s->io.read = NULL;
   s->read_from_callbacks = 0;
   s->img_buffer = s->img_buffer_original = (stbi_uc *) buffer;
   s->img_buffer_end = s->img_buffer_original_end = (stbi_uc *) buffer+len;
}

// initialize a callback-based context
static void stbi__start_callbacks(stbi__context *s, stbi_io_callbacks *c, void *user)
{
   s->io = *c;
   s->io_user_data = user;
   s->buflen = sizeof(s->buffer_start);
   s->read_from_callbacks = 1;
   s->img_buffer_original = s->buffer_start;
   stbi__refill_buffer(s);
   s->img_buffer_original_end = s->img_buffer_end;
}

#ifndef STBI_NO_STDIO

static int stbi__stdio_read(void *user, char *data, int size)
{
   return (int) fread(data,1,size,(FILE*) user);
}

static void stbi__stdio_skip(void *user, int n)
{
   fseek((FILE*) user, n, SEEK_CUR);
}

static int stbi__stdio_eof(void *user)
{
   return feof((FILE*) user);
}

static stbi_io_callbacks stbi__stdio_callbacks =
{
   stbi__stdio_read,
   stbi__stdio_skip,
   stbi__stdio_eof,
};

static void stbi__start_file(stbi__context *s, FILE *f)
{
   stbi__start_callbacks(s, &stbi__stdio_callbacks, (void *) f);
}

//static void stop_file(stbi__context *s) { }

#endif // !STBI_NO_STDIO

static void stbi__rewind(stbi__context *s)
{
   // conceptually rewind SHOULD rewind to the beginning of the stream,
   // but we just rewind to the beginning of the initial buffer, because
   // we only use it after doing 'test', which only ever looks at at most 92 bytes
   s->img_buffer = s->img_buffer_original;
   s->img_buffer_end = s->img_buffer_original_end;
}

enum
{
   STBI_ORDER_RGB,
   STBI_ORDER_BGR
};

typedef struct
{
   int bits_per_channel;
   int num_channels;
   int channel_order;
} stbi__result_info;

#ifndef STBI_NO_JPEG
static int      stbi__jpeg_test(stbi__context *s);
static void    *stbi__jpeg_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__jpeg_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PNG
static int      stbi__png_test(stbi__context *s);
static void    *stbi__png_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__png_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_BMP
static int      stbi__bmp_test(stbi__context *s);
static void    *stbi__bmp_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__bmp_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_TGA
static int      stbi__tga_test(stbi__context *s);
static void    *stbi__tga_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__tga_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PSD
static int      stbi__psd_test(stbi__context *s);
static void    *stbi__psd_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri, int bpc);
static int      stbi__psd_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_HDR
static int      stbi__hdr_test(stbi__context *s);
static float   *stbi__hdr_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__hdr_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PIC
static int      stbi__pic_test(stbi__context *s);
static void    *stbi__pic_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__pic_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_GIF
static int      stbi__gif_test(stbi__context *s);
static void    *stbi__gif_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__gif_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PNM
static int      stbi__pnm_test(stbi__context *s);
static void    *stbi__pnm_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__pnm_info(stbi__context *s, int *x, int *y, int *comp);
#endif

// this is not threadsafe
static const char *stbi__g_failure_reason;

STBIDEF const char *stbi_failure_reason(void)
{
   return stbi__g_failure_reason;
}

static int stbi__err(const char *str)
{
   stbi__g_failure_reason = str;
   return 0;
}

static void *stbi__malloc(size_t size)
{
    return STBI_MALLOC(size);
}

// stb_image uses ints pervasively, including for offset calculations.
// therefore the largest decoded image size we can support with the
// current code, even on 64-bit targets, is INT_MAX. this is not a
// significant limitation for the intended use case.
//
// we do, however, need to make sure our size calculations don't
// overflow. hence a few helper functions for size calculations that
// multiply integers together, making sure that they're non-negative
// and no overflow occurs.

// return 1 if the sum is valid, 0 on overflow.
// negative terms are considered invalid.
static int stbi__addsizes_valid(int a, int b)
{
   if (b < 0) return 0;
   // now 0 <= b <= INT_MAX, hence also
   // 0 <= INT_MAX - b <= INTMAX.
   // And "a + b <= INT_MAX" (which might overflow) is the
   // same as a <= INT_MAX - b (no overflow)
   return a <= INT_MAX - b;
}

// returns 1 if the product is valid, 0 on overflow.
// negative factors are considered invalid.
static int stbi__mul2sizes_valid(int a, int b)
{
   if (a < 0 || b < 0) return 0;
   if (b == 0) return 1; // mul-by-0 is always safe
   // portable way to check for no overflows in a*b
   return a <= INT_MAX/b;
}

// returns 1 if "a*b + add" has no negative terms/factors and doesn't overflow
static int stbi__mad2sizes_valid(int a, int b, int add)
{
   return stbi__mul2sizes_valid(a, b) && stbi__addsizes_valid(a*b, add);
}

// returns 1 if "a*b*c + add" has no negative terms/factors and doesn't overflow
static int stbi__mad3sizes_valid(int a, int b, int c, int add)
{
   return stbi__mul2sizes_valid(a, b) && stbi__mul2sizes_valid(a*b, c) &&
      stbi__addsizes_valid(a*b*c, add);
}

// returns 1 if "a*b*c*d + add" has no negative terms/factors and doesn't overflow
static int stbi__mad4sizes_valid(int a, int b, int c, int d, int add)
{
   return stbi__mul2sizes_valid(a, b) && stbi__mul2sizes_valid(a*b, c) &&
      stbi__mul2sizes_valid(a*b*c, d) && stbi__addsizes_valid(a*b*c*d, add);
}

// mallocs with size overflow checking
static void *stbi__malloc_mad2(int a, int b, int add)
{
   if (!stbi__mad2sizes_valid(a, b, add)) return NULL;
   return stbi__malloc(a*b + add);
}

static void *stbi__malloc_mad3(int a, int b, int c, int add)
{
   if (!stbi__mad3sizes_valid(a, b, c, add)) return NULL;
   return stbi__malloc(a*b*c + add);
}

static void *stbi__malloc_mad4(int a, int b, int c, int d, int add)
{
   if (!stbi__mad4sizes_valid(a, b, c, d, add)) return NULL;
   return stbi__malloc(a*b*c*d + add);
}

// stbi__err - error
// stbi__errpf - error returning pointer to float
// stbi__errpuc - error returning pointer to unsigned char

#ifdef STBI_NO_FAILURE_STRINGS
   #define stbi__err(x,y)  0
#elif defined(STBI_FAILURE_USERMSG)
   #define stbi__err(x,y)  stbi__err(y)
#else
   #define stbi__err(x,y)  stbi__err(x)
#endif

#define stbi__errpf(x,y)   ((float *)(size_t) (stbi__err(x,y)?NULL:NULL))
#define stbi__errpuc(x,y)  ((unsigned char *)(size_t) (stbi__err(x,y)?NULL:NULL))

STBIDEF void stbi_image_free(void *retval_from_stbi_load)
{
   STBI_FREE(retval_from_stbi_load);
}

#ifndef STBI_NO_LINEAR
static float   *stbi__ldr_to_hdr(stbi_uc *data, int x, int y, int comp);
#endif

#ifndef STBI_NO_HDR
static stbi_uc *stbi__hdr_to_ldr(float   *data, int x, int y, int comp);
#endif

static int stbi__vertically_flip_on_load = 0;

STBIDEF void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip)
{
    stbi__vertically_flip_on_load = flag_true_if_should_flip;
}

static void *stbi__load_main(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri, int bpc)
{
   memset(ri, 0, sizeof(*ri)); // make sure it's initialized if we add new fields
   ri->bits_per_channel = 8; // default is 8 so most paths don't have to be changed
   ri->channel_order = STBI_ORDER_RGB; // all current input & output are this, but this is here so we can add BGR order
   ri->num_channels = 0;

   #ifndef STBI_NO_JPEG
   if (stbi__jpeg_test(s)) return stbi__jpeg_load(s,x,y,comp,req_comp, ri);
   #endif
   #ifndef STBI_NO_PNG
   if (stbi__png_test(s))  return stbi__png_load(s,x,y,comp,req_comp, ri);
   #endif
   #ifndef STBI_NO_BMP
   if (stbi__bmp_test(s))  return stbi__bmp_load(s,x,y,comp,req_comp, ri);
   #endif
   #ifndef STBI_NO_GIF
   if (stbi__gif_test(s))  return stbi__gif_load(s,x,y,comp,req_comp, ri);
   #endif
   #ifndef STBI_NO_PSD
   if (stbi__psd_test(s))  return stbi__psd_load(s,x,y,comp,req_comp, ri, bpc);
   #endif
   #ifndef STBI_NO_PIC
   if (stbi__pic_test(s))  return stbi__pic_load(s,x,y,comp,req_comp, ri);
   #endif
   #ifndef STBI_NO_PNM
   if (stbi__pnm_test(s))  return stbi__pnm_load(s,x,y,comp,req_comp, ri);
   #endif

   #ifndef STBI_NO_HDR
   if (stbi__hdr_test(s)) {
      float *hdr = stbi__hdr_load(s, x,y,comp,req_comp, ri);
      return stbi__hdr_to_ldr(hdr, *x, *y, req_comp ? req_comp : *comp);
   }
   #endif

   #ifndef STBI_NO_TGA
   // test tga last because it's a crappy test!
   if (stbi__tga_test(s))
      return stbi__tga_load(s,x,y,comp,req_comp, ri);
   #endif

   return stbi__errpuc("unknown image type", "Image not of any known type, or corrupt");
}

static stbi_uc *stbi__convert_16_to_8(stbi__uint16 *orig, int w, int h, int channels)
{
   int i;
   int img_len = w * h * channels;
   stbi_uc *reduced;

   reduced = (stbi_uc *) stbi__malloc(img_len);
   if (reduced == NULL) return stbi__errpuc("outofmem", "Out of memory");

   for (i = 0; i < img_len; ++i)
      reduced[i] = (stbi_uc)((orig[i] >> 8) & 0xFF); // top half of each byte is sufficient approx of 16->8 bit scaling

   STBI_FREE(orig);
   return reduced;
}

static stbi__uint16 *stbi__convert_8_to_16(stbi_uc *orig, int w, int h, int channels)
{
   int i;
   int img_len = w * h * channels;
   stbi__uint16 *enlarged;

   enlarged = (stbi__uint16 *) stbi__malloc(img_len*2);
   if (enlarged == NULL) return (stbi__uint16 *) stbi__errpuc("outofmem", "Out of memory");

   for (i = 0; i < img_len; ++i)
      enlarged[i] = (stbi__uint16)((orig[i] << 8) + orig[i]); // replicate to high and low byte, maps 0->0, 255->0xffff

   STBI_FREE(orig);
   return enlarged;
}

static unsigned char *stbi__load_and_postprocess_8bit(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   stbi__result_info ri;
   void *result = stbi__load_main(s, x, y, comp, req_comp, &ri, 8);

   if (result == NULL)
      return NULL;

   if (ri.bits_per_channel != 8) {
      STBI_ASSERT(ri.bits_per_channel == 16);
      result = stbi__convert_16_to_8((stbi__uint16 *) result, *x, *y, req_comp == 0 ? *comp : req_comp);
      ri.bits_per_channel = 8;
   }

   // @TODO: move stbi__convert_format to here

   if (stbi__vertically_flip_on_load) {
      int w = *x, h = *y;
      int channels = req_comp ? req_comp : *comp;
      int row,col,z;
      stbi_uc *image = (stbi_uc *) result;

      // @OPTIMIZE: use a bigger temp buffer and memcpy multiple pixels at once
      for (row = 0; row < (h>>1); row++) {
         for (col = 0; col < w; col++) {
            for (z = 0; z < channels; z++) {
               stbi_uc temp = image[(row * w + col) * channels + z];
               image[(row * w + col) * channels + z] = image[((h - row - 1) * w + col) * channels + z];
               image[((h - row - 1) * w + col) * channels + z] = temp;
            }
         }
      }
   }

   return (unsigned char *) result;
}

static stbi__uint16 *stbi__load_and_postprocess_16bit(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   stbi__result_info ri;
   void *result = stbi__load_main(s, x, y, comp, req_comp, &ri, 16);

   if (result == NULL)
      return NULL;

   if (ri.bits_per_channel != 16) {
      STBI_ASSERT(ri.bits_per_channel == 8);
      result = stbi__convert_8_to_16((stbi_uc *) result, *x, *y, req_comp == 0 ? *comp : req_comp);
      ri.bits_per_channel = 16;
   }

   // @TODO: move stbi__convert_format16 to here
   // @TODO: special case RGB-to-Y (and RGBA-to-YA) for 8-bit-to-16-bit case to keep more precision

   if (stbi__vertically_flip_on_load) {
      int w = *x, h = *y;
      int channels = req_comp ? req_comp : *comp;
      int row,col,z;
      stbi__uint16 *image = (stbi__uint16 *) result;

      // @OPTIMIZE: use a bigger temp buffer and memcpy multiple pixels at once
      for (row = 0; row < (h>>1); row++) {
         for (col = 0; col < w; col++) {
            for (z = 0; z < channels; z++) {
               stbi__uint16 temp = image[(row * w + col) * channels + z];
               image[(row * w + col) * channels + z] = image[((h - row - 1) * w + col) * channels + z];
               image[((h - row - 1) * w + col) * channels + z] = temp;
            }
         }
      }
   }

   return (stbi__uint16 *) result;
}

#ifndef STBI_NO_HDR
static void stbi__float_postprocess(float *result, int *x, int *y, int *comp, int req_comp)
{
   if (stbi__vertically_flip_on_load && result != NULL) {
      int w = *x, h = *y;
      int depth = req_comp ? req_comp : *comp;
      int row,col,z;
      float temp;

      // @OPTIMIZE: use a bigger temp buffer and memcpy multiple pixels at once
      for (row = 0; row < (h>>1); row++) {
         for (col = 0; col < w; col++) {
            for (z = 0; z < depth; z++) {
               temp = result[(row * w + col) * depth + z];
               result[(row * w + col) * depth + z] = result[((h - row - 1) * w + col) * depth + z];
               result[((h - row - 1) * w + col) * depth + z] = temp;
            }
         }
      }
   }
}
#endif

#ifndef STBI_NO_STDIO

static FILE *stbi__fopen(char const *filename, char const *mode)
{
   FILE *f;
#if defined(_MSC_VER) && _MSC_VER >= 1400
   if (0 != fopen_s(&f, filename, mode))
      f=0;
#else
   f = fopen(filename, mode);
#endif
   return f;
}


STBIDEF stbi_uc *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   FILE *f = stbi__fopen(filename, "rb");
   unsigned char *result;
   if (!f) return stbi__errpuc("can't fopen", "Unable to open file");
   result = stbi_load_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return result;
}

STBIDEF stbi_uc *stbi_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   unsigned char *result;
   stbi__context s;
   stbi__start_file(&s,f);
   result = stbi__load_and_postprocess_8bit(&s,x,y,comp,req_comp);
   if (result) {
      // need to 'unget' all the characters in the IO buffer
      fseek(f, - (int) (s.img_buffer_end - s.img_buffer), SEEK_CUR);
   }
   return result;
}

STBIDEF stbi__uint16 *stbi_load_from_file_16(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   stbi__uint16 *result;
   stbi__context s;
   stbi__start_file(&s,f);
   result = stbi__load_and_postprocess_16bit(&s,x,y,comp,req_comp);
   if (result) {
      // need to 'unget' all the characters in the IO buffer
      fseek(f, - (int) (s.img_buffer_end - s.img_buffer), SEEK_CUR);
   }
   return result;
}

STBIDEF stbi_us *stbi_load_16(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   FILE *f = stbi__fopen(filename, "rb");
   stbi__uint16 *result;
   if (!f) return (stbi_us *) stbi__errpuc("can't fopen", "Unable to open file");
   result = stbi_load_from_file_16(f,x,y,comp,req_comp);
   fclose(f);
   return result;
}


#endif //!STBI_NO_STDIO

STBIDEF stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_mem(&s,buffer,len);
   return stbi__load_and_postprocess_8bit(&s,x,y,comp,req_comp);
}

STBIDEF stbi_uc *stbi_load_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi__load_and_postprocess_8bit(&s,x,y,comp,req_comp);
}

#ifndef STBI_NO_LINEAR
static float *stbi__loadf_main(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
   unsigned char *data;
   #ifndef STBI_NO_HDR
   if (stbi__hdr_test(s)) {
      stbi__result_info ri;
      float *hdr_data = stbi__hdr_load(s,x,y,comp,req_comp, &ri);
      if (hdr_data)
         stbi__float_postprocess(hdr_data,x,y,comp,req_comp);
      return hdr_data;
   }
   #endif
   data = stbi__load_and_postprocess_8bit(s, x, y, comp, req_comp);
   if (data)
      return stbi__ldr_to_hdr(data, *x, *y, req_comp ? req_comp : *comp);
   return stbi__errpf("unknown image type", "Image not of any known type, or corrupt");
}

STBIDEF float *stbi_loadf_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_mem(&s,buffer,len);
   return stbi__loadf_main(&s,x,y,comp,req_comp);
}

STBIDEF float *stbi_loadf_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi__loadf_main(&s,x,y,comp,req_comp);
}

#ifndef STBI_NO_STDIO
STBIDEF float *stbi_loadf(char const *filename, int *x, int *y, int *comp, int req_comp)
{
   float *result;
   FILE *f = stbi__fopen(filename, "rb");
   if (!f) return stbi__errpf("can't fopen", "Unable to open file");
   result = stbi_loadf_from_file(f,x,y,comp,req_comp);
   fclose(f);
   return result;
}

STBIDEF float *stbi_loadf_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   stbi__context s;
   stbi__start_file(&s,f);
   return stbi__loadf_main(&s,x,y,comp,req_comp);
}
#endif // !STBI_NO_STDIO

#endif // !STBI_NO_LINEAR

// these is-hdr-or-not is defined independent of whether STBI_NO_LINEAR is
// defined, for API simplicity; if STBI_NO_LINEAR is defined, it always
// reports false!

STBIDEF int stbi_is_hdr_from_memory(stbi_uc const *buffer, int len)
{
   #ifndef STBI_NO_HDR
   stbi__context s;
   stbi__start_mem(&s,buffer,len);
   return stbi__hdr_test(&s);
   #else
   STBI_NOTUSED(buffer);
   STBI_NOTUSED(len);
   return 0;
   #endif
}

#ifndef STBI_NO_STDIO
STBIDEF int      stbi_is_hdr          (char const *filename)
{
   FILE *f = stbi__fopen(filename, "rb");
   int result=0;
   if (f) {
      result = stbi_is_hdr_from_file(f);
      fclose(f);
   }
   return result;
}

STBIDEF int      stbi_is_hdr_from_file(FILE *f)
{
   #ifndef STBI_NO_HDR
   stbi__context s;
   stbi__start_file(&s,f);
   return stbi__hdr_test(&s);
   #else
   STBI_NOTUSED(f);
   return 0;
   #endif
}
#endif // !STBI_NO_STDIO

STBIDEF int      stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user)
{
   #ifndef STBI_NO_HDR
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
   return stbi__hdr_test(&s);
   #else
   STBI_NOTUSED(clbk);
   STBI_NOTUSED(user);
   return 0;
   #endif
}

#ifndef STBI_NO_LINEAR
static float stbi__l2h_gamma=2.2f, stbi__l2h_scale=1.0f;

STBIDEF void   stbi_ldr_to_hdr_gamma(float gamma) { stbi__l2h_gamma = gamma; }
STBIDEF void   stbi_ldr_to_hdr_scale(float scale) { stbi__l2h_scale = scale; }
#endif

static float stbi__h2l_gamma_i=1.0f/2.2f, stbi__h2l_scale_i=1.0f;

STBIDEF void   stbi_hdr_to_ldr_gamma(float gamma) { stbi__h2l_gamma_i = 1/gamma; }
STBIDEF void   stbi_hdr_to_ldr_scale(float scale) { stbi__h2l_scale_i = 1/scale; }


//////////////////////////////////////////////////////////////////////////////
//
// Common code used by all image loaders
//

enum
{
   STBI__SCAN_load=0,
   STBI__SCAN_type,
   STBI__SCAN_header
};

static void stbi__refill_buffer(stbi__context *s)
{
   int n = (s->io.read)(s->io_user_data,(char*)s->buffer_start,s->buflen);
   if (n == 0) {
      // at end of file, treat same as if from memory, but need to handle case
      // where s->img_buffer isn't pointing to safe memory, e.g. 0-byte file
      s->read_from_callbacks = 0;
      s->img_buffer = s->buffer_start;
      s->img_buffer_end = s->buffer_start+1;
      *s->img_buffer = 0;
   } else {
      s->img_buffer = s->buffer_start;
      s->img_buffer_end = s->buffer_start + n;
   }
}

stbi_inline static stbi_uc stbi__get8(stbi__context *s)
{
   if (s->img_buffer < s->img_buffer_end)
      return *s->img_buffer++;
   if (s->read_from_callbacks) {
      stbi__refill_buffer(s);
      return *s->img_buffer++;
   }
   return 0;
}

stbi_inline static int stbi__at_eof(stbi__context *s)
{
   if (s->io.read) {
      if (!(s->io.eof)(s->io_user_data)) return 0;
      // if feof() is true, check if buffer = end
      // special case: we've only got the special 0 character at the end
      if (s->read_from_callbacks == 0) return 1;
   }

   return s->img_buffer >= s->img_buffer_end;
}

static void stbi__skip(stbi__context *s, int n)
{
   if (n < 0) {
      s->img_buffer = s->img_buffer_end;
      return;
   }
   if (s->io.read) {
      int blen = (int) (s->img_buffer_end - s->img_buffer);
      if (blen < n) {
         s->img_buffer = s->img_buffer_end;
         (s->io.skip)(s->io_user_data, n - blen);
         return;
      }
   }
   s->img_buffer += n;
}

static int stbi__getn(stbi__context *s, stbi_uc *buffer, int n)
{
   if (s->io.read) {
      int blen = (int) (s->img_buffer_end - s->img_buffer);
      if (blen < n) {
         int res, count;

         memcpy(buffer, s->img_buffer, blen);

         count = (s->io.read)(s->io_user_data, (char*) buffer + blen, n - blen);
         res = (count == (n-blen));
         s->img_buffer = s->img_buffer_end;
         return res;
      }
   }

   if (s->img_buffer+n <= s->img_buffer_end) {
      memcpy(buffer, s->img_buffer, n);
      s->img_buffer += n;
      return 1;
   } else
      return 0;
}

static int stbi__get16be(stbi__context *s)
{
   int z = stbi__get8(s);
   return (z << 8) + stbi__get8(s);
}

static stbi__uint32 stbi__get32be(stbi__context *s)
{
   stbi__uint32 z = stbi__get16be(s);
   return (z << 16) + stbi__get16be(s);
}

#if defined(STBI_NO_BMP) && defined(STBI_NO_TGA) && defined(STBI_NO_GIF)
// nothing
#else
static int stbi__get16le(stbi__context *s)
{
   int z = stbi__get8(s);
   return z + (stbi__get8(s) << 8);
}
#endif

#ifndef STBI_NO_BMP
static stbi__uint32 stbi__get32le(stbi__context *s)
{
   stbi__uint32 z = stbi__get16le(s);
   return z + (stbi__get16le(s) << 16);
}
#endif

#define STBI__BYTECAST(x)  ((stbi_uc) ((x) & 255))  // truncate int to byte without warnings


//////////////////////////////////////////////////////////////////////////////
//
//  generic converter from built-in img_n to req_comp
//    individual types do this automatically as much as possible (e.g. jpeg
//    does all cases internally since it needs to colorspace convert anyway,
//    and it never has alpha, so very few cases ). png can automatically
//    interleave an alpha=255 channel, but falls back to this for other cases
//
//  assume data buffer is malloced, so malloc a new one and free that one
//  only failure mode is malloc failing

static stbi_uc stbi__compute_y(int r, int g, int b)
{
   return (stbi_uc) (((r*77) + (g*150) +  (29*b)) >> 8);
}

static unsigned char *stbi__convert_format(unsigned char *data, int img_n, int req_comp, unsigned int x, unsigned int y)
{
   int i,j;
   unsigned char *good;

   if (req_comp == img_n) return data;
   STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

   good = (unsigned char *) stbi__malloc_mad3(req_comp, x, y, 0);
   if (good == NULL) {
      STBI_FREE(data);
      return stbi__errpuc("outofmem", "Out of memory");
   }

   for (j=0; j < (int) y; ++j) {
      unsigned char *src  = data + j * x * img_n   ;
      unsigned char *dest = good + j * x * req_comp;

      #define STBI__COMBO(a,b)  ((a)*8+(b))
      #define STBI__CASE(a,b)   case STBI__COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
      // convert source image with img_n components to one with req_comp components;
      // avoid switch per pixel, so use switch per scanline and massive macros
      switch (STBI__COMBO(img_n, req_comp)) {
         STBI__CASE(1,2) { dest[0]=src[0], dest[1]=255;                                     } break;
         STBI__CASE(1,3) { dest[0]=dest[1]=dest[2]=src[0];                                  } break;
         STBI__CASE(1,4) { dest[0]=dest[1]=dest[2]=src[0], dest[3]=255;                     } break;
         STBI__CASE(2,1) { dest[0]=src[0];                                                  } break;
         STBI__CASE(2,3) { dest[0]=dest[1]=dest[2]=src[0];                                  } break;
         STBI__CASE(2,4) { dest[0]=dest[1]=dest[2]=src[0], dest[3]=src[1];                  } break;
         STBI__CASE(3,4) { dest[0]=src[0],dest[1]=src[1],dest[2]=src[2],dest[3]=255;        } break;
         STBI__CASE(3,1) { dest[0]=stbi__compute_y(src[0],src[1],src[2]);                   } break;
         STBI__CASE(3,2) { dest[0]=stbi__compute_y(src[0],src[1],src[2]), dest[1] = 255;    } break;
         STBI__CASE(4,1) { dest[0]=stbi__compute_y(src[0],src[1],src[2]);                   } break;
         STBI__CASE(4,2) { dest[0]=stbi__compute_y(src[0],src[1],src[2]), dest[1] = src[3]; } break;
         STBI__CASE(4,3) { dest[0]=src[0],dest[1]=src[1],dest[2]=src[2];                    } break;
         default: STBI_ASSERT(0);
      }
      #undef STBI__CASE
   }

   STBI_FREE(data);
   return good;
}

static stbi__uint16 stbi__compute_y_16(int r, int g, int b)
{
   return (stbi__uint16) (((r*77) + (g*150) +  (29*b)) >> 8);
}

static stbi__uint16 *stbi__convert_format16(stbi__uint16 *data, int img_n, int req_comp, unsigned int x, unsigned int y)
{
   int i,j;
   stbi__uint16 *good;

   if (req_comp == img_n) return data;
   STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

   good = (stbi__uint16 *) stbi__malloc(req_comp * x * y * 2);
   if (good == NULL) {
      STBI_FREE(data);
      return (stbi__uint16 *) stbi__errpuc("outofmem", "Out of memory");
   }

   for (j=0; j < (int) y; ++j) {
      stbi__uint16 *src  = data + j * x * img_n   ;
      stbi__uint16 *dest = good + j * x * req_comp;

      #define STBI__COMBO(a,b)  ((a)*8+(b))
      #define STBI__CASE(a,b)   case STBI__COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
      // convert source image with img_n components to one with req_comp components;
      // avoid switch per pixel, so use switch per scanline and massive macros
      switch (STBI__COMBO(img_n, req_comp)) {
         STBI__CASE(1,2) { dest[0]=src[0], dest[1]=0xffff;                                     } break;
         STBI__CASE(1,3) { dest[0]=dest[1]=dest[2]=src[0];                                     } break;
         STBI__CASE(1,4) { dest[0]=dest[1]=dest[2]=src[0], dest[3]=0xffff;                     } break;
         STBI__CASE(2,1) { dest[0]=src[0];                                                     } break;
         STBI__CASE(2,3) { dest[0]=dest[1]=dest[2]=src[0];                                     } break;
         STBI__CASE(2,4) { dest[0]=dest[1]=dest[2]=src[0], dest[3]=src[1];                     } break;
         STBI__CASE(3,4) { dest[0]=src[0],dest[1]=src[1],dest[2]=src[2],dest[3]=0xffff;        } break;
         STBI__CASE(3,1) { dest[0]=stbi__compute_y_16(src[0],src[1],src[2]);                   } break;
         STBI__CASE(3,2) { dest[0]=stbi__compute_y_16(src[0],src[1],src[2]), dest[1] = 0xffff; } break;
         STBI__CASE(4,1) { dest[0]=stbi__compute_y_16(src[0],src[1],src[2]);                   } break;
         STBI__CASE(4,2) { dest[0]=stbi__compute_y_16(src[0],src[1],src[2]), dest[1] = src[3]; } break;
         STBI__CASE(4,3) { dest[0]=src[0],dest[1]=src[1],dest[2]=src[2];                       } break;
         default: STBI_ASSERT(0);
      }
      #undef STBI__CASE
   }

   STBI_FREE(data);
   return good;
}

#ifndef STBI_NO_LINEAR
static float   *stbi__ldr_to_hdr(stbi_uc *data, int x, int y, int comp)
{
   int i,k,n;
   float *output;
   if (!data) return NULL;
   output = (float *) stbi__malloc_mad4(x, y, comp, sizeof(float), 0);
   if (output == NULL) { STBI_FREE(data); return stbi__errpf("outofmem", "Out of memory"); }
   // compute number of non-alpha components
   if (comp & 1) n = comp; else n = comp-1;
   for (i=0; i < x*y; ++i) {
      for (k=0; k < n; ++k) {
         output[i*comp + k] = (float) (pow(data[i*comp+k]/255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
      }
      if (k < comp) output[i*comp + k] = data[i*comp+k]/255.0f;
   }
   STBI_FREE(data);
   return output;
}
#endif

#ifndef STBI_NO_HDR
#define stbi__float2int(x)   ((int) (x))
static stbi_uc *stbi__hdr_to_ldr(float   *data, int x, int y, int comp)
{
   int i,k,n;
   stbi_uc *output;
   if (!data) return NULL;
   output = (stbi_uc *) stbi__malloc_mad3(x, y, comp, 0);
   if (output == NULL) { STBI_FREE(data); return stbi__errpuc("outofmem", "Out of memory"); }
   // compute number of non-alpha components
   if (comp & 1) n = comp; else n = comp-1;
   for (i=0; i < x*y; ++i) {
      for (k=0; k < n; ++k) {
         float z = (float) pow(data[i*comp+k]*stbi__h2l_scale_i, stbi__h2l_gamma_i) * 255 + 0.5f;
         if (z < 0) z = 0;
         if (z > 255) z = 255;
         output[i*comp + k] = (stbi_uc) stbi__float2int(z);
      }
      if (k < comp) {
         float z = data[i*comp+k] * 255 + 0.5f;
         if (z < 0) z = 0;
         if (z > 255) z = 255;
         output[i*comp + k] = (stbi_uc) stbi__float2int(z);
      }
   }
   STBI_FREE(data);
   return output;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//  "baseline" JPEG/JFIF decoder
//
//    simple implementation
//      - doesn't support delayed output of y-dimension
//      - simple interface (only one output format: 8-bit interleaved RGB)
//      - doesn't try to recover corrupt jpegs
//      - doesn't allow partial loading, loading multiple at once
//      - still fast on x86 (copying globals into locals doesn't help x86)
//      - allocates lots of intermediate memory (full size of all components)
//        - non-interleaved case requires this anyway
//        - allows good upsampling (see next)
//    high-quality
//      - upsampled channels are bilinearly interpolated, even across blocks
//      - quality integer IDCT derived from IJG's 'slow'
//    performance
//      - fast huffman; reasonable integer IDCT
//      - some SIMD kernels for common paths on targets with SSE2/NEON
//      - uses a lot of intermediate memory, could cache poorly

#ifndef STBI_NO_JPEG

// huffman decoding acceleration
#define FAST_BITS   9  // larger handles more cases; smaller stomps less cache

typedef struct
{
   stbi_uc  fast[1 << FAST_BITS];
   // weirdly, repacking this into AoS is a 10% speed loss, instead of a win
   stbi__uint16 code[256];
   stbi_uc  values[256];
   stbi_uc  size[257];
   unsigned int maxcode[18];
   int    delta[17];   // old 'firstsymbol' - old 'firstcode'
} stbi__huffman;

typedef struct
{
   stbi__context *s;
   stbi__huffman huff_dc[4];
   stbi__huffman huff_ac[4];
   stbi_uc dequant[4][64];
   stbi__int16 fast_ac[4][1 << FAST_BITS];

// sizes for components, interleaved MCUs
   int img_h_max, img_v_max;
   int img_mcu_x, img_mcu_y;
   int img_mcu_w, img_mcu_h;

// definition of jpeg image component
   struct
   {
      int id;
      int h,v;
      int tq;
      int hd,ha;
      int dc_pred;

      int x,y,w2,h2;
      stbi_uc *data;
      void *raw_data, *raw_coeff;
      stbi_uc *linebuf;
      short   *coeff;   // progressive only
      int      coeff_w, coeff_h; // number of 8x8 coefficient blocks
   } img_comp[4];

   stbi__uint32   code_buffer; // jpeg entropy-coded buffer
   int            code_bits;   // number of valid bits
   unsigned char  marker;      // marker seen while filling entropy buffer
   int            nomore;      // flag if we saw a marker so must stop

   int            progressive;
   int            spec_start;
   int            spec_end;
   int            succ_high;
   int            succ_low;
   int            eob_run;
   int            rgb;

   int scan_n, order[4];
   int restart_interval, todo;

// kernels
   void (*idct_block_kernel)(stbi_uc *out, int out_stride, short data[64]);
   void (*YCbCr_to_RGB_kernel)(stbi_uc *out, const stbi_uc *y, const stbi_uc *pcb, const stbi_uc *pcr, int count, int step);
   stbi_uc *(*resample_row_hv_2_kernel)(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs);
} stbi__jpeg;

static int stbi__build_huffman(stbi__huffman *h, int *count)
{
   int i,j,k=0,code;
   // build size list for each symbol (from JPEG spec)
   for (i=0; i < 16; ++i)
      for (j=0; j < count[i]; ++j)
         h->size[k++] = (stbi_uc) (i+1);
   h->size[k] = 0;

   // compute actual symbols (from jpeg spec)
   code = 0;
   k = 0;
   for(j=1; j <= 16; ++j) {
      // compute delta to add to code to compute symbol id
      h->delta[j] = k - code;
      if (h->size[k] == j) {
         while (h->size[k] == j)
            h->code[k++] = (stbi__uint16) (code++);
         if (code-1 >= (1 << j)) return stbi__err("bad code lengths","Corrupt JPEG");
      }
      // compute largest code + 1 for this size, preshifted as needed later
      h->maxcode[j] = code << (16-j);
      code <<= 1;
   }
   h->maxcode[j] = 0xffffffff;

   // build non-spec acceleration table; 255 is flag for not-accelerated
   memset(h->fast, 255, 1 << FAST_BITS);
   for (i=0; i < k; ++i) {
      int s = h->size[i];
      if (s <= FAST_BITS) {
         int c = h->code[i] << (FAST_BITS-s);
         int m = 1 << (FAST_BITS-s);
         for (j=0; j < m; ++j) {
            h->fast[c+j] = (stbi_uc) i;
         }
      }
   }
   return 1;
}

// build a table that decodes both magnitude and value of small ACs in
// one go.
static void stbi__build_fast_ac(stbi__int16 *fast_ac, stbi__huffman *h)
{
   int i;
   for (i=0; i < (1 << FAST_BITS); ++i) {
      stbi_uc fast = h->fast[i];
      fast_ac[i] = 0;
      if (fast < 255) {
         int rs = h->values[fast];
         int run = (rs >> 4) & 15;
         int magbits = rs & 15;
         int len = h->size[fast];

         if (magbits && len + magbits <= FAST_BITS) {
            // magnitude code followed by receive_extend code
            int k = ((i << len) & ((1 << FAST_BITS) - 1)) >> (FAST_BITS - magbits);
            int m = 1 << (magbits - 1);
            if (k < m) k += (-1 << magbits) + 1;
            // if the result is small enough, we can fit it in fast_ac table
            if (k >= -128 && k <= 127)
               fast_ac[i] = (stbi__int16) ((k << 8) + (run << 4) + (len + magbits));
         }
      }
   }
}

static void stbi__grow_buffer_unsafe(stbi__jpeg *j)
{
   do {
      int b = j->nomore ? 0 : stbi__get8(j->s);
      if (b == 0xff) {
         int c = stbi__get8(j->s);
         if (c != 0) {
            j->marker = (unsigned char) c;
            j->nomore = 1;
            return;
         }
      }
      j->code_buffer |= b << (24 - j->code_bits);
      j->code_bits += 8;
   } while (j->code_bits <= 24);
}

// (1 << n) - 1
static stbi__uint32 stbi__bmask[17]={0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};

// decode a jpeg huffman value from the bitstream
stbi_inline static int stbi__jpeg_huff_decode(stbi__jpeg *j, stbi__huffman *h)
{
   unsigned int temp;
   int c,k;

   if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);

   // look at the top FAST_BITS and determine what symbol ID it is,
   // if the code is <= FAST_BITS
   c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS)-1);
   k = h->fast[c];
   if (k < 255) {
      int s = h->size[k];
      if (s > j->code_bits)
         return -1;
      j->code_buffer <<= s;
      j->code_bits -= s;
      return h->values[k];
   }

   // naive test is to shift the code_buffer down so k bits are
   // valid, then test against maxcode. To speed this up, we've
   // preshifted maxcode left so that it has (16-k) 0s at the
   // end; in other words, regardless of the number of bits, it
   // wants to be compared against something shifted to have 16;
   // that way we don't need to shift inside the loop.
   temp = j->code_buffer >> 16;
   for (k=FAST_BITS+1 ; ; ++k)
      if (temp < h->maxcode[k])
         break;
   if (k == 17) {
      // error! code not found
      j->code_bits -= 16;
      return -1;
   }

   if (k > j->code_bits)
      return -1;

   // convert the huffman code to the symbol id
   c = ((j->code_buffer >> (32 - k)) & stbi__bmask[k]) + h->delta[k];
   STBI_ASSERT((((j->code_buffer) >> (32 - h->size[c])) & stbi__bmask[h->size[c]]) == h->code[c]);

   // convert the id to a symbol
   j->code_bits -= k;
   j->code_buffer <<= k;
   return h->values[c];
}

// bias[n] = (-1<<n) + 1
static int const stbi__jbias[16] = {0,-1,-3,-7,-15,-31,-63,-127,-255,-511,-1023,-2047,-4095,-8191,-16383,-32767};

// combined JPEG 'receive' and JPEG 'extend', since baseline
// always extends everything it receives.
stbi_inline static int stbi__extend_receive(stbi__jpeg *j, int n)
{
   unsigned int k;
   int sgn;
   if (j->code_bits < n) stbi__grow_buffer_unsafe(j);

   sgn = (stbi__int32)j->code_buffer >> 31; // sign bit is always in MSB
   k = stbi_lrot(j->code_buffer, n);
   STBI_ASSERT(n >= 0 && n < (int) (sizeof(stbi__bmask)/sizeof(*stbi__bmask)));
   j->code_buffer = k & ~stbi__bmask[n];
   k &= stbi__bmask[n];
   j->code_bits -= n;
   return k + (stbi__jbias[n] & ~sgn);
}

// get some unsigned bits
stbi_inline static int stbi__jpeg_get_bits(stbi__jpeg *j, int n)
{
   unsigned int k;
   if (j->code_bits < n) stbi__grow_buffer_unsafe(j);
   k = stbi_lrot(j->code_buffer, n);
   j->code_buffer = k & ~stbi__bmask[n];
   k &= stbi__bmask[n];
   j->code_bits -= n;
   return k;
}

stbi_inline static int stbi__jpeg_get_bit(stbi__jpeg *j)
{
   unsigned int k;
   if (j->code_bits < 1) stbi__grow_buffer_unsafe(j);
   k = j->code_buffer;
   j->code_buffer <<= 1;
   --j->code_bits;
   return k & 0x80000000;
}

// given a value that's at position X in the zigzag stream,
// where does it appear in the 8x8 matrix coded as row-major?
static stbi_uc stbi__jpeg_dezigzag[64+15] =
{
    0,  1,  8, 16,  9,  2,  3, 10,
   17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34,
   27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36,
   29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46,
   53, 60, 61, 54, 47, 55, 62, 63,
   // let corrupt input sample past end
   63, 63, 63, 63, 63, 63, 63, 63,
   63, 63, 63, 63, 63, 63, 63
};

// decode one 64-entry block--
static int stbi__jpeg_decode_block(stbi__jpeg *j, short data[64], stbi__huffman *hdc, stbi__huffman *hac, stbi__int16 *fac, int b, stbi_uc *dequant)
{
   int diff,dc,k;
   int t;

   if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);
   t = stbi__jpeg_huff_decode(j, hdc);
   if (t < 0) return stbi__err("bad huffman code","Corrupt JPEG");

   // 0 all the ac values now so we can do it 32-bits at a time
   memset(data,0,64*sizeof(data[0]));

   diff = t ? stbi__extend_receive(j, t) : 0;
   dc = j->img_comp[b].dc_pred + diff;
   j->img_comp[b].dc_pred = dc;
   data[0] = (short) (dc * dequant[0]);

   // decode AC components, see JPEG spec
   k = 1;
   do {
      unsigned int zig;
      int c,r,s;
      if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);
      c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS)-1);
      r = fac[c];
      if (r) { // fast-AC path
         k += (r >> 4) & 15; // run
         s = r & 15; // combined length
         j->code_buffer <<= s;
         j->code_bits -= s;
         // decode into unzigzag'd location
         zig = stbi__jpeg_dezigzag[k++];
         data[zig] = (short) ((r >> 8) * dequant[zig]);
      } else {
         int rs = stbi__jpeg_huff_decode(j, hac);
         if (rs < 0) return stbi__err("bad huffman code","Corrupt JPEG");
         s = rs & 15;
         r = rs >> 4;
         if (s == 0) {
            if (rs != 0xf0) break; // end block
            k += 16;
         } else {
            k += r;
            // decode into unzigzag'd location
            zig = stbi__jpeg_dezigzag[k++];
            data[zig] = (short) (stbi__extend_receive(j,s) * dequant[zig]);
         }
      }
   } while (k < 64);
   return 1;
}

static int stbi__jpeg_decode_block_prog_dc(stbi__jpeg *j, short data[64], stbi__huffman *hdc, int b)
{
   int diff,dc;
   int t;
   if (j->spec_end != 0) return stbi__err("can't merge dc and ac", "Corrupt JPEG");

   if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);

   if (j->succ_high == 0) {
      // first scan for DC coefficient, must be first
      memset(data,0,64*sizeof(data[0])); // 0 all the ac values now
      t = stbi__jpeg_huff_decode(j, hdc);
      diff = t ? stbi__extend_receive(j, t) : 0;

      dc = j->img_comp[b].dc_pred + diff;
      j->img_comp[b].dc_pred = dc;
      data[0] = (short) (dc << j->succ_low);
   } else {
      // refinement scan for DC coefficient
      if (stbi__jpeg_get_bit(j))
         data[0] += (short) (1 << j->succ_low);
   }
   return 1;
}

// @OPTIMIZE: store non-zigzagged during the decode passes,
// and only de-zigzag when dequantizing
static int stbi__jpeg_decode_block_prog_ac(stbi__jpeg *j, short data[64], stbi__huffman *hac, stbi__int16 *fac)
{
   int k;
   if (j->spec_start == 0) return stbi__err("can't merge dc and ac", "Corrupt JPEG");

   if (j->succ_high == 0) {
      int shift = j->succ_low;

      if (j->eob_run) {
         --j->eob_run;
         return 1;
      }

      k = j->spec_start;
      do {
         unsigned int zig;
         int c,r,s;
         if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);
         c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS)-1);
         r = fac[c];
         if (r) { // fast-AC path
            k += (r >> 4) & 15; // run
            s = r & 15; // combined length
            j->code_buffer <<= s;
            j->code_bits -= s;
            zig = stbi__jpeg_dezigzag[k++];
            data[zig] = (short) ((r >> 8) << shift);
         } else {
            int rs = stbi__jpeg_huff_decode(j, hac);
            if (rs < 0) return stbi__err("bad huffman code","Corrupt JPEG");
            s = rs & 15;
            r = rs >> 4;
            if (s == 0) {
               if (r < 15) {
                  j->eob_run = (1 << r);
                  if (r)
                     j->eob_run += stbi__jpeg_get_bits(j, r);
                  --j->eob_run;
                  break;
               }
               k += 16;
            } else {
               k += r;
               zig = stbi__jpeg_dezigzag[k++];
               data[zig] = (short) (stbi__extend_receive(j,s) << shift);
            }
         }
      } while (k <= j->spec_end);
   } else {
      // refinement scan for these AC coefficients

      short bit = (short) (1 << j->succ_low);

      if (j->eob_run) {
         --j->eob_run;
         for (k = j->spec_start; k <= j->spec_end; ++k) {
            short *p = &data[stbi__jpeg_dezigzag[k]];
            if (*p != 0)
               if (stbi__jpeg_get_bit(j))
                  if ((*p & bit)==0) {
                     if (*p > 0)
                        *p += bit;
                     else
                        *p -= bit;
                  }
         }
      } else {
         k = j->spec_start;
         do {
            int r,s;
            int rs = stbi__jpeg_huff_decode(j, hac); // @OPTIMIZE see if we can use the fast path here, advance-by-r is so slow, eh
            if (rs < 0) return stbi__err("bad huffman code","Corrupt JPEG");
            s = rs & 15;
            r = rs >> 4;
            if (s == 0) {
               if (r < 15) {
                  j->eob_run = (1 << r) - 1;
                  if (r)
                     j->eob_run += stbi__jpeg_get_bits(j, r);
                  r = 64; // force end of block
               } else {
                  // r=15 s=0 should write 16 0s, so we just do
                  // a run of 15 0s and then write s (which is 0),
                  // so we don't have to do anything special here
               }
            } else {
               if (s != 1) return stbi__err("bad huffman code", "Corrupt JPEG");
               // sign bit
               if (stbi__jpeg_get_bit(j))
                  s = bit;
               else
                  s = -bit;
            }

            // advance by r
            while (k <= j->spec_end) {
               short *p = &data[stbi__jpeg_dezigzag[k++]];
               if (*p != 0) {
                  if (stbi__jpeg_get_bit(j))
                     if ((*p & bit)==0) {
                        if (*p > 0)
                           *p += bit;
                        else
                           *p -= bit;
                     }
               } else {
                  if (r == 0) {
                     *p = (short) s;
                     break;
                  }
                  --r;
               }
            }
         } while (k <= j->spec_end);
      }
   }
   return 1;
}

// take a -128..127 value and stbi__clamp it and convert to 0..255
stbi_inline static stbi_uc stbi__clamp(int x)
{
   // trick to use a single test to catch both cases
   if ((unsigned int) x > 255) {
      if (x < 0) return 0;
      if (x > 255) return 255;
   }
   return (stbi_uc) x;
}

#define stbi__f2f(x)  ((int) (((x) * 4096 + 0.5)))
#define stbi__fsh(x)  ((x) << 12)

// derived from jidctint -- DCT_ISLOW
#define STBI__IDCT_1D(s0,s1,s2,s3,s4,s5,s6,s7) \
   int t0,t1,t2,t3,p1,p2,p3,p4,p5,x0,x1,x2,x3; \
   p2 = s2;                                    \
   p3 = s6;                                    \
   p1 = (p2+p3) * stbi__f2f(0.5411961f);       \
   t2 = p1 + p3*stbi__f2f(-1.847759065f);      \
   t3 = p1 + p2*stbi__f2f( 0.765366865f);      \
   p2 = s0;                                    \
   p3 = s4;                                    \
   t0 = stbi__fsh(p2+p3);                      \
   t1 = stbi__fsh(p2-p3);                      \
   x0 = t0+t3;                                 \
   x3 = t0-t3;                                 \
   x1 = t1+t2;                                 \
   x2 = t1-t2;                                 \
   t0 = s7;                                    \
   t1 = s5;                                    \
   t2 = s3;                                    \
   t3 = s1;                                    \
   p3 = t0+t2;                                 \
   p4 = t1+t3;                                 \
   p1 = t0+t3;                                 \
   p2 = t1+t2;                                 \
   p5 = (p3+p4)*stbi__f2f( 1.175875602f);      \
   t0 = t0*stbi__f2f( 0.298631336f);           \
   t1 = t1*stbi__f2f( 2.053119869f);           \
   t2 = t2*stbi__f2f( 3.072711026f);           \
   t3 = t3*stbi__f2f( 1.501321110f);           \
   p1 = p5 + p1*stbi__f2f(-0.899976223f);      \
   p2 = p5 + p2*stbi__f2f(-2.562915447f);      \
   p3 = p3*stbi__f2f(-1.961570560f);           \
   p4 = p4*stbi__f2f(-0.390180644f);           \
   t3 += p1+p4;                                \
   t2 += p2+p3;                                \
   t1 += p2+p4;                                \
   t0 += p1+p3;

static void stbi__idct_block(stbi_uc *out, int out_stride, short data[64])
{
   int i,val[64],*v=val;
   stbi_uc *o;
   short *d = data;

   // columns
   for (i=0; i < 8; ++i,++d, ++v) {
      // if all zeroes, shortcut -- this avoids dequantizing 0s and IDCTing
      if (d[ 8]==0 && d[16]==0 && d[24]==0 && d[32]==0
           && d[40]==0 && d[48]==0 && d[56]==0) {
         //    no shortcut                 0     seconds
         //    (1|2|3|4|5|6|7)==0          0     seconds
         //    all separate               -0.047 seconds
         //    1 && 2|3 && 4|5 && 6|7:    -0.047 seconds
         int dcterm = d[0] << 2;
         v[0] = v[8] = v[16] = v[24] = v[32] = v[40] = v[48] = v[56] = dcterm;
      } else {
         STBI__IDCT_1D(d[ 0],d[ 8],d[16],d[24],d[32],d[40],d[48],d[56])
         // constants scaled things up by 1<<12; let's bring them back
         // down, but keep 2 extra bits of precision
         x0 += 512; x1 += 512; x2 += 512; x3 += 512;
         v[ 0] = (x0+t3) >> 10;
         v[56] = (x0-t3) >> 10;
         v[ 8] = (x1+t2) >> 10;
         v[48] = (x1-t2) >> 10;
         v[16] = (x2+t1) >> 10;
         v[40] = (x2-t1) >> 10;
         v[24] = (x3+t0) >> 10;
         v[32] = (x3-t0) >> 10;
      }
   }

   for (i=0, v=val, o=out; i < 8; ++i,v+=8,o+=out_stride) {
      // no fast case since the first 1D IDCT spread components out
      STBI__IDCT_1D(v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7])
      // constants scaled things up by 1<<12, plus we had 1<<2 from first
      // loop, plus horizontal and vertical each scale by sqrt(8) so together
      // we've got an extra 1<<3, so 1<<17 total we need to remove.
      // so we want to round that, which means adding 0.5 * 1<<17,
      // aka 65536. Also, we'll end up with -128 to 127 that we want
      // to encode as 0..255 by adding 128, so we'll add that before the shift
      x0 += 65536 + (128<<17);
      x1 += 65536 + (128<<17);
      x2 += 65536 + (128<<17);
      x3 += 65536 + (128<<17);
      // tried computing the shifts into temps, or'ing the temps to see
      // if any were out of range, but that was slower
      o[0] = stbi__clamp((x0+t3) >> 17);
      o[7] = stbi__clamp((x0-t3) >> 17);
      o[1] = stbi__clamp((x1+t2) >> 17);
      o[6] = stbi__clamp((x1-t2) >> 17);
      o[2] = stbi__clamp((x2+t1) >> 17);
      o[5] = stbi__clamp((x2-t1) >> 17);
      o[3] = stbi__clamp((x3+t0) >> 17);
      o[4] = stbi__clamp((x3-t0) >> 17);
   }
}

#ifdef STBI_SSE2
// sse2 integer IDCT. not the fastest possible implementation but it
// produces bit-identical results to the generic C version so it's
// fully "transparent".
static void stbi__idct_simd(stbi_uc *out, int out_stride, short data[64])
{
   // This is constructed to match our regular (generic) integer IDCT exactly.
   __m128i row0, row1, row2, row3, row4, row5, row6, row7;
   __m128i tmp;

   // dot product constant: even elems=x, odd elems=y
   #define dct_const(x,y)  _mm_setr_epi16((x),(y),(x),(y),(x),(y),(x),(y))

   // out(0) = c0[even]*x + c0[odd]*y   (c0, x, y 16-bit, out 32-bit)
   // out(1) = c1[even]*x + c1[odd]*y
   #define dct_rot(out0,out1, x,y,c0,c1) \
      __m128i c0##lo = _mm_unpacklo_epi16((x),(y)); \
      __m128i c0##hi = _mm_unpackhi_epi16((x),(y)); \
      __m128i out0##_l = _mm_madd_epi16(c0##lo, c0); \
      __m128i out0##_h = _mm_madd_epi16(c0##hi, c0); \
      __m128i out1##_l = _mm_madd_epi16(c0##lo, c1); \
      __m128i out1##_h = _mm_madd_epi16(c0##hi, c1)

   // out = in << 12  (in 16-bit, out 32-bit)
   #define dct_widen(out, in) \
      __m128i out##_l = _mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), (in)), 4); \
      __m128i out##_h = _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), (in)), 4)

   // wide add
   #define dct_wadd(out, a, b) \
      __m128i out##_l = _mm_add_epi32(a##_l, b##_l); \
      __m128i out##_h = _mm_add_epi32(a##_h, b##_h)

   // wide sub
   #define dct_wsub(out, a, b) \
      __m128i out##_l = _mm_sub_epi32(a##_l, b##_l); \
      __m128i out##_h = _mm_sub_epi32(a##_h, b##_h)

   // butterfly a/b, add bias, then shift by "s" and pack
   #define dct_bfly32o(out0, out1, a,b,bias,s) \
      { \
         __m128i abiased_l = _mm_add_epi32(a##_l, bias); \
         __m128i abiased_h = _mm_add_epi32(a##_h, bias); \
         dct_wadd(sum, abiased, b); \
         dct_wsub(dif, abiased, b); \
         out0 = _mm_packs_epi32(_mm_srai_epi32(sum_l, s), _mm_srai_epi32(sum_h, s)); \
         out1 = _mm_packs_epi32(_mm_srai_epi32(dif_l, s), _mm_srai_epi32(dif_h, s)); \
      }

   // 8-bit interleave step (for transposes)
   #define dct_interleave8(a, b) \
      tmp = a; \
      a = _mm_unpacklo_epi8(a, b); \
      b = _mm_unpackhi_epi8(tmp, b)

   // 16-bit interleave step (for transposes)
   #define dct_interleave16(a, b) \
      tmp = a; \
      a = _mm_unpacklo_epi16(a, b); \
      b = _mm_unpackhi_epi16(tmp, b)

   #define dct_pass(bias,shift) \
      { \
         /* even part */ \
         dct_rot(t2e,t3e, row2,row6, rot0_0,rot0_1); \
         __m128i sum04 = _mm_add_epi16(row0, row4); \
         __m128i dif04 = _mm_sub_epi16(row0, row4); \
         dct_widen(t0e, sum04); \
         dct_widen(t1e, dif04); \
         dct_wadd(x0, t0e, t3e); \
         dct_wsub(x3, t0e, t3e); \
         dct_wadd(x1, t1e, t2e); \
         dct_wsub(x2, t1e, t2e); \
         /* odd part */ \
         dct_rot(y0o,y2o, row7,row3, rot2_0,rot2_1); \
         dct_rot(y1o,y3o, row5,row1, rot3_0,rot3_1); \
         __m128i sum17 = _mm_add_epi16(row1, row7); \
         __m128i sum35 = _mm_add_epi16(row3, row5); \
         dct_rot(y4o,y5o, sum17,sum35, rot1_0,rot1_1); \
         dct_wadd(x4, y0o, y4o); \
         dct_wadd(x5, y1o, y5o); \
         dct_wadd(x6, y2o, y5o); \
         dct_wadd(x7, y3o, y4o); \
         dct_bfly32o(row0,row7, x0,x7,bias,shift); \
         dct_bfly32o(row1,row6, x1,x6,bias,shift); \
         dct_bfly32o(row2,row5, x2,x5,bias,shift); \
         dct_bfly32o(row3,row4, x3,x4,bias,shift); \
      }

   __m128i rot0_0 = dct_const(stbi__f2f(0.5411961f), stbi__f2f(0.5411961f) + stbi__f2f(-1.847759065f));
   __m128i rot0_1 = dct_const(stbi__f2f(0.5411961f) + stbi__f2f( 0.765366865f), stbi__f2f(0.5411961f));
   __m128i rot1_0 = dct_const(stbi__f2f(1.175875602f) + stbi__f2f(-0.899976223f), stbi__f2f(1.175875602f));
   __m128i rot1_1 = dct_const(stbi__f2f(1.175875602f), stbi__f2f(1.175875602f) + stbi__f2f(-2.562915447f));
   __m128i rot2_0 = dct_const(stbi__f2f(-1.961570560f) + stbi__f2f( 0.298631336f), stbi__f2f(-1.961570560f));
   __m128i rot2_1 = dct_const(stbi__f2f(-1.961570560f), stbi__f2f(-1.961570560f) + stbi__f2f( 3.072711026f));
   __m128i rot3_0 = dct_const(stbi__f2f(-0.390180644f) + stbi__f2f( 2.053119869f), stbi__f2f(-0.390180644f));
   __m128i rot3_1 = dct_const(stbi__f2f(-0.390180644f), stbi__f2f(-0.390180644f) + stbi__f2f( 1.501321110f));

   // rounding biases in column/row passes, see stbi__idct_block for explanation.
   __m128i bias_0 = _mm_set1_epi32(512);
   __m128i bias_1 = _mm_set1_epi32(65536 + (128<<17));

   // load
   row0 = _mm_load_si128((const __m128i *) (data + 0*8));
   row1 = _mm_load_si128((const __m128i *) (data + 1*8));
   row2 = _mm_load_si128((const __m128i *) (data + 2*8));
   row3 = _mm_load_si128((const __m128i *) (data + 3*8));
   row4 = _mm_load_si128((const __m128i *) (data + 4*8));
   row5 = _mm_load_si128((const __m128i *) (data + 5*8));
   row6 = _mm_load_si128((const __m128i *) (data + 6*8));
   row7 = _mm_load_si128((const __m128i *) (data + 7*8));

   // column pass
   dct_pass(bias_0, 10);

   {
      // 16bit 8x8 transpose pass 1
      dct_interleave16(row0, row4);
      dct_interleave16(row1, row5);
      dct_interleave16(row2, row6);
      dct_interleave16(row3, row7);

      // transpose pass 2
      dct_interleave16(row0, row2);
      dct_interleave16(row1, row3);
      dct_interleave16(row4, row6);
      dct_interleave16(row5, row7);

      // transpose pass 3
      dct_interleave16(row0, row1);
      dct_interleave16(row2, row3);
      dct_interleave16(row4, row5);
      dct_interleave16(row6, row7);
   }

   // row pass
   dct_pass(bias_1, 17);

   {
      // pack
      __m128i p0 = _mm_packus_epi16(row0, row1); // a0a1a2a3...a7b0b1b2b3...b7
      __m128i p1 = _mm_packus_epi16(row2, row3);
      __m128i p2 = _mm_packus_epi16(row4, row5);
      __m128i p3 = _mm_packus_epi16(row6, row7);

      // 8bit 8x8 transpose pass 1
      dct_interleave8(p0, p2); // a0e0a1e1...
      dct_interleave8(p1, p3); // c0g0c1g1...

      // transpose pass 2
      dct_interleave8(p0, p1); // a0c0e0g0...
      dct_interleave8(p2, p3); // b0d0f0h0...

      // transpose pass 3
      dct_interleave8(p0, p2); // a0b0c0d0...
      dct_interleave8(p1, p3); // a4b4c4d4...

      // store
      _mm_storel_epi64((__m128i *) out, p0); out += out_stride;
      _mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p0, 0x4e)); out += out_stride;
      _mm_storel_epi64((__m128i *) out, p2); out += out_stride;
      _mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p2, 0x4e)); out += out_stride;
      _mm_storel_epi64((__m128i *) out, p1); out += out_stride;
      _mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p1, 0x4e)); out += out_stride;
      _mm_storel_epi64((__m128i *) out, p3); out += out_stride;
      _mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p3, 0x4e));
   }

#undef dct_const
#undef dct_rot
#undef dct_widen
#undef dct_wadd
#undef dct_wsub
#undef dct_bfly32o
#undef dct_interleave8
#undef dct_interleave16
#undef dct_pass
}

#endif // STBI_SSE2

#ifdef STBI_NEON

// NEON integer IDCT. should produce bit-identical
// results to the generic C version.
static void stbi__idct_simd(stbi_uc *out, int out_stride, short data[64])
{
   int16x8_t row0, row1, row2, row3, row4, row5, row6, row7;

   int16x4_t rot0_0 = vdup_n_s16(stbi__f2f(0.5411961f));
   int16x4_t rot0_1 = vdup_n_s16(stbi__f2f(-1.847759065f));
   int16x4_t rot0_2 = vdup_n_s16(stbi__f2f( 0.765366865f));
   int16x4_t rot1_0 = vdup_n_s16(stbi__f2f( 1.175875602f));
   int16x4_t rot1_1 = vdup_n_s16(stbi__f2f(-0.899976223f));
   int16x4_t rot1_2 = vdup_n_s16(stbi__f2f(-2.562915447f));
   int16x4_t rot2_0 = vdup_n_s16(stbi__f2f(-1.961570560f));
   int16x4_t rot2_1 = vdup_n_s16(stbi__f2f(-0.390180644f));
   int16x4_t rot3_0 = vdup_n_s16(stbi__f2f( 0.298631336f));
   int16x4_t rot3_1 = vdup_n_s16(stbi__f2f( 2.053119869f));
   int16x4_t rot3_2 = vdup_n_s16(stbi__f2f( 3.072711026f));
   int16x4_t rot3_3 = vdup_n_s16(stbi__f2f( 1.501321110f));

#define dct_long_mul(out, inq, coeff) \
   int32x4_t out##_l = vmull_s16(vget_low_s16(inq), coeff); \
   int32x4_t out##_h = vmull_s16(vget_high_s16(inq), coeff)

#define dct_long_mac(out, acc, inq, coeff) \
   int32x4_t out##_l = vmlal_s16(acc##_l, vget_low_s16(inq), coeff); \
   int32x4_t out##_h = vmlal_s16(acc##_h, vget_high_s16(inq), coeff)

#define dct_widen(out, inq) \
   int32x4_t out##_l = vshll_n_s16(vget_low_s16(inq), 12); \
   int32x4_t out##_h = vshll_n_s16(vget_high_s16(inq), 12)

// wide add
#define dct_wadd(out, a, b) \
   int32x4_t out##_l = vaddq_s32(a##_l, b##_l); \
   int32x4_t out##_h = vaddq_s32(a##_h, b##_h)

// wide sub
#define dct_wsub(out, a, b) \
   int32x4_t out##_l = vsubq_s32(a##_l, b##_l); \
   int32x4_t out##_h = vsubq_s32(a##_h, b##_h)

// butterfly a/b, then shift using "shiftop" by "s" and pack
#define dct_bfly32o(out0,out1, a,b,shiftop,s) \
   { \
      dct_wadd(sum, a, b); \
      dct_wsub(dif, a, b); \
      out0 = vcombine_s16(shiftop(sum_l, s), shiftop(sum_h, s)); \
      out1 = vcombine_s16(shiftop(dif_l, s), shiftop(dif_h, s)); \
   }

#define dct_pass(shiftop, shift) \
   { \
      /* even part */ \
      int16x8_t sum26 = vaddq_s16(row2, row6); \
      dct_long_mul(p1e, sum26, rot0_0); \
      dct_long_mac(t2e, p1e, row6, rot0_1); \
      dct_long_mac(t3e, p1e, row2, rot0_2); \
      int16x8_t sum04 = vaddq_s16(row0, row4); \
      int16x8_t dif04 = vsubq_s16(row0, row4); \
      dct_widen(t0e, sum04); \
      dct_widen(t1e, dif04); \
      dct_wadd(x0, t0e, t3e); \
      dct_wsub(x3, t0e, t3e); \
      dct_wadd(x1, t1e, t2e); \
      dct_wsub(x2, t1e, t2e); \
      /* odd part */ \
      int16x8_t sum15 = vaddq_s16(row1, row5); \
      int16x8_t sum17 = vaddq_s16(row1, row7); \
      int16x8_t sum35 = vaddq_s16(row3, row5); \
      int16x8_t sum37 = vaddq_s16(row3, row7); \
      int16x8_t sumodd = vaddq_s16(sum17, sum35); \
      dct_long_mul(p5o, sumodd, rot1_0); \
      dct_long_mac(p1o, p5o, sum17, rot1_1); \
      dct_long_mac(p2o, p5o, sum35, rot1_2); \
      dct_long_mul(p3o, sum37, rot2_0); \
      dct_long_mul(p4o, sum15, rot2_1); \
      dct_wadd(sump13o, p1o, p3o); \
      dct_wadd(sump24o, p2o, p4o); \
      dct_wadd(sump23o, p2o, p3o); \
      dct_wadd(sump14o, p1o, p4o); \
      dct_long_mac(x4, sump13o, row7, rot3_0); \
      dct_long_mac(x5, sump24o, row5, rot3_1); \
      dct_long_mac(x6, sump23o, row3, rot3_2); \
      dct_long_mac(x7, sump14o, row1, rot3_3); \
      dct_bfly32o(row0,row7, x0,x7,shiftop,shift); \
      dct_bfly32o(row1,row6, x1,x6,shiftop,shift); \
      dct_bfly32o(row2,row5, x2,x5,shiftop,shift); \
      dct_bfly32o(row3,row4, x3,x4,shiftop,shift); \
   }

   // load
   row0 = vld1q_s16(data + 0*8);
   row1 = vld1q_s16(data + 1*8);
   row2 = vld1q_s16(data + 2*8);
   row3 = vld1q_s16(data + 3*8);
   row4 = vld1q_s16(data + 4*8);
   row5 = vld1q_s16(data + 5*8);
   row6 = vld1q_s16(data + 6*8);
   row7 = vld1q_s16(data + 7*8);

   // add DC bias
   row0 = vaddq_s16(row0, vsetq_lane_s16(1024, vdupq_n_s16(0), 0));

   // column pass
   dct_pass(vrshrn_n_s32, 10);

   // 16bit 8x8 transpose
   {
// these three map to a single VTRN.16, VTRN.32, and VSWP, respectively.
// whether compilers actually get this is another story, sadly.
#define dct_trn16(x, y) { int16x8x2_t t = vtrnq_s16(x, y); x = t.val[0]; y = t.val[1]; }
#define dct_trn32(x, y) { int32x4x2_t t = vtrnq_s32(vreinterpretq_s32_s16(x), vreinterpretq_s32_s16(y)); x = vreinterpretq_s16_s32(t.val[0]); y = vreinterpretq_s16_s32(t.val[1]); }
#define dct_trn64(x, y) { int16x8_t x0 = x; int16x8_t y0 = y; x = vcombine_s16(vget_low_s16(x0), vget_low_s16(y0)); y = vcombine_s16(vget_high_s16(x0), vget_high_s16(y0)); }

      // pass 1
      dct_trn16(row0, row1); // a0b0a2b2a4b4a6b6
      dct_trn16(row2, row3);
      dct_trn16(row4, row5);
      dct_trn16(row6, row7);

      // pass 2
      dct_trn32(row0, row2); // a0b0c0d0a4b4c4d4
      dct_trn32(row1, row3);
      dct_trn32(row4, row6);
      dct_trn32(row5, row7);

      // pass 3
      dct_trn64(row0, row4); // a0b0c0d0e0f0g0h0
      dct_trn64(row1, row5);
      dct_trn64(row2, row6);
      dct_trn64(row3, row7);

#undef dct_trn16
#undef dct_trn32
#undef dct_trn64
   }

   // row pass
   // vrshrn_n_s32 only supports shifts up to 16, we need
   // 17. so do a non-rounding shift of 16 first then follow
   // up with a rounding shift by 1.
   dct_pass(vshrn_n_s32, 16);

   {
      // pack and round
      uint8x8_t p0 = vqrshrun_n_s16(row0, 1);
      uint8x8_t p1 = vqrshrun_n_s16(row1, 1);
      uint8x8_t p2 = vqrshrun_n_s16(row2, 1);
      uint8x8_t p3 = vqrshrun_n_s16(row3, 1);
      uint8x8_t p4 = vqrshrun_n_s16(row4, 1);
      uint8x8_t p5 = vqrshrun_n_s16(row5, 1);
      uint8x8_t p6 = vqrshrun_n_s16(row6, 1);
      uint8x8_t p7 = vqrshrun_n_s16(row7, 1);

      // again, these can translate into one instruction, but often don't.
#define dct_trn8_8(x, y) { uint8x8x2_t t = vtrn_u8(x, y); x = t.val[0]; y = t.val[1]; }
#define dct_trn8_16(x, y) { uint16x4x2_t t = vtrn_u16(vreinterpret_u16_u8(x), vreinterpret_u16_u8(y)); x = vreinterpret_u8_u16(t.val[0]); y = vreinterpret_u8_u16(t.val[1]); }
#define dct_trn8_32(x, y) { uint32x2x2_t t = vtrn_u32(vreinterpret_u32_u8(x), vreinterpret_u32_u8(y)); x = vreinterpret_u8_u32(t.val[0]); y = vreinterpret_u8_u32(t.val[1]); }

      // sadly can't use interleaved stores here since we only write
      // 8 bytes to each scan line!

      // 8x8 8-bit transpose pass 1
      dct_trn8_8(p0, p1);
      dct_trn8_8(p2, p3);
      dct_trn8_8(p4, p5);
      dct_trn8_8(p6, p7);

      // pass 2
      dct_trn8_16(p0, p2);
      dct_trn8_16(p1, p3);
      dct_trn8_16(p4, p6);
      dct_trn8_16(p5, p7);

      // pass 3
      dct_trn8_32(p0, p4);
      dct_trn8_32(p1, p5);
      dct_trn8_32(p2, p6);
      dct_trn8_32(p3, p7);

      // store
      vst1_u8(out, p0); out += out_stride;
      vst1_u8(out, p1); out += out_stride;
      vst1_u8(out, p2); out += out_stride;
      vst1_u8(out, p3); out += out_stride;
      vst1_u8(out, p4); out += out_stride;
      vst1_u8(out, p5); out += out_stride;
      vst1_u8(out, p6); out += out_stride;
      vst1_u8(out, p7);

#undef dct_trn8_8
#undef dct_trn8_16
#undef dct_trn8_32
   }

#undef dct_long_mul
#undef dct_long_mac
#undef dct_widen
#undef dct_wadd
#undef dct_wsub
#undef dct_bfly32o
#undef dct_pass
}

#endif // STBI_NEON

#define STBI__MARKER_none  0xff
// if there's a pending marker from the entropy stream, return that
// otherwise, fetch from the stream and get a marker. if there's no
// marker, return 0xff, which is never a valid marker value
static stbi_uc stbi__get_marker(stbi__jpeg *j)
{
   stbi_uc x;
   if (j->marker != STBI__MARKER_none) { x = j->marker; j->marker = STBI__MARKER_none; return x; }
   x = stbi__get8(j->s);
   if (x != 0xff) return STBI__MARKER_none;
   while (x == 0xff)
      x = stbi__get8(j->s);
   return x;
}

// in each scan, we'll have scan_n components, and the order
// of the components is specified by order[]
#define STBI__RESTART(x)     ((x) >= 0xd0 && (x) <= 0xd7)

// after a restart interval, stbi__jpeg_reset the entropy decoder and
// the dc prediction
static void stbi__jpeg_reset(stbi__jpeg *j)
{
   j->code_bits = 0;
   j->code_buffer = 0;
   j->nomore = 0;
   j->img_comp[0].dc_pred = j->img_comp[1].dc_pred = j->img_comp[2].dc_pred = 0;
   j->marker = STBI__MARKER_none;
   j->todo = j->restart_interval ? j->restart_interval : 0x7fffffff;
   j->eob_run = 0;
   // no more than 1<<31 MCUs if no restart_interal? that's plenty safe,
   // since we don't even allow 1<<30 pixels
}

static int stbi__parse_entropy_coded_data(stbi__jpeg *z)
{
   stbi__jpeg_reset(z);
   if (!z->progressive) {
      if (z->scan_n == 1) {
         int i,j;
         STBI_SIMD_ALIGN(short, data[64]);
         int n = z->order[0];
         // non-interleaved data, we just need to process one block at a time,
         // in trivial scanline order
         // number of blocks to do just depends on how many actual "pixels" this
         // component has, independent of interleaved MCU blocking and such
         int w = (z->img_comp[n].x+7) >> 3;
         int h = (z->img_comp[n].y+7) >> 3;
         for (j=0; j < h; ++j) {
            for (i=0; i < w; ++i) {
               int ha = z->img_comp[n].ha;
               if (!stbi__jpeg_decode_block(z, data, z->huff_dc+z->img_comp[n].hd, z->huff_ac+ha, z->fast_ac[ha], n, z->dequant[z->img_comp[n].tq])) return 0;
               z->idct_block_kernel(z->img_comp[n].data+z->img_comp[n].w2*j*8+i*8, z->img_comp[n].w2, data);
               // every data block is an MCU, so countdown the restart interval
               if (--z->todo <= 0) {
                  if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
                  // if it's NOT a restart, then just bail, so we get corrupt data
                  // rather than no data
                  if (!STBI__RESTART(z->marker)) return 1;
                  stbi__jpeg_reset(z);
               }
            }
         }
         return 1;
      } else { // interleaved
         int i,j,k,x,y;
         STBI_SIMD_ALIGN(short, data[64]);
         for (j=0; j < z->img_mcu_y; ++j) {
            for (i=0; i < z->img_mcu_x; ++i) {
               // scan an interleaved mcu... process scan_n components in order
               for (k=0; k < z->scan_n; ++k) {
                  int n = z->order[k];
                  // scan out an mcu's worth of this component; that's just determined
                  // by the basic H and V specified for the component
                  for (y=0; y < z->img_comp[n].v; ++y) {
                     for (x=0; x < z->img_comp[n].h; ++x) {
                        int x2 = (i*z->img_comp[n].h + x)*8;
                        int y2 = (j*z->img_comp[n].v + y)*8;
                        int ha = z->img_comp[n].ha;
                        if (!stbi__jpeg_decode_block(z, data, z->huff_dc+z->img_comp[n].hd, z->huff_ac+ha, z->fast_ac[ha], n, z->dequant[z->img_comp[n].tq])) return 0;
                        z->idct_block_kernel(z->img_comp[n].data+z->img_comp[n].w2*y2+x2, z->img_comp[n].w2, data);
                     }
                  }
               }
               // after all interleaved components, that's an interleaved MCU,
               // so now count down the restart interval
               if (--z->todo <= 0) {
                  if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
                  if (!STBI__RESTART(z->marker)) return 1;
                  stbi__jpeg_reset(z);
               }
            }
         }
         return 1;
      }
   } else {
      if (z->scan_n == 1) {
         int i,j;
         int n = z->order[0];
         // non-interleaved data, we just need to process one block at a time,
         // in trivial scanline order
         // number of blocks to do just depends on how many actual "pixels" this
         // component has, independent of interleaved MCU blocking and such
         int w = (z->img_comp[n].x+7) >> 3;
         int h = (z->img_comp[n].y+7) >> 3;
         for (j=0; j < h; ++j) {
            for (i=0; i < w; ++i) {
               short *data = z->img_comp[n].coeff + 64 * (i + j * z->img_comp[n].coeff_w);
               if (z->spec_start == 0) {
                  if (!stbi__jpeg_decode_block_prog_dc(z, data, &z->huff_dc[z->img_comp[n].hd], n))
                     return 0;
               } else {
                  int ha = z->img_comp[n].ha;
                  if (!stbi__jpeg_decode_block_prog_ac(z, data, &z->huff_ac[ha], z->fast_ac[ha]))
                     return 0;
               }
               // every data block is an MCU, so countdown the restart interval
               if (--z->todo <= 0) {
                  if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
                  if (!STBI__RESTART(z->marker)) return 1;
                  stbi__jpeg_reset(z);
               }
            }
         }
         return 1;
      } else { // interleaved
         int i,j,k,x,y;
         for (j=0; j < z->img_mcu_y; ++j) {
            for (i=0; i < z->img_mcu_x; ++i) {
               // scan an interleaved mcu... process scan_n components in order
               for (k=0; k < z->scan_n; ++k) {
                  int n = z->order[k];
                  // scan out an mcu's worth of this component; that's just determined
                  // by the basic H and V specified for the component
                  for (y=0; y < z->img_comp[n].v; ++y) {
                     for (x=0; x < z->img_comp[n].h; ++x) {
                        int x2 = (i*z->img_comp[n].h + x);
                        int y2 = (j*z->img_comp[n].v + y);
                        short *data = z->img_comp[n].coeff + 64 * (x2 + y2 * z->img_comp[n].coeff_w);
                        if (!stbi__jpeg_decode_block_prog_dc(z, data, &z->huff_dc[z->img_comp[n].hd], n))
                           return 0;
                     }
                  }
               }
               // after all interleaved components, that's an interleaved MCU,
               // so now count down the restart interval
               if (--z->todo <= 0) {
                  if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
                  if (!STBI__RESTART(z->marker)) return 1;
                  stbi__jpeg_reset(z);
               }
            }
         }
         return 1;
      }
   }
}

static void stbi__jpeg_dequantize(short *data, stbi_uc *dequant)
{
   int i;
   for (i=0; i < 64; ++i)
      data[i] *= dequant[i];
}

static void stbi__jpeg_finish(stbi__jpeg *z)
{
   if (z->progressive) {
      // dequantize and idct the data
      int i,j,n;
      for (n=0; n < z->s->img_n; ++n) {
         int w = (z->img_comp[n].x+7) >> 3;
         int h = (z->img_comp[n].y+7) >> 3;
         for (j=0; j < h; ++j) {
            for (i=0; i < w; ++i) {
               short *data = z->img_comp[n].coeff + 64 * (i + j * z->img_comp[n].coeff_w);
               stbi__jpeg_dequantize(data, z->dequant[z->img_comp[n].tq]);
               z->idct_block_kernel(z->img_comp[n].data+z->img_comp[n].w2*j*8+i*8, z->img_comp[n].w2, data);
            }
         }
      }
   }
}

static int stbi__process_marker(stbi__jpeg *z, int m)
{
   int L;
   switch (m) {
      case STBI__MARKER_none: // no marker found
         return stbi__err("expected marker","Corrupt JPEG");

      case 0xDD: // DRI - specify restart interval
         if (stbi__get16be(z->s) != 4) return stbi__err("bad DRI len","Corrupt JPEG");
         z->restart_interval = stbi__get16be(z->s);
         return 1;

      case 0xDB: // DQT - define quantization table
         L = stbi__get16be(z->s)-2;
         while (L > 0) {
            int q = stbi__get8(z->s);
            int p = q >> 4;
            int t = q & 15,i;
            if (p != 0) return stbi__err("bad DQT type","Corrupt JPEG");
            if (t > 3) return stbi__err("bad DQT table","Corrupt JPEG");
            for (i=0; i < 64; ++i)
               z->dequant[t][stbi__jpeg_dezigzag[i]] = stbi__get8(z->s);
            L -= 65;
         }
         return L==0;

      case 0xC4: // DHT - define huffman table
         L = stbi__get16be(z->s)-2;
         while (L > 0) {
            stbi_uc *v;
            int sizes[16],i,n=0;
            int q = stbi__get8(z->s);
            int tc = q >> 4;
            int th = q & 15;
            if (tc > 1 || th > 3) return stbi__err("bad DHT header","Corrupt JPEG");
            for (i=0; i < 16; ++i) {
               sizes[i] = stbi__get8(z->s);
               n += sizes[i];
            }
            L -= 17;
            if (tc == 0) {
               if (!stbi__build_huffman(z->huff_dc+th, sizes)) return 0;
               v = z->huff_dc[th].values;
            } else {
               if (!stbi__build_huffman(z->huff_ac+th, sizes)) return 0;
               v = z->huff_ac[th].values;
            }
            for (i=0; i < n; ++i)
               v[i] = stbi__get8(z->s);
            if (tc != 0)
               stbi__build_fast_ac(z->fast_ac[th], z->huff_ac + th);
            L -= n;
         }
         return L==0;
   }
   // check for comment block or APP blocks
   if ((m >= 0xE0 && m <= 0xEF) || m == 0xFE) {
      stbi__skip(z->s, stbi__get16be(z->s)-2);
      return 1;
   }
   return 0;
}

// after we see SOS
static int stbi__process_scan_header(stbi__jpeg *z)
{
   int i;
   int Ls = stbi__get16be(z->s);
   z->scan_n = stbi__get8(z->s);
   if (z->scan_n < 1 || z->scan_n > 4 || z->scan_n > (int) z->s->img_n) return stbi__err("bad SOS component count","Corrupt JPEG");
   if (Ls != 6+2*z->scan_n) return stbi__err("bad SOS len","Corrupt JPEG");
   for (i=0; i < z->scan_n; ++i) {
      int id = stbi__get8(z->s), which;
      int q = stbi__get8(z->s);
      for (which = 0; which < z->s->img_n; ++which)
         if (z->img_comp[which].id == id)
            break;
      if (which == z->s->img_n) return 0; // no match
      z->img_comp[which].hd = q >> 4;   if (z->img_comp[which].hd > 3) return stbi__err("bad DC huff","Corrupt JPEG");
      z->img_comp[which].ha = q & 15;   if (z->img_comp[which].ha > 3) return stbi__err("bad AC huff","Corrupt JPEG");
      z->order[i] = which;
   }

   {
      int aa;
      z->spec_start = stbi__get8(z->s);
      z->spec_end   = stbi__get8(z->s); // should be 63, but might be 0
      aa = stbi__get8(z->s);
      z->succ_high = (aa >> 4);
      z->succ_low  = (aa & 15);
      if (z->progressive) {
         if (z->spec_start > 63 || z->spec_end > 63  || z->spec_start > z->spec_end || z->succ_high > 13 || z->succ_low > 13)
            return stbi__err("bad SOS", "Corrupt JPEG");
      } else {
         if (z->spec_start != 0) return stbi__err("bad SOS","Corrupt JPEG");
         if (z->succ_high != 0 || z->succ_low != 0) return stbi__err("bad SOS","Corrupt JPEG");
         z->spec_end = 63;
      }
   }

   return 1;
}

static int stbi__free_jpeg_components(stbi__jpeg *z, int ncomp, int why)
{
   int i;
   for (i=0; i < ncomp; ++i) {
      if (z->img_comp[i].raw_data) {
         STBI_FREE(z->img_comp[i].raw_data);
         z->img_comp[i].raw_data = NULL;
         z->img_comp[i].data = NULL;
      }
      if (z->img_comp[i].raw_coeff) {
         STBI_FREE(z->img_comp[i].raw_coeff);
         z->img_comp[i].raw_coeff = 0;
         z->img_comp[i].coeff = 0;
      }
      if (z->img_comp[i].linebuf) {
         STBI_FREE(z->img_comp[i].linebuf);
         z->img_comp[i].linebuf = NULL;
      }
   }
   return why;
}

static int stbi__process_frame_header(stbi__jpeg *z, int scan)
{
   stbi__context *s = z->s;
   int Lf,p,i,q, h_max=1,v_max=1,c;
   Lf = stbi__get16be(s);         if (Lf < 11) return stbi__err("bad SOF len","Corrupt JPEG"); // JPEG
   p  = stbi__get8(s);            if (p != 8) return stbi__err("only 8-bit","JPEG format not supported: 8-bit only"); // JPEG baseline
   s->img_y = stbi__get16be(s);   if (s->img_y == 0) return stbi__err("no header height", "JPEG format not supported: delayed height"); // Legal, but we don't handle it--but neither does IJG
   s->img_x = stbi__get16be(s);   if (s->img_x == 0) return stbi__err("0 width","Corrupt JPEG"); // JPEG requires
   c = stbi__get8(s);
   if (c != 3 && c != 1) return stbi__err("bad component count","Corrupt JPEG");    // JFIF requires
   s->img_n = c;
   for (i=0; i < c; ++i) {
      z->img_comp[i].data = NULL;
      z->img_comp[i].linebuf = NULL;
   }

   if (Lf != 8+3*s->img_n) return stbi__err("bad SOF len","Corrupt JPEG");

   z->rgb = 0;
   for (i=0; i < s->img_n; ++i) {
      static unsigned char rgb[3] = { 'R', 'G', 'B' };
      z->img_comp[i].id = stbi__get8(s);
      if (z->img_comp[i].id != i+1)   // JFIF requires
         if (z->img_comp[i].id != i) {  // some version of jpegtran outputs non-JFIF-compliant files!
            // somethings output this (see http://fileformats.archiveteam.org/wiki/JPEG#Color_format)
            if (z->img_comp[i].id != rgb[i])
               return stbi__err("bad component ID","Corrupt JPEG");
            ++z->rgb;
         }
      q = stbi__get8(s);
      z->img_comp[i].h = (q >> 4);  if (!z->img_comp[i].h || z->img_comp[i].h > 4) return stbi__err("bad H","Corrupt JPEG");
      z->img_comp[i].v = q & 15;    if (!z->img_comp[i].v || z->img_comp[i].v > 4) return stbi__err("bad V","Corrupt JPEG");
      z->img_comp[i].tq = stbi__get8(s);  if (z->img_comp[i].tq > 3) return stbi__err("bad TQ","Corrupt JPEG");
   }

   if (scan != STBI__SCAN_load) return 1;

   if (!stbi__mad3sizes_valid(s->img_x, s->img_y, s->img_n, 0)) return stbi__err("too large", "Image too large to decode");

   for (i=0; i < s->img_n; ++i) {
      if (z->img_comp[i].h > h_max) h_max = z->img_comp[i].h;
      if (z->img_comp[i].v > v_max) v_max = z->img_comp[i].v;
   }

   // compute interleaved mcu info
   z->img_h_max = h_max;
   z->img_v_max = v_max;
   z->img_mcu_w = h_max * 8;
   z->img_mcu_h = v_max * 8;
   // these sizes can't be more than 17 bits
   z->img_mcu_x = (s->img_x + z->img_mcu_w-1) / z->img_mcu_w;
   z->img_mcu_y = (s->img_y + z->img_mcu_h-1) / z->img_mcu_h;

   for (i=0; i < s->img_n; ++i) {
      // number of effective pixels (e.g. for non-interleaved MCU)
      z->img_comp[i].x = (s->img_x * z->img_comp[i].h + h_max-1) / h_max;
      z->img_comp[i].y = (s->img_y * z->img_comp[i].v + v_max-1) / v_max;
      // to simplify generation, we'll allocate enough memory to decode
      // the bogus oversized data from using interleaved MCUs and their
      // big blocks (e.g. a 16x16 iMCU on an image of width 33); we won't
      // discard the extra data until colorspace conversion
      //
      // img_mcu_x, img_mcu_y: <=17 bits; comp[i].h and .v are <=4 (checked earlier)
      // so these muls can't overflow with 32-bit ints (which we require)
      z->img_comp[i].w2 = z->img_mcu_x * z->img_comp[i].h * 8;
      z->img_comp[i].h2 = z->img_mcu_y * z->img_comp[i].v * 8;
      z->img_comp[i].coeff = 0;
      z->img_comp[i].raw_coeff = 0;
      z->img_comp[i].linebuf = NULL;
      z->img_comp[i].raw_data = stbi__malloc_mad2(z->img_comp[i].w2, z->img_comp[i].h2, 15);
      if (z->img_comp[i].raw_data == NULL)
         return stbi__free_jpeg_components(z, i+1, stbi__err("outofmem", "Out of memory"));
      // align blocks for idct using mmx/sse
      z->img_comp[i].data = (stbi_uc*) (((size_t) z->img_comp[i].raw_data + 15) & ~15);
      if (z->progressive) {
         // w2, h2 are multiples of 8 (see above)
         z->img_comp[i].coeff_w = z->img_comp[i].w2 / 8;
         z->img_comp[i].coeff_h = z->img_comp[i].h2 / 8;
         z->img_comp[i].raw_coeff = stbi__malloc_mad3(z->img_comp[i].w2, z->img_comp[i].h2, sizeof(short), 15);
         if (z->img_comp[i].raw_coeff == NULL)
            return stbi__free_jpeg_components(z, i+1, stbi__err("outofmem", "Out of memory"));
         z->img_comp[i].coeff = (short*) (((size_t) z->img_comp[i].raw_coeff + 15) & ~15);
      }
   }

   return 1;
}

// use comparisons since in some cases we handle more than one case (e.g. SOF)
#define stbi__DNL(x)         ((x) == 0xdc)
#define stbi__SOI(x)         ((x) == 0xd8)
#define stbi__EOI(x)         ((x) == 0xd9)
#define stbi__SOF(x)         ((x) == 0xc0 || (x) == 0xc1 || (x) == 0xc2)
#define stbi__SOS(x)         ((x) == 0xda)

#define stbi__SOF_progressive(x)   ((x) == 0xc2)

static int stbi__decode_jpeg_header(stbi__jpeg *z, int scan)
{
   int m;
   z->marker = STBI__MARKER_none; // initialize cached marker to empty
   m = stbi__get_marker(z);
   if (!stbi__SOI(m)) return stbi__err("no SOI","Corrupt JPEG");
   if (scan == STBI__SCAN_type) return 1;
   m = stbi__get_marker(z);
   while (!stbi__SOF(m)) {
      if (!stbi__process_marker(z,m)) return 0;
      m = stbi__get_marker(z);
      while (m == STBI__MARKER_none) {
         // some files have extra padding after their blocks, so ok, we'll scan
         if (stbi__at_eof(z->s)) return stbi__err("no SOF", "Corrupt JPEG");
         m = stbi__get_marker(z);
      }
   }
   z->progressive = stbi__SOF_progressive(m);
   if (!stbi__process_frame_header(z, scan)) return 0;
   return 1;
}

// decode image to YCbCr format
static int stbi__decode_jpeg_image(stbi__jpeg *j)
{
   int m;
   for (m = 0; m < 4; m++) {
      j->img_comp[m].raw_data = NULL;
      j->img_comp[m].raw_coeff = NULL;
   }
   j->restart_interval = 0;
   if (!stbi__decode_jpeg_header(j, STBI__SCAN_load)) return 0;
   m = stbi__get_marker(j);
   while (!stbi__EOI(m)) {
      if (stbi__SOS(m)) {
         if (!stbi__process_scan_header(j)) return 0;
         if (!stbi__parse_entropy_coded_data(j)) return 0;
         if (j->marker == STBI__MARKER_none ) {
            // handle 0s at the end of image data from IP Kamera 9060
            while (!stbi__at_eof(j->s)) {
               int x = stbi__get8(j->s);
               if (x == 255) {
                  j->marker = stbi__get8(j->s);
                  break;
               } else if (x != 0) {
                  return stbi__err("junk before marker", "Corrupt JPEG");
               }
            }
            // if we reach eof without hitting a marker, stbi__get_marker() below will fail and we'll eventually return 0
         }
      } else {
         if (!stbi__process_marker(j, m)) return 0;
      }
      m = stbi__get_marker(j);
   }
   if (j->progressive)
      stbi__jpeg_finish(j);
   return 1;
}

// static jfif-centered resampling (across block boundaries)

typedef stbi_uc *(*resample_row_func)(stbi_uc *out, stbi_uc *in0, stbi_uc *in1,
                                    int w, int hs);

#define stbi__div4(x) ((stbi_uc) ((x) >> 2))

static stbi_uc *resample_row_1(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   STBI_NOTUSED(out);
   STBI_NOTUSED(in_far);
   STBI_NOTUSED(w);
   STBI_NOTUSED(hs);
   return in_near;
}

static stbi_uc* stbi__resample_row_v_2(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   // need to generate two samples vertically for every one in input
   int i;
   STBI_NOTUSED(hs);
   for (i=0; i < w; ++i)
      out[i] = stbi__div4(3*in_near[i] + in_far[i] + 2);
   return out;
}

static stbi_uc*  stbi__resample_row_h_2(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   // need to generate two samples horizontally for every one in input
   int i;
   stbi_uc *input = in_near;

   if (w == 1) {
      // if only one sample, can't do any interpolation
      out[0] = out[1] = input[0];
      return out;
   }

   out[0] = input[0];
   out[1] = stbi__div4(input[0]*3 + input[1] + 2);
   for (i=1; i < w-1; ++i) {
      int n = 3*input[i]+2;
      out[i*2+0] = stbi__div4(n+input[i-1]);
      out[i*2+1] = stbi__div4(n+input[i+1]);
   }
   out[i*2+0] = stbi__div4(input[w-2]*3 + input[w-1] + 2);
   out[i*2+1] = input[w-1];

   STBI_NOTUSED(in_far);
   STBI_NOTUSED(hs);

   return out;
}

#define stbi__div16(x) ((stbi_uc) ((x) >> 4))

static stbi_uc *stbi__resample_row_hv_2(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   // need to generate 2x2 samples for every one in input
   int i,t0,t1;
   if (w == 1) {
      out[0] = out[1] = stbi__div4(3*in_near[0] + in_far[0] + 2);
      return out;
   }

   t1 = 3*in_near[0] + in_far[0];
   out[0] = stbi__div4(t1+2);
   for (i=1; i < w; ++i) {
      t0 = t1;
      t1 = 3*in_near[i]+in_far[i];
      out[i*2-1] = stbi__div16(3*t0 + t1 + 8);
      out[i*2  ] = stbi__div16(3*t1 + t0 + 8);
   }
   out[w*2-1] = stbi__div4(t1+2);

   STBI_NOTUSED(hs);

   return out;
}

#if defined(STBI_SSE2) || defined(STBI_NEON)
static stbi_uc *stbi__resample_row_hv_2_simd(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   // need to generate 2x2 samples for every one in input
   int i=0,t0,t1;

   if (w == 1) {
      out[0] = out[1] = stbi__div4(3*in_near[0] + in_far[0] + 2);
      return out;
   }

   t1 = 3*in_near[0] + in_far[0];
   // process groups of 8 pixels for as long as we can.
   // note we can't handle the last pixel in a row in this loop
   // because we need to handle the filter boundary conditions.
   for (; i < ((w-1) & ~7); i += 8) {
#if defined(STBI_SSE2)
      // load and perform the vertical filtering pass
      // this uses 3*x + y = 4*x + (y - x)
      __m128i zero  = _mm_setzero_si128();
      __m128i farb  = _mm_loadl_epi64((__m128i *) (in_far + i));
      __m128i nearb = _mm_loadl_epi64((__m128i *) (in_near + i));
      __m128i farw  = _mm_unpacklo_epi8(farb, zero);
      __m128i nearw = _mm_unpacklo_epi8(nearb, zero);
      __m128i diff  = _mm_sub_epi16(farw, nearw);
      __m128i nears = _mm_slli_epi16(nearw, 2);
      __m128i curr  = _mm_add_epi16(nears, diff); // current row

      // horizontal filter works the same based on shifted vers of current
      // row. "prev" is current row shifted right by 1 pixel; we need to
      // insert the previous pixel value (from t1).
      // "next" is current row shifted left by 1 pixel, with first pixel
      // of next block of 8 pixels added in.
      __m128i prv0 = _mm_slli_si128(curr, 2);
      __m128i nxt0 = _mm_srli_si128(curr, 2);
      __m128i prev = _mm_insert_epi16(prv0, t1, 0);
      __m128i next = _mm_insert_epi16(nxt0, 3*in_near[i+8] + in_far[i+8], 7);

      // horizontal filter, polyphase implementation since it's convenient:
      // even pixels = 3*cur + prev = cur*4 + (prev - cur)
      // odd  pixels = 3*cur + next = cur*4 + (next - cur)
      // note the shared term.
      __m128i bias  = _mm_set1_epi16(8);
      __m128i curs = _mm_slli_epi16(curr, 2);
      __m128i prvd = _mm_sub_epi16(prev, curr);
      __m128i nxtd = _mm_sub_epi16(next, curr);
      __m128i curb = _mm_add_epi16(curs, bias);
      __m128i even = _mm_add_epi16(prvd, curb);
      __m128i odd  = _mm_add_epi16(nxtd, curb);

      // interleave even and odd pixels, then undo scaling.
      __m128i int0 = _mm_unpacklo_epi16(even, odd);
      __m128i int1 = _mm_unpackhi_epi16(even, odd);
      __m128i de0  = _mm_srli_epi16(int0, 4);
      __m128i de1  = _mm_srli_epi16(int1, 4);

      // pack and write output
      __m128i outv = _mm_packus_epi16(de0, de1);
      _mm_storeu_si128((__m128i *) (out + i*2), outv);
#elif defined(STBI_NEON)
      // load and perform the vertical filtering pass
      // this uses 3*x + y = 4*x + (y - x)
      uint8x8_t farb  = vld1_u8(in_far + i);
      uint8x8_t nearb = vld1_u8(in_near + i);
      int16x8_t diff  = vreinterpretq_s16_u16(vsubl_u8(farb, nearb));
      int16x8_t nears = vreinterpretq_s16_u16(vshll_n_u8(nearb, 2));
      int16x8_t curr  = vaddq_s16(nears, diff); // current row

      // horizontal filter works the same based on shifted vers of current
      // row. "prev" is current row shifted right by 1 pixel; we need to
      // insert the previous pixel value (from t1).
      // "next" is current row shifted left by 1 pixel, with first pixel
      // of next block of 8 pixels added in.
      int16x8_t prv0 = vextq_s16(curr, curr, 7);
      int16x8_t nxt0 = vextq_s16(curr, curr, 1);
      int16x8_t prev = vsetq_lane_s16(t1, prv0, 0);
      int16x8_t next = vsetq_lane_s16(3*in_near[i+8] + in_far[i+8], nxt0, 7);

      // horizontal filter, polyphase implementation since it's convenient:
      // even pixels = 3*cur + prev = cur*4 + (prev - cur)
      // odd  pixels = 3*cur + next = cur*4 + (next - cur)
      // note the shared term.
      int16x8_t curs = vshlq_n_s16(curr, 2);
      int16x8_t prvd = vsubq_s16(prev, curr);
      int16x8_t nxtd = vsubq_s16(next, curr);
      int16x8_t even = vaddq_s16(curs, prvd);
      int16x8_t odd  = vaddq_s16(curs, nxtd);

      // undo scaling and round, then store with even/odd phases interleaved
      uint8x8x2_t o;
      o.val[0] = vqrshrun_n_s16(even, 4);
      o.val[1] = vqrshrun_n_s16(odd,  4);
      vst2_u8(out + i*2, o);
#endif

      // "previous" value for next iter
      t1 = 3*in_near[i+7] + in_far[i+7];
   }

   t0 = t1;
   t1 = 3*in_near[i] + in_far[i];
   out[i*2] = stbi__div16(3*t1 + t0 + 8);

   for (++i; i < w; ++i) {
      t0 = t1;
      t1 = 3*in_near[i]+in_far[i];
      out[i*2-1] = stbi__div16(3*t0 + t1 + 8);
      out[i*2  ] = stbi__div16(3*t1 + t0 + 8);
   }
   out[w*2-1] = stbi__div4(t1+2);

   STBI_NOTUSED(hs);

   return out;
}
#endif

static stbi_uc *stbi__resample_row_generic(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs)
{
   // resample with nearest-neighbor
   int i,j;
   STBI_NOTUSED(in_far);
   for (i=0; i < w; ++i)
      for (j=0; j < hs; ++j)
         out[i*hs+j] = in_near[i];
   return out;
}

#ifdef STBI_JPEG_OLD
// this is the same YCbCr-to-RGB calculation that stb_image has used
// historically before the algorithm changes in 1.49
#define float2fixed(x)  ((int) ((x) * 65536 + 0.5))
static void stbi__YCbCr_to_RGB_row(stbi_uc *out, const stbi_uc *y, const stbi_uc *pcb, const stbi_uc *pcr, int count, int step)
{
   int i;
   for (i=0; i < count; ++i) {
      int y_fixed = (y[i] << 16) + 32768; // rounding
      int r,g,b;
      int cr = pcr[i] - 128;
      int cb = pcb[i] - 128;
      r = y_fixed + cr*float2fixed(1.40200f);
      g = y_fixed - cr*float2fixed(0.71414f) - cb*float2fixed(0.34414f);
      b = y_fixed                            + cb*float2fixed(1.77200f);
      r >>= 16;
      g >>= 16;
      b >>= 16;
      if ((unsigned) r > 255) { if (r < 0) r = 0; else r = 255; }
      if ((unsigned) g > 255) { if (g < 0) g = 0; else g = 255; }
      if ((unsigned) b > 255) { if (b < 0) b = 0; else b = 255; }
      out[0] = (stbi_uc)r;
      out[1] = (stbi_uc)g;
      out[2] = (stbi_uc)b;
      out[3] = 255;
      out += step;
   }
}
#else
// this is a reduced-precision calculation of YCbCr-to-RGB introduced
// to make sure the code produces the same results in both SIMD and scalar
#define float2fixed(x)  (((int) ((x) * 4096.0f + 0.5f)) << 8)
static void stbi__YCbCr_to_RGB_row(stbi_uc *out, const stbi_uc *y, const stbi_uc *pcb, const stbi_uc *pcr, int count, int step)
{
   int i;
   for (i=0; i < count; ++i) {
      int y_fixed = (y[i] << 20) + (1<<19); // rounding
      int r,g,b;
      int cr = pcr[i] - 128;
      int cb = pcb[i] - 128;
      r = y_fixed +  cr* float2fixed(1.40200f);
      g = y_fixed + (cr*-float2fixed(0.71414f)) + ((cb*-float2fixed(0.34414f)) & 0xffff0000);
      b = y_fixed                               +   cb* float2fixed(1.77200f);
      r >>= 20;
      g >>= 20;
      b >>= 20;
      if ((unsigned) r > 255) { if (r < 0) r = 0; else r = 255; }
      if ((unsigned) g > 255) { if (g < 0) g = 0; else g = 255; }
      if ((unsigned) b > 255) { if (b < 0) b = 0; else b = 255; }
      out[0] = (stbi_uc)r;
      out[1] = (stbi_uc)g;
      out[2] = (stbi_uc)b;
      out[3] = 255;
      out += step;
   }
}
#endif

#if defined(STBI_SSE2) || defined(STBI_NEON)
static void stbi__YCbCr_to_RGB_simd(stbi_uc *out, stbi_uc const *y, stbi_uc const *pcb, stbi_uc const *pcr, int count, int step)
{
   int i = 0;

#ifdef STBI_SSE2
   // step == 3 is pretty ugly on the final interleave, and i'm not convinced
   // it's useful in practice (you wouldn't use it for textures, for example).
   // so just accelerate step == 4 case.
   if (step == 4) {
      // this is a fairly straightforward implementation and not super-optimized.
      __m128i signflip  = _mm_set1_epi8(-0x80);
      __m128i cr_const0 = _mm_set1_epi16(   (short) ( 1.40200f*4096.0f+0.5f));
      __m128i cr_const1 = _mm_set1_epi16( - (short) ( 0.71414f*4096.0f+0.5f));
      __m128i cb_const0 = _mm_set1_epi16( - (short) ( 0.34414f*4096.0f+0.5f));
      __m128i cb_const1 = _mm_set1_epi16(   (short) ( 1.77200f*4096.0f+0.5f));
      __m128i y_bias = _mm_set1_epi8((char) (unsigned char) 128);
      __m128i xw = _mm_set1_epi16(255); // alpha channel

      for (; i+7 < count; i += 8) {
         // load
         __m128i y_bytes = _mm_loadl_epi64((__m128i *) (y+i));
         __m128i cr_bytes = _mm_loadl_epi64((__m128i *) (pcr+i));
         __m128i cb_bytes = _mm_loadl_epi64((__m128i *) (pcb+i));
         __m128i cr_biased = _mm_xor_si128(cr_bytes, signflip); // -128
         __m128i cb_biased = _mm_xor_si128(cb_bytes, signflip); // -128

         // unpack to short (and left-shift cr, cb by 8)
         __m128i yw  = _mm_unpacklo_epi8(y_bias, y_bytes);
         __m128i crw = _mm_unpacklo_epi8(_mm_setzero_si128(), cr_biased);
         __m128i cbw = _mm_unpacklo_epi8(_mm_setzero_si128(), cb_biased);

         // color transform
         __m128i yws = _mm_srli_epi16(yw, 4);
         __m128i cr0 = _mm_mulhi_epi16(cr_const0, crw);
         __m128i cb0 = _mm_mulhi_epi16(cb_const0, cbw);
         __m128i cb1 = _mm_mulhi_epi16(cbw, cb_const1);
         __m128i cr1 = _mm_mulhi_epi16(crw, cr_const1);
         __m128i rws = _mm_add_epi16(cr0, yws);
         __m128i gwt = _mm_add_epi16(cb0, yws);
         __m128i bws = _mm_add_epi16(yws, cb1);
         __m128i gws = _mm_add_epi16(gwt, cr1);

         // descale
         __m128i rw = _mm_srai_epi16(rws, 4);
         __m128i bw = _mm_srai_epi16(bws, 4);
         __m128i gw = _mm_srai_epi16(gws, 4);

         // back to byte, set up for transpose
         __m128i brb = _mm_packus_epi16(rw, bw);
         __m128i gxb = _mm_packus_epi16(gw, xw);

         // transpose to interleave channels
         __m128i t0 = _mm_unpacklo_epi8(brb, gxb);
         __m128i t1 = _mm_unpackhi_epi8(brb, gxb);
         __m128i o0 = _mm_unpacklo_epi16(t0, t1);
         __m128i o1 = _mm_unpackhi_epi16(t0, t1);

         // store
         _mm_storeu_si128((__m128i *) (out + 0), o0);
         _mm_storeu_si128((__m128i *) (out + 16), o1);
         out += 32;
      }
   }
#endif

#ifdef STBI_NEON
   // in this version, step=3 support would be easy to add. but is there demand?
   if (step == 4) {
      // this is a fairly straightforward implementation and not super-optimized.
      uint8x8_t signflip = vdup_n_u8(0x80);
      int16x8_t cr_const0 = vdupq_n_s16(   (short) ( 1.40200f*4096.0f+0.5f));
      int16x8_t cr_const1 = vdupq_n_s16( - (short) ( 0.71414f*4096.0f+0.5f));
      int16x8_t cb_const0 = vdupq_n_s16( - (short) ( 0.34414f*4096.0f+0.5f));
      int16x8_t cb_const1 = vdupq_n_s16(   (short) ( 1.77200f*4096.0f+0.5f));

      for (; i+7 < count; i += 8) {
         // load
         uint8x8_t y_bytes  = vld1_u8(y + i);
         uint8x8_t cr_bytes = vld1_u8(pcr + i);
         uint8x8_t cb_bytes = vld1_u8(pcb + i);
         int8x8_t cr_biased = vreinterpret_s8_u8(vsub_u8(cr_bytes, signflip));
         int8x8_t cb_biased = vreinterpret_s8_u8(vsub_u8(cb_bytes, signflip));

         // expand to s16
         int16x8_t yws = vreinterpretq_s16_u16(vshll_n_u8(y_bytes, 4));
         int16x8_t crw = vshll_n_s8(cr_biased, 7);
         int16x8_t cbw = vshll_n_s8(cb_biased, 7);

         // color transform
         int16x8_t cr0 = vqdmulhq_s16(crw, cr_const0);
         int16x8_t cb0 = vqdmulhq_s16(cbw, cb_const0);
         int16x8_t cr1 = vqdmulhq_s16(crw, cr_const1);
         int16x8_t cb1 = vqdmulhq_s16(cbw, cb_const1);
         int16x8_t rws = vaddq_s16(yws, cr0);
         int16x8_t gws = vaddq_s16(vaddq_s16(yws, cb0), cr1);
         int16x8_t bws = vaddq_s16(yws, cb1);

         // undo scaling, round, convert to byte
         uint8x8x4_t o;
         o.val[0] = vqrshrun_n_s16(rws, 4);
         o.val[1] = vqrshrun_n_s16(gws, 4);
         o.val[2] = vqrshrun_n_s16(bws, 4);
         o.val[3] = vdup_n_u8(255);

         // store, interleaving r/g/b/a
         vst4_u8(out, o);
         out += 8*4;
      }
   }
#endif

   for (; i < count; ++i) {
      int y_fixed = (y[i] << 20) + (1<<19); // rounding
      int r,g,b;
      int cr = pcr[i] - 128;
      int cb = pcb[i] - 128;
      r = y_fixed + cr* float2fixed(1.40200f);
      g = y_fixed + cr*-float2fixed(0.71414f) + ((cb*-float2fixed(0.34414f)) & 0xffff0000);
      b = y_fixed                             +   cb* float2fixed(1.77200f);
      r >>= 20;
      g >>= 20;
      b >>= 20;
      if ((unsigned) r > 255) { if (r < 0) r = 0; else r = 255; }
      if ((unsigned) g > 255) { if (g < 0) g = 0; else g = 255; }
      if ((unsigned) b > 255) { if (b < 0) b = 0; else b = 255; }
      out[0] = (stbi_uc)r;
      out[1] = (stbi_uc)g;
      out[2] = (stbi_uc)b;
      out[3] = 255;
      out += step;
   }
}
#endif

// set up the kernels
static void stbi__setup_jpeg(stbi__jpeg *j)
{
   j->idct_block_kernel = stbi__idct_block;
   j->YCbCr_to_RGB_kernel = stbi__YCbCr_to_RGB_row;
   j->resample_row_hv_2_kernel = stbi__resample_row_hv_2;

#ifdef STBI_SSE2
   if (stbi__sse2_available()) {
      j->idct_block_kernel = stbi__idct_simd;
      #ifndef STBI_JPEG_OLD
      j->YCbCr_to_RGB_kernel = stbi__YCbCr_to_RGB_simd;
      #endif
      j->resample_row_hv_2_kernel = stbi__resample_row_hv_2_simd;
   }
#endif

#ifdef STBI_NEON
   j->idct_block_kernel = stbi__idct_simd;
   #ifndef STBI_JPEG_OLD
   j->YCbCr_to_RGB_kernel = stbi__YCbCr_to_RGB_simd;
   #endif
   j->resample_row_hv_2_kernel = stbi__resample_row_hv_2_simd;
#endif
}

// clean up the temporary component buffers
static void stbi__cleanup_jpeg(stbi__jpeg *j)
{
   stbi__free_jpeg_components(j, j->s->img_n, 0);
}

typedef struct
{
   resample_row_func resample;
   stbi_uc *line0,*line1;
   int hs,vs;   // expansion factor in each axis
   int w_lores; // horizontal pixels pre-expansion
   int ystep;   // how far through vertical expansion we are
   int ypos;    // which pre-expansion row we're on
} stbi__resample;

static stbi_uc *load_jpeg_image(stbi__jpeg *z, int *out_x, int *out_y, int *comp, int req_comp)
{
   int n, decode_n;
   z->s->img_n = 0; // make stbi__cleanup_jpeg safe

   // validate req_comp
   if (req_comp < 0 || req_comp > 4) return stbi__errpuc("bad req_comp", "Internal error");

   // load a jpeg image from whichever source, but leave in YCbCr format
   if (!stbi__decode_jpeg_image(z)) { stbi__cleanup_jpeg(z); return NULL; }

   // determine actual number of components to generate
   n = req_comp ? req_comp : z->s->img_n;

   if (z->s->img_n == 3 && n < 3)
      decode_n = 1;
   else
      decode_n = z->s->img_n;

   // resample and color-convert
   {
      int k;
      unsigned int i,j;
      stbi_uc *output;
      stbi_uc *coutput[4];

      stbi__resample res_comp[4];

      for (k=0; k < decode_n; ++k) {
         stbi__resample *r = &res_comp[k];

         // allocate line buffer big enough for upsampling off the edges
         // with upsample factor of 4
         z->img_comp[k].linebuf = (stbi_uc *) stbi__malloc(z->s->img_x + 3);
         if (!z->img_comp[k].linebuf) { stbi__cleanup_jpeg(z); return stbi__errpuc("outofmem", "Out of memory"); }

         r->hs      = z->img_h_max / z->img_comp[k].h;
         r->vs      = z->img_v_max / z->img_comp[k].v;
         r->ystep   = r->vs >> 1;
         r->w_lores = (z->s->img_x + r->hs-1) / r->hs;
         r->ypos    = 0;
         r->line0   = r->line1 = z->img_comp[k].data;

         if      (r->hs == 1 && r->vs == 1) r->resample = resample_row_1;
         else if (r->hs == 1 && r->vs == 2) r->resample = stbi__resample_row_v_2;
         else if (r->hs == 2 && r->vs == 1) r->resample = stbi__resample_row_h_2;
         else if (r->hs == 2 && r->vs == 2) r->resample = z->resample_row_hv_2_kernel;
         else                               r->resample = stbi__resample_row_generic;
      }

      // can't error after this so, this is safe
      output = (stbi_uc *) stbi__malloc_mad3(n, z->s->img_x, z->s->img_y, 1);
      if (!output) { stbi__cleanup_jpeg(z); return stbi__errpuc("outofmem", "Out of memory"); }

      // now go ahead and resample
      for (j=0; j < z->s->img_y; ++j) {
         stbi_uc *out = output + n * z->s->img_x * j;
         for (k=0; k < decode_n; ++k) {
            stbi__resample *r = &res_comp[k];
            int y_bot = r->ystep >= (r->vs >> 1);
            coutput[k] = r->resample(z->img_comp[k].linebuf,
                                     y_bot ? r->line1 : r->line0,
                                     y_bot ? r->line0 : r->line1,
                                     r->w_lores, r->hs);
            if (++r->ystep >= r->vs) {
               r->ystep = 0;
               r->line0 = r->line1;
               if (++r->ypos < z->img_comp[k].y)
                  r->line1 += z->img_comp[k].w2;
            }
         }
         if (n >= 3) {
            stbi_uc *y = coutput[0];
            if (z->s->img_n == 3) {
               if (z->rgb == 3) {
                  for (i=0; i < z->s->img_x; ++i) {
                     out[0] = y[i];
                     out[1] = coutput[1][i];
                     out[2] = coutput[2][i];
                     out[3] = 255;
                     out += n;
                  }
               } else {
                  z->YCbCr_to_RGB_kernel(out, y, coutput[1], coutput[2], z->s->img_x, n);
               }
            } else
               for (i=0; i < z->s->img_x; ++i) {
                  out[0] = out[1] = out[2] = y[i];
                  out[3] = 255; // not used if n==3
                  out += n;
               }
         } else {
            stbi_uc *y = coutput[0];
            if (n == 1)
               for (i=0; i < z->s->img_x; ++i) out[i] = y[i];
            else
               for (i=0; i < z->s->img_x; ++i) *out++ = y[i], *out++ = 255;
         }
      }
      stbi__cleanup_jpeg(z);
      *out_x = z->s->img_x;
      *out_y = z->s->img_y;
      if (comp) *comp  = z->s->img_n; // report original components, not output
      return output;
   }
}

static void *stbi__jpeg_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri)
{
   unsigned char* result;
   stbi__jpeg* j = (stbi__jpeg*) stbi__malloc(sizeof(stbi__jpeg));
   j->s = s;
   stbi__setup_jpeg(j);
   result = load_jpeg_image(j, x,y,comp,req_comp);
   STBI_FREE(j);
   return result;
}

static int stbi__jpeg_test(stbi__context *s)
{
   int r;
   stbi__jpeg j;
   j.s = s;
   stbi__setup_jpeg(&j);
   r = stbi__decode_jpeg_header(&j, STBI__SCAN_type);
   stbi__rewind(s);
   return r;
}

static int stbi__jpeg_info_raw(stbi__jpeg *j, int *x, int *y, int *comp)
{
   if (!stbi__decode_jpeg_header(j, STBI__SCAN_header)) {
      stbi__rewind( j->s );
      return 0;
   }
   if (x) *x = j->s->img_x;
   if (y) *y = j->s->img_y;
   if (comp) *comp = j->s->img_n;
   return 1;
}

static int stbi__jpeg_info(stbi__context *s, int *x, int *y, int *comp)
{
   int result;
   stbi__jpeg* j = (stbi__jpeg*) (stbi__malloc(sizeof(stbi__jpeg)));
   j->s = s;
   result = stbi__jpeg_info_raw(j, x, y, comp);
   STBI_FREE(j);
   return result;
}
#endif

// public domain zlib decode    v0.2  Sean Barrett 2006-11-18
//    simple implementation
//      - all input must be provided in an upfront buffer
//      - all output is written to a single output buffer (can malloc/realloc)
//    performance
//      - fast huffman

#ifndef STBI_NO_ZLIB

// fast-way is faster to check than jpeg huffman, but slow way is slower
#define STBI__ZFAST_BITS  9 // accelerate all cases in default tables
#define STBI__ZFAST_MASK  ((1 << STBI__ZFAST_BITS) - 1)

// zlib-style huffman encoding
// (jpegs packs from left, zlib from right, so can't share code)
typedef struct
{
   stbi__uint16 fast[1 << STBI__ZFAST_BITS];
   stbi__uint16 firstcode[16];
   int maxcode[17];
   stbi__uint16 firstsymbol[16];
   stbi_uc  size[288];
   stbi__uint16 value[288];
} stbi__zhuffman;

stbi_inline static int stbi__bitreverse16(int n)
{
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n;
}

stbi_inline static int stbi__bit_reverse(int v, int bits)
{
   STBI_ASSERT(bits <= 16);
   // to bit reverse n bits, reverse 16 and shift
   // e.g. 11 bits, bit reverse and shift away 5
   return stbi__bitreverse16(v) >> (16-bits);
}

static int stbi__zbuild_huffman(stbi__zhuffman *z, stbi_uc *sizelist, int num)
{
   int i,k=0;
   int code, next_code[16], sizes[17];

   // DEFLATE spec for generating codes
   memset(sizes, 0, sizeof(sizes));
   memset(z->fast, 0, sizeof(z->fast));
   for (i=0; i < num; ++i)
      ++sizes[sizelist[i]];
   sizes[0] = 0;
   for (i=1; i < 16; ++i)
      if (sizes[i] > (1 << i))
         return stbi__err("bad sizes", "Corrupt PNG");
   code = 0;
   for (i=1; i < 16; ++i) {
      next_code[i] = code;
      z->firstcode[i] = (stbi__uint16) code;
      z->firstsymbol[i] = (stbi__uint16) k;
      code = (code + sizes[i]);
      if (sizes[i])
         if (code-1 >= (1 << i)) return stbi__err("bad codelengths","Corrupt PNG");
      z->maxcode[i] = code << (16-i); // preshift for inner loop
      code <<= 1;
      k += sizes[i];
   }
   z->maxcode[16] = 0x10000; // sentinel
   for (i=0; i < num; ++i) {
      int s = sizelist[i];
      if (s) {
         int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
         stbi__uint16 fastv = (stbi__uint16) ((s << 9) | i);
         z->size [c] = (stbi_uc     ) s;
         z->value[c] = (stbi__uint16) i;
         if (s <= STBI__ZFAST_BITS) {
            int j = stbi__bit_reverse(next_code[s],s);
            while (j < (1 << STBI__ZFAST_BITS)) {
               z->fast[j] = fastv;
               j += (1 << s);
            }
         }
         ++next_code[s];
      }
   }
   return 1;
}

// zlib-from-memory implementation for PNG reading
//    because PNG allows splitting the zlib stream arbitrarily,
//    and it's annoying structurally to have PNG call ZLIB call PNG,
//    we require PNG read all the IDATs and combine them into a single
//    memory buffer

typedef struct
{
   stbi_uc *zbuffer, *zbuffer_end;
   int num_bits;
   stbi__uint32 code_buffer;

   char *zout;
   char *zout_start;
   char *zout_end;
   int   z_expandable;

   stbi__zhuffman z_length, z_distance;
} stbi__zbuf;

stbi_inline static stbi_uc stbi__zget8(stbi__zbuf *z)
{
   if (z->zbuffer >= z->zbuffer_end) return 0;
   return *z->zbuffer++;
}

static void stbi__fill_bits(stbi__zbuf *z)
{
   do {
      STBI_ASSERT(z->code_buffer < (1U << z->num_bits));
      z->code_buffer |= (unsigned int) stbi__zget8(z) << z->num_bits;
      z->num_bits += 8;
   } while (z->num_bits <= 24);
}

stbi_inline static unsigned int stbi__zreceive(stbi__zbuf *z, int n)
{
   unsigned int k;
   if (z->num_bits < n) stbi__fill_bits(z);
   k = z->code_buffer & ((1 << n) - 1);
   z->code_buffer >>= n;
   z->num_bits -= n;
   return k;
}

static int stbi__zhuffman_decode_slowpath(stbi__zbuf *a, stbi__zhuffman *z)
{
   int b,s,k;
   // not resolved by fast table, so compute it the slow way
   // use jpeg approach, which requires MSbits at top
   k = stbi__bit_reverse(a->code_buffer, 16);
   for (s=STBI__ZFAST_BITS+1; ; ++s)
      if (k < z->maxcode[s])
         break;
   if (s == 16) return -1; // invalid code!
   // code size is s, so:
   b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
   STBI_ASSERT(z->size[b] == s);
   a->code_buffer >>= s;
   a->num_bits -= s;
   return z->value[b];
}

stbi_inline static int stbi__zhuffman_decode(stbi__zbuf *a, stbi__zhuffman *z)
{
   int b,s;
   if (a->num_bits < 16) stbi__fill_bits(a);
   b = z->fast[a->code_buffer & STBI__ZFAST_MASK];
   if (b) {
      s = b >> 9;
      a->code_buffer >>= s;
      a->num_bits -= s;
      return b & 511;
   }
   return stbi__zhuffman_decode_slowpath(a, z);
}

static int stbi__zexpand(stbi__zbuf *z, char *zout, int n)  // need to make room for n bytes
{
   char *q;
   int cur, limit, old_limit;
   z->zout = zout;
   if (!z->z_expandable) return stbi__err("output buffer limit","Corrupt PNG");
   cur   = (int) (z->zout     - z->zout_start);
   limit = old_limit = (int) (z->zout_end - z->zout_start);
   while (cur + n > limit)
      limit *= 2;
   q = (char *) STBI_REALLOC_SIZED(z->zout_start, old_limit, limit);
   STBI_NOTUSED(old_limit);
   if (q == NULL) return stbi__err("outofmem", "Out of memory");
   z->zout_start = q;
   z->zout       = q + cur;
   z->zout_end   = q + limit;
   return 1;
}

static int stbi__zlength_base[31] = {
   3,4,5,6,7,8,9,10,11,13,
   15,17,19,23,27,31,35,43,51,59,
   67,83,99,115,131,163,195,227,258,0,0 };

static int stbi__zlength_extra[31]=
{ 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };

static int stbi__zdist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};

static int stbi__zdist_extra[32] =
{ 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int stbi__parse_huffman_block(stbi__zbuf *a)
{
   char *zout = a->zout;
   for(;;) {
      int z = stbi__zhuffman_decode(a, &a->z_length);
      if (z < 256) {
         if (z < 0) return stbi__err("bad huffman code","Corrupt PNG"); // error in huffman codes
         if (zout >= a->zout_end) {
            if (!stbi__zexpand(a, zout, 1)) return 0;
            zout = a->zout;
         }
         *zout++ = (char) z;
      } else {
         stbi_uc *p;
         int len,dist;
         if (z == 256) {
            a->zout = zout;
            return 1;
         }
         z -= 257;
         len = stbi__zlength_base[z];
         if (stbi__zlength_extra[z]) len += stbi__zreceive(a, stbi__zlength_extra[z]);
         z = stbi__zhuffman_decode(a, &a->z_distance);
         if (z < 0) return stbi__err("bad huffman code","Corrupt PNG");
         dist = stbi__zdist_base[z];
         if (stbi__zdist_extra[z]) dist += stbi__zreceive(a, stbi__zdist_extra[z]);
         if (zout - a->zout_start < dist) return stbi__err("bad dist","Corrupt PNG");
         if (zout + len > a->zout_end) {
            if (!stbi__zexpand(a, zout, len)) return 0;
            zout = a->zout;
         }
         p = (stbi_uc *) (zout - dist);
         if (dist == 1) { // run of one byte; common in images.
            stbi_uc v = *p;
            if (len) { do *zout++ = v; while (--len); }
         } else {
            if (len) { do *zout++ = *p++; while (--len); }
         }
      }
   }
}

static int stbi__compute_huffman_codes(stbi__zbuf *a)
{
   static stbi_uc length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
   stbi__zhuffman z_codelength;
   stbi_uc lencodes[286+32+137];//padding for maximum single op
   stbi_uc codelength_sizes[19];
   int i,n;

   int hlit  = stbi__zreceive(a,5) + 257;
   int hdist = stbi__zreceive(a,5) + 1;
   int hclen = stbi__zreceive(a,4) + 4;
   int ntot  = hlit + hdist;

   memset(codelength_sizes, 0, sizeof(codelength_sizes));
   for (i=0; i < hclen; ++i) {
      int s = stbi__zreceive(a,3);
      codelength_sizes[length_dezigzag[i]] = (stbi_uc) s;
   }
   if (!stbi__zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;

   n = 0;
   while (n < ntot) {
      int c = stbi__zhuffman_decode(a, &z_codelength);
      if (c < 0 || c >= 19) return stbi__err("bad codelengths", "Corrupt PNG");
      if (c < 16)
         lencodes[n++] = (stbi_uc) c;
      else {
         stbi_uc fill = 0;
         if (c == 16) {
            c = stbi__zreceive(a,2)+3;
            if (n == 0) return stbi__err("bad codelengths", "Corrupt PNG");
            fill = lencodes[n-1];
         } else if (c == 17)
            c = stbi__zreceive(a,3)+3;
         else {
            STBI_ASSERT(c == 18);
            c = stbi__zreceive(a,7)+11;
         }
         if (ntot - n < c) return stbi__err("bad codelengths", "Corrupt PNG");
         memset(lencodes+n, fill, c);
         n += c;
      }
   }
   if (n != ntot) return stbi__err("bad codelengths","Corrupt PNG");
   if (!stbi__zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
   if (!stbi__zbuild_huffman(&a->z_distance, lencodes+hlit, hdist)) return 0;
   return 1;
}

static int stbi__parse_uncompressed_block(stbi__zbuf *a)
{
   stbi_uc header[4];
   int len,nlen,k;
   if (a->num_bits & 7)
      stbi__zreceive(a, a->num_bits & 7); // discard
   // drain the bit-packed data into header
   k = 0;
   while (a->num_bits > 0) {
      header[k++] = (stbi_uc) (a->code_buffer & 255); // suppress MSVC run-time check
      a->code_buffer >>= 8;
      a->num_bits -= 8;
   }
   STBI_ASSERT(a->num_bits == 0);
   // now fill header the normal way
   while (k < 4)
      header[k++] = stbi__zget8(a);
   len  = header[1] * 256 + header[0];
   nlen = header[3] * 256 + header[2];
   if (nlen != (len ^ 0xffff)) return stbi__err("zlib corrupt","Corrupt PNG");
   if (a->zbuffer + len > a->zbuffer_end) return stbi__err("read past buffer","Corrupt PNG");
   if (a->zout + len > a->zout_end)
      if (!stbi__zexpand(a, a->zout, len)) return 0;
   memcpy(a->zout, a->zbuffer, len);
   a->zbuffer += len;
   a->zout += len;
   return 1;
}

static int stbi__parse_zlib_header(stbi__zbuf *a)
{
   int cmf   = stbi__zget8(a);
   int cm    = cmf & 15;
   /* int cinfo = cmf >> 4; */
   int flg   = stbi__zget8(a);
   if ((cmf*256+flg) % 31 != 0) return stbi__err("bad zlib header","Corrupt PNG"); // zlib spec
   if (flg & 32) return stbi__err("no preset dict","Corrupt PNG"); // preset dictionary not allowed in png
   if (cm != 8) return stbi__err("bad compression","Corrupt PNG"); // DEFLATE required for png
   // window = 1 << (8 + cinfo)... but who cares, we fully buffer output
   return 1;
}

// @TODO: should statically initialize these for optimal thread safety
static stbi_uc stbi__zdefault_length[288], stbi__zdefault_distance[32];
static void stbi__init_zdefaults(void)
{
   int i;   // use <= to match clearly with spec
   for (i=0; i <= 143; ++i)     stbi__zdefault_length[i]   = 8;
   for (   ; i <= 255; ++i)     stbi__zdefault_length[i]   = 9;
   for (   ; i <= 279; ++i)     stbi__zdefault_length[i]   = 7;
   for (   ; i <= 287; ++i)     stbi__zdefault_length[i]   = 8;

   for (i=0; i <=  31; ++i)     stbi__zdefault_distance[i] = 5;
}

static int stbi__parse_zlib(stbi__zbuf *a, int parse_header)
{
   int final, type;
   if (parse_header)
      if (!stbi__parse_zlib_header(a)) return 0;
   a->num_bits = 0;
   a->code_buffer = 0;
   do {
      final = stbi__zreceive(a,1);
      type = stbi__zreceive(a,2);
      if (type == 0) {
         if (!stbi__parse_uncompressed_block(a)) return 0;
      } else if (type == 3) {
         return 0;
      } else {
         if (type == 1) {
            // use fixed code lengths
            if (!stbi__zdefault_distance[31]) stbi__init_zdefaults();
            if (!stbi__zbuild_huffman(&a->z_length  , stbi__zdefault_length  , 288)) return 0;
            if (!stbi__zbuild_huffman(&a->z_distance, stbi__zdefault_distance,  32)) return 0;
         } else {
            if (!stbi__compute_huffman_codes(a)) return 0;
         }
         if (!stbi__parse_huffman_block(a)) return 0;
      }
   } while (!final);
   return 1;
}

static int stbi__do_zlib(stbi__zbuf *a, char *obuf, int olen, int exp, int parse_header)
{
   a->zout_start = obuf;
   a->zout       = obuf;
   a->zout_end   = obuf + olen;
   a->z_expandable = exp;

   return stbi__parse_zlib(a, parse_header);
}

STBIDEF char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen)
{
   stbi__zbuf a;
   char *p = (char *) stbi__malloc(initial_size);
   if (p == NULL) return NULL;
   a.zbuffer = (stbi_uc *) buffer;
   a.zbuffer_end = (stbi_uc *) buffer + len;
   if (stbi__do_zlib(&a, p, initial_size, 1, 1)) {
      if (outlen) *outlen = (int) (a.zout - a.zout_start);
      return a.zout_start;
   } else {
      STBI_FREE(a.zout_start);
      return NULL;
   }
}

STBIDEF char *stbi_zlib_decode_malloc(char const *buffer, int len, int *outlen)
{
   return stbi_zlib_decode_malloc_guesssize(buffer, len, 16384, outlen);
}

STBIDEF char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header)
{
   stbi__zbuf a;
   char *p = (char *) stbi__malloc(initial_size);
   if (p == NULL) return NULL;
   a.zbuffer = (stbi_uc *) buffer;
   a.zbuffer_end = (stbi_uc *) buffer + len;
   if (stbi__do_zlib(&a, p, initial_size, 1, parse_header)) {
      if (outlen) *outlen = (int) (a.zout - a.zout_start);
      return a.zout_start;
   } else {
      STBI_FREE(a.zout_start);
      return NULL;
   }
}

STBIDEF int stbi_zlib_decode_buffer(char *obuffer, int olen, char const *ibuffer, int ilen)
{
   stbi__zbuf a;
   a.zbuffer = (stbi_uc *) ibuffer;
   a.zbuffer_end = (stbi_uc *) ibuffer + ilen;
   if (stbi__do_zlib(&a, obuffer, olen, 0, 1))
      return (int) (a.zout - a.zout_start);
   else
      return -1;
}

STBIDEF char *stbi_zlib_decode_noheader_malloc(char const *buffer, int len, int *outlen)
{
   stbi__zbuf a;
   char *p = (char *) stbi__malloc(16384);
   if (p == NULL) return NULL;
   a.zbuffer = (stbi_uc *) buffer;
   a.zbuffer_end = (stbi_uc *) buffer+len;
   if (stbi__do_zlib(&a, p, 16384, 1, 0)) {
      if (outlen) *outlen = (int) (a.zout - a.zout_start);
      return a.zout_start;
   } else {
      STBI_FREE(a.zout_start);
      return NULL;
   }
}

STBIDEF int stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen)
{
   stbi__zbuf a;
   a.zbuffer = (stbi_uc *) ibuffer;
   a.zbuffer_end = (stbi_uc *) ibuffer + ilen;
   if (stbi__do_zlib(&a, obuffer, olen, 0, 0))
      return (int) (a.zout - a.zout_start);
   else
      return -1;
}
#endif

// public domain "baseline" PNG decoder   v0.10  Sean Barrett 2006-11-18
//    simple implementation
//      - only 8-bit samples
//      - no CRC checking
//      - allocates lots of intermediate memory
//        - avoids problem of streaming data between subsystems
//        - avoids explicit window management
//    performance
//      - uses stb_zlib, a PD zlib implementation with fast huffman decoding

#ifndef STBI_NO_PNG
typedef struct
{
   stbi__uint32 length;
   stbi__uint32 type;
} stbi__pngchunk;

static stbi__pngchunk stbi__get_chunk_header(stbi__context *s)
{
   stbi__pngchunk c;
   c.length = stbi__get32be(s);
   c.type   = stbi__get32be(s);
   return c;
}

static int stbi__check_png_header(stbi__context *s)
{
   static stbi_uc png_sig[8] = { 137,80,78,71,13,10,26,10 };
   int i;
   for (i=0; i < 8; ++i)
      if (stbi__get8(s) != png_sig[i]) return stbi__err("bad png sig","Not a PNG");
   return 1;
}

typedef struct
{
   stbi__context *s;
   stbi_uc *idata, *expanded, *out;
   int depth;
} stbi__png;


enum {
   STBI__F_none=0,
   STBI__F_sub=1,
   STBI__F_up=2,
   STBI__F_avg=3,
   STBI__F_paeth=4,
   // synthetic filters used for first scanline to avoid needing a dummy row of 0s
   STBI__F_avg_first,
   STBI__F_paeth_first
};

static stbi_uc first_row_filter[5] =
{
   STBI__F_none,
   STBI__F_sub,
   STBI__F_none,
   STBI__F_avg_first,
   STBI__F_paeth_first
};

static int stbi__paeth(int a, int b, int c)
{
   int p = a + b - c;
   int pa = abs(p-a);
   int pb = abs(p-b);
   int pc = abs(p-c);
   if (pa <= pb && pa <= pc) return a;
   if (pb <= pc) return b;
   return c;
}

static stbi_uc stbi__depth_scale_table[9] = { 0, 0xff, 0x55, 0, 0x11, 0,0,0, 0x01 };

// create the png data from post-deflated data
static int stbi__create_png_image_raw(stbi__png *a, stbi_uc *raw, stbi__uint32 raw_len, int out_n, stbi__uint32 x, stbi__uint32 y, int depth, int color)
{
   int bytes = (depth == 16? 2 : 1);
   stbi__context *s = a->s;
   stbi__uint32 i,j,stride = x*out_n*bytes;
   stbi__uint32 img_len, img_width_bytes;
   int k;
   int img_n = s->img_n; // copy it into a local for later

   int output_bytes = out_n*bytes;
   int filter_bytes = img_n*bytes;
   int width = x;

   STBI_ASSERT(out_n == s->img_n || out_n == s->img_n+1);
   a->out = (stbi_uc *) stbi__malloc_mad3(x, y, output_bytes, 0); // extra bytes to write off the end into
   if (!a->out) return stbi__err("outofmem", "Out of memory");

   img_width_bytes = (((img_n * x * depth) + 7) >> 3);
   img_len = (img_width_bytes + 1) * y;
   if (s->img_x == x && s->img_y == y) {
      if (raw_len != img_len) return stbi__err("not enough pixels","Corrupt PNG");
   } else { // interlaced:
      if (raw_len < img_len) return stbi__err("not enough pixels","Corrupt PNG");
   }

   for (j=0; j < y; ++j) {
      stbi_uc *cur = a->out + stride*j;
      stbi_uc *prior = cur - stride;
      int filter = *raw++;

      if (filter > 4)
         return stbi__err("invalid filter","Corrupt PNG");

      if (depth < 8) {
         STBI_ASSERT(img_width_bytes <= x);
         cur += x*out_n - img_width_bytes; // store output to the rightmost img_len bytes, so we can decode in place
         filter_bytes = 1;
         width = img_width_bytes;
      }

      // if first row, use special filter that doesn't sample previous row
      if (j == 0) filter = first_row_filter[filter];

      // handle first byte explicitly
      for (k=0; k < filter_bytes; ++k) {
         switch (filter) {
            case STBI__F_none       : cur[k] = raw[k]; break;
            case STBI__F_sub        : cur[k] = raw[k]; break;
            case STBI__F_up         : cur[k] = STBI__BYTECAST(raw[k] + prior[k]); break;
            case STBI__F_avg        : cur[k] = STBI__BYTECAST(raw[k] + (prior[k]>>1)); break;
            case STBI__F_paeth      : cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(0,prior[k],0)); break;
            case STBI__F_avg_first  : cur[k] = raw[k]; break;
            case STBI__F_paeth_first: cur[k] = raw[k]; break;
         }
      }

      if (depth == 8) {
         if (img_n != out_n)
            cur[img_n] = 255; // first pixel
         raw += img_n;
         cur += out_n;
         prior += out_n;
      } else if (depth == 16) {
         if (img_n != out_n) {
            cur[filter_bytes]   = 255; // first pixel top byte
            cur[filter_bytes+1] = 255; // first pixel bottom byte
         }
         raw += filter_bytes;
         cur += output_bytes;
         prior += output_bytes;
      } else {
         raw += 1;
         cur += 1;
         prior += 1;
      }

      // this is a little gross, so that we don't switch per-pixel or per-component
      if (depth < 8 || img_n == out_n) {
         int nk = (width - 1)*filter_bytes;
         #define STBI__CASE(f) \
             case f:     \
                for (k=0; k < nk; ++k)
         switch (filter) {
            // "none" filter turns into a memcpy here; make that explicit.
            case STBI__F_none:         memcpy(cur, raw, nk); break;
            STBI__CASE(STBI__F_sub)          { cur[k] = STBI__BYTECAST(raw[k] + cur[k-filter_bytes]); } break;
            STBI__CASE(STBI__F_up)           { cur[k] = STBI__BYTECAST(raw[k] + prior[k]); } break;
            STBI__CASE(STBI__F_avg)          { cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k-filter_bytes])>>1)); } break;
            STBI__CASE(STBI__F_paeth)        { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k-filter_bytes],prior[k],prior[k-filter_bytes])); } break;
            STBI__CASE(STBI__F_avg_first)    { cur[k] = STBI__BYTECAST(raw[k] + (cur[k-filter_bytes] >> 1)); } break;
            STBI__CASE(STBI__F_paeth_first)  { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k-filter_bytes],0,0)); } break;
         }
         #undef STBI__CASE
         raw += nk;
      } else {
         STBI_ASSERT(img_n+1 == out_n);
         #define STBI__CASE(f) \
             case f:     \
                for (i=x-1; i >= 1; --i, cur[filter_bytes]=255,raw+=filter_bytes,cur+=output_bytes,prior+=output_bytes) \
                   for (k=0; k < filter_bytes; ++k)
         switch (filter) {
            STBI__CASE(STBI__F_none)         { cur[k] = raw[k]; } break;
            STBI__CASE(STBI__F_sub)          { cur[k] = STBI__BYTECAST(raw[k] + cur[k- output_bytes]); } break;
            STBI__CASE(STBI__F_up)           { cur[k] = STBI__BYTECAST(raw[k] + prior[k]); } break;
            STBI__CASE(STBI__F_avg)          { cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k- output_bytes])>>1)); } break;
            STBI__CASE(STBI__F_paeth)        { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k- output_bytes],prior[k],prior[k- output_bytes])); } break;
            STBI__CASE(STBI__F_avg_first)    { cur[k] = STBI__BYTECAST(raw[k] + (cur[k- output_bytes] >> 1)); } break;
            STBI__CASE(STBI__F_paeth_first)  { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k- output_bytes],0,0)); } break;
         }
         #undef STBI__CASE

         // the loop above sets the high byte of the pixels' alpha, but for
         // 16 bit png files we also need the low byte set. we'll do that here.
         if (depth == 16) {
            cur = a->out + stride*j; // start at the beginning of the row again
            for (i=0; i < x; ++i,cur+=output_bytes) {
               cur[filter_bytes+1] = 255;
            }
         }
      }
   }

   // we make a separate pass to expand bits to pixels; for performance,
   // this could run two scanlines behind the above code, so it won't
   // intefere with filtering but will still be in the cache.
   if (depth < 8) {
      for (j=0; j < y; ++j) {
         stbi_uc *cur = a->out + stride*j;
         stbi_uc *in  = a->out + stride*j + x*out_n - img_width_bytes;
         // unpack 1/2/4-bit into a 8-bit buffer. allows us to keep the common 8-bit path optimal at minimal cost for 1/2/4-bit
         // png guarante byte alignment, if width is not multiple of 8/4/2 we'll decode dummy trailing data that will be skipped in the later loop
         stbi_uc scale = (color == 0) ? stbi__depth_scale_table[depth] : 1; // scale grayscale values to 0..255 range

         // note that the final byte might overshoot and write more data than desired.
         // we can allocate enough data that this never writes out of memory, but it
         // could also overwrite the next scanline. can it overwrite non-empty data
         // on the next scanline? yes, consider 1-pixel-wide scanlines with 1-bit-per-pixel.
         // so we need to explicitly clamp the final ones

         if (depth == 4) {
            for (k=x*img_n; k >= 2; k-=2, ++in) {
               *cur++ = scale * ((*in >> 4)       );
               *cur++ = scale * ((*in     ) & 0x0f);
            }
            if (k > 0) *cur++ = scale * ((*in >> 4)       );
         } else if (depth == 2) {
            for (k=x*img_n; k >= 4; k-=4, ++in) {
               *cur++ = scale * ((*in >> 6)       );
               *cur++ = scale * ((*in >> 4) & 0x03);
               *cur++ = scale * ((*in >> 2) & 0x03);
               *cur++ = scale * ((*in     ) & 0x03);
            }
            if (k > 0) *cur++ = scale * ((*in >> 6)       );
            if (k > 1) *cur++ = scale * ((*in >> 4) & 0x03);
            if (k > 2) *cur++ = scale * ((*in >> 2) & 0x03);
         } else if (depth == 1) {
            for (k=x*img_n; k >= 8; k-=8, ++in) {
               *cur++ = scale * ((*in >> 7)       );
               *cur++ = scale * ((*in >> 6) & 0x01);
               *cur++ = scale * ((*in >> 5) & 0x01);
               *cur++ = scale * ((*in >> 4) & 0x01);
               *cur++ = scale * ((*in >> 3) & 0x01);
               *cur++ = scale * ((*in >> 2) & 0x01);
               *cur++ = scale * ((*in >> 1) & 0x01);
               *cur++ = scale * ((*in     ) & 0x01);
            }
            if (k > 0) *cur++ = scale * ((*in >> 7)       );
            if (k > 1) *cur++ = scale * ((*in >> 6) & 0x01);
            if (k > 2) *cur++ = scale * ((*in >> 5) & 0x01);
            if (k > 3) *cur++ = scale * ((*in >> 4) & 0x01);
            if (k > 4) *cur++ = scale * ((*in >> 3) & 0x01);
            if (k > 5) *cur++ = scale * ((*in >> 2) & 0x01);
            if (k > 6) *cur++ = scale * ((*in >> 1) & 0x01);
         }
         if (img_n != out_n) {
            int q;
            // insert alpha = 255
            cur = a->out + stride*j;
            if (img_n == 1) {
               for (q=x-1; q >= 0; --q) {
                  cur[q*2+1] = 255;
                  cur[q*2+0] = cur[q];
               }
            } else {
               STBI_ASSERT(img_n == 3);
               for (q=x-1; q >= 0; --q) {
                  cur[q*4+3] = 255;
                  cur[q*4+2] = cur[q*3+2];
                  cur[q*4+1] = cur[q*3+1];
                  cur[q*4+0] = cur[q*3+0];
               }
            }
         }
      }
   } else if (depth == 16) {
      // force the image data from big-endian to platform-native.
      // this is done in a separate pass due to the decoding relying
      // on the data being untouched, but could probably be done
      // per-line during decode if care is taken.
      stbi_uc *cur = a->out;
      stbi__uint16 *cur16 = (stbi__uint16*)cur;

      for(i=0; i < x*y*out_n; ++i,cur16++,cur+=2) {
         *cur16 = (cur[0] << 8) | cur[1];
      }
   }

   return 1;
}

static int stbi__create_png_image(stbi__png *a, stbi_uc *image_data, stbi__uint32 image_data_len, int out_n, int depth, int color, int interlaced)
{
   int bytes = (depth == 16 ? 2 : 1);
   int out_bytes = out_n * bytes;
   stbi_uc *final;
   int p;
   if (!interlaced)
      return stbi__create_png_image_raw(a, image_data, image_data_len, out_n, a->s->img_x, a->s->img_y, depth, color);

   // de-interlacing
   final = (stbi_uc *) stbi__malloc_mad3(a->s->img_x, a->s->img_y, out_bytes, 0);
   for (p=0; p < 7; ++p) {
      int xorig[] = { 0,4,0,2,0,1,0 };
      int yorig[] = { 0,0,4,0,2,0,1 };
      int xspc[]  = { 8,8,4,4,2,2,1 };
      int yspc[]  = { 8,8,8,4,4,2,2 };
      int i,j,x,y;
      // pass1_x[4] = 0, pass1_x[5] = 1, pass1_x[12] = 1
      x = (a->s->img_x - xorig[p] + xspc[p]-1) / xspc[p];
      y = (a->s->img_y - yorig[p] + yspc[p]-1) / yspc[p];
      if (x && y) {
         stbi__uint32 img_len = ((((a->s->img_n * x * depth) + 7) >> 3) + 1) * y;
         if (!stbi__create_png_image_raw(a, image_data, image_data_len, out_n, x, y, depth, color)) {
            STBI_FREE(final);
            return 0;
         }
         for (j=0; j < y; ++j) {
            for (i=0; i < x; ++i) {
               int out_y = j*yspc[p]+yorig[p];
               int out_x = i*xspc[p]+xorig[p];
               memcpy(final + out_y*a->s->img_x*out_bytes + out_x*out_bytes,
                      a->out + (j*x+i)*out_bytes, out_bytes);
            }
         }
         STBI_FREE(a->out);
         image_data += img_len;
         image_data_len -= img_len;
      }
   }
   a->out = final;

   return 1;
}

static int stbi__compute_transparency(stbi__png *z, stbi_uc tc[3], int out_n)
{
   stbi__context *s = z->s;
   stbi__uint32 i, pixel_count = s->img_x * s->img_y;
   stbi_uc *p = z->out;

   // compute color-based transparency, assuming we've
   // already got 255 as the alpha value in the output
   STBI_ASSERT(out_n == 2 || out_n == 4);

   if (out_n == 2) {
      for (i=0; i < pixel_count; ++i) {
         p[1] = (p[0] == tc[0] ? 0 : 255);
         p += 2;
      }
   } else {
      for (i=0; i < pixel_count; ++i) {
         if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
            p[3] = 0;
         p += 4;
      }
   }
   return 1;
}

static int stbi__compute_transparency16(stbi__png *z, stbi__uint16 tc[3], int out_n)
{
   stbi__context *s = z->s;
   stbi__uint32 i, pixel_count = s->img_x * s->img_y;
   stbi__uint16 *p = (stbi__uint16*) z->out;

   // compute color-based transparency, assuming we've
   // already got 65535 as the alpha value in the output
   STBI_ASSERT(out_n == 2 || out_n == 4);

   if (out_n == 2) {
      for (i = 0; i < pixel_count; ++i) {
         p[1] = (p[0] == tc[0] ? 0 : 65535);
         p += 2;
      }
   } else {
      for (i = 0; i < pixel_count; ++i) {
         if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
            p[3] = 0;
         p += 4;
      }
   }
   return 1;
}

static int stbi__expand_png_palette(stbi__png *a, stbi_uc *palette, int len, int pal_img_n)
{
   stbi__uint32 i, pixel_count = a->s->img_x * a->s->img_y;
   stbi_uc *p, *temp_out, *orig = a->out;

   p = (stbi_uc *) stbi__malloc_mad2(pixel_count, pal_img_n, 0);
   if (p == NULL) return stbi__err("outofmem", "Out of memory");

   // between here and free(out) below, exitting would leak
   temp_out = p;

   if (pal_img_n == 3) {
      for (i=0; i < pixel_count; ++i) {
         int n = orig[i]*4;
         p[0] = palette[n  ];
         p[1] = palette[n+1];
         p[2] = palette[n+2];
         p += 3;
      }
   } else {
      for (i=0; i < pixel_count; ++i) {
         int n = orig[i]*4;
         p[0] = palette[n  ];
         p[1] = palette[n+1];
         p[2] = palette[n+2];
         p[3] = palette[n+3];
         p += 4;
      }
   }
   STBI_FREE(a->out);
   a->out = temp_out;

   STBI_NOTUSED(len);

   return 1;
}

static int stbi__unpremultiply_on_load = 0;
static int stbi__de_iphone_flag = 0;

STBIDEF void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply)
{
   stbi__unpremultiply_on_load = flag_true_if_should_unpremultiply;
}

STBIDEF void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert)
{
   stbi__de_iphone_flag = flag_true_if_should_convert;
}

static void stbi__de_iphone(stbi__png *z)
{
   stbi__context *s = z->s;
   stbi__uint32 i, pixel_count = s->img_x * s->img_y;
   stbi_uc *p = z->out;

   if (s->img_out_n == 3) {  // convert bgr to rgb
      for (i=0; i < pixel_count; ++i) {
         stbi_uc t = p[0];
         p[0] = p[2];
         p[2] = t;
         p += 3;
      }
   } else {
      STBI_ASSERT(s->img_out_n == 4);
      if (stbi__unpremultiply_on_load) {
         // convert bgr to rgb and unpremultiply
         for (i=0; i < pixel_count; ++i) {
            stbi_uc a = p[3];
            stbi_uc t = p[0];
            if (a) {
               p[0] = p[2] * 255 / a;
               p[1] = p[1] * 255 / a;
               p[2] =  t   * 255 / a;
            } else {
               p[0] = p[2];
               p[2] = t;
            }
            p += 4;
         }
      } else {
         // convert bgr to rgb
         for (i=0; i < pixel_count; ++i) {
            stbi_uc t = p[0];
            p[0] = p[2];
            p[2] = t;
            p += 4;
         }
      }
   }
}

#define STBI__PNG_TYPE(a,b,c,d)  (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))

static int stbi__parse_png_file(stbi__png *z, int scan, int req_comp)
{
   stbi_uc palette[1024], pal_img_n=0;
   stbi_uc has_trans=0, tc[3];
   stbi__uint16 tc16[3];
   stbi__uint32 ioff=0, idata_limit=0, i, pal_len=0;
   int first=1,k,interlace=0, color=0, is_iphone=0;
   stbi__context *s = z->s;

   z->expanded = NULL;
   z->idata = NULL;
   z->out = NULL;

   if (!stbi__check_png_header(s)) return 0;

   if (scan == STBI__SCAN_type) return 1;

   for (;;) {
      stbi__pngchunk c = stbi__get_chunk_header(s);
      switch (c.type) {
         case STBI__PNG_TYPE('C','g','B','I'):
            is_iphone = 1;
            stbi__skip(s, c.length);
            break;
         case STBI__PNG_TYPE('I','H','D','R'): {
            int comp,filter;
            if (!first) return stbi__err("multiple IHDR","Corrupt PNG");
            first = 0;
            if (c.length != 13) return stbi__err("bad IHDR len","Corrupt PNG");
            s->img_x = stbi__get32be(s); if (s->img_x > (1 << 24)) return stbi__err("too large","Very large image (corrupt?)");
            s->img_y = stbi__get32be(s); if (s->img_y > (1 << 24)) return stbi__err("too large","Very large image (corrupt?)");
            z->depth = stbi__get8(s);  if (z->depth != 1 && z->depth != 2 && z->depth != 4 && z->depth != 8 && z->depth != 16)  return stbi__err("1/2/4/8/16-bit only","PNG not supported: 1/2/4/8/16-bit only");
            color = stbi__get8(s);  if (color > 6)         return stbi__err("bad ctype","Corrupt PNG");
			if (color == 3 && z->depth == 16)                  return stbi__err("bad ctype","Corrupt PNG");
            if (color == 3) pal_img_n = 3; else if (color & 1) return stbi__err("bad ctype","Corrupt PNG");
            comp  = stbi__get8(s);  if (comp) return stbi__err("bad comp method","Corrupt PNG");
            filter= stbi__get8(s);  if (filter) return stbi__err("bad filter method","Corrupt PNG");
            interlace = stbi__get8(s); if (interlace>1) return stbi__err("bad interlace method","Corrupt PNG");
            if (!s->img_x || !s->img_y) return stbi__err("0-pixel image","Corrupt PNG");
            if (!pal_img_n) {
               s->img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
               if ((1 << 30) / s->img_x / s->img_n < s->img_y) return stbi__err("too large", "Image too large to decode");
               if (scan == STBI__SCAN_header) return 1;
            } else {
               // if paletted, then pal_n is our final components, and
               // img_n is # components to decompress/filter.
               s->img_n = 1;
               if ((1 << 30) / s->img_x / 4 < s->img_y) return stbi__err("too large","Corrupt PNG");
               // if SCAN_header, have to scan to see if we have a tRNS
            }
            break;
         }

         case STBI__PNG_TYPE('P','L','T','E'):  {
            if (first) return stbi__err("first not IHDR", "Corrupt PNG");
            if (c.length > 256*3) return stbi__err("invalid PLTE","Corrupt PNG");
            pal_len = c.length / 3;
            if (pal_len * 3 != c.length) return stbi__err("invalid PLTE","Corrupt PNG");
            for (i=0; i < pal_len; ++i) {
               palette[i*4+0] = stbi__get8(s);
               palette[i*4+1] = stbi__get8(s);
               palette[i*4+2] = stbi__get8(s);
               palette[i*4+3] = 255;
            }
            break;
         }

         case STBI__PNG_TYPE('t','R','N','S'): {
            if (first) return stbi__err("first not IHDR", "Corrupt PNG");
            if (z->idata) return stbi__err("tRNS after IDAT","Corrupt PNG");
            if (pal_img_n) {
               if (scan == STBI__SCAN_header) { s->img_n = 4; return 1; }
               if (pal_len == 0) return stbi__err("tRNS before PLTE","Corrupt PNG");
               if (c.length > pal_len) return stbi__err("bad tRNS len","Corrupt PNG");
               pal_img_n = 4;
               for (i=0; i < c.length; ++i)
                  palette[i*4+3] = stbi__get8(s);
            } else {
               if (!(s->img_n & 1)) return stbi__err("tRNS with alpha","Corrupt PNG");
               if (c.length != (stbi__uint32) s->img_n*2) return stbi__err("bad tRNS len","Corrupt PNG");
               has_trans = 1;
               if (z->depth == 16) {
                  for (k = 0; k < s->img_n; ++k) tc16[k] = (stbi__uint16)stbi__get16be(s); // copy the values as-is
               } else {
                  for (k = 0; k < s->img_n; ++k) tc[k] = (stbi_uc)(stbi__get16be(s) & 255) * stbi__depth_scale_table[z->depth]; // non 8-bit images will be larger
               }
            }
            break;
         }

         case STBI__PNG_TYPE('I','D','A','T'): {
            if (first) return stbi__err("first not IHDR", "Corrupt PNG");
            if (pal_img_n && !pal_len) return stbi__err("no PLTE","Corrupt PNG");
            if (scan == STBI__SCAN_header) { s->img_n = pal_img_n; return 1; }
            if ((int)(ioff + c.length) < (int)ioff) return 0;
            if (ioff + c.length > idata_limit) {
               stbi__uint32 idata_limit_old = idata_limit;
               stbi_uc *p;
               if (idata_limit == 0) idata_limit = c.length > 4096 ? c.length : 4096;
               while (ioff + c.length > idata_limit)
                  idata_limit *= 2;
               STBI_NOTUSED(idata_limit_old);
               p = (stbi_uc *) STBI_REALLOC_SIZED(z->idata, idata_limit_old, idata_limit); if (p == NULL) return stbi__err("outofmem", "Out of memory");
               z->idata = p;
            }
            if (!stbi__getn(s, z->idata+ioff,c.length)) return stbi__err("outofdata","Corrupt PNG");
            ioff += c.length;
            break;
         }

         case STBI__PNG_TYPE('I','E','N','D'): {
            stbi__uint32 raw_len, bpl;
            if (first) return stbi__err("first not IHDR", "Corrupt PNG");
            if (scan != STBI__SCAN_load) return 1;
            if (z->idata == NULL) return stbi__err("no IDAT","Corrupt PNG");
            // initial guess for decoded data size to avoid unnecessary reallocs
            bpl = (s->img_x * z->depth + 7) / 8; // bytes per line, per component
            raw_len = bpl * s->img_y * s->img_n /* pixels */ + s->img_y /* filter mode per row */;
            z->expanded = (stbi_uc *) stbi_zlib_decode_malloc_guesssize_headerflag((char *) z->idata, ioff, raw_len, (int *) &raw_len, !is_iphone);
            if (z->expanded == NULL) return 0; // zlib should set error
            STBI_FREE(z->idata); z->idata = NULL;
            if ((req_comp == s->img_n+1 && req_comp != 3 && !pal_img_n) || has_trans)
               s->img_out_n = s->img_n+1;
            else
               s->img_out_n = s->img_n;
            if (!stbi__create_png_image(z, z->expanded, raw_len, s->img_out_n, z->depth, color, interlace)) return 0;
            if (has_trans) {
               if (z->depth == 16) {
                  if (!stbi__compute_transparency16(z, tc16, s->img_out_n)) return 0;
               } else {
                  if (!stbi__compute_transparency(z, tc, s->img_out_n)) return 0;
               }
            }
            if (is_iphone && stbi__de_iphone_flag && s->img_out_n > 2)
               stbi__de_iphone(z);
            if (pal_img_n) {
               // pal_img_n == 3 or 4
               s->img_n = pal_img_n; // record the actual colors we had
               s->img_out_n = pal_img_n;
               if (req_comp >= 3) s->img_out_n = req_comp;
               if (!stbi__expand_png_palette(z, palette, pal_len, s->img_out_n))
                  return 0;
            }
            STBI_FREE(z->expanded); z->expanded = NULL;
            return 1;
         }

         default:
            // if critical, fail
            if (first) return stbi__err("first not IHDR", "Corrupt PNG");
            if ((c.type & (1 << 29)) == 0) {
               #ifndef STBI_NO_FAILURE_STRINGS
               // not threadsafe
               static char invalid_chunk[] = "XXXX PNG chunk not known";
               invalid_chunk[0] = STBI__BYTECAST(c.type >> 24);
               invalid_chunk[1] = STBI__BYTECAST(c.type >> 16);
               invalid_chunk[2] = STBI__BYTECAST(c.type >>  8);
               invalid_chunk[3] = STBI__BYTECAST(c.type >>  0);
               #endif
               return stbi__err(invalid_chunk, "PNG not supported: unknown PNG chunk type");
            }
            stbi__skip(s, c.length);
            break;
      }
      // end of PNG chunk, read and skip CRC
      stbi__get32be(s);
   }
}

static void *stbi__do_png(stbi__png *p, int *x, int *y, int *n, int req_comp, stbi__result_info *ri)
{
   void *result=NULL;
   if (req_comp < 0 || req_comp > 4) return stbi__errpuc("bad req_comp", "Internal error");
   if (stbi__parse_png_file(p, STBI__SCAN_load, req_comp)) {
      if (p->depth < 8)
         ri->bits_per_channel = 8;
      else
         ri->bits_per_channel = p->depth;
      result = p->out;
      p->out = NULL;
      if (req_comp && req_comp != p->s->img_out_n) {
         if (ri->bits_per_channel == 8)
            result = stbi__convert_format((unsigned char *) result, p->s->img_out_n, req_comp, p->s->img_x, p->s->img_y);
         else
            result = stbi__convert_format16((stbi__uint16 *) result, p->s->img_out_n, req_comp, p->s->img_x, p->s->img_y);
         p->s->img_out_n = req_comp;
         if (result == NULL) return result;
      }
      *x = p->s->img_x;
      *y = p->s->img_y;
      if (n) *n = p->s->img_n;
   }
   STBI_FREE(p->out);      p->out      = NULL;
   STBI_FREE(p->expanded); p->expanded = NULL;
   STBI_FREE(p->idata);    p->idata    = NULL;

   return result;
}

static void *stbi__png_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri)
{
   stbi__png p;
   p.s = s;
   return stbi__do_png(&p, x,y,comp,req_comp, ri);
}

static int stbi__png_test(stbi__context *s)
{
   int r;
   r = stbi__check_png_header(s);
   stbi__rewind(s);
   return r;
}

static int stbi__png_info_raw(stbi__png *p, int *x, int *y, int *comp)
{
   if (!stbi__parse_png_file(p, STBI__SCAN_header, 0)) {
      stbi__rewind( p->s );
      return 0;
   }
   if (x) *x = p->s->img_x;
   if (y) *y = p->s->img_y;
   if (comp) *comp = p->s->img_n;
   return 1;
}

static int stbi__png_info(stbi__context *s, int *x, int *y, int *comp)
{
   stbi__png p;
   p.s = s;
   return stbi__png_info_raw(&p, x, y, comp);
}
#endif

// Microsoft/Windows BMP image

#ifndef STBI_NO_BMP
static int stbi__bmp_test_raw(stbi__context *s)
{
   int r;
   int sz;
   if (stbi__get8(s) != 'B') return 0;
   if (stbi__get8(s) != 'M') return 0;
   stbi__get32le(s); // discard filesize
   stbi__get16le(s); // discard reserved
   stbi__get16le(s); // discard reserved
   stbi__get32le(s); // discard data offset
   sz = stbi__get32le(s);
   r = (sz == 12 || sz == 40 || sz == 56 || sz == 108 || sz == 124);
   return r;
}

static int stbi__bmp_test(stbi__context *s)
{
   int r = stbi__bmp_test_raw(s);
   stbi__rewind(s);
   return r;
}


// returns 0..31 for the highest set bit
static int stbi__high_bit(unsigned int z)
{
   int n=0;
   if (z == 0) return -1;
   if (z >= 0x10000) n += 16, z >>= 16;
   if (z >= 0x00100) n +=  8, z >>=  8;
   if (z >= 0x00010) n +=  4, z >>=  4;
   if (z >= 0x00004) n +=  2, z >>=  2;
   if (z >= 0x00002) n +=  1, z >>=  1;
   return n;
}

static int stbi__bitcount(unsigned int a)
{
   a = (a & 0x55555555) + ((a >>  1) & 0x55555555); // max 2
   a = (a & 0x33333333) + ((a >>  2) & 0x33333333); // max 4
   a = (a + (a >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
   a = (a + (a >> 8)); // max 16 per 8 bits
   a = (a + (a >> 16)); // max 32 per 8 bits
   return a & 0xff;
}

static int stbi__shiftsigned(int v, int shift, int bits)
{
   int result;
   int z=0;

   if (shift < 0) v <<= -shift;
   else v >>= shift;
   result = v;

   z = bits;
   while (z < 8) {
      result += v >> z;
      z += bits;
   }
   return result;
}

typedef struct
{
   int bpp, offset, hsz;
   unsigned int mr,mg,mb,ma, all_a;
} stbi__bmp_data;

static void *stbi__bmp_parse_header(stbi__context *s, stbi__bmp_data *info)
{
   int hsz;
   if (stbi__get8(s) != 'B' || stbi__get8(s) != 'M') return stbi__errpuc("not BMP", "Corrupt BMP");
   stbi__get32le(s); // discard filesize
   stbi__get16le(s); // discard reserved
   stbi__get16le(s); // discard reserved
   info->offset = stbi__get32le(s);
   info->hsz = hsz = stbi__get32le(s);
   info->mr = info->mg = info->mb = info->ma = 0;
   
   if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124) return stbi__errpuc("unknown BMP", "BMP type not supported: unknown");
   if (hsz == 12) {
      s->img_x = stbi__get16le(s);
      s->img_y = stbi__get16le(s);
   } else {
      s->img_x = stbi__get32le(s);
      s->img_y = stbi__get32le(s);
   }
   if (stbi__get16le(s) != 1) return stbi__errpuc("bad BMP", "bad BMP");
   info->bpp = stbi__get16le(s);
   if (info->bpp == 1) return stbi__errpuc("monochrome", "BMP type not supported: 1-bit");
   if (hsz != 12) {
      int compress = stbi__get32le(s);
      if (compress == 1 || compress == 2) return stbi__errpuc("BMP RLE", "BMP type not supported: RLE");
      stbi__get32le(s); // discard sizeof
      stbi__get32le(s); // discard hres
      stbi__get32le(s); // discard vres
      stbi__get32le(s); // discard colorsused
      stbi__get32le(s); // discard max important
      if (hsz == 40 || hsz == 56) {
         if (hsz == 56) {
            stbi__get32le(s);
            stbi__get32le(s);
            stbi__get32le(s);
            stbi__get32le(s);
         }
         if (info->bpp == 16 || info->bpp == 32) {
            if (compress == 0) {
               if (info->bpp == 32) {
                  info->mr = 0xffu << 16;
                  info->mg = 0xffu <<  8;
                  info->mb = 0xffu <<  0;
                  info->ma = 0xffu << 24;
                  info->all_a = 0; // if all_a is 0 at end, then we loaded alpha channel but it was all 0
               } else {
                  info->mr = 31u << 10;
                  info->mg = 31u <<  5;
                  info->mb = 31u <<  0;
               }
            } else if (compress == 3) {
               info->mr = stbi__get32le(s);
               info->mg = stbi__get32le(s);
               info->mb = stbi__get32le(s);
               // not documented, but generated by photoshop and handled by mspaint
               if (info->mr == info->mg && info->mg == info->mb) {
                  // ?!?!?
                  return stbi__errpuc("bad BMP", "bad BMP");
               }
            } else
               return stbi__errpuc("bad BMP", "bad BMP");
         }
      } else {
         int i;
         if (hsz != 108 && hsz != 124)
            return stbi__errpuc("bad BMP", "bad BMP");
         info->mr = stbi__get32le(s);
         info->mg = stbi__get32le(s);
         info->mb = stbi__get32le(s);
         info->ma = stbi__get32le(s);
         stbi__get32le(s); // discard color space
         for (i=0; i < 12; ++i)
            stbi__get32le(s); // discard color space parameters
         if (hsz == 124) {
            stbi__get32le(s); // discard rendering intent
            stbi__get32le(s); // discard offset of profile data
            stbi__get32le(s); // discard size of profile data
            stbi__get32le(s); // discard reserved
         }
      }
   }
   return (void *) 1;
}


static void *stbi__bmp_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri)
{
   stbi_uc *out;
   unsigned int mr=0,mg=0,mb=0,ma=0, all_a;
   stbi_uc pal[256][4];
   int psize=0,i,j,width;
   int flip_vertically, pad, target;
   stbi__bmp_data info;
   STBI_NOTUSED(ri);

   info.all_a = 255;   
   if (stbi__bmp_parse_header(s, &info) == NULL)
      return NULL; // error code already set

   flip_vertically = ((int) s->img_y) > 0;
   s->img_y = abs((int) s->img_y);

   mr = info.mr;
   mg = info.mg;
   mb = info.mb;
   ma = info.ma;
   all_a = info.all_a;

   if (info.hsz == 12) {
      if (info.bpp < 24)
         psize = (info.offset - 14 - 24) / 3;
   } else {
      if (info.bpp < 16)
         psize = (info.offset - 14 - info.hsz) >> 2;
   }

   s->img_n = ma ? 4 : 3;
   if (req_comp && req_comp >= 3) // we can directly decode 3 or 4
      target = req_comp;
   else
      target = s->img_n; // if they want monochrome, we'll post-convert

   // sanity-check size
   if (!stbi__mad3sizes_valid(target, s->img_x, s->img_y, 0))
      return stbi__errpuc("too large", "Corrupt BMP");

   out = (stbi_uc *) stbi__malloc_mad3(target, s->img_x, s->img_y, 0);
   if (!out) return stbi__errpuc("outofmem", "Out of memory");
   if (info.bpp < 16) {
      int z=0;
      if (psize == 0 || psize > 256) { STBI_FREE(out); return stbi__errpuc("invalid", "Corrupt BMP"); }
      for (i=0; i < psize; ++i) {
         pal[i][2] = stbi__get8(s);
         pal[i][1] = stbi__get8(s);
         pal[i][0] = stbi__get8(s);
         if (info.hsz != 12) stbi__get8(s);
         pal[i][3] = 255;
      }
      stbi__skip(s, info.offset - 14 - info.hsz - psize * (info.hsz == 12 ? 3 : 4));
      if (info.bpp == 4) width = (s->img_x + 1) >> 1;
      else if (info.bpp == 8) width = s->img_x;
      else { STBI_FREE(out); return stbi__errpuc("bad bpp", "Corrupt BMP"); }
      pad = (-width)&3;
      for (j=0; j < (int) s->img_y; ++j) {
         for (i=0; i < (int) s->img_x; i += 2) {
            int v=stbi__get8(s),v2=0;
            if (info.bpp == 4) {
               v2 = v & 15;
               v >>= 4;
            }
            out[z++] = pal[v][0];
            out[z++] = pal[v][1];
            out[z++] = pal[v][2];
            if (target == 4) out[z++] = 255;
            if (i+1 == (int) s->img_x) break;
            v = (info.bpp == 8) ? stbi__get8(s) : v2;
            out[z++] = pal[v][0];
            out[z++] = pal[v][1];
            out[z++] = pal[v][2];
            if (target == 4) out[z++] = 255;
         }
         stbi__skip(s, pad);
      }
   } else {
      int rshift=0,gshift=0,bshift=0,ashift=0,rcount=0,gcount=0,bcount=0,acount=0;
      int z = 0;
      int easy=0;
      stbi__skip(s, info.offset - 14 - info.hsz);
      if (info.bpp == 24) width = 3 * s->img_x;
      else if (info.bpp == 16) width = 2*s->img_x;
      else /* bpp = 32 and pad = 0 */ width=0;
      pad = (-width) & 3;
      if (info.bpp == 24) {
         easy = 1;
      } else if (info.bpp == 32) {
         if (mb == 0xff && mg == 0xff00 && mr == 0x00ff0000 && ma == 0xff000000)
            easy = 2;
      }
      if (!easy) {
         if (!mr || !mg || !mb) { STBI_FREE(out); return stbi__errpuc("bad masks", "Corrupt BMP"); }
         // right shift amt to put high bit in position #7
         rshift = stbi__high_bit(mr)-7; rcount = stbi__bitcount(mr);
         gshift = stbi__high_bit(mg)-7; gcount = stbi__bitcount(mg);
         bshift = stbi__high_bit(mb)-7; bcount = stbi__bitcount(mb);
         ashift = stbi__high_bit(ma)-7; acount = stbi__bitcount(ma);
      }
      for (j=0; j < (int) s->img_y; ++j) {
         if (easy) {
            for (i=0; i < (int) s->img_x; ++i) {
               unsigned char a;
               out[z+2] = stbi__get8(s);
               out[z+1] = stbi__get8(s);
               out[z+0] = stbi__get8(s);
               z += 3;
               a = (easy == 2 ? stbi__get8(s) : 255);
               all_a |= a;
               if (target == 4) out[z++] = a;
            }
         } else {
            int bpp = info.bpp;
            for (i=0; i < (int) s->img_x; ++i) {
               stbi__uint32 v = (bpp == 16 ? (stbi__uint32) stbi__get16le(s) : stbi__get32le(s));
               int a;
               out[z++] = STBI__BYTECAST(stbi__shiftsigned(v & mr, rshift, rcount));
               out[z++] = STBI__BYTECAST(stbi__shiftsigned(v & mg, gshift, gcount));
               out[z++] = STBI__BYTECAST(stbi__shiftsigned(v & mb, bshift, bcount));
               a = (ma ? stbi__shiftsigned(v & ma, ashift, acount) : 255);
               all_a |= a;
               if (target == 4) out[z++] = STBI__BYTECAST(a);
            }
         }
         stbi__skip(s, pad);
      }
   }
   
   // if alpha channel is all 0s, replace with all 255s
   if (target == 4 && all_a == 0)
      for (i=4*s->img_x*s->img_y-1; i >= 0; i -= 4)
         out[i] = 255;

   if (flip_vertically) {
      stbi_uc t;
      for (j=0; j < (int) s->img_y>>1; ++j) {
         stbi_uc *p1 = out +      j     *s->img_x*target;
         stbi_uc *p2 = out + (s->img_y-1-j)*s->img_x*target;
         for (i=0; i < (int) s->img_x*target; ++i) {
            t = p1[i], p1[i] = p2[i], p2[i] = t;
         }
      }
   }

   if (req_comp && req_comp != target) {
      out = stbi__convert_format(out, target, req_comp, s->img_x, s->img_y);
      if (out == NULL) return out; // stbi__convert_format frees input on failure
   }

   *x = s->img_x;
   *y = s->img_y;
   if (comp) *comp = s->img_n;
   return out;
}
#endif

// Targa Truevision - TGA
// by Jonathan Dummer
#ifndef STBI_NO_TGA
// returns STBI_rgb or whatever, 0 on error
static int stbi__tga_get_comp(int bits_per_pixel, int is_grey, int* is_rgb16)
{
   // only RGB or RGBA (incl. 16bit) or grey allowed
   if(is_rgb16) *is_rgb16 = 0;
   switch(bits_per_pixel) {
      case 8:  return STBI_grey;
      case 16: if(is_grey) return STBI_grey_alpha;
            // else: fall-through
      case 15: if(is_rgb16) *is_rgb16 = 1;
            return STBI_rgb;
      case 24: // fall-through
      case 32: return bits_per_pixel/8;
      default: return 0;
   }
}

static int stbi__tga_info(stbi__context *s, int *x, int *y, int *comp)
{
    int tga_w, tga_h, tga_comp, tga_image_type, tga_bits_per_pixel, tga_colormap_bpp;
    int sz, tga_colormap_type;
    stbi__get8(s);                   // discard Offset
    tga_colormap_type = stbi__get8(s); // colormap type
    if( tga_colormap_type > 1 ) {
        stbi__rewind(s);
        return 0;      // only RGB or indexed allowed
    }
    tga_image_type = stbi__get8(s); // image type
    if ( tga_colormap_type == 1 ) { // colormapped (paletted) image
        if (tga_image_type != 1 && tga_image_type != 9) {
            stbi__rewind(s);
            return 0;
        }
        stbi__skip(s,4);       // skip index of first colormap entry and number of entries
        sz = stbi__get8(s);    //   check bits per palette color entry
        if ( (sz != 8) && (sz != 15) && (sz != 16) && (sz != 24) && (sz != 32) ) {
            stbi__rewind(s);
            return 0;
        }
        stbi__skip(s,4);       // skip image x and y origin
        tga_colormap_bpp = sz;
    } else { // "normal" image w/o colormap - only RGB or grey allowed, +/- RLE
        if ( (tga_image_type != 2) && (tga_image_type != 3) && (tga_image_type != 10) && (tga_image_type != 11) ) {
            stbi__rewind(s);
            return 0; // only RGB or grey allowed, +/- RLE
        }
        stbi__skip(s,9); // skip colormap specification and image x/y origin
        tga_colormap_bpp = 0;
    }
    tga_w = stbi__get16le(s);
    if( tga_w < 1 ) {
        stbi__rewind(s);
        return 0;   // test width
    }
    tga_h = stbi__get16le(s);
    if( tga_h < 1 ) {
        stbi__rewind(s);
        return 0;   // test height
    }
    tga_bits_per_pixel = stbi__get8(s); // bits per pixel
    stbi__get8(s); // ignore alpha bits
    if (tga_colormap_bpp != 0) {
        if((tga_bits_per_pixel != 8) && (tga_bits_per_pixel != 16)) {
            // when using a colormap, tga_bits_per_pixel is the size of the indexes
            // I don't think anything but 8 or 16bit indexes makes sense
            stbi__rewind(s);
            return 0;
        }
        tga_comp = stbi__tga_get_comp(tga_colormap_bpp, 0, NULL);
    } else {
        tga_comp = stbi__tga_get_comp(tga_bits_per_pixel, (tga_image_type == 3) || (tga_image_type == 11), NULL);
    }
    if(!tga_comp) {
      stbi__rewind(s);
      return 0;
    }
    if (x) *x = tga_w;
    if (y) *y = tga_h;
    if (comp) *comp = tga_comp;
    return 1;                   // seems to have passed everything
}

static int stbi__tga_test(stbi__context *s)
{
   int res = 0;
   int sz, tga_color_type;
   stbi__get8(s);      //   discard Offset
   tga_color_type = stbi__get8(s);   //   color type
   if ( tga_color_type > 1 ) goto errorEnd;   //   only RGB or indexed allowed
   sz = stbi__get8(s);   //   image type
   if ( tga_color_type == 1 ) { // colormapped (paletted) image
      if (sz != 1 && sz != 9) goto errorEnd; // colortype 1 demands image type 1 or 9
      stbi__skip(s,4);       // skip index of first colormap entry and number of entries
      sz = stbi__get8(s);    //   check bits per palette color entry
      if ( (sz != 8) && (sz != 15) && (sz != 16) && (sz != 24) && (sz != 32) ) goto errorEnd;
      stbi__skip(s,4);       // skip image x and y origin
   } else { // "normal" image w/o colormap
      if ( (sz != 2) && (sz != 3) && (sz != 10) && (sz != 11) ) goto errorEnd; // only RGB or grey allowed, +/- RLE
      stbi__skip(s,9); // skip colormap specification and image x/y origin
   }
   if ( stbi__get16le(s) < 1 ) goto errorEnd;      //   test width
   if ( stbi__get16le(s) < 1 ) goto errorEnd;      //   test height
   sz = stbi__get8(s);   //   bits per pixel
   if ( (tga_color_type == 1) && (sz != 8) && (sz != 16) ) goto errorEnd; // for colormapped images, bpp is size of an index
   if ( (sz != 8) && (sz != 15) && (sz != 16) && (sz != 24) && (sz != 32) ) goto errorEnd;

   res = 1; // if we got this far, everything's good and we can return 1 instead of 0

errorEnd:
   stbi__rewind(s);
   return res;
}

// read 16bit value and convert to 24bit RGB
static void stbi__tga_read_rgb16(stbi__context *s, stbi_uc* out)
{
   stbi__uint16 px = (stbi__uint16)stbi__get16le(s);
   stbi__uint16 fiveBitMask = 31;
   // we have 3 channels with 5bits each
   int r = (px >> 10) & fiveBitMask;
   int g = (px >> 5) & fiveBitMask;
   int b = px & fiveBitMask;
   // Note that this saves the data in RGB(A) order, so it doesn't need to be swapped later
   out[0] = (stbi_uc)((r * 255)/31);
   out[1] = (stbi_uc)((g * 255)/31);
   out[2] = (stbi_uc)((b * 255)/31);

   // some people claim that the most significant bit might be used for alpha
   // (possibly if an alpha-bit is set in the "image descriptor byte")
   // but that only made 16bit test images completely translucent..
   // so let's treat all 15 and 16bit TGAs as RGB with no alpha.
}

static void *stbi__tga_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri)
{
   //   read in the TGA header stuff
   int tga_offset = stbi__get8(s);
   int tga_indexed = stbi__get8(s);
   int tga_image_type = stbi__get8(s);
   int tga_is_RLE = 0;
   int tga_palette_start = stbi__get16le(s);
   int tga_palette_len = stbi__get16le(s);
   int tga_palette_bits = stbi__get8(s);
   int tga_x_origin = stbi__get16le(s);
   int tga_y_origin = stbi__get16le(s);
   int tga_width = stbi__get16le(s);
   int tga_height = stbi__get16le(s);
   int tga_bits_per_pixel = stbi__get8(s);
   int tga_comp, tga_rgb16=0;
   int tga_inverted = stbi__get8(s);
   // int tga_alpha_bits = tga_inverted & 15; // the 4 lowest bits - unused (useless?)
   //   image data
   unsigned char *tga_data;
   unsigned char *tga_palette = NULL;
   int i, j;
   unsigned char raw_data[4] = {0};
   int RLE_count = 0;
   int RLE_repeating = 0;
   int read_next_pixel = 1;
   STBI_NOTUSED(ri);

   //   do a tiny bit of precessing
   if ( tga_image_type >= 8 )
   {
      tga_image_type -= 8;
      tga_is_RLE = 1;
   }
   tga_inverted = 1 - ((tga_inverted >> 5) & 1);

   //   If I'm paletted, then I'll use the number of bits from the palette
   if ( tga_indexed ) tga_comp = stbi__tga_get_comp(tga_palette_bits, 0, &tga_rgb16);
   else tga_comp = stbi__tga_get_comp(tga_bits_per_pixel, (tga_image_type == 3), &tga_rgb16);

   if(!tga_comp) // shouldn't really happen, stbi__tga_test() should have ensured basic consistency
      return stbi__errpuc("bad format", "Can't find out TGA pixelformat");

   //   tga info
   *x = tga_width;
   *y = tga_height;
   if (comp) *comp = tga_comp;

   if (!stbi__mad3sizes_valid(tga_width, tga_height, tga_comp, 0))
      return stbi__errpuc("too large", "Corrupt TGA");

   tga_data = (unsigned char*)stbi__malloc_mad3(tga_width, tga_height, tga_comp, 0);
   if (!tga_data) return stbi__errpuc("outofmem", "Out of memory");

   // skip to the data's starting position (offset usually = 0)
   stbi__skip(s, tga_offset );

   if ( !tga_indexed && !tga_is_RLE && !tga_rgb16 ) {
      for (i=0; i < tga_height; ++i) {
         int row = tga_inverted ? tga_height -i - 1 : i;
         stbi_uc *tga_row = tga_data + row*tga_width*tga_comp;
         stbi__getn(s, tga_row, tga_width * tga_comp);
      }
   } else  {
      //   do I need to load a palette?
      if ( tga_indexed)
      {
         //   any data to skip? (offset usually = 0)
         stbi__skip(s, tga_palette_start );
         //   load the palette
         tga_palette = (unsigned char*)stbi__malloc_mad2(tga_palette_len, tga_comp, 0);
         if (!tga_palette) {
            STBI_FREE(tga_data);
            return stbi__errpuc("outofmem", "Out of memory");
         }
         if (tga_rgb16) {
            stbi_uc *pal_entry = tga_palette;
            STBI_ASSERT(tga_comp == STBI_rgb);
            for (i=0; i < tga_palette_len; ++i) {
               stbi__tga_read_rgb16(s, pal_entry);
               pal_entry += tga_comp;
            }
         } else if (!stbi__getn(s, tga_palette, tga_palette_len * tga_comp)) {
               STBI_FREE(tga_data);
               STBI_FREE(tga_palette);
               return stbi__errpuc("bad palette", "Corrupt TGA");
         }
      }
      //   load the data
      for (i=0; i < tga_width * tga_height; ++i)
      {
         //   if I'm in RLE mode, do I need to get a RLE stbi__pngchunk?
         if ( tga_is_RLE )
         {
            if ( RLE_count == 0 )
            {
               //   yep, get the next byte as a RLE command
               int RLE_cmd = stbi__get8(s);
               RLE_count = 1 + (RLE_cmd & 127);
               RLE_repeating = RLE_cmd >> 7;
               read_next_pixel = 1;
            } else if ( !RLE_repeating )
            {
               read_next_pixel = 1;
            }
         } else
         {
            read_next_pixel = 1;
         }
         //   OK, if I need to read a pixel, do it now
         if ( read_next_pixel )
         {
            //   load however much data we did have
            if ( tga_indexed )
            {
               // read in index, then perform the lookup
               int pal_idx = (tga_bits_per_pixel == 8) ? stbi__get8(s) : stbi__get16le(s);
               if ( pal_idx >= tga_palette_len ) {
                  // invalid index
                  pal_idx = 0;
               }
               pal_idx *= tga_comp;
               for (j = 0; j < tga_comp; ++j) {
                  raw_data[j] = tga_palette[pal_idx+j];
               }
            } else if(tga_rgb16) {
               STBI_ASSERT(tga_comp == STBI_rgb);
               stbi__tga_read_rgb16(s, raw_data);
            } else {
               //   read in the data raw
               for (j = 0; j < tga_comp; ++j) {
                  raw_data[j] = stbi__get8(s);
               }
            }
            //   clear the reading flag for the next pixel
            read_next_pixel = 0;
         } // end of reading a pixel

         // copy data
         for (j = 0; j < tga_comp; ++j)
           tga_data[i*tga_comp+j] = raw_data[j];

         //   in case we're in RLE mode, keep counting down
         --RLE_count;
      }
      //   do I need to invert the image?
      if ( tga_inverted )
      {
         for (j = 0; j*2 < tga_height; ++j)
         {
            int index1 = j * tga_width * tga_comp;
            int index2 = (tga_height - 1 - j) * tga_width * tga_comp;
            for (i = tga_width * tga_comp; i > 0; --i)
            {
               unsigned char temp = tga_data[index1];
               tga_data[index1] = tga_data[index2];
               tga_data[index2] = temp;
               ++index1;
               ++index2;
            }
         }
      }
      //   clear my palette, if I had one
      if ( tga_palette != NULL )
      {
         STBI_FREE( tga_palette );
      }
   }

   // swap RGB - if the source data was RGB16, it already is in the right order
   if (tga_comp >= 3 && !tga_rgb16)
   {
      unsigned char* tga_pixel = tga_data;
      for (i=0; i < tga_width * tga_height; ++i)
      {
         unsigned char temp = tga_pixel[0];
         tga_pixel[0] = tga_pixel[2];
         tga_pixel[2] = temp;
         tga_pixel += tga_comp;
      }
   }

   // convert to target component count
   if (req_comp && req_comp != tga_comp)
      tga_data = stbi__convert_format(tga_data, tga_comp, req_comp, tga_width, tga_height);

   //   the things I do to get rid of an error message, and yet keep
   //   Microsoft's C compilers happy... [8^(
   tga_palette_start = tga_palette_len = tga_palette_bits =
         tga_x_origin = tga_y_origin = 0;
   //   OK, done
   return tga_data;
}
#endif

// *************************************************************************************************
// Photoshop PSD loader -- PD by Thatcher Ulrich, integration by Nicolas Schulz, tweaked by STB

#ifndef STBI_NO_PSD
static int stbi__psd_test(stbi__context *s)
{
   int r = (stbi__get32be(s) == 0x38425053);
   stbi__rewind(s);
   return r;
}

static int stbi__psd_decode_rle(stbi__context *s, stbi_uc *p, int pixelCount)
{
   int count, nleft, len;

   count = 0;
   while ((nleft = pixelCount - count) > 0) {
      len = stbi__get8(s);
      if (len == 128) {
         // No-op.
      } else if (len < 128) {
         // Copy next len+1 bytes literally.
         len++;
         if (len > nleft) return 0; // corrupt data
         count += len;
         while (len) {
            *p = stbi__get8(s);
            p += 4;
            len--;
         }
      } else if (len > 128) {
         stbi_uc   val;
         // Next -len+1 bytes in the dest are replicated from next source byte.
         // (Interpret len as a negative 8-bit int.)
         len = 257 - len;
         if (len > nleft) return 0; // corrupt data
         val = stbi__get8(s);
         count += len;
         while (len) {
            *p = val;
            p += 4;
            len--;
         }
      }
   }

   return 1;
}

static void *stbi__psd_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri, int bpc)
{
   int pixelCount;
   int channelCount, compression;
   int channel, i;
   int bitdepth;
   int w,h;
   stbi_uc *out;
   STBI_NOTUSED(ri);

   // Check identifier
   if (stbi__get32be(s) != 0x38425053)   // "8BPS"
      return stbi__errpuc("not PSD", "Corrupt PSD image");

   // Check file type version.
   if (stbi__get16be(s) != 1)
      return stbi__errpuc("wrong version", "Unsupported version of PSD image");

   // Skip 6 reserved bytes.
   stbi__skip(s, 6 );

   // Read the number of channels (R, G, B, A, etc).
   channelCount = stbi__get16be(s);
   if (channelCount < 0 || channelCount > 16)
      return stbi__errpuc("wrong channel count", "Unsupported number of channels in PSD image");

   // Read the rows and columns of the image.
   h = stbi__get32be(s);
   w = stbi__get32be(s);

   // Make sure the depth is 8 bits.
   bitdepth = stbi__get16be(s);
   if (bitdepth != 8 && bitdepth != 16)
      return stbi__errpuc("unsupported bit depth", "PSD bit depth is not 8 or 16 bit");

   // Make sure the color mode is RGB.
   // Valid options are:
   //   0: Bitmap
   //   1: Grayscale
   //   2: Indexed color
   //   3: RGB color
   //   4: CMYK color
   //   7: Multichannel
   //   8: Duotone
   //   9: Lab color
   if (stbi__get16be(s) != 3)
      return stbi__errpuc("wrong color format", "PSD is not in RGB color format");

   // Skip the Mode Data.  (It's the palette for indexed color; other info for other modes.)
   stbi__skip(s,stbi__get32be(s) );

   // Skip the image resources.  (resolution, pen tool paths, etc)
   stbi__skip(s, stbi__get32be(s) );

   // Skip the reserved data.
   stbi__skip(s, stbi__get32be(s) );

   // Find out if the data is compressed.
   // Known values:
   //   0: no compression
   //   1: RLE compressed
   compression = stbi__get16be(s);
   if (compression > 1)
      return stbi__errpuc("bad compression", "PSD has an unknown compression format");

   // Check size
   if (!stbi__mad3sizes_valid(4, w, h, 0))
      return stbi__errpuc("too large", "Corrupt PSD");

   // Create the destination image.

   if (!compression && bitdepth == 16 && bpc == 16) {
      out = (stbi_uc *) stbi__malloc_mad3(8, w, h, 0);
      ri->bits_per_channel = 16;
   } else
      out = (stbi_uc *) stbi__malloc(4 * w*h);

   if (!out) return stbi__errpuc("outofmem", "Out of memory");
   pixelCount = w*h;

   // Initialize the data to zero.
   //memset( out, 0, pixelCount * 4 );

   // Finally, the image data.
   if (compression) {
      // RLE as used by .PSD and .TIFF
      // Loop until you get the number of unpacked bytes you are expecting:
      //     Read the next source byte into n.
      //     If n is between 0 and 127 inclusive, copy the next n+1 bytes literally.
      //     Else if n is between -127 and -1 inclusive, copy the next byte -n+1 times.
      //     Else if n is 128, noop.
      // Endloop

      // The RLE-compressed data is preceeded by a 2-byte data count for each row in the data,
      // which we're going to just skip.
      stbi__skip(s, h * channelCount * 2 );

      // Read the RLE data by channel.
      for (channel = 0; channel < 4; channel++) {
         stbi_uc *p;

         p = out+channel;
         if (channel >= channelCount) {
            // Fill this channel with default data.
            for (i = 0; i < pixelCount; i++, p += 4)
               *p = (channel == 3 ? 255 : 0);
         } else {
            // Read the RLE data.
            if (!stbi__psd_decode_rle(s, p, pixelCount)) {
               STBI_FREE(out);
               return stbi__errpuc("corrupt", "bad RLE data");
            }
         }
      }

   } else {
      // We're at the raw image data.  It's each channel in order (Red, Green, Blue, Alpha, ...)
      // where each channel consists of an 8-bit (or 16-bit) value for each pixel in the image.

      // Read the data by channel.
      for (channel = 0; channel < 4; channel++) {
         if (channel >= channelCount) {
            // Fill this channel with default data.
            if (bitdepth == 16 && bpc == 16) {
               stbi__uint16 *q = ((stbi__uint16 *) out) + channel;
               stbi__uint16 val = channel == 3 ? 65535 : 0;
               for (i = 0; i < pixelCount; i++, q += 4)
                  *q = val;
            } else {
               stbi_uc *p = out+channel;
               stbi_uc val = channel == 3 ? 255 : 0;
               for (i = 0; i < pixelCount; i++, p += 4)
                  *p = val;
            }
         } else {
            if (ri->bits_per_channel == 16) {    // output bpc
               stbi__uint16 *q = ((stbi__uint16 *) out) + channel;
               for (i = 0; i < pixelCount; i++, q += 4)
                  *q = (stbi__uint16) stbi__get16be(s);
            } else {
               stbi_uc *p = out+channel;
               if (bitdepth == 16) {  // input bpc
                  for (i = 0; i < pixelCount; i++, p += 4)
                     *p = (stbi_uc) (stbi__get16be(s) >> 8);
               } else {
                  for (i = 0; i < pixelCount; i++, p += 4)
                     *p = stbi__get8(s);
               }
            }
         }
      }
   }

   // remove weird white matte from PSD
   if (channelCount >= 4) {
      if (ri->bits_per_channel == 16) {
         for (i=0; i < w*h; ++i) {
            stbi__uint16 *pixel = (stbi__uint16 *) out + 4*i;
            if (pixel[3] != 0 && pixel[3] != 65535) {
               float a = pixel[3] / 65535.0f;
               float ra = 1.0f / a;
               float inv_a = 65535.0f * (1 - ra);
               pixel[0] = (stbi__uint16) (pixel[0]*ra + inv_a);
               pixel[1] = (stbi__uint16) (pixel[1]*ra + inv_a);
               pixel[2] = (stbi__uint16) (pixel[2]*ra + inv_a);
            }
         }
      } else {
         for (i=0; i < w*h; ++i) {
            unsigned char *pixel = out + 4*i;
            if (pixel[3] != 0 && pixel[3] != 255) {
               float a = pixel[3] / 255.0f;
               float ra = 1.0f / a;
               float inv_a = 255.0f * (1 - ra);
               pixel[0] = (unsigned char) (pixel[0]*ra + inv_a);
               pixel[1] = (unsigned char) (pixel[1]*ra + inv_a);
               pixel[2] = (unsigned char) (pixel[2]*ra + inv_a);
            }
         }
      }
   }

   // convert to desired output format
   if (req_comp && req_comp != 4) {
      if (ri->bits_per_channel == 16)
         out = (stbi_uc *) stbi__convert_format16((stbi__uint16 *) out, 4, req_comp, w, h);
      else
         out = stbi__convert_format(out, 4, req_comp, w, h);
      if (out == NULL) return out; // stbi__convert_format frees input on failure
   }

   if (comp) *comp = 4;
   *y = h;
   *x = w;

   return out;
}
#endif

// *************************************************************************************************
// Softimage PIC loader
// by Tom Seddon
//
// See http://softimage.wiki.softimage.com/index.php/INFO:_PIC_file_format
// See http://ozviz.wasp.uwa.edu.au/~pbourke/dataformats/softimagepic/

#ifndef STBI_NO_PIC
static int stbi__pic_is4(stbi__context *s,const char *str)
{
   int i;
   for (i=0; i<4; ++i)
      if (stbi__get8(s) != (stbi_uc)str[i])
         return 0;

   return 1;
}

static int stbi__pic_test_core(stbi__context *s)
{
   int i;

   if (!stbi__pic_is4(s,"\x53\x80\xF6\x34"))
      return 0;

   for(i=0;i<84;++i)
      stbi__get8(s);

   if (!stbi__pic_is4(s,"PICT"))
      return 0;

   return 1;
}

typedef struct
{
   stbi_uc size,type,channel;
} stbi__pic_packet;

static stbi_uc *stbi__readval(stbi__context *s, int channel, stbi_uc *dest)
{
   int mask=0x80, i;

   for (i=0; i<4; ++i, mask>>=1) {
      if (channel & mask) {
         if (stbi__at_eof(s)) return stbi__errpuc("bad file","PIC file too short");
         dest[i]=stbi__get8(s);
      }
   }

   return dest;
}

static void stbi__copyval(int channel,stbi_uc *dest,const stbi_uc *src)
{
   int mask=0x80,i;

   for (i=0;i<4; ++i, mask>>=1)
      if (channel&mask)
         dest[i]=src[i];
}

static stbi_uc *stbi__pic_load_core(stbi__context *s,int width,int height,int *comp, stbi_uc *result)
{
   int act_comp=0,num_packets=0,y,chained;
   stbi__pic_packet packets[10];

   // this will (should...) cater for even some bizarre stuff like having data
    // for the same channel in multiple packets.
   do {
      stbi__pic_packet *packet;

      if (num_packets==sizeof(packets)/sizeof(packets[0]))
         return stbi__errpuc("bad format","too many packets");

      packet = &packets[num_packets++];

      chained = stbi__get8(s);
      packet->size    = stbi__get8(s);
      packet->type    = stbi__get8(s);
      packet->channel = stbi__get8(s);

      act_comp |= packet->channel;

      if (stbi__at_eof(s))          return stbi__errpuc("bad file","file too short (reading packets)");
      if (packet->size != 8)  return stbi__errpuc("bad format","packet isn't 8bpp");
   } while (chained);

   *comp = (act_comp & 0x10 ? 4 : 3); // has alpha channel?

   for(y=0; y<height; ++y) {
      int packet_idx;

      for(packet_idx=0; packet_idx < num_packets; ++packet_idx) {
         stbi__pic_packet *packet = &packets[packet_idx];
         stbi_uc *dest = result+y*width*4;

         switch (packet->type) {
            default:
               return stbi__errpuc("bad format","packet has bad compression type");

            case 0: {//uncompressed
               int x;

               for(x=0;x<width;++x, dest+=4)
                  if (!stbi__readval(s,packet->channel,dest))
                     return 0;
               break;
            }

            case 1://Pure RLE
               {
                  int left=width, i;

                  while (left>0) {
                     stbi_uc count,value[4];

                     count=stbi__get8(s);
                     if (stbi__at_eof(s))   return stbi__errpuc("bad file","file too short (pure read count)");

                     if (count > left)
                        count = (stbi_uc) left;

                     if (!stbi__readval(s,packet->channel,value))  return 0;

                     for(i=0; i<count; ++i,dest+=4)
                        stbi__copyval(packet->channel,dest,value);
                     left -= count;
                  }
               }
               break;

            case 2: {//Mixed RLE
               int left=width;
               while (left>0) {
                  int count = stbi__get8(s), i;
                  if (stbi__at_eof(s))  return stbi__errpuc("bad file","file too short (mixed read count)");

                  if (count >= 128) { // Repeated
                     stbi_uc value[4];

                     if (count==128)
                        count = stbi__get16be(s);
                     else
                        count -= 127;
                     if (count > left)
                        return stbi__errpuc("bad file","scanline overrun");

                     if (!stbi__readval(s,packet->channel,value))
                        return 0;

                     for(i=0;i<count;++i, dest += 4)
                        stbi__copyval(packet->channel,dest,value);
                  } else { // Raw
                     ++count;
                     if (count>left) return stbi__errpuc("bad file","scanline overrun");

                     for(i=0;i<count;++i, dest+=4)
                        if (!stbi__readval(s,packet->channel,dest))
                           return 0;
                  }
                  left-=count;
               }
               break;
            }
         }
      }
   }

   return result;
}

static void *stbi__pic_load(stbi__context *s,int *px,int *py,int *comp,int req_comp, stbi__result_info *ri)
{
   stbi_uc *result;
   int i, x,y;
   STBI_NOTUSED(ri);

   for (i=0; i<92; ++i)
      stbi__get8(s);

   x = stbi__get16be(s);
   y = stbi__get16be(s);
   if (stbi__at_eof(s))  return stbi__errpuc("bad file","file too short (pic header)");
   if (!stbi__mad3sizes_valid(x, y, 4, 0)) return stbi__errpuc("too large", "PIC image too large to decode");

   stbi__get32be(s); //skip `ratio'
   stbi__get16be(s); //skip `fields'
   stbi__get16be(s); //skip `pad'

   // intermediate buffer is RGBA
   result = (stbi_uc *) stbi__malloc_mad3(x, y, 4, 0);
   memset(result, 0xff, x*y*4);

   if (!stbi__pic_load_core(s,x,y,comp, result)) {
      STBI_FREE(result);
      result=0;
   }
   *px = x;
   *py = y;
   if (req_comp == 0) req_comp = *comp;
   result=stbi__convert_format(result,4,req_comp,x,y);

   return result;
}

static int stbi__pic_test(stbi__context *s)
{
   int r = stbi__pic_test_core(s);
   stbi__rewind(s);
   return r;
}
#endif

// *************************************************************************************************
// GIF loader -- public domain by Jean-Marc Lienher -- simplified/shrunk by stb

#ifndef STBI_NO_GIF
typedef struct
{
   stbi__int16 prefix;
   stbi_uc first;
   stbi_uc suffix;
} stbi__gif_lzw;

typedef struct
{
   int w,h;
   stbi_uc *out, *old_out;             // output buffer (always 4 components)
   int flags, bgindex, ratio, transparent, eflags, delay;
   stbi_uc  pal[256][4];
   stbi_uc lpal[256][4];
   stbi__gif_lzw codes[4096];
   stbi_uc *color_table;
   int parse, step;
   int lflags;
   int start_x, start_y;
   int max_x, max_y;
   int cur_x, cur_y;
   int line_size;
} stbi__gif;

static int stbi__gif_test_raw(stbi__context *s)
{
   int sz;
   if (stbi__get8(s) != 'G' || stbi__get8(s) != 'I' || stbi__get8(s) != 'F' || stbi__get8(s) != '8') return 0;
   sz = stbi__get8(s);
   if (sz != '9' && sz != '7') return 0;
   if (stbi__get8(s) != 'a') return 0;
   return 1;
}

static int stbi__gif_test(stbi__context *s)
{
   int r = stbi__gif_test_raw(s);
   stbi__rewind(s);
   return r;
}

static void stbi__gif_parse_colortable(stbi__context *s, stbi_uc pal[256][4], int num_entries, int transp)
{
   int i;
   for (i=0; i < num_entries; ++i) {
      pal[i][2] = stbi__get8(s);
      pal[i][1] = stbi__get8(s);
      pal[i][0] = stbi__get8(s);
      pal[i][3] = transp == i ? 0 : 255;
   }
}

static int stbi__gif_header(stbi__context *s, stbi__gif *g, int *comp, int is_info)
{
   stbi_uc version;
   if (stbi__get8(s) != 'G' || stbi__get8(s) != 'I' || stbi__get8(s) != 'F' || stbi__get8(s) != '8')
      return stbi__err("not GIF", "Corrupt GIF");

   version = stbi__get8(s);
   if (version != '7' && version != '9')    return stbi__err("not GIF", "Corrupt GIF");
   if (stbi__get8(s) != 'a')                return stbi__err("not GIF", "Corrupt GIF");

   stbi__g_failure_reason = "";
   g->w = stbi__get16le(s);
   g->h = stbi__get16le(s);
   g->flags = stbi__get8(s);
   g->bgindex = stbi__get8(s);
   g->ratio = stbi__get8(s);
   g->transparent = -1;

   if (comp != 0) *comp = 4;  // can't actually tell whether it's 3 or 4 until we parse the comments

   if (is_info) return 1;

   if (g->flags & 0x80)
      stbi__gif_parse_colortable(s,g->pal, 2 << (g->flags & 7), -1);

   return 1;
}

static int stbi__gif_info_raw(stbi__context *s, int *x, int *y, int *comp)
{
   stbi__gif* g = (stbi__gif*) stbi__malloc(sizeof(stbi__gif));
   if (!stbi__gif_header(s, g, comp, 1)) {
      STBI_FREE(g);
      stbi__rewind( s );
      return 0;
   }
   if (x) *x = g->w;
   if (y) *y = g->h;
   STBI_FREE(g);
   return 1;
}

static void stbi__out_gif_code(stbi__gif *g, stbi__uint16 code)
{
   stbi_uc *p, *c;

   // recurse to decode the prefixes, since the linked-list is backwards,
   // and working backwards through an interleaved image would be nasty
   if (g->codes[code].prefix >= 0)
      stbi__out_gif_code(g, g->codes[code].prefix);

   if (g->cur_y >= g->max_y) return;

   p = &g->out[g->cur_x + g->cur_y];
   c = &g->color_table[g->codes[code].suffix * 4];

   if (c[3] >= 128) {
      p[0] = c[2];
      p[1] = c[1];
      p[2] = c[0];
      p[3] = c[3];
   }
   g->cur_x += 4;

   if (g->cur_x >= g->max_x) {
      g->cur_x = g->start_x;
      g->cur_y += g->step;

      while (g->cur_y >= g->max_y && g->parse > 0) {
         g->step = (1 << g->parse) * g->line_size;
         g->cur_y = g->start_y + (g->step >> 1);
         --g->parse;
      }
   }
}

static stbi_uc *stbi__process_gif_raster(stbi__context *s, stbi__gif *g)
{
   stbi_uc lzw_cs;
   stbi__int32 len, init_code;
   stbi__uint32 first;
   stbi__int32 codesize, codemask, avail, oldcode, bits, valid_bits, clear;
   stbi__gif_lzw *p;

   lzw_cs = stbi__get8(s);
   if (lzw_cs > 12) return NULL;
   clear = 1 << lzw_cs;
   first = 1;
   codesize = lzw_cs + 1;
   codemask = (1 << codesize) - 1;
   bits = 0;
   valid_bits = 0;
   for (init_code = 0; init_code < clear; init_code++) {
      g->codes[init_code].prefix = -1;
      g->codes[init_code].first = (stbi_uc) init_code;
      g->codes[init_code].suffix = (stbi_uc) init_code;
   }

   // support no starting clear code
   avail = clear+2;
   oldcode = -1;

   len = 0;
   for(;;) {
      if (valid_bits < codesize) {
         if (len == 0) {
            len = stbi__get8(s); // start new block
            if (len == 0)
               return g->out;
         }
         --len;
         bits |= (stbi__int32) stbi__get8(s) << valid_bits;
         valid_bits += 8;
      } else {
         stbi__int32 code = bits & codemask;
         bits >>= codesize;
         valid_bits -= codesize;
         // @OPTIMIZE: is there some way we can accelerate the non-clear path?
         if (code == clear) {  // clear code
            codesize = lzw_cs + 1;
            codemask = (1 << codesize) - 1;
            avail = clear + 2;
            oldcode = -1;
            first = 0;
         } else if (code == clear + 1) { // end of stream code
            stbi__skip(s, len);
            while ((len = stbi__get8(s)) > 0)
               stbi__skip(s,len);
            return g->out;
         } else if (code <= avail) {
            if (first) return stbi__errpuc("no clear code", "Corrupt GIF");

            if (oldcode >= 0) {
               p = &g->codes[avail++];
               if (avail > 4096)        return stbi__errpuc("too many codes", "Corrupt GIF");
               p->prefix = (stbi__int16) oldcode;
               p->first = g->codes[oldcode].first;
               p->suffix = (code == avail) ? p->first : g->codes[code].first;
            } else if (code == avail)
               return stbi__errpuc("illegal code in raster", "Corrupt GIF");

            stbi__out_gif_code(g, (stbi__uint16) code);

            if ((avail & codemask) == 0 && avail <= 0x0FFF) {
               codesize++;
               codemask = (1 << codesize) - 1;
            }

            oldcode = code;
         } else {
            return stbi__errpuc("illegal code in raster", "Corrupt GIF");
         }
      }
   }
}

static void stbi__fill_gif_background(stbi__gif *g, int x0, int y0, int x1, int y1)
{
   int x, y;
   stbi_uc *c = g->pal[g->bgindex];
   for (y = y0; y < y1; y += 4 * g->w) {
      for (x = x0; x < x1; x += 4) {
         stbi_uc *p  = &g->out[y + x];
         p[0] = c[2];
         p[1] = c[1];
         p[2] = c[0];
         p[3] = 0;
      }
   }
}

// this function is designed to support animated gifs, although stb_image doesn't support it
static stbi_uc *stbi__gif_load_next(stbi__context *s, stbi__gif *g, int *comp, int req_comp)
{
   int i;
   stbi_uc *prev_out = 0;

   if (g->out == 0 && !stbi__gif_header(s, g, comp,0))
      return 0; // stbi__g_failure_reason set by stbi__gif_header

   if (!stbi__mad3sizes_valid(g->w, g->h, 4, 0))
      return stbi__errpuc("too large", "GIF too large");

   prev_out = g->out;
   g->out = (stbi_uc *) stbi__malloc_mad3(4, g->w, g->h, 0);
   if (g->out == 0) return stbi__errpuc("outofmem", "Out of memory");

   switch ((g->eflags & 0x1C) >> 2) {
      case 0: // unspecified (also always used on 1st frame)
         stbi__fill_gif_background(g, 0, 0, 4 * g->w, 4 * g->w * g->h);
         break;
      case 1: // do not dispose
         if (prev_out) memcpy(g->out, prev_out, 4 * g->w * g->h);
         g->old_out = prev_out;
         break;
      case 2: // dispose to background
         if (prev_out) memcpy(g->out, prev_out, 4 * g->w * g->h);
         stbi__fill_gif_background(g, g->start_x, g->start_y, g->max_x, g->max_y);
         break;
      case 3: // dispose to previous
         if (g->old_out) {
            for (i = g->start_y; i < g->max_y; i += 4 * g->w)
               memcpy(&g->out[i + g->start_x], &g->old_out[i + g->start_x], g->max_x - g->start_x);
         }
         break;
   }

   for (;;) {
      switch (stbi__get8(s)) {
         case 0x2C: /* Image Descriptor */
         {
            int prev_trans = -1;
            stbi__int32 x, y, w, h;
            stbi_uc *o;

            x = stbi__get16le(s);
            y = stbi__get16le(s);
            w = stbi__get16le(s);
            h = stbi__get16le(s);
            if (((x + w) > (g->w)) || ((y + h) > (g->h)))
               return stbi__errpuc("bad Image Descriptor", "Corrupt GIF");

            g->line_size = g->w * 4;
            g->start_x = x * 4;
            g->start_y = y * g->line_size;
            g->max_x   = g->start_x + w * 4;
            g->max_y   = g->start_y + h * g->line_size;
            g->cur_x   = g->start_x;
            g->cur_y   = g->start_y;

            g->lflags = stbi__get8(s);

            if (g->lflags & 0x40) {
               g->step = 8 * g->line_size; // first interlaced spacing
               g->parse = 3;
            } else {
               g->step = g->line_size;
               g->parse = 0;
            }

            if (g->lflags & 0x80) {
               stbi__gif_parse_colortable(s,g->lpal, 2 << (g->lflags & 7), g->eflags & 0x01 ? g->transparent : -1);
               g->color_table = (stbi_uc *) g->lpal;
            } else if (g->flags & 0x80) {
               if (g->transparent >= 0 && (g->eflags & 0x01)) {
                  prev_trans = g->pal[g->transparent][3];
                  g->pal[g->transparent][3] = 0;
               }
               g->color_table = (stbi_uc *) g->pal;
            } else
               return stbi__errpuc("missing color table", "Corrupt GIF");

            o = stbi__process_gif_raster(s, g);
            if (o == NULL) return NULL;

            if (prev_trans != -1)
               g->pal[g->transparent][3] = (stbi_uc) prev_trans;

            return o;
         }

         case 0x21: // Comment Extension.
         {
            int len;
            if (stbi__get8(s) == 0xF9) { // Graphic Control Extension.
               len = stbi__get8(s);
               if (len == 4) {
                  g->eflags = stbi__get8(s);
                  g->delay = stbi__get16le(s);
                  g->transparent = stbi__get8(s);
               } else {
                  stbi__skip(s, len);
                  break;
               }
            }
            while ((len = stbi__get8(s)) != 0)
               stbi__skip(s, len);
            break;
         }

         case 0x3B: // gif stream termination code
            return (stbi_uc *) s; // using '1' causes warning on some compilers

         default:
            return stbi__errpuc("unknown code", "Corrupt GIF");
      }
   }

   STBI_NOTUSED(req_comp);
}

static void *stbi__gif_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri)
{
   stbi_uc *u = 0;
   stbi__gif* g = (stbi__gif*) stbi__malloc(sizeof(stbi__gif));
   memset(g, 0, sizeof(*g));
   STBI_NOTUSED(ri);

   u = stbi__gif_load_next(s, g, comp, req_comp);
   if (u == (stbi_uc *) s) u = 0;  // end of animated gif marker
   if (u) {
      *x = g->w;
      *y = g->h;
      if (req_comp && req_comp != 4)
         u = stbi__convert_format(u, 4, req_comp, g->w, g->h);
   }
   else if (g->out)
      STBI_FREE(g->out);
   STBI_FREE(g);
   return u;
}

static int stbi__gif_info(stbi__context *s, int *x, int *y, int *comp)
{
   return stbi__gif_info_raw(s,x,y,comp);
}
#endif

// *************************************************************************************************
// Radiance RGBE HDR loader
// originally by Nicolas Schulz
#ifndef STBI_NO_HDR
static int stbi__hdr_test_core(stbi__context *s, const char *signature)
{
   int i;
   for (i=0; signature[i]; ++i)
      if (stbi__get8(s) != signature[i])
          return 0;
   stbi__rewind(s);
   return 1;
}

static int stbi__hdr_test(stbi__context* s)
{
   int r = stbi__hdr_test_core(s, "#?RADIANCE\n");
   stbi__rewind(s);
   if(!r) {
       r = stbi__hdr_test_core(s, "#?RGBE\n");
       stbi__rewind(s);
   }
   return r;
}

#define STBI__HDR_BUFLEN  1024
static char *stbi__hdr_gettoken(stbi__context *z, char *buffer)
{
   int len=0;
   char c = '\0';

   c = (char) stbi__get8(z);

   while (!stbi__at_eof(z) && c != '\n') {
      buffer[len++] = c;
      if (len == STBI__HDR_BUFLEN-1) {
         // flush to end of line
         while (!stbi__at_eof(z) && stbi__get8(z) != '\n')
            ;
         break;
      }
      c = (char) stbi__get8(z);
   }

   buffer[len] = 0;
   return buffer;
}

static void stbi__hdr_convert(float *output, stbi_uc *input, int req_comp)
{
   if ( input[3] != 0 ) {
      float f1;
      // Exponent
      f1 = (float) ldexp(1.0f, input[3] - (int)(128 + 8));
      if (req_comp <= 2)
         output[0] = (input[0] + input[1] + input[2]) * f1 / 3;
      else {
         output[0] = input[0] * f1;
         output[1] = input[1] * f1;
         output[2] = input[2] * f1;
      }
      if (req_comp == 2) output[1] = 1;
      if (req_comp == 4) output[3] = 1;
   } else {
      switch (req_comp) {
         case 4: output[3] = 1; /* fallthrough */
         case 3: output[0] = output[1] = output[2] = 0;
                 break;
         case 2: output[1] = 1; /* fallthrough */
         case 1: output[0] = 0;
                 break;
      }
   }
}

static float *stbi__hdr_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri)
{
   char buffer[STBI__HDR_BUFLEN];
   char *token;
   int valid = 0;
   int width, height;
   stbi_uc *scanline;
   float *hdr_data;
   int len;
   unsigned char count, value;
   int i, j, k, c1,c2, z;
   const char *headerToken;
   STBI_NOTUSED(ri);

   // Check identifier
   headerToken = stbi__hdr_gettoken(s,buffer);
   if (strcmp(headerToken, "#?RADIANCE") != 0 && strcmp(headerToken, "#?RGBE") != 0)
      return stbi__errpf("not HDR", "Corrupt HDR image");

   // Parse header
   for(;;) {
      token = stbi__hdr_gettoken(s,buffer);
      if (token[0] == 0) break;
      if (strcmp(token, "FORMAT=32-bit_rle_rgbe") == 0) valid = 1;
   }

   if (!valid)    return stbi__errpf("unsupported format", "Unsupported HDR format");

   // Parse width and height
   // can't use sscanf() if we're not using stdio!
   token = stbi__hdr_gettoken(s,buffer);
   if (strncmp(token, "-Y ", 3))  return stbi__errpf("unsupported data layout", "Unsupported HDR format");
   token += 3;
   height = (int) strtol(token, &token, 10);
   while (*token == ' ') ++token;
   if (strncmp(token, "+X ", 3))  return stbi__errpf("unsupported data layout", "Unsupported HDR format");
   token += 3;
   width = (int) strtol(token, NULL, 10);

   *x = width;
   *y = height;

   if (comp) *comp = 3;
   if (req_comp == 0) req_comp = 3;

   if (!stbi__mad4sizes_valid(width, height, req_comp, sizeof(float), 0))
      return stbi__errpf("too large", "HDR image is too large");

   // Read data
   hdr_data = (float *) stbi__malloc_mad4(width, height, req_comp, sizeof(float), 0);
   if (!hdr_data)
      return stbi__errpf("outofmem", "Out of memory");

   // Load image data
   // image data is stored as some number of sca
   if ( width < 8 || width >= 32768) {
      // Read flat data
      for (j=0; j < height; ++j) {
         for (i=0; i < width; ++i) {
            stbi_uc rgbe[4];
           main_decode_loop:
            stbi__getn(s, rgbe, 4);
            stbi__hdr_convert(hdr_data + j * width * req_comp + i * req_comp, rgbe, req_comp);
         }
      }
   } else {
      // Read RLE-encoded data
      scanline = NULL;

      for (j = 0; j < height; ++j) {
         c1 = stbi__get8(s);
         c2 = stbi__get8(s);
         len = stbi__get8(s);
         if (c1 != 2 || c2 != 2 || (len & 0x80)) {
            // not run-length encoded, so we have to actually use THIS data as a decoded
            // pixel (note this can't be a valid pixel--one of RGB must be >= 128)
            stbi_uc rgbe[4];
            rgbe[0] = (stbi_uc) c1;
            rgbe[1] = (stbi_uc) c2;
            rgbe[2] = (stbi_uc) len;
            rgbe[3] = (stbi_uc) stbi__get8(s);
            stbi__hdr_convert(hdr_data, rgbe, req_comp);
            i = 1;
            j = 0;
            STBI_FREE(scanline);
            goto main_decode_loop; // yes, this makes no sense
         }
         len <<= 8;
         len |= stbi__get8(s);
         if (len != width) { STBI_FREE(hdr_data); STBI_FREE(scanline); return stbi__errpf("invalid decoded scanline length", "corrupt HDR"); }
         if (scanline == NULL) {
            scanline = (stbi_uc *) stbi__malloc_mad2(width, 4, 0);
            if (!scanline) {
               STBI_FREE(hdr_data);
               return stbi__errpf("outofmem", "Out of memory");
            }
         }

         for (k = 0; k < 4; ++k) {
            int nleft;
            i = 0;
            while ((nleft = width - i) > 0) {
               count = stbi__get8(s);
               if (count > 128) {
                  // Run
                  value = stbi__get8(s);
                  count -= 128;
                  if (count > nleft) { STBI_FREE(hdr_data); STBI_FREE(scanline); return stbi__errpf("corrupt", "bad RLE data in HDR"); }
                  for (z = 0; z < count; ++z)
                     scanline[i++ * 4 + k] = value;
               } else {
                  // Dump
                  if (count > nleft) { STBI_FREE(hdr_data); STBI_FREE(scanline); return stbi__errpf("corrupt", "bad RLE data in HDR"); }
                  for (z = 0; z < count; ++z)
                     scanline[i++ * 4 + k] = stbi__get8(s);
               }
            }
         }
         for (i=0; i < width; ++i)
            stbi__hdr_convert(hdr_data+(j*width + i)*req_comp, scanline + i*4, req_comp);
      }
      if (scanline)
         STBI_FREE(scanline);
   }

   return hdr_data;
}

static int stbi__hdr_info(stbi__context *s, int *x, int *y, int *comp)
{
   char buffer[STBI__HDR_BUFLEN];
   char *token;
   int valid = 0;

   if (stbi__hdr_test(s) == 0) {
       stbi__rewind( s );
       return 0;
   }

   for(;;) {
      token = stbi__hdr_gettoken(s,buffer);
      if (token[0] == 0) break;
      if (strcmp(token, "FORMAT=32-bit_rle_rgbe") == 0) valid = 1;
   }

   if (!valid) {
       stbi__rewind( s );
       return 0;
   }
   token = stbi__hdr_gettoken(s,buffer);
   if (strncmp(token, "-Y ", 3)) {
       stbi__rewind( s );
       return 0;
   }
   token += 3;
   *y = (int) strtol(token, &token, 10);
   while (*token == ' ') ++token;
   if (strncmp(token, "+X ", 3)) {
       stbi__rewind( s );
       return 0;
   }
   token += 3;
   *x = (int) strtol(token, NULL, 10);
   *comp = 3;
   return 1;
}
#endif // STBI_NO_HDR

#ifndef STBI_NO_BMP
static int stbi__bmp_info(stbi__context *s, int *x, int *y, int *comp)
{
   void *p;
   stbi__bmp_data info;

   info.all_a = 255;   
   p = stbi__bmp_parse_header(s, &info);
   stbi__rewind( s );
   if (p == NULL)
      return 0;
   *x = s->img_x;
   *y = s->img_y;
   *comp = info.ma ? 4 : 3;
   return 1;
}
#endif

#ifndef STBI_NO_PSD
static int stbi__psd_info(stbi__context *s, int *x, int *y, int *comp)
{
   int channelCount;
   if (stbi__get32be(s) != 0x38425053) {
       stbi__rewind( s );
       return 0;
   }
   if (stbi__get16be(s) != 1) {
       stbi__rewind( s );
       return 0;
   }
   stbi__skip(s, 6);
   channelCount = stbi__get16be(s);
   if (channelCount < 0 || channelCount > 16) {
       stbi__rewind( s );
       return 0;
   }
   *y = stbi__get32be(s);
   *x = stbi__get32be(s);
   if (stbi__get16be(s) != 8) {
       stbi__rewind( s );
       return 0;
   }
   if (stbi__get16be(s) != 3) {
       stbi__rewind( s );
       return 0;
   }
   *comp = 4;
   return 1;
}
#endif

#ifndef STBI_NO_PIC
static int stbi__pic_info(stbi__context *s, int *x, int *y, int *comp)
{
   int act_comp=0,num_packets=0,chained;
   stbi__pic_packet packets[10];

   if (!stbi__pic_is4(s,"\x53\x80\xF6\x34")) {
      stbi__rewind(s);
      return 0;
   }

   stbi__skip(s, 88);

   *x = stbi__get16be(s);
   *y = stbi__get16be(s);
   if (stbi__at_eof(s)) {
      stbi__rewind( s);
      return 0;
   }
   if ( (*x) != 0 && (1 << 28) / (*x) < (*y)) {
      stbi__rewind( s );
      return 0;
   }

   stbi__skip(s, 8);

   do {
      stbi__pic_packet *packet;

      if (num_packets==sizeof(packets)/sizeof(packets[0]))
         return 0;

      packet = &packets[num_packets++];
      chained = stbi__get8(s);
      packet->size    = stbi__get8(s);
      packet->type    = stbi__get8(s);
      packet->channel = stbi__get8(s);
      act_comp |= packet->channel;

      if (stbi__at_eof(s)) {
          stbi__rewind( s );
          return 0;
      }
      if (packet->size != 8) {
          stbi__rewind( s );
          return 0;
      }
   } while (chained);

   *comp = (act_comp & 0x10 ? 4 : 3);

   return 1;
}
#endif

// *************************************************************************************************
// Portable Gray Map and Portable Pixel Map loader
// by Ken Miller
//
// PGM: http://netpbm.sourceforge.net/doc/pgm.html
// PPM: http://netpbm.sourceforge.net/doc/ppm.html
//
// Known limitations:
//    Does not support comments in the header section
//    Does not support ASCII image data (formats P2 and P3)
//    Does not support 16-bit-per-channel

#ifndef STBI_NO_PNM

static int      stbi__pnm_test(stbi__context *s)
{
   char p, t;
   p = (char) stbi__get8(s);
   t = (char) stbi__get8(s);
   if (p != 'P' || (t != '5' && t != '6')) {
       stbi__rewind( s );
       return 0;
   }
   return 1;
}

static void *stbi__pnm_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri)
{
   stbi_uc *out;
   STBI_NOTUSED(ri);

   if (!stbi__pnm_info(s, (int *)&s->img_x, (int *)&s->img_y, (int *)&s->img_n))
      return 0;

   *x = s->img_x;
   *y = s->img_y;
   *comp = s->img_n;

   if (!stbi__mad3sizes_valid(s->img_n, s->img_x, s->img_y, 0))
      return stbi__errpuc("too large", "PNM too large");

   out = (stbi_uc *) stbi__malloc_mad3(s->img_n, s->img_x, s->img_y, 0);
   if (!out) return stbi__errpuc("outofmem", "Out of memory");
   stbi__getn(s, out, s->img_n * s->img_x * s->img_y);

   if (req_comp && req_comp != s->img_n) {
      out = stbi__convert_format(out, s->img_n, req_comp, s->img_x, s->img_y);
      if (out == NULL) return out; // stbi__convert_format frees input on failure
   }
   return out;
}

static int      stbi__pnm_isspace(char c)
{
   return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

static void     stbi__pnm_skip_whitespace(stbi__context *s, char *c)
{
   for (;;) {
      while (!stbi__at_eof(s) && stbi__pnm_isspace(*c))
         *c = (char) stbi__get8(s);

      if (stbi__at_eof(s) || *c != '#')
         break;

      while (!stbi__at_eof(s) && *c != '\n' && *c != '\r' )
         *c = (char) stbi__get8(s);
   }
}

static int      stbi__pnm_isdigit(char c)
{
   return c >= '0' && c <= '9';
}

static int      stbi__pnm_getinteger(stbi__context *s, char *c)
{
   int value = 0;

   while (!stbi__at_eof(s) && stbi__pnm_isdigit(*c)) {
      value = value*10 + (*c - '0');
      *c = (char) stbi__get8(s);
   }

   return value;
}

static int      stbi__pnm_info(stbi__context *s, int *x, int *y, int *comp)
{
   int maxv;
   char c, p, t;

   stbi__rewind( s );

   // Get identifier
   p = (char) stbi__get8(s);
   t = (char) stbi__get8(s);
   if (p != 'P' || (t != '5' && t != '6')) {
       stbi__rewind( s );
       return 0;
   }

   *comp = (t == '6') ? 3 : 1;  // '5' is 1-component .pgm; '6' is 3-component .ppm

   c = (char) stbi__get8(s);
   stbi__pnm_skip_whitespace(s, &c);

   *x = stbi__pnm_getinteger(s, &c); // read width
   stbi__pnm_skip_whitespace(s, &c);

   *y = stbi__pnm_getinteger(s, &c); // read height
   stbi__pnm_skip_whitespace(s, &c);

   maxv = stbi__pnm_getinteger(s, &c);  // read max value

   if (maxv > 255)
      return stbi__err("max value > 255", "PPM image not 8-bit");
   else
      return 1;
}
#endif

static int stbi__info_main(stbi__context *s, int *x, int *y, int *comp)
{
   #ifndef STBI_NO_JPEG
   if (stbi__jpeg_info(s, x, y, comp)) return 1;
   #endif

   #ifndef STBI_NO_PNG
   if (stbi__png_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_GIF
   if (stbi__gif_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_BMP
   if (stbi__bmp_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_PSD
   if (stbi__psd_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_PIC
   if (stbi__pic_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_PNM
   if (stbi__pnm_info(s, x, y, comp))  return 1;
   #endif

   #ifndef STBI_NO_HDR
   if (stbi__hdr_info(s, x, y, comp))  return 1;
   #endif

   // test tga last because it's a crappy test!
   #ifndef STBI_NO_TGA
   if (stbi__tga_info(s, x, y, comp))
       return 1;
   #endif
   return stbi__err("unknown image type", "Image not of any known type, or corrupt");
}

#ifndef STBI_NO_STDIO
STBIDEF int stbi_info(char const *filename, int *x, int *y, int *comp)
{
    FILE *f = stbi__fopen(filename, "rb");
    int result;
    if (!f) return stbi__err("can't fopen", "Unable to open file");
    result = stbi_info_from_file(f, x, y, comp);
    fclose(f);
    return result;
}

STBIDEF int stbi_info_from_file(FILE *f, int *x, int *y, int *comp)
{
   int r;
   stbi__context s;
   long pos = ftell(f);
   stbi__start_file(&s, f);
   r = stbi__info_main(&s,x,y,comp);
   fseek(f,pos,SEEK_SET);
   return r;
}
#endif // !STBI_NO_STDIO

STBIDEF int stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp)
{
   stbi__context s;
   stbi__start_mem(&s,buffer,len);
   return stbi__info_main(&s,x,y,comp);
}

STBIDEF int stbi_info_from_callbacks(stbi_io_callbacks const *c, void *user, int *x, int *y, int *comp)
{
   stbi__context s;
   stbi__start_callbacks(&s, (stbi_io_callbacks *) c, user);
   return stbi__info_main(&s,x,y,comp);
}

#endif // STB_IMAGE_IMPLEMENTATION

/*
   revision history:
      2.13  (2016-11-29) add 16-bit API, only supported for PNG right now
      2.12  (2016-04-02) fix typo in 2.11 PSD fix that caused crashes
      2.11  (2016-04-02) allocate large structures on the stack
                         remove white matting for transparent PSD
                         fix reported channel count for PNG & BMP
                         re-enable SSE2 in non-gcc 64-bit
                         support RGB-formatted JPEG
                         read 16-bit PNGs (only as 8-bit)
      2.10  (2016-01-22) avoid warning introduced in 2.09 by STBI_REALLOC_SIZED
      2.09  (2016-01-16) allow comments in PNM files
                         16-bit-per-pixel TGA (not bit-per-component)
                         info() for TGA could break due to .hdr handling
                         info() for BMP to shares code instead of sloppy parse
                         can use STBI_REALLOC_SIZED if allocator doesn't support realloc
                         code cleanup
      2.08  (2015-09-13) fix to 2.07 cleanup, reading RGB PSD as RGBA
      2.07  (2015-09-13) fix compiler warnings
                         partial animated GIF support
                         limited 16-bpc PSD support
                         #ifdef unused functions
                         bug with < 92 byte PIC,PNM,HDR,TGA
      2.06  (2015-04-19) fix bug where PSD returns wrong '*comp' value
      2.05  (2015-04-19) fix bug in progressive JPEG handling, fix warning
      2.04  (2015-04-15) try to re-enable SIMD on MinGW 64-bit
      2.03  (2015-04-12) extra corruption checking (mmozeiko)
                         stbi_set_flip_vertically_on_load (nguillemot)
                         fix NEON support; fix mingw support
      2.02  (2015-01-19) fix incorrect assert, fix warning
      2.01  (2015-01-17) fix various warnings; suppress SIMD on gcc 32-bit without -msse2
      2.00b (2014-12-25) fix STBI_MALLOC in progressive JPEG
      2.00  (2014-12-25) optimize JPG, including x86 SSE2 & NEON SIMD (ryg)
                         progressive JPEG (stb)
                         PGM/PPM support (Ken Miller)
                         STBI_MALLOC,STBI_REALLOC,STBI_FREE
                         GIF bugfix -- seemingly never worked
                         STBI_NO_*, STBI_ONLY_*
      1.48  (2014-12-14) fix incorrectly-named assert()
      1.47  (2014-12-14) 1/2/4-bit PNG support, both direct and paletted (Omar Cornut & stb)
                         optimize PNG (ryg)
                         fix bug in interlaced PNG with user-specified channel count (stb)
      1.46  (2014-08-26)
              fix broken tRNS chunk (colorkey-style transparency) in non-paletted PNG
      1.45  (2014-08-16)
              fix MSVC-ARM internal compiler error by wrapping malloc
      1.44  (2014-08-07)
              various warning fixes from Ronny Chevalier
      1.43  (2014-07-15)
              fix MSVC-only compiler problem in code changed in 1.42
      1.42  (2014-07-09)
              don't define _CRT_SECURE_NO_WARNINGS (affects user code)
              fixes to stbi__cleanup_jpeg path
              added STBI_ASSERT to avoid requiring assert.h
      1.41  (2014-06-25)
              fix search&replace from 1.36 that messed up comments/error messages
      1.40  (2014-06-22)
              fix gcc struct-initialization warning
      1.39  (2014-06-15)
              fix to TGA optimization when req_comp != number of components in TGA;
              fix to GIF loading because BMP wasn't rewinding (whoops, no GIFs in my test suite)
              add support for BMP version 5 (more ignored fields)
      1.38  (2014-06-06)
              suppress MSVC warnings on integer casts truncating values
              fix accidental rename of 'skip' field of I/O
      1.37  (2014-06-04)
              remove duplicate typedef
      1.36  (2014-06-03)
              convert to header file single-file library
              if de-iphone isn't set, load iphone images color-swapped instead of returning NULL
      1.35  (2014-05-27)
              various warnings
              fix broken STBI_SIMD path
              fix bug where stbi_load_from_file no longer left file pointer in correct place
              fix broken non-easy path for 32-bit BMP (possibly never used)
              TGA optimization by Arseny Kapoulkine
      1.34  (unknown)
              use STBI_NOTUSED in stbi__resample_row_generic(), fix one more leak in tga failure case
      1.33  (2011-07-14)
              make stbi_is_hdr work in STBI_NO_HDR (as specified), minor compiler-friendly improvements
      1.32  (2011-07-13)
              support for "info" function for all supported filetypes (SpartanJ)
      1.31  (2011-06-20)
              a few more leak fixes, bug in PNG handling (SpartanJ)
      1.30  (2011-06-11)
              added ability to load files via callbacks to accomidate custom input streams (Ben Wenger)
              removed deprecated format-specific test/load functions
              removed support for installable file formats (stbi_loader) -- would have been broken for IO callbacks anyway
              error cases in bmp and tga give messages and don't leak (Raymond Barbiero, grisha)
              fix inefficiency in decoding 32-bit BMP (David Woo)
      1.29  (2010-08-16)
              various warning fixes from Aurelien Pocheville
      1.28  (2010-08-01)
              fix bug in GIF palette transparency (SpartanJ)
      1.27  (2010-08-01)
              cast-to-stbi_uc to fix warnings
      1.26  (2010-07-24)
              fix bug in file buffering for PNG reported by SpartanJ
      1.25  (2010-07-17)
              refix trans_data warning (Won Chun)
      1.24  (2010-07-12)
              perf improvements reading from files on platforms with lock-heavy fgetc()
              minor perf improvements for jpeg
              deprecated type-specific functions so we'll get feedback if they're needed
              attempt to fix trans_data warning (Won Chun)
      1.23    fixed bug in iPhone support
      1.22  (2010-07-10)
              removed image *writing* support
              stbi_info support from Jetro Lauha
              GIF support from Jean-Marc Lienher
              iPhone PNG-extensions from James Brown
              warning-fixes from Nicolas Schulz and Janez Zemva (i.stbi__err. Janez (U+017D)emva)
      1.21    fix use of 'stbi_uc' in header (reported by jon blow)
      1.20    added support for Softimage PIC, by Tom Seddon
      1.19    bug in interlaced PNG corruption check (found by ryg)
      1.18  (2008-08-02)
              fix a threading bug (local mutable static)
      1.17    support interlaced PNG
      1.16    major bugfix - stbi__convert_format converted one too many pixels
      1.15    initialize some fields for thread safety
      1.14    fix threadsafe conversion bug
              header-file-only version (#define STBI_HEADER_FILE_ONLY before including)
      1.13    threadsafe
      1.12    const qualifiers in the API
      1.11    Support installable IDCT, colorspace conversion routines
      1.10    Fixes for 64-bit (don't use "unsigned long")
              optimized upsampling by Fabian "ryg" Giesen
      1.09    Fix format-conversion for PSD code (bad global variables!)
      1.08    Thatcher Ulrich's PSD code integrated by Nicolas Schulz
      1.07    attempt to fix C++ warning/errors again
      1.06    attempt to fix C++ warning/errors again
      1.05    fix TGA loading to return correct *comp and use good luminance calc
      1.04    default float alpha is 1, not 255; use 'void *' for stbi_image_free
      1.03    bugfixes to STBI_NO_STDIO, STBI_NO_HDR
      1.02    support for (subset of) HDR files, float interface for preferred access to them
      1.01    fix bug: possible bug in handling right-side up bmps... not sure
              fix bug: the stbi__bmp_load() and stbi__tga_load() functions didn't work at all
      1.00    interface to zlib that skips zlib header
      0.99    correct handling of alpha in palette
      0.98    TGA loader by lonesock; dynamically add loaders (untested)
      0.97    jpeg errors on too large a file; also catch another malloc failure
      0.96    fix detection of invalid v value - particleman@mollyrocket forum
      0.95    during header scan, seek to markers in case of padding
      0.94    STBI_NO_STDIO to disable stdio usage; rename all #defines the same
      0.93    handle jpegtran output; verbose errors
      0.92    read 4,8,16,24,32-bit BMP files of several formats
      0.91    output 24-bit Windows 3.0 BMP files
      0.90    fix a few more warnings; bump version number to approach 1.0
      0.61    bugfixes due to Marc LeBlanc, Christopher Lloyd
      0.60    fix compiling as c++
      0.59    fix warnings: merge Dave Moore's -Wall fixes
      0.58    fix bug: zlib uncompressed mode len/nlen was wrong endian
      0.57    fix bug: jpg last huffman symbol before marker was >9 bits but less than 16 available
      0.56    fix bug: zlib uncompressed mode len vs. nlen
      0.55    fix bug: restart_interval not initialized to 0
      0.54    allow NULL for 'int *comp'
      0.53    fix bug in png 3->4; speedup png decoding
      0.52    png handles req_comp=3,4 directly; minor cleanup; jpeg comments
      0.51    obey req_comp requests, 1-component jpegs return as 1-component,
              on 'test' only check type, not whether we support this variant
      0.50  (2006-11-19)
              first released version
*/
/* stb_image_write - v1.03 - public domain - http://nothings.org/stb/stb_image_write.h
   writes out PNG/BMP/TGA images to C stdio - Sean Barrett 2010-2015
                                     no warranty implied; use at your own risk

   Before #including,

       #define STB_IMAGE_WRITE_IMPLEMENTATION

   in the file that you want to have the implementation.

   Will probably not work correctly with strict-aliasing optimizations.

ABOUT:

   This header file is a library for writing images to C stdio. It could be
   adapted to write to memory or a general streaming interface; let me know.

   The PNG output is not optimal; it is 20-50% larger than the file
   written by a decent optimizing implementation. This library is designed
   for source code compactness and simplicity, not optimal image file size
   or run-time performance.

BUILDING:

   You can #define STBIW_ASSERT(x) before the #include to avoid using assert.h.
   You can #define STBIW_MALLOC(), STBIW_REALLOC(), and STBIW_FREE() to replace
   malloc,realloc,free.
   You can define STBIW_MEMMOVE() to replace memmove()

USAGE:

   There are four functions, one for each image file format:

     int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
     int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);

   There are also four equivalent functions that use an arbitrary write function. You are
   expected to open/close your file-equivalent before and after calling these:

     int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data, int stride_in_bytes);
     int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
     int stbi_write_tga_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
     int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const float *data);

   where the callback is:
      void stbi_write_func(void *context, void *data, int size);

   You can define STBI_WRITE_NO_STDIO to disable the file variant of these
   functions, so the library will not use stdio.h at all. However, this will
   also disable HDR writing, because it requires stdio for formatted output.

   Each function returns 0 on failure and non-0 on success.

   The functions create an image file defined by the parameters. The image
   is a rectangle of pixels stored from left-to-right, top-to-bottom.
   Each pixel contains 'comp' channels of data stored interleaved with 8-bits
   per channel, in the following order: 1=Y, 2=YA, 3=RGB, 4=RGBA. (Y is
   monochrome color.) The rectangle is 'w' pixels wide and 'h' pixels tall.
   The *data pointer points to the first byte of the top-left-most pixel.
   For PNG, "stride_in_bytes" is the distance in bytes from the first byte of
   a row of pixels to the first byte of the next row of pixels.

   PNG creates output files with the same number of components as the input.
   The BMP format expands Y to RGB in the file format and does not
   output alpha.

   PNG supports writing rectangles of data even when the bytes storing rows of
   data are not consecutive in memory (e.g. sub-rectangles of a larger image),
   by supplying the stride between the beginning of adjacent rows. The other
   formats do not. (Thus you cannot write a native-format BMP through the BMP
   writer, both because it is in BGR order and because it may have padding
   at the end of the line.)

   HDR expects linear float data. Since the format is always 32-bit rgb(e)
   data, alpha (if provided) is discarded, and for monochrome data it is
   replicated across all three channels.

   TGA supports RLE or non-RLE compressed data. To use non-RLE-compressed
   data, set the global variable 'stbi_write_tga_with_rle' to 0.

CREDITS:

   PNG/BMP/TGA
      Sean Barrett
   HDR
      Baldur Karlsson
   TGA monochrome:
      Jean-Sebastien Guay
   misc enhancements:
      Tim Kelsey
   TGA RLE
      Alan Hickman
   initial file IO callback implementation
      Emmanuel Julien
   bugfixes:
      github:Chribba
      Guillaume Chereau
      github:jry2
      github:romigrou
      Sergio Gonzalez
      Jonas Karlsson
      Filip Wasil
      Thatcher Ulrich
      github:poppolopoppo
      
LICENSE

This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.

*/

#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef STB_IMAGE_WRITE_STATIC
#define STBIWDEF static
#else
#define STBIWDEF extern
extern int stbi_write_tga_with_rle;
#endif

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes);
STBIWDEF int stbi_write_bmp(char const *filename, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_tga(char const *filename, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);
#endif

typedef void stbi_write_func(void *context, void *data, int size);

STBIWDEF int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data, int stride_in_bytes);
STBIWDEF int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_tga_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const float *data);

#ifdef __cplusplus
}
#endif

#endif//INCLUDE_STB_IMAGE_WRITE_H

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

#ifdef _WIN32
   #ifndef _CRT_SECURE_NO_WARNINGS
   #define _CRT_SECURE_NO_WARNINGS
   #endif
   #ifndef _CRT_NONSTDC_NO_DEPRECATE
   #define _CRT_NONSTDC_NO_DEPRECATE
   #endif
#endif

#ifndef STBI_WRITE_NO_STDIO
#include <stdio.h>
#endif // STBI_WRITE_NO_STDIO

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(STBIW_MALLOC) && defined(STBIW_FREE) && (defined(STBIW_REALLOC) || defined(STBIW_REALLOC_SIZED))
// ok
#elif !defined(STBIW_MALLOC) && !defined(STBIW_FREE) && !defined(STBIW_REALLOC) && !defined(STBIW_REALLOC_SIZED)
// ok
#else
#error "Must define all or none of STBIW_MALLOC, STBIW_FREE, and STBIW_REALLOC (or STBIW_REALLOC_SIZED)."
#endif

#ifndef STBIW_MALLOC
#define STBIW_MALLOC(sz)        malloc(sz)
#define STBIW_REALLOC(p,newsz)  realloc(p,newsz)
#define STBIW_FREE(p)           free(p)
#endif

#ifndef STBIW_REALLOC_SIZED
#define STBIW_REALLOC_SIZED(p,oldsz,newsz) STBIW_REALLOC(p,newsz)
#endif


#ifndef STBIW_MEMMOVE
#define STBIW_MEMMOVE(a,b,sz) memmove(a,b,sz)
#endif


#ifndef STBIW_ASSERT
#include <assert.h>
#define STBIW_ASSERT(x) assert(x)
#endif

#define STBIW_UCHAR(x) (unsigned char) ((x) & 0xff)

typedef struct
{
   stbi_write_func *func;
   void *context;
} stbi__write_context;

// initialize a callback-based context
static void stbi__start_write_callbacks(stbi__write_context *s, stbi_write_func *c, void *context)
{
   s->func    = c;
   s->context = context;
}

#ifndef STBI_WRITE_NO_STDIO

static void stbi__stdio_write(void *context, void *data, int size)
{
   fwrite(data,1,size,(FILE*) context);
}

static int stbi__start_write_file(stbi__write_context *s, const char *filename)
{
   FILE *f = fopen(filename, "wb");
   stbi__start_write_callbacks(s, stbi__stdio_write, (void *) f);
   return f != NULL;
}

static void stbi__end_write_file(stbi__write_context *s)
{
   fclose((FILE *)s->context);
}

#endif // !STBI_WRITE_NO_STDIO

typedef unsigned int stbiw_uint32;
typedef int stb_image_write_test[sizeof(stbiw_uint32)==4 ? 1 : -1];

#ifdef STB_IMAGE_WRITE_STATIC
static int stbi_write_tga_with_rle = 1;
#else
int stbi_write_tga_with_rle = 1;
#endif

static void stbiw__writefv(stbi__write_context *s, const char *fmt, va_list v)
{
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { unsigned char x = STBIW_UCHAR(va_arg(v, int));
                     s->func(s->context,&x,1);
                     break; }
         case '2': { int x = va_arg(v,int);
                     unsigned char b[2];
                     b[0] = STBIW_UCHAR(x);
                     b[1] = STBIW_UCHAR(x>>8);
                     s->func(s->context,b,2);
                     break; }
         case '4': { stbiw_uint32 x = va_arg(v,int);
                     unsigned char b[4];
                     b[0]=STBIW_UCHAR(x);
                     b[1]=STBIW_UCHAR(x>>8);
                     b[2]=STBIW_UCHAR(x>>16);
                     b[3]=STBIW_UCHAR(x>>24);
                     s->func(s->context,b,4);
                     break; }
         default:
            STBIW_ASSERT(0);
            return;
      }
   }
}

static void stbiw__writef(stbi__write_context *s, const char *fmt, ...)
{
   va_list v;
   va_start(v, fmt);
   stbiw__writefv(s, fmt, v);
   va_end(v);
}

static void stbiw__write3(stbi__write_context *s, unsigned char a, unsigned char b, unsigned char c)
{
   unsigned char arr[3];
   arr[0] = a, arr[1] = b, arr[2] = c;
   s->func(s->context, arr, 3);
}

static void stbiw__write_pixel(stbi__write_context *s, int rgb_dir, int comp, int write_alpha, int expand_mono, unsigned char *d)
{
   unsigned char bg[3] = { 255, 0, 255}, px[3];
   int k;

   if (write_alpha < 0)
      s->func(s->context, &d[comp - 1], 1);

   switch (comp) {
      case 1:
         s->func(s->context,d,1);
         break;
      case 2:
         if (expand_mono)
            stbiw__write3(s, d[0], d[0], d[0]); // monochrome bmp
         else
            s->func(s->context, d, 1);  // monochrome TGA
         break;
      case 4:
         if (!write_alpha) {
            // composite against pink background
            for (k = 0; k < 3; ++k)
               px[k] = bg[k] + ((d[k] - bg[k]) * d[3]) / 255;
            stbiw__write3(s, px[1 - rgb_dir], px[1], px[1 + rgb_dir]);
            break;
         }
         /* FALLTHROUGH */
      case 3:
         stbiw__write3(s, d[1 - rgb_dir], d[1], d[1 + rgb_dir]);
         break;
   }
   if (write_alpha > 0)
      s->func(s->context, &d[comp - 1], 1);
}

static void stbiw__write_pixels(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad, int expand_mono)
{
   stbiw_uint32 zero = 0;
   int i,j, j_end;

   if (y <= 0)
      return;

   if (vdir < 0)
      j_end = -1, j = y-1;
   else
      j_end =  y, j = 0;

   for (; j != j_end; j += vdir) {
      for (i=0; i < x; ++i) {
         unsigned char *d = (unsigned char *) data + (j*x+i)*comp;
         stbiw__write_pixel(s, rgb_dir, comp, write_alpha, expand_mono, d);
      }
      s->func(s->context, &zero, scanline_pad);
   }
}

static int stbiw__outfile(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, int expand_mono, void *data, int alpha, int pad, const char *fmt, ...)
{
   if (y < 0 || x < 0) {
      return 0;
   } else {
      va_list v;
      va_start(v, fmt);
      stbiw__writefv(s, fmt, v);
      va_end(v);
      stbiw__write_pixels(s,rgb_dir,vdir,x,y,comp,data,alpha,pad, expand_mono);
      return 1;
   }
}

static int stbi_write_bmp_core(stbi__write_context *s, int x, int y, int comp, const void *data)
{
   int pad = (-x*3) & 3;
   return stbiw__outfile(s,-1,-1,x,y,comp,1,(void *) data,0,pad,
           "11 4 22 4" "4 44 22 444444",
           'B', 'M', 14+40+(x*3+pad)*y, 0,0, 14+40,  // file header
            40, x,y, 1,24, 0,0,0,0,0,0);             // bitmap header
}

STBIWDEF int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data)
{
   stbi__write_context s;
   stbi__start_write_callbacks(&s, func, context);
   return stbi_write_bmp_core(&s, x, y, comp, data);
}

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data)
{
   stbi__write_context s;
   if (stbi__start_write_file(&s,filename)) {
      int r = stbi_write_bmp_core(&s, x, y, comp, data);
      stbi__end_write_file(&s);
      return r;
   } else
      return 0;
}
#endif //!STBI_WRITE_NO_STDIO

static int stbi_write_tga_core(stbi__write_context *s, int x, int y, int comp, void *data)
{
   int has_alpha = (comp == 2 || comp == 4);
   int colorbytes = has_alpha ? comp-1 : comp;
   int format = colorbytes < 2 ? 3 : 2; // 3 color channels (RGB/RGBA) = 2, 1 color channel (Y/YA) = 3

   if (y < 0 || x < 0)
      return 0;

   if (!stbi_write_tga_with_rle) {
      return stbiw__outfile(s, -1, -1, x, y, comp, 0, (void *) data, has_alpha, 0,
         "111 221 2222 11", 0, 0, format, 0, 0, 0, 0, 0, x, y, (colorbytes + has_alpha) * 8, has_alpha * 8);
   } else {
      int i,j,k;

      stbiw__writef(s, "111 221 2222 11", 0,0,format+8, 0,0,0, 0,0,x,y, (colorbytes + has_alpha) * 8, has_alpha * 8);

      for (j = y - 1; j >= 0; --j) {
          unsigned char *row = (unsigned char *) data + j * x * comp;
         int len;

         for (i = 0; i < x; i += len) {
            unsigned char *begin = row + i * comp;
            int diff = 1;
            len = 1;

            if (i < x - 1) {
               ++len;
               diff = memcmp(begin, row + (i + 1) * comp, comp);
               if (diff) {
                  const unsigned char *prev = begin;
                  for (k = i + 2; k < x && len < 128; ++k) {
                     if (memcmp(prev, row + k * comp, comp)) {
                        prev += comp;
                        ++len;
                     } else {
                        --len;
                        break;
                     }
                  }
               } else {
                  for (k = i + 2; k < x && len < 128; ++k) {
                     if (!memcmp(begin, row + k * comp, comp)) {
                        ++len;
                     } else {
                        break;
                     }
                  }
               }
            }

            if (diff) {
               unsigned char header = STBIW_UCHAR(len - 1);
               s->func(s->context, &header, 1);
               for (k = 0; k < len; ++k) {
                  stbiw__write_pixel(s, -1, comp, has_alpha, 0, begin + k * comp);
               }
            } else {
               unsigned char header = STBIW_UCHAR(len - 129);
               s->func(s->context, &header, 1);
               stbiw__write_pixel(s, -1, comp, has_alpha, 0, begin);
            }
         }
      }
   }
   return 1;
}

int stbi_write_tga_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data)
{
   stbi__write_context s;
   stbi__start_write_callbacks(&s, func, context);
   return stbi_write_tga_core(&s, x, y, comp, (void *) data);
}

#ifndef STBI_WRITE_NO_STDIO
int stbi_write_tga(char const *filename, int x, int y, int comp, const void *data)
{
   stbi__write_context s;
   if (stbi__start_write_file(&s,filename)) {
      int r = stbi_write_tga_core(&s, x, y, comp, (void *) data);
      stbi__end_write_file(&s);
      return r;
   } else
      return 0;
}
#endif

// *************************************************************************************************
// Radiance RGBE HDR writer
// by Baldur Karlsson

#define stbiw__max(a, b)  ((a) > (b) ? (a) : (b))

void stbiw__linear_to_rgbe(unsigned char *rgbe, float *linear)
{
   int exponent;
   float maxcomp = stbiw__max(linear[0], stbiw__max(linear[1], linear[2]));

   if (maxcomp < 1e-32f) {
      rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
   } else {
      float normalize = (float) frexp(maxcomp, &exponent) * 256.0f/maxcomp;

      rgbe[0] = (unsigned char)(linear[0] * normalize);
      rgbe[1] = (unsigned char)(linear[1] * normalize);
      rgbe[2] = (unsigned char)(linear[2] * normalize);
      rgbe[3] = (unsigned char)(exponent + 128);
   }
}

void stbiw__write_run_data(stbi__write_context *s, int length, unsigned char databyte)
{
   unsigned char lengthbyte = STBIW_UCHAR(length+128);
   STBIW_ASSERT(length+128 <= 255);
   s->func(s->context, &lengthbyte, 1);
   s->func(s->context, &databyte, 1);
}

void stbiw__write_dump_data(stbi__write_context *s, int length, unsigned char *data)
{
   unsigned char lengthbyte = STBIW_UCHAR(length);
   STBIW_ASSERT(length <= 128); // inconsistent with spec but consistent with official code
   s->func(s->context, &lengthbyte, 1);
   s->func(s->context, data, length);
}

void stbiw__write_hdr_scanline(stbi__write_context *s, int width, int ncomp, unsigned char *scratch, float *scanline)
{
   unsigned char scanlineheader[4] = { 2, 2, 0, 0 };
   unsigned char rgbe[4];
   float linear[3];
   int x;

   scanlineheader[2] = (width&0xff00)>>8;
   scanlineheader[3] = (width&0x00ff);

   /* skip RLE for images too small or large */
   if (width < 8 || width >= 32768) {
      for (x=0; x < width; x++) {
         switch (ncomp) {
            case 4: /* fallthrough */
            case 3: linear[2] = scanline[x*ncomp + 2];
                    linear[1] = scanline[x*ncomp + 1];
                    linear[0] = scanline[x*ncomp + 0];
                    break;
            default:
                    linear[0] = linear[1] = linear[2] = scanline[x*ncomp + 0];
                    break;
         }
         stbiw__linear_to_rgbe(rgbe, linear);
         s->func(s->context, rgbe, 4);
      }
   } else {
      int c,r;
      /* encode into scratch buffer */
      for (x=0; x < width; x++) {
         switch(ncomp) {
            case 4: /* fallthrough */
            case 3: linear[2] = scanline[x*ncomp + 2];
                    linear[1] = scanline[x*ncomp + 1];
                    linear[0] = scanline[x*ncomp + 0];
                    break;
            default:
                    linear[0] = linear[1] = linear[2] = scanline[x*ncomp + 0];
                    break;
         }
         stbiw__linear_to_rgbe(rgbe, linear);
         scratch[x + width*0] = rgbe[0];
         scratch[x + width*1] = rgbe[1];
         scratch[x + width*2] = rgbe[2];
         scratch[x + width*3] = rgbe[3];
      }

      s->func(s->context, scanlineheader, 4);

      /* RLE each component separately */
      for (c=0; c < 4; c++) {
         unsigned char *comp = &scratch[width*c];

         x = 0;
         while (x < width) {
            // find first run
            r = x;
            while (r+2 < width) {
               if (comp[r] == comp[r+1] && comp[r] == comp[r+2])
                  break;
               ++r;
            }
            if (r+2 >= width)
               r = width;
            // dump up to first run
            while (x < r) {
               int len = r-x;
               if (len > 128) len = 128;
               stbiw__write_dump_data(s, len, &comp[x]);
               x += len;
            }
            // if there's a run, output it
            if (r+2 < width) { // same test as what we break out of in search loop, so only true if we break'd
               // find next byte after run
               while (r < width && comp[r] == comp[x])
                  ++r;
               // output run up to r
               while (x < r) {
                  int len = r-x;
                  if (len > 127) len = 127;
                  stbiw__write_run_data(s, len, comp[x]);
                  x += len;
               }
            }
         }
      }
   }
}

static int stbi_write_hdr_core(stbi__write_context *s, int x, int y, int comp, float *data)
{
   if (y <= 0 || x <= 0 || data == NULL)
      return 0;
   else {
      // Each component is stored separately. Allocate scratch space for full output scanline.
      unsigned char *scratch = (unsigned char *) STBIW_MALLOC(x*4);
      int i, len;
      char buffer[128];
      char header[] = "#?RADIANCE\n# Written by stb_image_write.h\nFORMAT=32-bit_rle_rgbe\n";
      s->func(s->context, header, sizeof(header)-1);

      len = sprintf(buffer, "EXPOSURE=          1.0000000000000\n\n-Y %d +X %d\n", y, x);
      s->func(s->context, buffer, len);

      for(i=0; i < y; i++)
         stbiw__write_hdr_scanline(s, x, comp, scratch, data + comp*i*x);
      STBIW_FREE(scratch);
      return 1;
   }
}

int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const float *data)
{
   stbi__write_context s;
   stbi__start_write_callbacks(&s, func, context);
   return stbi_write_hdr_core(&s, x, y, comp, (float *) data);
}

#ifndef STBI_WRITE_NO_STDIO
int stbi_write_hdr(char const *filename, int x, int y, int comp, const float *data)
{
   stbi__write_context s;
   if (stbi__start_write_file(&s,filename)) {
      int r = stbi_write_hdr_core(&s, x, y, comp, (float *) data);
      stbi__end_write_file(&s);
      return r;
   } else
      return 0;
}
#endif // STBI_WRITE_NO_STDIO


//////////////////////////////////////////////////////////////////////////////
//
// PNG writer
//

// stretchy buffer; stbiw__sbpush() == vector<>::push_back() -- stbiw__sbcount() == vector<>::size()
#define stbiw__sbraw(a) ((int *) (a) - 2)
#define stbiw__sbm(a)   stbiw__sbraw(a)[0]
#define stbiw__sbn(a)   stbiw__sbraw(a)[1]

#define stbiw__sbneedgrow(a,n)  ((a)==0 || stbiw__sbn(a)+n >= stbiw__sbm(a))
#define stbiw__sbmaybegrow(a,n) (stbiw__sbneedgrow(a,(n)) ? stbiw__sbgrow(a,n) : 0)
#define stbiw__sbgrow(a,n)  stbiw__sbgrowf((void **) &(a), (n), sizeof(*(a)))

#define stbiw__sbpush(a, v)      (stbiw__sbmaybegrow(a,1), (a)[stbiw__sbn(a)++] = (v))
#define stbiw__sbcount(a)        ((a) ? stbiw__sbn(a) : 0)
#define stbiw__sbfree(a)         ((a) ? STBIW_FREE(stbiw__sbraw(a)),0 : 0)

static void *stbiw__sbgrowf(void **arr, int increment, int itemsize)
{
   int m = *arr ? 2*stbiw__sbm(*arr)+increment : increment+1;
   void *p = STBIW_REALLOC_SIZED(*arr ? stbiw__sbraw(*arr) : 0, *arr ? (stbiw__sbm(*arr)*itemsize + sizeof(int)*2) : 0, itemsize * m + sizeof(int)*2);
   STBIW_ASSERT(p);
   if (p) {
      if (!*arr) ((int *) p)[1] = 0;
      *arr = (void *) ((int *) p + 2);
      stbiw__sbm(*arr) = m;
   }
   return *arr;
}

static unsigned char *stbiw__zlib_flushf(unsigned char *data, unsigned int *bitbuffer, int *bitcount)
{
   while (*bitcount >= 8) {
      stbiw__sbpush(data, STBIW_UCHAR(*bitbuffer));
      *bitbuffer >>= 8;
      *bitcount -= 8;
   }
   return data;
}

static int stbiw__zlib_bitrev(int code, int codebits)
{
   int res=0;
   while (codebits--) {
      res = (res << 1) | (code & 1);
      code >>= 1;
   }
   return res;
}

static unsigned int stbiw__zlib_countm(unsigned char *a, unsigned char *b, int limit)
{
   int i;
   for (i=0; i < limit && i < 258; ++i)
      if (a[i] != b[i]) break;
   return i;
}

static unsigned int stbiw__zhash(unsigned char *data)
{
   stbiw_uint32 hash = data[0] + (data[1] << 8) + (data[2] << 16);
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;
   return hash;
}

#define stbiw__zlib_flush() (out = stbiw__zlib_flushf(out, &bitbuf, &bitcount))
#define stbiw__zlib_add(code,codebits) \
      (bitbuf |= (code) << bitcount, bitcount += (codebits), stbiw__zlib_flush())
#define stbiw__zlib_huffa(b,c)  stbiw__zlib_add(stbiw__zlib_bitrev(b,c),c)
// default huffman tables
#define stbiw__zlib_huff1(n)  stbiw__zlib_huffa(0x30 + (n), 8)
#define stbiw__zlib_huff2(n)  stbiw__zlib_huffa(0x190 + (n)-144, 9)
#define stbiw__zlib_huff3(n)  stbiw__zlib_huffa(0 + (n)-256,7)
#define stbiw__zlib_huff4(n)  stbiw__zlib_huffa(0xc0 + (n)-280,8)
#define stbiw__zlib_huff(n)  ((n) <= 143 ? stbiw__zlib_huff1(n) : (n) <= 255 ? stbiw__zlib_huff2(n) : (n) <= 279 ? stbiw__zlib_huff3(n) : stbiw__zlib_huff4(n))
#define stbiw__zlib_huffb(n) ((n) <= 143 ? stbiw__zlib_huff1(n) : stbiw__zlib_huff2(n))

#define stbiw__ZHASH   16384

unsigned char * stbi_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality)
{
   static unsigned short lengthc[] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258, 259 };
   static unsigned char  lengtheb[]= { 0,0,0,0,0,0,0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5,  0 };
   static unsigned short distc[]   = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577, 32768 };
   static unsigned char  disteb[]  = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13 };
   unsigned int bitbuf=0;
   int i,j, bitcount=0;
   unsigned char *out = NULL;
   unsigned char ***hash_table = (unsigned char***) STBIW_MALLOC(stbiw__ZHASH * sizeof(char**));
   if (quality < 5) quality = 5;

   stbiw__sbpush(out, 0x78);   // DEFLATE 32K window
   stbiw__sbpush(out, 0x5e);   // FLEVEL = 1
   stbiw__zlib_add(1,1);  // BFINAL = 1
   stbiw__zlib_add(1,2);  // BTYPE = 1 -- fixed huffman

   for (i=0; i < stbiw__ZHASH; ++i)
      hash_table[i] = NULL;

   i=0;
   while (i < data_len-3) {
      // hash next 3 bytes of data to be compressed
      int h = stbiw__zhash(data+i)&(stbiw__ZHASH-1), best=3;
      unsigned char *bestloc = 0;
      unsigned char **hlist = hash_table[h];
      int n = stbiw__sbcount(hlist);
      for (j=0; j < n; ++j) {
         if (hlist[j]-data > i-32768) { // if entry lies within window
            int d = stbiw__zlib_countm(hlist[j], data+i, data_len-i);
            if (d >= best) best=d,bestloc=hlist[j];
         }
      }
      // when hash table entry is too long, delete half the entries
      if (hash_table[h] && stbiw__sbn(hash_table[h]) == 2*quality) {
         STBIW_MEMMOVE(hash_table[h], hash_table[h]+quality, sizeof(hash_table[h][0])*quality);
         stbiw__sbn(hash_table[h]) = quality;
      }
      stbiw__sbpush(hash_table[h],data+i);

      if (bestloc) {
         // "lazy matching" - check match at *next* byte, and if it's better, do cur byte as literal
         h = stbiw__zhash(data+i+1)&(stbiw__ZHASH-1);
         hlist = hash_table[h];
         n = stbiw__sbcount(hlist);
         for (j=0; j < n; ++j) {
            if (hlist[j]-data > i-32767) {
               int e = stbiw__zlib_countm(hlist[j], data+i+1, data_len-i-1);
               if (e > best) { // if next match is better, bail on current match
                  bestloc = NULL;
                  break;
               }
            }
         }
      }

      if (bestloc) {
         int d = (int) (data+i - bestloc); // distance back
         STBIW_ASSERT(d <= 32767 && best <= 258);
         for (j=0; best > lengthc[j+1]-1; ++j);
         stbiw__zlib_huff(j+257);
         if (lengtheb[j]) stbiw__zlib_add(best - lengthc[j], lengtheb[j]);
         for (j=0; d > distc[j+1]-1; ++j);
         stbiw__zlib_add(stbiw__zlib_bitrev(j,5),5);
         if (disteb[j]) stbiw__zlib_add(d - distc[j], disteb[j]);
         i += best;
      } else {
         stbiw__zlib_huffb(data[i]);
         ++i;
      }
   }
   // write out final bytes
   for (;i < data_len; ++i)
      stbiw__zlib_huffb(data[i]);
   stbiw__zlib_huff(256); // end of block
   // pad with 0 bits to byte boundary
   while (bitcount)
      stbiw__zlib_add(0,1);

   for (i=0; i < stbiw__ZHASH; ++i)
      (void) stbiw__sbfree(hash_table[i]);
   STBIW_FREE(hash_table);

   {
      // compute adler32 on input
      unsigned int s1=1, s2=0;
      int blocklen = (int) (data_len % 5552);
      j=0;
      while (j < data_len) {
         for (i=0; i < blocklen; ++i) s1 += data[j+i], s2 += s1;
         s1 %= 65521, s2 %= 65521;
         j += blocklen;
         blocklen = 5552;
      }
      stbiw__sbpush(out, STBIW_UCHAR(s2 >> 8));
      stbiw__sbpush(out, STBIW_UCHAR(s2));
      stbiw__sbpush(out, STBIW_UCHAR(s1 >> 8));
      stbiw__sbpush(out, STBIW_UCHAR(s1));
   }
   *out_len = stbiw__sbn(out);
   // make returned pointer freeable
   STBIW_MEMMOVE(stbiw__sbraw(out), out, *out_len);
   return (unsigned char *) stbiw__sbraw(out);
}

static unsigned int stbiw__crc32(unsigned char *buffer, int len)
{
   static unsigned int crc_table[256] =
   {
      0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
      0x0eDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
      0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
      0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
      0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
      0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
      0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
      0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
      0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
      0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
      0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
      0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
      0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
      0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
      0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
      0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
      0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
      0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
      0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
      0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
      0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
      0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
      0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
      0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
      0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
      0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
      0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
      0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
      0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
      0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
      0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
      0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
   };

   unsigned int crc = ~0u;
   int i;
   for (i=0; i < len; ++i)
      crc = (crc >> 8) ^ crc_table[buffer[i] ^ (crc & 0xff)];
   return ~crc;
}

#define stbiw__wpng4(o,a,b,c,d) ((o)[0]=STBIW_UCHAR(a),(o)[1]=STBIW_UCHAR(b),(o)[2]=STBIW_UCHAR(c),(o)[3]=STBIW_UCHAR(d),(o)+=4)
#define stbiw__wp32(data,v) stbiw__wpng4(data, (v)>>24,(v)>>16,(v)>>8,(v));
#define stbiw__wptag(data,s) stbiw__wpng4(data, s[0],s[1],s[2],s[3])

static void stbiw__wpcrc(unsigned char **data, int len)
{
   unsigned int crc = stbiw__crc32(*data - len - 4, len+4);
   stbiw__wp32(*data, crc);
}

static unsigned char stbiw__paeth(int a, int b, int c)
{
   int p = a + b - c, pa = abs(p-a), pb = abs(p-b), pc = abs(p-c);
   if (pa <= pb && pa <= pc) return STBIW_UCHAR(a);
   if (pb <= pc) return STBIW_UCHAR(b);
   return STBIW_UCHAR(c);
}

unsigned char *stbi_write_png_to_mem(unsigned char *pixels, int stride_bytes, int x, int y, int n, int *out_len)
{
   int ctype[5] = { -1, 0, 4, 2, 6 };
   unsigned char sig[8] = { 137,80,78,71,13,10,26,10 };
   unsigned char *out,*o, *filt, *zlib;
   signed char *line_buffer;
   int i,j,k,p,zlen;

   if (stride_bytes == 0)
      stride_bytes = x * n;

   filt = (unsigned char *) STBIW_MALLOC((x*n+1) * y); if (!filt) return 0;
   line_buffer = (signed char *) STBIW_MALLOC(x * n); if (!line_buffer) { STBIW_FREE(filt); return 0; }
   for (j=0; j < y; ++j) {
      static int mapping[] = { 0,1,2,3,4 };
      static int firstmap[] = { 0,1,0,5,6 };
      int *mymap = j ? mapping : firstmap;
      int best = 0, bestval = 0x7fffffff;
      for (p=0; p < 2; ++p) {
         for (k= p?best:0; k < 5; ++k) {
            int type = mymap[k],est=0;
            unsigned char *z = pixels + stride_bytes*j;
            for (i=0; i < n; ++i)
               switch (type) {
                  case 0: line_buffer[i] = z[i]; break;
                  case 1: line_buffer[i] = z[i]; break;
                  case 2: line_buffer[i] = z[i] - z[i-stride_bytes]; break;
                  case 3: line_buffer[i] = z[i] - (z[i-stride_bytes]>>1); break;
                  case 4: line_buffer[i] = (signed char) (z[i] - stbiw__paeth(0,z[i-stride_bytes],0)); break;
                  case 5: line_buffer[i] = z[i]; break;
                  case 6: line_buffer[i] = z[i]; break;
               }
            for (i=n; i < x*n; ++i) {
               switch (type) {
                  case 0: line_buffer[i] = z[i]; break;
                  case 1: line_buffer[i] = z[i] - z[i-n]; break;
                  case 2: line_buffer[i] = z[i] - z[i-stride_bytes]; break;
                  case 3: line_buffer[i] = z[i] - ((z[i-n] + z[i-stride_bytes])>>1); break;
                  case 4: line_buffer[i] = z[i] - stbiw__paeth(z[i-n], z[i-stride_bytes], z[i-stride_bytes-n]); break;
                  case 5: line_buffer[i] = z[i] - (z[i-n]>>1); break;
                  case 6: line_buffer[i] = z[i] - stbiw__paeth(z[i-n], 0,0); break;
               }
            }
            if (p) break;
            for (i=0; i < x*n; ++i)
               est += abs((signed char) line_buffer[i]);
            if (est < bestval) { bestval = est; best = k; }
         }
      }
      // when we get here, best contains the filter type, and line_buffer contains the data
      filt[j*(x*n+1)] = (unsigned char) best;
      STBIW_MEMMOVE(filt+j*(x*n+1)+1, line_buffer, x*n);
   }
   STBIW_FREE(line_buffer);
   zlib = stbi_zlib_compress(filt, y*( x*n+1), &zlen, 8); // increase 8 to get smaller but use more memory
   STBIW_FREE(filt);
   if (!zlib) return 0;

   // each tag requires 12 bytes of overhead
   out = (unsigned char *) STBIW_MALLOC(8 + 12+13 + 12+zlen + 12);
   if (!out) return 0;
   *out_len = 8 + 12+13 + 12+zlen + 12;

   o=out;
   STBIW_MEMMOVE(o,sig,8); o+= 8;
   stbiw__wp32(o, 13); // header length
   stbiw__wptag(o, "IHDR");
   stbiw__wp32(o, x);
   stbiw__wp32(o, y);
   *o++ = 8;
   *o++ = STBIW_UCHAR(ctype[n]);
   *o++ = 0;
   *o++ = 0;
   *o++ = 0;
   stbiw__wpcrc(&o,13);

   stbiw__wp32(o, zlen);
   stbiw__wptag(o, "IDAT");
   STBIW_MEMMOVE(o, zlib, zlen);
   o += zlen;
   STBIW_FREE(zlib);
   stbiw__wpcrc(&o, zlen);

   stbiw__wp32(o,0);
   stbiw__wptag(o, "IEND");
   stbiw__wpcrc(&o,0);

   STBIW_ASSERT(o == out + *out_len);

   return out;
}

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes)
{
   FILE *f;
   int len;
   unsigned char *png = stbi_write_png_to_mem((unsigned char *) data, stride_bytes, x, y, comp, &len);
   if (png == NULL) return 0;
   f = fopen(filename, "wb");
   if (!f) { STBIW_FREE(png); return 0; }
   fwrite(png, 1, len, f);
   fclose(f);
   STBIW_FREE(png);
   return 1;
}
#endif

STBIWDEF int stbi_write_png_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int stride_bytes)
{
   int len;
   unsigned char *png = stbi_write_png_to_mem((unsigned char *) data, stride_bytes, x, y, comp, &len);
   if (png == NULL) return 0;
   func(context, png, len);
   STBIW_FREE(png);
   return 1;
}

#endif // STB_IMAGE_WRITE_IMPLEMENTATION

/* Revision history
      1.02 (2016-04-02)
             avoid allocating large structures on the stack
      1.01 (2016-01-16)
             STBIW_REALLOC_SIZED: support allocators with no realloc support
             avoid race-condition in crc initialization
             minor compile issues
      1.00 (2015-09-14)
             installable file IO function
      0.99 (2015-09-13)
             warning fixes; TGA rle support
      0.98 (2015-04-08)
             added STBIW_MALLOC, STBIW_ASSERT etc
      0.97 (2015-01-18)
             fixed HDR asserts, rewrote HDR rle logic
      0.96 (2015-01-17)
             add HDR output
             fix monochrome BMP
      0.95 (2014-08-17)
		       add monochrome TGA output
      0.94 (2014-05-31)
             rename private functions to avoid conflicts with stb_image.h
      0.93 (2014-05-27)
             warning fixes
      0.92 (2010-08-01)
             casts to unsigned char to fix warnings
      0.91 (2010-07-17)
             first public release
      0.90   first internal release
*/


float * load_mnist_bmp(const char * filename, ...) {
  char buf[PATH_MAX];
  va_list ap;
  va_start(ap, filename);
  vsnprintf(buf, sizeof(buf), filename, ap);
  va_end(ap);

  int width;
  int height;
  int nchannels;
  unsigned char * img8 = stbi_load(filename, &width, &height, &nchannels, 0);
  
  float * imgf = malloc(sizeof(float)*28*28);
  assert(NULL != imgf);
  
  int x, y;
  const float dx = (width)/28.0;
  const float dy = (height)/28.0;
  for(y=0 ; y<28 ; y++) {
    for(x=0 ; x<28 ; x++) {
      int sx = x*dx;
      int sy = y*dy;
      imgf[28*y+x] = img8[ ((width)*sy + sx)*nchannels ] / 255.0f;
    }
  }

  free(img8);
  return imgf;
}

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint32_t LONG;

// https://msdn.microsoft.com/en-us/library/dd183374(VS.85).aspx
typedef struct tagBITMAPFILEHEADER {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
} __attribute__ ((packed)) BITMAPFILEHEADER, *PBITMAPFILEHEADER;

// https://msdn.microsoft.com/en-us/library/dd183376(VS.85).aspx
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} __attribute__ ((packed)) BITMAPINFOHEADER, *PBITMAPINFOHEADER;

// https://msdn.microsoft.com/en-us/library/dd162938(v=vs.85).aspx
typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} __attribute__ ((packed)) RGBQUAD;

BITMAPFILEHEADER bmp_header(int width, int height, int color) {
  BITMAPFILEHEADER bmpfile;
  int colortablesize = color == 1 ? sizeof(RGBQUAD)*256 : 0;

  bmpfile.bfType = 0x4d42; // 'BM' magic number
  bmpfile.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colortablesize + width*height*color;
  bmpfile.bfReserved1 = 0;
  bmpfile.bfReserved2 = 0;
  bmpfile.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colortablesize;
  assert(sizeof(BITMAPFILEHEADER) == 14);
  return bmpfile;
}

BITMAPINFOHEADER bmp_info(int width, int height, int color) {
  BITMAPINFOHEADER bmpinfo;
  bmpinfo.biSize = 40;
  bmpinfo.biWidth = width;
  bmpinfo.biHeight = -height; // negative = top-down DIB
  bmpinfo.biPlanes = 1;
  bmpinfo.biBitCount = color * 8;
  bmpinfo.biCompression = 0;
  bmpinfo.biSizeImage = 0;
  bmpinfo.biXPelsPerMeter = 0;
  bmpinfo.biYPelsPerMeter = 0;
  bmpinfo.biClrUsed = 0;
  bmpinfo.biClrImportant = 0;
  assert(sizeof(BITMAPINFOHEADER) == 40);
  return bmpinfo;
}

void save_mnist_bmp(const float * x, const char * filename, ...) {
  const int width = 28;
  const int height = 28;
  
  char buf[PATH_MAX];
  va_list ap;
  va_start(ap, filename);
  vsnprintf(buf, sizeof(buf), filename, ap);
  va_end(ap);
  
  assert(width % 4 == 0);

  FILE * fp = fopen(buf, "wb");
  assert(fp);

  BITMAPFILEHEADER bmpFile = bmp_header(width, height, 1);
  BITMAPINFOHEADER bmpInfo = bmp_info(width, height, 1);
  fwrite(&bmpFile, sizeof(BITMAPFILEHEADER), 1, fp);
  fwrite(&bmpInfo, sizeof(BITMAPINFOHEADER), 1, fp);

  RGBQUAD bmiColors[256];
  assert(sizeof(RGBQUAD) == 4);
  int i;
  for(i=0 ; i<256 ; i++) {
    bmiColors[i].rgbBlue = i;
    bmiColors[i].rgbGreen = i;
    bmiColors[i].rgbRed = i;
    bmiColors[i].rgbReserved = 0;
  }
  fwrite(bmiColors, sizeof(RGBQUAD)*256, 1, fp);

  unsigned char * px = (unsigned char *)malloc(width*height*sizeof(unsigned char));
  for(i=0 ; i<width*height ; i++) {
    px[i] = 255 * x[i];
  }
  fwrite(px, width*height, 1, fp);
  free(px);

  fclose(fp);
}


const float A_784x10[28*28*10] = {
	-0.0061602,	-0.0548926,	0.0345619,	-0.0543053,	0.00168036,	0.0376199,	0.00169737,	-0.0265992,	0.0233614,	0.0101369,	0.107312,	0.0187748,	0.0515662,	0.0395646,	-0.00646043,	-0.0170118,	-0.0347066,	0.0660852,	-0.0472672,	-0.134975,	-0.0390292,	-0.026343,	-0.101787,	0.00254396,	-0.0451854,	-0.000665008,	-0.00932776,	-0.0343176,	-0.00372435,	0.043348,	0.0190369,	-0.037844,	0.0708272,	0.0407041,	-0.0169373,	0.033941,	0.0610599,	0.00218034,	-0.0151842,	0.0635275,	-0.0541755,	-0.00154499,	-0.0245546,	0.00194176,	0.0814805,	-0.00190943,	-0.0210445,	-0.0163924,	-0.0310654,	-0.0807981,	-0.0499329,	0.0402616,	0.0557958,	-0.0669514,	-0.0490807,	0.0792753,	0.00801752,	-0.0081774,	0.0716284,	-0.100244,	0.0288673,	-0.000856692,	0.0217192,	-0.0663518,	0.0484944,	0.0163036,	-0.083436,	-0.0193534,	0.0315416,	-0.0368717,	-0.0657893,	-0.10472,	-0.0544319,	-0.0977455,	0.00402872,	-0.0307105,	-0.0137805,	-0.0132446,	-0.00339995,	0.0557491,	-0.00839138,	0.0670442,	0.0556675,	0.0227573,	0.00669776,	0.00298038,	0.00640998,	-0.0927983,	0.0471587,	-0.0297751,	0.0218386,	0.0182308,	-0.0517044,	-0.0318735,	0.0694154,	-0.0285504,	0.00542553,	-0.148228,	0.0563513,	-0.037556,	-0.0579335,	-0.0780784,	-0.158873,	-0.0575713,	-0.0583913,	-0.0666167,	-0.121373,	-0.00123623,	0.0160702,	-0.00901561,	-0.0472365,	-0.0499504,	-0.0355726,	0.0463527,	-0.0194015,	0.060723,	-0.0229633,	-0.0477387,	-0.0594257,	-0.0395015,	0.048825,	0.034294,	0.0312703,	0.0914421,	-0.109334,	0.05136,	-0.0225483,	-0.024169,	0.0297052,	0.0401036,	-0.0484766,	-0.0894852,	-0.171331,	-0.0690245,	0.0469421,	-0.0250704,	-0.151687,	-0.0688815,	-0.109853,	-0.05106,	-0.00784085,	-0.0134379,	0.0591414,	0.0443957,	0.0874616,	-0.0768754,	-0.0919196,	-0.0166048,	-0.0011264,	-0.106535,	0.0476066,	0.0898423,	0.0754792,	0.123915,	0.227361,	0.201367,	0.174493,	0.244472,	0.199324,	0.065712,	-0.0692231,	0.0296542,	0.0475104,	-0.0573852,	-0.135806,	-0.118016,	-0.0455945,	-0.0188316,	-0.0250541,	-0.0419342,	0.112948,	-0.12814,	-0.0136678,	-0.0418442,	-0.118327,	0.0191053,	-0.0422462,	-0.104305,	0.02215,	0.0873564,	0.0158744,	0.161687,	0.131973,	0.15264,	0.165691,	0.17084,	0.271715,	0.162754,	0.18267,	0.156257,	0.168417,	-0.0958224,	-0.153621,	-0.168269,	0.000627857,	0.0154236,	-0.0381199,	0.0409562,	0.0359129,	0.0359157,	-0.0404361,	-0.0258648,	0.0140706,	-0.0963192,	-0.157706,	-0.0252101,	0.0339822,	0.075748,	0.118805,	0.147089,	0.106822,	0.125129,	0.145098,	0.256181,	0.221705,	0.186308,	0.0417246,	0.11461,	0.132098,	0.0817393,	-0.0247014,	-0.210954,	-0.00752626,	0.0485544,	-0.00572429,	0.0288053,	-0.0808467,	-0.0192031,	-0.136866,	-0.0898709,	-0.0395678,	-0.134251,	-0.0300837,	0.0638984,	0.0246342,	-0.0313968,	0.064382,	0.0926203,	0.0984521,	0.262606,	0.371115,	0.262426,	0.287239,	0.0923537,	0.0523107,	0.0629345,	0.169201,	0.223464,	-0.00827617,	-0.194607,	-0.0510615,	-0.0823412,	0.00418222,	-0.0217107,	0.0214437,	-0.104675,	-0.026905,	-0.0662176,	-0.0504733,	-0.0540976,	-0.0503389,	0.0692286,	0.037808,	-0.0456245,	0.0692893,	0.188969,	0.0410511,	-0.0737092,	0.124612,	0.237898,	0.259119,	0.241404,	0.112164,	0.0123162,	0.0945122,	0.319266,	0.122585,	-0.0740339,	-0.107236,	0.00208346,	0.0530959,	0.125241,	0.0345875,	0.0908146,	-0.00892769,	0.0429559,	-0.0703381,	-0.184327,	-0.0308646,	0.0118076,	0.0317242,	0.035849,	-0.00543862,	0.0860203,	-0.0804725,	-0.0964978,	-0.144472,	0.223075,	0.236676,	0.0689643,	0.161097,	0.0142303,	0.196119,	0.313615,	0.181286,	-0.126019,	-0.0361584,	-0.0356074,	0.0616124,	-0.0353048,	0.0374611,	-0.101277,	-0.0170633,	-0.100979,	-0.0245204,	0.0397828,	-0.026125,	-0.124027,	-0.0735543,	0.00283588,	-0.16418,	-0.0910155,	-0.309474,	-0.53367,	-0.474916,	-0.0972854,	-0.0446338,	0.110572,	0.132679,	0.180338,	0.293268,	0.256849,	0.233057,	-0.0454948,	-0.0353806,	0.0481868,	0.0757743,	-0.0496393,	-0.0042185,	-0.0543884,	-0.13687,	0.0274818,	0.0970418,	-0.0759086,	0.032098,	-0.0197556,	0.0609813,	-0.0255905,	-0.0231265,	-0.0752622,	-0.562346,	-0.656807,	-0.605176,	-0.17139,	0.0412194,	0.0028477,	0.000604891,	0.147643,	0.233476,	0.34389,	0.221304,	0.0476928,	0.022015,	0.0089523,	-0.0872032,	0.00642974,	0.0207452,	0.0418861,	-0.107911,	0.0151327,	0.232904,	0.228174,	0.0777714,	0.0483157,	0.107488,	0.13027,	0.00703269,	-0.334284,	-0.639358,	-0.594845,	-0.592558,	-0.29283,	-0.138612,	-0.00457956,	-0.110304,	0.0265588,	0.258101,	0.320412,	0.301379,	0.0457749,	0.0577035,	0.068566,	-0.100198,	0.0197136,	-0.086827,	-0.0530244,	-0.0181015,	0.0696285,	0.133148,	0.293232,	0.209606,	0.162219,	0.194143,	0.185604,	-0.012114,	-0.237397,	-0.603502,	-0.631788,	-0.598626,	-0.202917,	-0.0920302,	-0.101355,	-0.0353738,	0.179926,	0.113375,	0.320203,	0.20793,	-0.0279653,	0.0256738,	-0.00153504,	0.0338057,	-0.033944,	0.00329268,	-0.00684987,	0.0134365,	0.191017,	0.0986276,	0.249589,	0.149877,	0.272764,	0.254482,	0.0844658,	-0.140627,	-0.536936,	-0.648397,	-0.54252,	-0.458285,	-0.208237,	-0.032265,	-0.0668847,	0.138571,	0.0607399,	0.153701,	0.303113,	0.133727,	0.0178127,	-0.0227011,	0.0737528,	-0.00728431,	0.0132836,	-0.0338222,	0.00263697,	0.0591953,	0.243643,	0.13422,	0.249804,	0.236776,	0.197835,	0.183941,	0.161076,	-0.2457,	-0.626865,	-0.583243,	-0.489123,	-0.235275,	-0.149881,	0.0191317,	0.0260106,	0.0369615,	0.105413,	0.162582,	0.154928,	0.0651435,	0.0293921,	-0.0527042,	-0.0202912,	-0.013459,	0.0820373,	0.033676,	-0.0883089,	0.00354965,	0.179328,	0.187133,	0.155679,	0.119892,	0.154586,	0.311935,	0.0023307,	-0.258026,	-0.53895,	-0.553857,	-0.305505,	-0.048517,	0.0991095,	-0.0532467,	0.0364114,	0.0953007,	0.0362309,	0.12865,	0.0963479,	0.0148943,	-0.042779,	0.0851195,	-0.020982,	-0.120658,	-0.0389712,	0.0726712,	-0.205811,	-0.0601675,	0.214862,	0.137021,	0.0807467,	0.171729,	0.267402,	0.213685,	0.0594178,	-0.28418,	-0.512169,	-0.45403,	-0.204829,	-0.101668,	0.00567284,	0.0925211,	-0.0240158,	-0.014891,	0.0963624,	0.246075,	0.0582711,	0.00684846,	-0.000972306,	0.0564726,	-0.0434485,	0.0612335,	-0.105081,	0.0705559,	0.0159396,	-0.0415838,	0.0963639,	0.184707,	0.143212,	0.110764,	0.209646,	0.288281,	0.208932,	0.124096,	-0.211715,	-0.168319,	-0.0895009,	-0.0429717,	-0.0374431,	-0.010362,	-0.0902146,	-0.0184337,	0.0412968,	0.0532729,	-0.0161696,	-0.0410752,	-0.0185051,	-0.0719555,	-0.0162776,	0.0385385,	0.0635588,	-0.0501532,	-0.0272068,	0.000744408,	0.0814299,	0.0312414,	0.215633,	0.143418,	0.133005,	0.373193,	0.390781,	0.316986,	0.0999779,	0.00808166,	0.049062,	-0.0195195,	-0.0589085,	-0.0858015,	-0.13278,	-0.0778197,	-0.0164537,	0.00576104,	-0.0431741,	-0.0275255,	0.0247926,	-0.0462737,	0.00289898,	-0.0351306,	0.0515478,	0.0292914,	0.0250812,	-0.0708426,	-0.00757785,	0.109798,	0.0851776,	0.033216,	0.0541072,	0.287676,	0.265078,	0.181473,	0.119649,	0.0142854,	-0.0172169,	0.00588238,	-0.162961,	-0.121812,	-0.144293,	0.0176186,	-0.0366095,	-0.00334633,	0.00848983,	-0.0491277,	0.0103126,	0.0354442,	-0.0345615,	-0.00469384,	-0.0176244,	0.0178721,	-0.0916802,	-0.0433787,	-0.0130353,	0.0954583,	0.112208,	0.167239,	0.175792,	0.18995,	0.268797,	0.194429,	0.302184,	0.194465,	0.157055,	0.0239354,	0.0306505,	-0.110944,	-0.10226,	-0.109354,	-0.172637,	0.0144861,	-0.00336816,	0.055443,	0.00871713,	-0.0258578,	0.0735092,	0.0511083,	0.0641415,	-0.0484229,	-0.0658574,	-0.0855252,	-0.0664632,	-0.0177746,	0.0926819,	0.0286375,	0.136813,	0.378988,	0.368309,	0.244751,	0.324714,	0.251866,	0.117231,	0.0630303,	-0.0513847,	-0.141003,	-0.205478,	-0.237033,	-0.0943688,	-0.00402977,	-0.0413693,	-0.000656677,	-0.00561644,	-0.0556003,	0.0119074,	-0.0429234,	-1.69506e-05,	-0.0222671,	-0.057632,	-0.0414735,	-0.0662653,	-0.0998002,	-0.0681922,	-0.0476426,	0.0827807,	-0.0531937,	-0.0477735,	0.14198,	0.129979,	0.0844342,	-0.0612487,	-0.122543,	-0.0875782,	-0.151045,	-0.177573,	-0.152227,	-0.110455,	-0.0234986,	-0.0276436,	-0.0132081,	-0.0170505,	0.0557626,	0.00258701,	0.00860537,	0.0017097,	0.0273573,	0.0473327,	-0.0684772,	0.0722983,	-0.0711885,	-0.192963,	-0.130256,	-0.0997265,	-0.31858,	-0.234584,	-0.124055,	-0.236255,	-0.120738,	-0.153384,	-0.212925,	-0.0995285,	-0.150843,	-0.111589,	-0.0760723,	0.0354926,	-0.02926,	0.00478337,	-0.0159291,	0.0995167,	0.0638751,	0.00121833,	0.0696922,	-0.0266575,	0.106984,	-0.035725,	-0.0443899,	0.0423696,	0.0175054,	-0.0458079,	0.0314237,	-0.064547,	-0.0138312,	-0.0550257,	-0.0573144,	0.0237386,	-0.150408,	-0.110162,	-0.0119331,	-0.0673776,	-0.0332684,	0.0145734,	-0.0462634,	0.0610677,	-0.0049716,	-0.000210155,	-0.0563898,	0.0380973,	-0.0430968,	0.0220037,	-0.0359346,	0.00832679,	0.0330518,	-0.0422082,	0.0269908,	-0.0227121,	-0.0362614,	-0.0445282,	0.019708,	0.0541857,	0.0866995,	-0.0180959,	-0.0479251,	-0.0149288,	-0.0848809,	-0.0266466,	0.0308251,	0.104397,	-0.107966,	-0.0208551,	-0.0249926,	0.032406,	0.0262236,	0.0177431,	0.00654357,	-0.067279,	-0.0463851,	0.0394833,
	-0.0274286,	-0.111903,	-0.00949961,	-0.0254555,	-0.0449538,	0.0600028,	0.01671,	0.0197478,	-0.0694173,	-0.0238967,	-0.0428261,	-0.0179525,	-0.00507236,	-0.0418315,	0.0239469,	0.0913683,	0.0316478,	-0.062107,	-0.00755842,	0.0985425,	0.0826995,	0.00422862,	0.0892373,	0.127315,	-0.0377019,	-0.0443744,	0.0646712,	-0.0583746,	-0.0157164,	0.0485795,	-0.0242672,	-0.0342382,	-0.00455091,	-0.0594236,	0.0294523,	-0.0512052,	0.0706234,	0.00762759,	-0.0124481,	0.0854494,	0.0144923,	0.0904461,	0.0285962,	0.0587567,	0.049853,	-0.0610463,	-0.022365,	0.0547702,	-0.0540535,	-0.0673871,	-0.00204859,	0.00827359,	-0.0794789,	0.0640543,	-0.0590856,	0.0611611,	0.0184354,	-0.0256587,	-0.0143659,	0.0326536,	0.0309837,	0.00105247,	-0.00405687,	-0.0575268,	-0.0690415,	0.0117393,	0.0220967,	-0.0394603,	-0.0393234,	0.0681569,	0.0516285,	0.142397,	0.0180991,	-0.0372075,	-0.00028696,	-0.0854591,	0.0409435,	-0.0393139,	-0.0268463,	-0.0202239,	0.00889917,	-0.0290284,	-0.0347332,	-0.0278666,	-0.0550235,	0.027799,	0.0106906,	0.0311559,	0.0588728,	-0.0294192,	-0.0392514,	-0.0105247,	-0.0531027,	-0.0651228,	-0.0668321,	-0.0764083,	-0.0871537,	-0.0913638,	-0.0261532,	0.0233719,	0.0487123,	-0.00445434,	-0.128907,	-0.150905,	-0.0166814,	-0.0963565,	-0.0894663,	0.00497486,	-0.00570675,	-0.0366581,	0.0446837,	-0.021203,	-0.00851806,	0.0036861,	-0.0702142,	0.112523,	0.0429146,	-0.0317101,	-0.0388025,	-0.0515066,	-0.0302716,	-0.0610612,	-0.0932836,	-0.0773468,	0.0628637,	0.159841,	0.214712,	0.100238,	0.061142,	-0.0356555,	-0.0823019,	0.16895,	0.0693224,	0.0355975,	0.112845,	0.0217534,	-0.0625282,	-0.0801514,	-0.0204439,	0.0296207,	0.0839514,	-0.00872962,	-0.0302147,	0.0207212,	-0.0462387,	-0.0833051,	0.0178871,	-0.141448,	-0.183034,	-0.213124,	-0.194277,	-0.0740767,	-0.0906413,	0.0672639,	0.10004,	0.138655,	0.030056,	-0.0626194,	-0.00727542,	0.0643996,	0.154352,	0.189378,	0.24932,	0.0467893,	0.0861131,	-0.0354911,	-0.0778637,	-0.0550552,	0.0453061,	-0.0465716,	-0.0398821,	-0.0932746,	0.00490003,	-0.0259282,	-0.06711,	-0.221053,	-0.277764,	-0.304026,	-0.175975,	-0.0788173,	-0.0772397,	-0.0862558,	-0.0517213,	-0.136576,	-0.228075,	-0.230251,	-0.0835654,	-0.0556389,	0.0602716,	0.122017,	0.102082,	0.0313302,	0.0372297,	-0.0749058,	-0.050855,	0.0677621,	-0.0414306,	0.0242257,	0.015522,	0.0890298,	-0.0632381,	-0.10424,	-0.0599766,	-0.270799,	-0.284289,	-0.296444,	-0.262255,	-0.17747,	-0.0712608,	-0.0610358,	-0.109302,	-0.249978,	-0.240617,	-0.188014,	-0.128055,	-0.0205911,	0.0437002,	-0.0155523,	0.0050901,	-0.0966106,	-0.0982969,	-0.170905,	-0.0581096,	-0.00225703,	0.0447159,	-0.189461,	0.0606694,	0.0236192,	-0.0369762,	-0.175608,	-0.14793,	-0.214868,	-0.338131,	-0.291492,	-0.158752,	-0.274451,	-0.0819782,	-0.05524,	-0.0227091,	0.00453223,	-0.162733,	0.0364861,	-0.0217031,	-0.0930611,	-0.0984325,	-0.125068,	-0.150924,	-0.257722,	-0.226802,	-0.110402,	-0.0389327,	-0.0296404,	-0.0384819,	-0.0415322,	0.136414,	0.0445441,	-0.006129,	-0.182329,	-0.0851759,	-0.216859,	-0.240793,	-0.334848,	-0.22586,	-0.0429155,	-0.0179597,	-0.0386053,	0.193823,	0.330483,	0.101639,	-0.0525116,	-0.12239,	-0.193026,	-0.304556,	-0.319415,	-0.259696,	-0.364309,	-0.130387,	-0.128633,	-0.027895,	0.0559551,	0.0032756,	-0.0122466,	0.0386978,	0.0252397,	-0.0298463,	-0.0688774,	-0.180042,	-0.137157,	-0.115245,	-0.32659,	-0.0940094,	-0.155598,	-0.02783,	0.10725,	0.489138,	0.581574,	0.27697,	0.0640461,	-0.167009,	-0.252574,	-0.225561,	-0.232819,	-0.277916,	-0.248819,	-0.176843,	-0.152756,	0.0122545,	-0.0186867,	-0.0873765,	0.0387042,	-0.000258701,	-0.0923511,	-0.0445382,	-0.031609,	-0.194342,	-0.175495,	-0.107298,	-0.241807,	-0.103332,	-0.1132,	-0.10679,	0.225349,	0.644112,	0.600428,	0.179893,	-0.0495053,	-0.0800427,	-0.187885,	-0.173349,	-0.174391,	-0.124683,	-0.143488,	-0.102835,	-0.0844135,	0.0253506,	0.0223254,	0.0399739,	-0.0444347,	-0.0326497,	0.0110766,	0.0323154,	-0.01355,	-0.00720269,	-0.0534143,	-0.0339337,	-0.141481,	-0.321469,	-0.400689,	-0.146676,	0.224259,	0.822037,	0.477279,	0.108821,	0.00766723,	-0.0508839,	-0.220331,	-0.256428,	-0.0218838,	-0.0416902,	0.0354693,	-0.129851,	0.0809262,	-0.0740829,	-0.0782185,	-0.00728284,	0.0208906,	0.00051057,	0.094684,	-0.043864,	0.00903682,	-0.072112,	-0.0826813,	-0.0746592,	-0.209435,	-0.404518,	-0.581289,	-0.109835,	0.322615,	0.743883,	0.466121,	0.0873628,	0.0304269,	-0.145555,	-0.215706,	-0.218742,	-0.129231,	0.00636226,	-0.0622107,	-0.0153239,	0.123147,	-0.0747226,	-0.0572832,	0.0156465,	-0.0341338,	0.08047,	0.0602684,	-0.0217653,	-0.0833716,	-0.113179,	-0.138453,	-0.0273241,	-0.222901,	-0.4422,	-0.412301,	-0.0591409,	0.347365,	0.643935,	0.4063,	0.135827,	-0.306631,	-0.310276,	-0.289104,	-0.16456,	-0.115584,	-0.104788,	-0.0646534,	-0.130299,	-0.0406328,	0.0760042,	-0.000825537,	-0.00829677,	0.0954292,	-0.0317903,	-0.107711,	0.0587474,	-0.146434,	-0.103944,	-0.126003,	-0.0943083,	-0.27087,	-0.369543,	-0.198546,	0.0468961,	0.473742,	0.69499,	0.239108,	-0.0475485,	-0.47232,	-0.322817,	-0.247371,	-0.209757,	-0.0900419,	-0.0646611,	-0.0518675,	-0.019496,	0.00265517,	-0.164668,	-0.0190961,	0.00045372,	0.0352066,	-0.00378139,	-0.00294481,	0.0680764,	-0.0754011,	-0.0846295,	-0.211816,	-0.240911,	-0.241877,	-0.176979,	-0.0414724,	-0.0636129,	0.433116,	0.543799,	0.0382223,	-0.243958,	-0.511397,	-0.462933,	-0.350628,	-0.233352,	-0.158022,	-0.0605712,	-0.162411,	-0.0239707,	-0.0627164,	-0.0166825,	0.0112152,	0.0362692,	-0.00162206,	0.00629928,	0.0666546,	0.0162585,	-0.145801,	-0.355814,	-0.280935,	-0.281396,	-0.0789141,	-0.0274581,	-0.0990324,	0.135082,	0.467096,	0.415019,	0.0359213,	-0.335886,	-0.483161,	-0.289563,	-0.255732,	-0.0732028,	-0.158215,	-0.0152075,	-0.0263093,	-0.108111,	-0.0455424,	-0.0270459,	-0.0132422,	-0.0495859,	-0.104978,	-0.0115295,	-0.00992063,	-0.0651956,	-0.250195,	-0.260374,	-0.359546,	-0.167945,	0.0441678,	-0.1711,	-0.0956536,	0.0466674,	0.171165,	0.219182,	-0.175066,	-0.358966,	-0.193302,	-0.121653,	-0.129382,	-0.033992,	-0.097829,	-0.144879,	-0.0586246,	-0.0334967,	0.00752202,	0.023541,	-0.0191299,	-0.0711596,	-0.00697928,	-0.047507,	0.028152,	-0.0140447,	-0.247496,	-0.236329,	-0.136485,	-0.197145,	-0.0193229,	-0.0684759,	-0.0510547,	0.0396018,	0.138914,	0.0349501,	-0.0842407,	0.0369216,	0.0679455,	0.0362772,	0.0723246,	0.0419806,	0.0324469,	-0.0775661,	-0.031482,	0.0648315,	-0.0141535,	0.0291713,	-0.0139167,	0.03091,	-0.0202464,	-0.104494,	0.00075743,	-0.0473578,	-0.0853496,	-0.0619594,	-0.0309049,	0.0742798,	0.0823061,	0.0906876,	0.11695,	0.052934,	0.0235609,	0.021109,	0.0731066,	0.128685,	0.150314,	0.171408,	0.050303,	0.0478562,	0.0562138,	-0.101106,	-0.095978,	0.00202938,	-0.000670982,	0.000177425,	0.0167693,	-0.021158,	-0.0729646,	-0.0185388,	0.0649892,	0.00993213,	0.037524,	0.0873936,	0.151777,	0.0847424,	-0.00953711,	0.112919,	-0.052579,	-0.232675,	-0.11568,	-0.00876677,	-0.0556852,	0.0792286,	0.158175,	0.131978,	0.135428,	-0.023881,	-0.0637156,	-0.0675379,	-0.133419,	0.0119232,	0.0475762,	0.0111759,	-0.0275467,	0.0956131,	0.0139298,	-0.0704265,	-0.0745036,	-0.0795193,	0.181602,	0.223604,	0.19604,	0.0770407,	0.071285,	0.010984,	-0.254389,	-0.304698,	-0.283086,	-0.0113653,	0.0468193,	0.125528,	0.290287,	0.222167,	0.0550217,	-0.0364379,	0.0406666,	-0.0608438,	0.0775,	-0.0873764,	-0.0163807,	0.0769824,	-0.0878419,	0.0286171,	0.0027986,	-0.035328,	-0.00433974,	0.0808487,	0.227254,	0.238982,	0.143668,	0.0772261,	-0.0291955,	-0.0608751,	-0.0951673,	-0.0937282,	-0.141393,	-0.148171,	-0.0149689,	0.129354,	0.158186,	0.0748445,	0.0411693,	-0.0391396,	-0.101255,	-0.0348484,	-0.0343698,	-0.0198575,	0.0388755,	-0.016659,	-0.00345016,	-0.0429921,	-0.00010433,	0.0213966,	-0.0583413,	0.0645742,	0.042872,	0.011877,	-0.119368,	-0.121608,	-0.138841,	-0.238516,	-0.224464,	-0.225087,	-0.221008,	-0.28514,	-0.11075,	-0.200126,	0.0376351,	0.0132441,	0.0311991,	-0.0384066,	-0.0201335,	-0.0188087,	-0.0485277,	0.0493915,	-0.0178236,	0.0245211,	0.0215609,	-0.04928,	0.0868643,	-0.0190324,	-0.0109874,	-0.102346,	0.0546123,	0.0389194,	-0.0545737,	-0.109514,	-0.0923774,	-0.199785,	-0.248054,	-0.129623,	-0.304069,	-0.221861,	-0.163572,	-0.252061,	-0.0996699,	0.0332038,	0.0208582,	-0.0696274,	-0.00477292,	-0.0411406,	-0.0221258,	0.0495406,	0.0841738,	-0.0804518,	0.0174134,	-0.0207987,	0.0197188,	-0.05982,	0.016469,	0.0293703,	0.00546735,	-0.0194454,	-0.0161364,	0.0154891,	-0.0965617,	-0.126173,	-0.0385933,	-0.0696061,	-0.0421978,	-0.0722845,	-0.0362264,	0.0512805,	0.0393359,	0.0852794,	0.0837162,	-0.0112447,	-0.00992139,	-0.00619835,	-0.069504,	-0.0624349,	-0.0770623,	-0.00854178,	0.0562052,	-0.0385061,	0.0121918,	0.0787479,	0.0213336,	-0.0786949,	0.0366059,	-0.0677845,	-0.0656155,	-0.00452243,	0.0281373,	0.0426012,	-0.055639,	0.0112658,	0.0251926,	-0.00649386,	-0.0509546,	-0.0672202,	0.0325677,	0.051165,	-0.041919,	0.0413731,	0.0334691,	0.00112992,	-0.0248607,	0.00605646,	0.0300957,	0.0439212,	-0.116181,
	0.0339426,	-0.0220702,	0.0272975,	-0.0388139,	0.0247358,	-0.0148556,	-0.0135904,	-0.0405522,	0.0255647,	0.0373811,	0.0312114,	0.00759,	-0.0266163,	-0.0153334,	-0.0872194,	0.0317948,	-0.0118772,	-0.050542,	-0.0111688,	-0.0587419,	-0.0415364,	-0.0266549,	-0.0475519,	0.00297951,	-0.043901,	0.0273485,	0.0373821,	0.0413998,	-0.0742432,	-0.066093,	-0.0640258,	-0.0264377,	-0.0498354,	-0.0404236,	-0.0394301,	0.000391236,	0.0349846,	-0.000411114,	0.0409393,	-0.0333822,	-0.089845,	0.0613856,	0.0359357,	-0.025379,	0.0649045,	0.0446613,	0.00416576,	0.0361091,	-0.0051313,	-0.0205699,	0.03733,	-0.000214872,	0.0304944,	-0.0160317,	-0.0583279,	0.0662377,	-0.062472,	0.00928213,	-0.0332,	0.0611216,	0.0201167,	-0.053408,	-0.000166605,	0.0256776,	0.0193029,	0.0372703,	-0.00768842,	0.0273598,	0.131835,	-0.0679077,	-0.033754,	0.104735,	-0.00622804,	-0.0219222,	0.0545223,	-0.0863491,	-0.00862718,	0.0294196,	-0.116473,	0.061848,	0.00274958,	0.0350843,	-0.0307163,	0.030606,	-0.0044196,	-0.0872085,	0.000958299,	0.0119785,	0.00302516,	0.0846703,	0.0387551,	0.000212134,	0.0114088,	0.165213,	0.361784,	0.222209,	0.220786,	0.2948,	0.276577,	0.149467,	0.143347,	0.0541659,	-0.0939577,	-0.119928,	-0.134951,	-0.00968499,	-0.040317,	-0.0673001,	-0.0117996,	0.00969062,	-0.0228914,	-0.0704196,	0.0221196,	-0.0162074,	-0.0215183,	0.0163719,	0.0203701,	0.00501508,	-0.0262351,	0.0165248,	0.137536,	0.17709,	0.230025,	0.316941,	0.326391,	0.259432,	0.189995,	0.0992573,	0.129351,	0.0767892,	0.00969236,	-0.0774229,	-0.137342,	-0.146934,	-0.272478,	-0.0914832,	-0.0970479,	-0.043873,	0.0290218,	0.0530463,	-0.0377842,	0.0334775,	0.0765935,	-0.00115239,	0.048709,	0.0137662,	0.139984,	0.13191,	0.267862,	0.264849,	0.217247,	0.344999,	0.134165,	0.28756,	0.189773,	0.185774,	0.308681,	0.252723,	0.185237,	-0.0320869,	-0.0376821,	-0.0519877,	-0.107633,	-0.182977,	-0.133623,	-0.0346377,	-0.0266518,	-0.0116109,	-0.0525421,	0.0375856,	0.0884363,	-0.0778395,	-0.0400558,	0.119936,	0.0663209,	0.158915,	0.130769,	0.189523,	0.1233,	0.106654,	0.0299405,	0.118812,	0.202456,	0.134907,	0.164613,	0.0775403,	-0.0172586,	0.0328641,	-0.0458648,	-0.101499,	-0.0893383,	-0.28887,	-0.101236,	0.00687505,	-0.0294015,	0.00695687,	-0.0707529,	-0.119848,	-0.0451741,	-0.0558759,	0.114165,	0.165493,	0.240887,	0.0350321,	0.151913,	0.0508134,	0.0277978,	0.125758,	0.0473963,	0.116693,	0.0256924,	-0.00925016,	-0.0314438,	-0.184072,	-0.0540034,	-0.097674,	-0.051633,	0.0324436,	-0.00500136,	-0.212383,	-0.156393,	-0.051004,	-0.0752106,	-0.00487273,	0.0235617,	0.0357938,	0.0464781,	0.0516516,	0.078553,	0.12936,	0.0403577,	0.1626,	0.0908246,	0.128227,	0.194877,	0.156907,	0.09255,	0.0418553,	0.204493,	0.0837876,	0.0205073,	0.0275088,	-0.0246104,	0.0966911,	-0.0187159,	-0.0831205,	-0.116986,	-0.159211,	-0.238621,	-0.039588,	0.0198663,	-0.0579283,	0.0416399,	-0.0294478,	-0.0297411,	0.0901028,	0.0412203,	0.156188,	0.115787,	0.0131403,	-0.00594033,	0.127433,	0.225783,	0.0857346,	0.06063,	0.0572505,	0.166852,	0.143013,	0.138609,	0.0415182,	0.0447087,	0.0590883,	0.00244549,	-0.0312862,	-0.0988282,	-0.0990853,	-0.21794,	-0.0222858,	-0.0101591,	-0.0314825,	-0.0345302,	0.0380551,	-0.0364392,	-0.0473175,	0.0776545,	0.16912,	-0.0142333,	-0.14522,	-0.0831041,	-0.144361,	-0.16405,	-0.178563,	-0.283802,	-0.293594,	-0.127917,	-0.132898,	-0.0336242,	0.0984989,	0.0223335,	-0.049787,	-0.029851,	-0.11826,	-0.0294657,	-0.0840847,	-0.0498474,	-0.00895268,	-0.0234304,	-0.00134114,	-0.0194537,	-0.018457,	-0.0788469,	-0.0527042,	-0.0514789,	-0.165837,	-0.251507,	-0.443621,	-0.52269,	-0.528721,	-0.596571,	-0.612802,	-0.587658,	-0.711056,	-0.545445,	-0.378811,	-0.152392,	-0.249739,	-0.109807,	0.148545,	-0.107229,	-0.00144446,	-0.0585702,	-0.107017,	-0.111709,	0.0280393,	0.0725284,	0.0386438,	0.0325071,	-0.0453197,	-0.0375704,	-0.009366,	-0.110075,	-0.49817,	-0.676862,	-0.784388,	-0.51885,	-0.538965,	-0.535217,	-0.488018,	-0.569972,	-0.534548,	-0.388365,	-0.392091,	-0.345908,	-0.203438,	-0.158113,	0.0217565,	-0.0406313,	-0.0462215,	-0.129877,	-0.275263,	0.0325663,	0.11291,	0.0610406,	-0.000839823,	0.0749492,	0.0675602,	-0.0290062,	-0.0810498,	-0.21738,	-0.553054,	-0.746851,	-0.65784,	-0.431878,	-0.222897,	-0.11723,	-0.131349,	-0.0295766,	0.00187111,	-0.0556097,	-0.192889,	-0.264357,	-0.153833,	-0.16525,	-0.0417324,	-0.0460386,	-0.0259258,	-0.110864,	-0.139381,	0.0655852,	0.154927,	0.138032,	0.0255831,	0.0438275,	-0.0745029,	-0.0495325,	0.0553002,	-0.140388,	-0.388042,	-0.357204,	-0.0710969,	-0.0191609,	-0.0690948,	0.0670196,	0.107765,	0.0780912,	0.249645,	0.0817536,	-0.124149,	-0.102279,	-0.191927,	-0.0599639,	-0.0349485,	-0.0774234,	-0.128886,	-0.182301,	-0.045249,	0.147883,	0.16915,	0.0170528,	-0.0622509,	-0.00928234,	-0.175145,	-0.0208728,	0.0444041,	-0.0352069,	0.00183831,	0.146653,	0.045002,	0.0897354,	0.121533,	0.0571543,	-0.0305477,	0.0425806,	0.20656,	0.0907768,	0.0606718,	-0.0236516,	-0.162658,	-0.105693,	-0.0942288,	-0.21674,	-0.135372,	-0.17762,	-0.0462395,	0.276037,	0.357788,	0.114324,	-0.00465837,	0.0294999,	-0.030036,	-0.0268541,	0.126986,	0.197116,	0.209251,	0.174675,	0.121482,	-0.0292485,	0.0787442,	0.0642055,	0.211468,	0.259908,	0.361108,	0.0881846,	-0.022898,	0.11632,	0.105474,	-0.0457236,	-0.0312735,	-0.0780749,	0.015671,	0.128006,	0.208253,	0.35741,	0.461964,	0.227292,	0.0877394,	-0.00137341,	0.0581764,	-0.00443987,	0.148733,	0.26625,	0.258403,	0.206461,	0.0486326,	0.034148,	0.107641,	0.250728,	0.172397,	0.327961,	0.207307,	0.0537127,	0.111392,	0.0164087,	-0.0111064,	0.0697498,	0.005381,	0.0442011,	0.169066,	0.192981,	0.193936,	0.449054,	0.456723,	0.157538,	-0.0241821,	-0.0257571,	0.00842488,	0.0103732,	0.0978616,	0.335325,	0.358714,	0.206526,	0.282768,	0.114804,	0.111732,	0.191915,	0.164804,	0.254724,	0.253049,	0.137037,	0.00201211,	0.166933,	0.201492,	0.152527,	0.0862768,	0.0470697,	0.131093,	0.208922,	0.351284,	0.471221,	0.355014,	0.0783404,	0.0273689,	0.0285453,	0.045977,	0.0147669,	-0.02086,	0.254358,	0.237682,	0.141109,	0.294584,	0.270151,	0.147857,	0.250125,	0.265677,	0.253782,	0.23082,	0.174897,	0.0451938,	0.146466,	0.112235,	0.00157463,	-0.0150184,	0.0919451,	0.170917,	0.263936,	0.290511,	0.389407,	0.196692,	0.071064,	-0.0948296,	-0.0511828,	0.0815283,	0.0598347,	-0.0204412,	0.211297,	0.207943,	0.242184,	0.274635,	0.255361,	0.257097,	0.228947,	0.259042,	0.276503,	0.0520196,	0.0353544,	0.120734,	0.0680833,	0.0299347,	0.187445,	0.198486,	0.190884,	0.23653,	0.25989,	0.218306,	0.238487,	0.136573,	-0.00836905,	-0.0312501,	0.0542663,	-0.00364416,	0.056178,	0.0927679,	0.0221715,	0.124104,	0.129343,	0.167432,	0.11121,	0.222411,	0.181025,	0.182003,	0.0447863,	0.0772876,	0.0362039,	-0.0443039,	0.0580623,	0.109753,	0.303478,	0.272262,	0.0657547,	0.2205,	0.240483,	0.323372,	0.182683,	0.125006,	0.0381032,	0.0365993,	-0.0118335,	0.0143584,	0.109212,	0.130839,	0.0720771,	0.146461,	0.172987,	0.173605,	0.218089,	0.282364,	0.174607,	0.18403,	0.130978,	0.0543296,	-0.0759655,	-0.00588925,	-0.0868815,	0.0453781,	0.215672,	0.214713,	0.241817,	0.201203,	0.314495,	0.186772,	0.0576629,	0.0476892,	-0.0798982,	0.0551487,	0.076774,	0.10443,	-0.105964,	-0.0449339,	0.00348553,	-0.0277652,	-0.0448408,	0.102111,	0.0734195,	0.0885611,	-0.016982,	0.109808,	0.0528698,	-0.0301316,	0.0071922,	-0.0563949,	-0.0581466,	0.0670128,	0.203161,	0.241072,	0.195546,	0.287994,	0.153856,	0.0911739,	0.131593,	0.0817126,	0.0103411,	0.0115921,	-0.017062,	-0.0643019,	-0.0576424,	0.0229982,	-0.0442499,	-0.0867189,	-0.24986,	-0.265763,	-0.169286,	-0.127834,	-0.172161,	-0.152979,	-0.027585,	0.0253164,	0.171184,	-0.0390486,	0.00393862,	0.0111941,	0.113444,	0.0615651,	-0.0613858,	0.0701613,	-0.0337011,	0.00796555,	0.131537,	-0.0571455,	-0.0304619,	-0.037184,	0.00419057,	0.029006,	-0.0020003,	-0.0683668,	-0.000734865,	-0.00183464,	-0.19078,	-0.101307,	-0.134132,	-0.173351,	-0.200778,	-0.158397,	-0.199398,	-0.0971166,	-0.13167,	0.0297098,	-0.032857,	-0.126844,	-0.147606,	-0.192038,	-0.036386,	-0.020872,	-0.086544,	-0.132701,	-0.0102319,	0.0365951,	0.0451804,	0.150942,	-0.0185095,	-0.00158846,	0.0187126,	0.065837,	0.00213878,	-0.065287,	0.019958,	-0.0603615,	0.0503994,	-0.0476923,	-0.0112425,	-0.0516413,	-0.0801763,	-0.00137026,	-0.0317678,	-0.0161524,	-0.0681794,	-0.093668,	-0.0170071,	-0.0358991,	-0.077255,	-0.0505579,	-0.104701,	-0.0450972,	-0.00869607,	-0.00767142,	-0.0655188,	0.00652695,	-0.00401389,	-0.0346803,	-0.0424506,	-0.114132,	-0.0135313,	0.00154588,	-0.0248974,	0.0937059,	0.0157747,	0.0239102,	0.0464916,	-0.0108513,	0.0767606,	-0.153465,	-0.0529494,	0.00657037,	-0.00840098,	0.0844609,	-0.0777495,	0.0544159,	0.00397118,	-0.104885,	-0.0667269,	-0.0734432,	0.0254562,	-0.0411193,	0.0318445,	-0.0227563,
	-0.0521651,	0.0835123,	-0.0361058,	-0.0227401,	0.0433177,	0.062938,	-0.062438,	-0.0351793,	-0.13069,	-0.0850534,	0.00699429,	-0.0744093,	0.0587964,	-0.0413836,	0.0123867,	0.034177,	-0.0275844,	-0.0621318,	-0.0369046,	0.0186996,	0.0132867,	0.0943874,	-0.001167,	0.0399912,	0.000831897,	-0.00514653,	0.032994,	0.0123315,	0.122329,	0.0305791,	-0.0299328,	-0.0203632,	0.044893,	-0.0737173,	0.0543988,	0.0521595,	-0.0429177,	-0.00226912,	0.0594322,	0.0810929,	0.0378652,	0.0864874,	0.00440595,	0.0276035,	-0.00174055,	0.0311082,	-0.0212456,	0.012465,	-0.0466736,	0.00529278,	-0.0234267,	-0.0280692,	0.0977457,	-0.0131784,	0.0503244,	0.0491331,	0.000490578,	0.0255656,	-0.0309304,	0.0391987,	0.0279233,	-0.103269,	-0.0097356,	-0.0478376,	-0.00462878,	0.0583081,	0.0190064,	-0.00794893,	-0.0201859,	-0.0404342,	-0.0246411,	0.00308333,	-0.163984,	-0.00720501,	-0.0717471,	-0.00721324,	0.0125988,	-0.0799117,	-0.017091,	-0.0666659,	0.0129771,	0.0553713,	-0.0137773,	-0.0928841,	0.0202117,	0.0425312,	0.000249965,	-0.017329,	-0.0522372,	0.0483912,	0.0359336,	0.0594903,	-0.0161553,	0.138262,	0.0622803,	0.113079,	0.229919,	0.229595,	0.271373,	0.267243,	0.179126,	0.146117,	0.137122,	0.0551926,	0.0453297,	0.0118948,	0.007955,	-0.0269992,	-0.0226007,	0.0446483,	-0.0164958,	0.00327174,	0.0727984,	-0.0688384,	-0.0169562,	-0.103798,	-0.101207,	0.0689556,	0.0851228,	0.255279,	0.15207,	0.279705,	0.283864,	0.352904,	0.392521,	0.295875,	0.302088,	0.283636,	0.207093,	0.132625,	0.194015,	0.142966,	-0.0811375,	-0.181753,	-0.0291348,	-0.125787,	0.000892833,	-0.0446973,	0.0361574,	-0.0220804,	0.0418225,	-0.00925373,	0.0311357,	0.0292591,	0.15274,	0.126871,	0.247733,	0.21522,	0.251474,	0.180867,	0.189601,	0.149833,	0.137514,	0.0840486,	0.107431,	0.130475,	0.238632,	0.000798571,	0.0539845,	-0.000158746,	-0.0858979,	-0.206357,	-0.159192,	-0.212415,	-0.00763232,	0.03922,	0.0237141,	-0.071429,	-0.0651918,	0.0437011,	0.0132542,	0.0734321,	0.0768902,	0.250344,	0.246615,	0.145935,	0.12876,	0.214503,	0.244455,	0.0981669,	0.166352,	0.211395,	0.149269,	0.173116,	0.193398,	0.124349,	0.134025,	-0.0209874,	0.00820908,	-0.00955937,	-0.0401829,	-0.188489,	-0.0563845,	-0.0577419,	0.0347499,	0.0699811,	0.059907,	0.0408704,	-0.0542877,	0.0724547,	0.300461,	0.225988,	0.218114,	0.0646728,	0.0618836,	0.113929,	0.051432,	-0.00817434,	0.179445,	0.19724,	0.198932,	0.252777,	0.053703,	0.132324,	0.0579399,	0.104485,	0.0469912,	0.007918,	-0.168134,	-0.190083,	-0.136477,	-0.0390411,	0.0676549,	-0.103023,	-0.0137614,	0.0400797,	0.0181885,	0.109653,	0.271207,	0.176991,	0.137643,	0.0268117,	0.051577,	-0.0367487,	0.12533,	0.0494054,	0.0784357,	0.148467,	0.202515,	0.230146,	0.102161,	0.123777,	0.2323,	0.127371,	0.207726,	0.0992595,	-0.0117146,	-0.198467,	-0.222811,	0.0139106,	0.0039456,	0.00442566,	-0.00965394,	-0.0633405,	-0.100877,	0.0559253,	0.226233,	0.0768715,	0.176546,	0.0656015,	0.0437095,	-0.0424277,	-0.141016,	-0.382186,	-0.254126,	-0.0783452,	0.224725,	0.290888,	0.231831,	0.177681,	0.136654,	0.0898463,	0.237535,	0.146944,	0.122062,	-0.205683,	-0.116515,	-0.0456424,	0.0730881,	0.0226101,	0.0948623,	0.0226566,	-0.0121011,	0.0529031,	0.145118,	-0.0363325,	-0.0663919,	-0.140263,	-0.216931,	-0.46351,	-0.410529,	-0.468748,	-0.405477,	-0.0274473,	0.198711,	0.170982,	0.227757,	0.178177,	0.100701,	0.179217,	0.320482,	0.11452,	-0.0276853,	-0.049652,	-0.0964509,	-0.0122855,	-0.10954,	-0.0320546,	0.0385251,	-0.0797149,	-0.00305517,	0.085058,	0.0632091,	-0.15124,	-0.243962,	-0.424048,	-0.461786,	-0.372973,	-0.278552,	-0.17422,	-0.119149,	0.105555,	0.180859,	0.125532,	0.0210473,	0.21528,	0.149317,	0.220322,	0.141121,	-0.10532,	-0.0593589,	-0.210407,	-0.0806171,	-0.025938,	-0.0424015,	-0.00533242,	0.0562695,	0.0320071,	0.0593732,	-0.0283123,	-0.0118639,	-0.22171,	-0.307428,	-0.411432,	-0.277955,	-0.14211,	-0.0520914,	-0.0674089,	0.0412947,	0.12162,	0.356358,	0.119221,	0.0709678,	0.0966057,	0.109113,	-0.038305,	-0.105086,	-0.316724,	-0.209134,	-0.144507,	-0.068169,	0.0375109,	-0.0513154,	-0.0328662,	-0.0136367,	-0.0567874,	-0.0425788,	0.000408577,	-0.127972,	-0.25327,	-0.389886,	-0.298477,	-0.269113,	-0.0308671,	-0.0631994,	-0.139247,	0.0481988,	0.233193,	0.173115,	-0.0289789,	0.0388502,	0.0542619,	0.121773,	-0.0941944,	-0.27859,	-0.330025,	-0.346265,	-0.116669,	-0.0478979,	-0.0403337,	-0.00587005,	-0.0824004,	-0.00201957,	0.0136677,	-0.0151563,	0.0170126,	0.0383883,	-0.183036,	-0.386108,	-0.290996,	-0.183465,	-0.100256,	-0.15253,	-0.180166,	0.0719582,	0.124486,	0.182719,	0.0642293,	-0.045189,	0.063265,	0.0200915,	-0.0324661,	-0.127045,	-0.0690968,	-0.171644,	-0.0563193,	-0.0597049,	0.060507,	-0.00709589,	-0.0197394,	-0.0739323,	-0.0417313,	0.00444775,	0.0408907,	-0.00878725,	-0.122093,	-0.121205,	-0.285457,	-0.25249,	-0.24964,	-0.119414,	-0.0359917,	0.118372,	0.121782,	0.148057,	0.0914646,	-0.0835788,	-0.0465149,	-0.0480093,	0.0397529,	0.152462,	0.0885608,	0.080695,	0.0205068,	0.00572559,	0.0424879,	0.0490643,	-0.0852542,	-0.0123655,	-0.0238183,	0.0443684,	0.0396863,	0.0789183,	0.0498712,	-0.239386,	-0.176583,	-0.321285,	-0.255732,	-0.257323,	-0.0748084,	0.0551419,	0.203813,	-0.0741435,	-0.15545,	-0.18953,	-0.0247843,	0.155747,	0.260214,	0.123689,	0.0821692,	0.236793,	0.154825,	0.0901248,	-0.107881,	0.00102702,	-0.0611057,	-0.106218,	-0.0540873,	0.0527997,	0.229273,	0.237321,	0.026784,	-0.0689426,	-0.266013,	-0.249207,	-0.391718,	-0.603048,	-0.48097,	-0.293113,	-0.166173,	-0.296571,	-0.136994,	0.0342161,	0.14646,	0.0484855,	0.25451,	0.0883722,	0.198783,	0.301649,	0.146476,	-0.023166,	0.00717965,	-0.0716575,	0.0388259,	-0.00243893,	0.0681995,	-0.0132774,	0.295999,	0.295544,	0.250611,	0.068792,	-0.151371,	-0.179055,	-0.408448,	-0.442924,	-0.412266,	-0.478523,	-0.372914,	-0.332841,	0.00310761,	0.265575,	0.208872,	0.295991,	0.163668,	0.120717,	0.262998,	0.340814,	0.127183,	-0.0687805,	-0.0902917,	0.010857,	0.00430467,	0.00112757,	0.0385639,	0.0429459,	0.271207,	0.294985,	0.344571,	0.228582,	0.00938089,	0.00380165,	-0.0649198,	-0.168065,	-0.224186,	-0.289232,	-0.165152,	-0.0322822,	0.0610523,	0.157606,	0.261106,	0.150556,	0.168991,	0.0935681,	0.315672,	0.101717,	0.0216744,	-0.141436,	0.000209729,	0.00609893,	0.0464441,	0.0686931,	-0.0124082,	0.0433713,	0.205112,	0.312828,	0.268781,	0.236659,	0.191128,	0.196973,	0.162389,	-0.0598151,	-0.0526826,	-0.256844,	-0.113574,	0.0534915,	-0.00183304,	0.141499,	0.163978,	0.216104,	0.13563,	0.247274,	0.171066,	0.0630984,	-0.0344737,	-0.00787348,	0.0121652,	0.0494082,	-0.055126,	-0.0434001,	-0.0531021,	0.0432956,	0.115914,	0.221274,	0.0906895,	0.226565,	0.0770974,	0.14603,	-0.0282422,	-0.0650979,	-0.127075,	-0.0436264,	-0.0737562,	0.00907347,	-0.0198538,	0.069054,	0.0485757,	0.220474,	0.189256,	0.0385763,	-0.0727198,	-0.0459751,	-0.148787,	0.0181542,	-0.100666,	-0.125957,	-0.0617076,	0.00253822,	0.042031,	0.0964412,	0.162925,	0.180918,	0.0965848,	0.0976303,	0.0904997,	-0.0626925,	-0.0182752,	0.132031,	0.0584767,	-0.0585026,	-0.0269713,	0.00702565,	0.123513,	0.0217317,	0.0917213,	0.157565,	0.164216,	0.0509162,	0.0322492,	-0.0832201,	-0.0478526,	-0.0160854,	-0.0332004,	0.0101057,	0.0199774,	-0.0179225,	-0.000242617,	0.0970836,	0.142613,	0.217572,	0.241577,	0.298695,	0.135229,	0.0638133,	0.0951914,	0.122139,	0.12222,	-0.0271944,	0.0358199,	-0.00711749,	0.00963199,	-0.0463845,	0.0233989,	0.153834,	0.0639555,	-0.0588749,	-0.185205,	-0.0595403,	-0.0880308,	-0.039792,	-0.00296545,	0.0354228,	0.033293,	0.0183418,	0.0600686,	-0.0732799,	0.0904999,	0.0337406,	0.156344,	0.308027,	0.37771,	0.233773,	0.263589,	0.295469,	0.293761,	0.242312,	0.289311,	0.251709,	0.132994,	0.136827,	0.114445,	-0.0660383,	-0.093028,	-0.0498516,	-0.0352931,	-0.00178754,	0.0648885,	-0.069145,	-0.043558,	-0.0273064,	0.00366394,	0.0270825,	0.0324837,	-0.0377009,	0.112299,	-0.0109162,	0.114589,	0.0280521,	0.23951,	0.256019,	0.364907,	0.402835,	0.360964,	0.475153,	0.280565,	0.289307,	0.0969779,	0.00507518,	-0.0398954,	-0.0113733,	0.0198469,	-0.0651996,	-0.0243367,	0.0496047,	-0.0717946,	0.112418,	-0.0264686,	0.0668123,	-0.0360604,	-0.0351702,	-0.0614504,	-0.000365893,	0.0581254,	-0.113989,	0.0746465,	0.0728764,	0.0284754,	0.0738502,	-0.115198,	0.0274525,	0.0360539,	-0.105546,	-0.154821,	-0.028965,	-0.0171445,	0.0023422,	-0.0502247,	0.0327198,	0.00437313,	-0.0186284,	0.0348517,	0.0180567,	0.0237374,	-0.074384,	0.00539835,	0.117364,	0.0433728,	-0.0107341,	0.0319997,	0.0392983,	-0.0342277,	-0.0449292,	-0.0258327,	0.0242704,	-0.0499922,	0.0256383,	-0.0483219,	0.0109689,	0.0412689,	-0.0968426,	-0.0627596,	0.0763512,	-0.059266,	-0.0117802,	0.00287055,	-0.0333471,	-0.00338929,	-0.0168358,	0.0140844,	-0.0235728,	0.0741327,	-0.0278335,	0.0192616,	-0.00223761,	0.0357612,
	0.00810522,	0.0376298,	-0.0773115,	-0.00465878,	-0.03028,	0.0174031,	0.0266376,	0.0972302,	0.0610118,	0.0435455,	0.0685281,	0.0240434,	-0.0484309,	0.0126059,	0.0450104,	-0.0179537,	-0.0577621,	-0.0581955,	0.0902097,	0.0505512,	0.0154557,	0.0362094,	0.0037,	-0.0403026,	-0.0402204,	0.00888765,	-0.0509706,	0.0141882,	0.0163483,	0.0964324,	-0.022617,	-0.0582059,	-0.105256,	-0.0854674,	0.0915791,	0.00392613,	-0.0220917,	-0.000461857,	0.0305719,	-0.0517154,	-0.0576116,	0.0424469,	0.0580589,	-0.00677225,	-0.0527938,	-0.0404287,	-0.0272415,	-0.0344146,	-0.0240936,	-0.0960599,	0.0324979,	-0.00185325,	0.0281371,	-0.0731537,	0.0104032,	-0.0121664,	0.030337,	-0.00701909,	-0.0469455,	0.0823939,	0.0291973,	0.00655371,	0.00875248,	-0.15569,	-0.10224,	-0.133313,	-0.0720876,	-0.144004,	-0.130458,	-0.202984,	-0.107106,	-0.141771,	-0.0947733,	-0.130132,	-0.140385,	-0.0224382,	-0.0263164,	-0.00313699,	-0.0903063,	-0.105822,	0.00922582,	-0.0659808,	-0.00618115,	0.07694,	0.0419623,	0.0360757,	0.0260803,	-0.01767,	0.0190379,	-0.12719,	-0.0289142,	-0.0889395,	-0.118978,	-0.0242901,	-0.270071,	-0.135656,	-0.265021,	-0.371661,	-0.270497,	-0.201769,	-0.227447,	-0.213979,	-0.0365102,	-0.00691855,	-0.00321054,	0.0556934,	0.0826607,	0.0445671,	-0.0687841,	0.10215,	-0.049261,	0.0597766,	-0.0816421,	-0.0314782,	0.0158964,	-0.109075,	0.0510103,	0.0376833,	-0.106801,	0.0358279,	-0.0634181,	-0.167776,	-0.309797,	-0.210353,	-0.306893,	-0.242753,	-0.288887,	-0.2715,	-0.257305,	-0.10592,	-0.12425,	-0.0156273,	0.0998524,	0.115845,	0.149961,	0.149827,	0.137867,	0.0612909,	-0.0842999,	-0.0208296,	-0.146311,	-0.0253652,	0.0151567,	-0.0104031,	0.00787975,	0.00503057,	0.160779,	0.0278779,	-0.030039,	-0.0106023,	-0.12273,	-0.11375,	-0.164028,	-0.234839,	-0.104941,	-0.0798558,	0.00103829,	0.0335198,	0.0296067,	0.057193,	0.105316,	0.0800153,	0.182643,	0.243961,	0.0698233,	0.00228392,	-0.0883359,	0.0644699,	0.0621372,	-0.0287046,	0.00442493,	0.010308,	-0.000760023,	0.0412118,	0.0410883,	-0.00415331,	0.0471356,	0.00857495,	-0.0507692,	-0.142092,	-0.236552,	-0.253229,	-0.280082,	-0.231132,	-0.263006,	-0.195177,	-0.218433,	-0.0387835,	-0.0363195,	0.0297732,	0.264937,	0.391089,	0.142247,	-0.0117596,	0.0537944,	0.0483099,	0.0277969,	0.0708889,	-0.0376228,	0.0360561,	-0.0227363,	0.0782751,	0.0501753,	0.0142846,	0.0505141,	-0.027435,	-0.110407,	-0.197444,	-0.309866,	-0.27831,	-0.397862,	-0.395235,	-0.433085,	-0.209727,	-0.23085,	-0.0433214,	-0.119122,	0.0036188,	0.135223,	0.220568,	0.129053,	0.0290824,	0.0242752,	0.0067378,	-0.00109398,	0.0211224,	0.0111076,	0.0422583,	0.049244,	0.022055,	-0.0925278,	-0.027625,	-0.168893,	-0.0789602,	-0.103918,	-0.120132,	-0.31442,	-0.223819,	-0.38434,	-0.408767,	-0.337124,	-0.169011,	-0.0566787,	-0.142976,	-0.110572,	-0.00996512,	0.0614973,	0.148851,	-0.0294231,	-0.00569717,	0.0474669,	0.0368033,	-0.0247658,	-0.0571051,	0.0641194,	-0.056922,	-0.0508353,	0.0586037,	-0.0925545,	-0.0265212,	-0.051367,	-0.0289556,	-0.137851,	-0.0818126,	-0.0795331,	-0.136636,	-0.624463,	-0.465234,	-0.0793062,	-0.0801561,	0.00215283,	0.0207128,	-0.0128586,	-0.147358,	-0.083278,	-0.151934,	-0.183723,	-0.0921429,	0.0506809,	-0.0477079,	0.0716399,	-0.0359047,	-0.00275104,	-0.1135,	-0.0697447,	-0.0236411,	-0.127813,	-0.0693283,	0.00387324,	0.0959744,	0.0870406,	0.0652134,	0.0901299,	-0.258736,	-0.606778,	-0.149812,	0.00148112,	0.0707066,	-0.0367928,	0.00717175,	-0.151395,	-0.135418,	-0.0825014,	-0.263802,	-0.219685,	-0.111976,	0.0158446,	-0.0138142,	-0.0176376,	0.0286912,	-0.0264466,	-0.0108411,	-0.138891,	-0.106288,	-0.0184576,	0.067282,	0.18753,	0.336284,	0.278348,	0.315912,	0.236595,	-0.431906,	-0.629836,	-0.061677,	0.322794,	0.148257,	-0.0508234,	-0.0345342,	-0.0510408,	-0.104491,	-0.21624,	-0.250706,	-0.282859,	-0.0650062,	-0.017464,	0.0517785,	0.0283333,	0.0588923,	-0.0440577,	-0.150901,	-0.157417,	-0.0775826,	0.0347979,	0.241368,	0.316593,	0.246543,	0.495832,	0.470847,	0.529835,	-0.113417,	-0.290721,	0.191332,	0.348249,	0.0893356,	0.00133928,	0.072605,	-0.0512743,	0.0803014,	0.0189547,	-0.151061,	-0.0293888,	0.00938668,	0.0746993,	-0.0245707,	-0.0378844,	-0.0339558,	-0.0310934,	-0.0256165,	-0.0638228,	0.147324,	0.301714,	0.319209,	0.374143,	0.350768,	0.388029,	0.644604,	0.367093,	-0.188559,	-0.0792853,	0.224933,	0.288279,	0.362114,	0.105261,	0.170426,	0.291248,	0.195134,	0.0161069,	-0.0162413,	-0.133578,	0.0550375,	-0.0769889,	-0.0167881,	-0.0521878,	-0.0298402,	0.024503,	0.00540238,	0.036852,	0.293475,	0.244164,	0.365244,	0.233696,	0.275708,	0.313077,	0.407797,	0.252815,	-0.109033,	0.00107956,	0.143433,	0.375042,	0.473977,	0.283033,	0.335428,	0.148744,	0.0795074,	0.0461096,	-0.0441699,	-0.152193,	0.0417714,	-0.0397414,	-0.0296257,	-0.0377735,	0.0530643,	-0.0227094,	0.127932,	0.143063,	0.138596,	0.268896,	0.271416,	0.376189,	0.323336,	0.245642,	0.287426,	0.153959,	-0.0726173,	0.031885,	0.351444,	0.430002,	0.304954,	0.15624,	0.164546,	-0.0140404,	-0.046448,	-0.0215089,	-0.0924722,	-0.185012,	-0.0257833,	0.0221886,	0.0827624,	-0.00665403,	0.00357516,	-0.0201743,	-0.0335414,	-0.0204643,	0.027029,	0.0853335,	0.255145,	0.354638,	0.210075,	-0.0173791,	0.0189037,	0.0293383,	0.132369,	0.273078,	0.496545,	0.414575,	0.33023,	0.161179,	0.0461043,	0.15537,	-0.0307218,	-0.114116,	-0.0180284,	-0.200833,	-0.094171,	-0.094358,	-0.097511,	0.0466706,	-0.00451327,	-0.0251084,	-0.0841829,	-0.116916,	-0.193209,	-0.0551683,	0.0875356,	0.222291,	0.0928859,	-0.0262182,	-0.107212,	0.0411378,	0.251707,	0.496052,	0.412049,	0.394086,	0.139597,	0.180013,	-0.112885,	-0.0380296,	-0.0297218,	-0.188911,	-0.0956664,	-0.166112,	-0.0663361,	0.0218481,	-0.119552,	0.045553,	0.0686409,	0.111274,	-0.101483,	-0.232048,	-0.0745615,	-0.114602,	-0.0354711,	0.0122158,	0.0404629,	-0.292093,	-0.171522,	0.1438,	0.223223,	0.208592,	0.202687,	0.113193,	-0.130715,	-0.105969,	-0.0846562,	-0.0851543,	-0.131175,	-0.236871,	-0.176577,	-0.146598,	0.0350098,	-0.0492397,	-0.023554,	-0.0303539,	0.0401428,	0.0822659,	-0.123008,	-0.0812576,	-0.116774,	-0.170758,	-0.13969,	-0.397512,	-0.314474,	-0.37277,	-0.266146,	-0.305137,	-0.162636,	0.126474,	0.0654225,	0.0131839,	-0.177946,	-0.210362,	-0.240434,	-0.191183,	-0.141262,	-0.321424,	-0.169655,	-0.160978,	0.0472363,	-0.0419506,	-0.103112,	0.0338213,	-0.0259708,	-0.0457409,	0.0533698,	-0.00182523,	-0.172123,	-0.242896,	-0.3787,	-0.46984,	-0.398738,	-0.412396,	-0.296182,	-0.212423,	-0.366318,	-0.136584,	-0.0374034,	-0.0560969,	0.0182667,	-0.0179543,	-0.129905,	-0.18287,	0.0250692,	-0.132131,	-0.18741,	0.0168324,	-0.0236031,	0.0409939,	-0.022683,	0.0129063,	-0.0597062,	-0.0399381,	-0.0532343,	-0.108277,	-0.151423,	-0.166817,	-0.261528,	-0.212021,	-0.27515,	-0.196586,	0.00669648,	-0.208096,	-0.099835,	-0.0881386,	0.0402477,	-0.0702919,	0.047859,	-0.148382,	0.00943649,	0.0431793,	-0.073808,	-0.0656502,	0.0340954,	-0.0441215,	0.0543569,	0.00275616,	-0.0886686,	-0.023953,	-0.0972492,	0.0267718,	-0.064276,	-0.0139123,	-0.182555,	-0.125606,	-0.20586,	-0.182392,	-0.158869,	-0.173673,	-0.0528285,	-0.0671501,	-0.07601,	-0.0246403,	-0.0419574,	-0.0366008,	-0.00168816,	0.102133,	0.156115,	0.114154,	0.121888,	-0.113019,	-0.0650756,	-0.082325,	-0.029008,	-0.028846,	0.044911,	0.0526196,	0.0402985,	0.0201356,	0.00598432,	-0.0653861,	-0.0237149,	-0.0040461,	-0.0210502,	-0.0821271,	0.0177358,	0.0387526,	0.0657055,	-0.0736416,	-0.0280789,	0.0246424,	0.0301578,	0.092531,	0.125332,	0.191965,	0.221347,	0.153253,	0.179886,	0.00966564,	-0.0129862,	-0.0408666,	-0.0934369,	-0.0122361,	-0.0316943,	0.0221982,	0.0653129,	0.0115044,	0.0345912,	0.0635709,	-0.0433515,	-0.0369116,	-0.0381687,	-0.0907425,	-0.0101714,	-0.0554519,	0.0210376,	-0.0496898,	-0.151265,	-0.0699656,	-0.0251461,	-0.0187197,	-0.0690318,	-0.0085922,	0.00682619,	0.0362602,	0.0550442,	0.024192,	0.0900861,	-0.0479922,	0.0246166,	-0.0816711,	-0.0305151,	-0.0183844,	-0.0655865,	-0.0456292,	0.00104708,	-0.00300311,	-0.0173188,	-0.0774214,	-0.0804986,	-0.152303,	-0.223987,	-0.132401,	-0.252466,	-0.404881,	-0.225772,	-0.285694,	-0.230203,	-0.30093,	-0.34409,	-0.291166,	-0.150846,	-0.147383,	-0.120246,	-0.04748,	-0.0309444,	-0.00025649,	-0.0240681,	-0.0166998,	0.0824034,	0.0578601,	0.00229967,	-0.073975,	0.0127213,	0.00542718,	-0.0558771,	-0.122934,	-0.0916707,	-0.150594,	-0.19477,	-0.2819,	-0.243297,	-0.149914,	-0.240239,	-0.274061,	-0.192334,	-0.313199,	-0.257223,	-0.201815,	-0.145575,	-0.0422589,	-0.0328664,	-0.00890332,	-0.0679682,	0.015034,	0.0578749,	0.0137389,	0.0981498,	-0.0175453,	-0.0128407,	-0.00484436,	-0.0754952,	0.0358357,	0.0836221,	-0.0352815,	0.00565591,	0.0702389,	-0.0652848,	-0.0516091,	-0.0269604,	-0.0170682,	-0.0775888,	-0.119979,	0.0290725,	-0.0697111,	-0.00592643,	0.00586812,	-0.0786968,	0.00828046,	0.0010309,	-0.0240732,	-0.0305941,	-0.0935637,	0.0413735,	-0.0280847,	-0.138519,
	-0.0312161,	-0.0568717,	-0.0111107,	0.0175864,	-0.0669672,	-0.0254296,	-0.0316038,	-0.022584,	-0.0137423,	-0.0740081,	-0.0542606,	0.0452458,	0.0782119,	0.0796698,	-0.0109522,	0.120766,	0.0164015,	-0.0441799,	0.029221,	-0.0457771,	0.0549584,	-0.0294358,	0.0512685,	0.0253495,	-0.0378746,	-0.0134728,	-0.0156087,	-0.0230668,	-0.0257586,	-0.0539007,	0.00948976,	0.0268113,	-0.0371896,	-0.053317,	0.0463966,	-0.0522678,	0.0167916,	0.0314878,	-0.0635348,	-0.0556223,	0.054815,	-0.070354,	0.00587823,	-0.0370089,	-0.0781304,	-0.0138089,	0.00540876,	0.0374339,	-0.0614778,	0.0726808,	-0.0127613,	-0.0360698,	0.017568,	0.0762144,	-0.116617,	0.0350076,	0.0707783,	-0.0270884,	0.0125285,	-0.0138256,	0.0507901,	-0.013892,	0.0018806,	0.0464484,	-0.0489037,	-0.0345986,	-0.0712742,	0.0374453,	-0.0351584,	0.0047519,	-0.0648329,	-0.0580614,	-0.0756522,	0.0100141,	0.00894309,	-0.0248659,	-0.00601526,	-0.105347,	-0.134716,	0.00687074,	-0.0466586,	0.161194,	0.00722246,	0.00349582,	-0.0682099,	0.0057339,	0.0602179,	-0.0708785,	-0.0256825,	-0.0557146,	-0.168374,	-0.110779,	-0.0192966,	-0.0167899,	-0.00675532,	-0.0711708,	0.0180426,	-0.0242606,	-0.0561166,	-0.000520957,	-0.0755314,	0.0668573,	0.112958,	0.0459738,	0.112844,	0.0987563,	0.0705544,	-0.0180612,	0.050044,	0.128707,	0.0170892,	0.0195778,	0.027223,	-0.0407281,	0.0121142,	0.0110765,	-0.0738948,	-0.0199744,	-0.167637,	-0.121709,	-0.207314,	-0.129033,	-0.0919152,	-0.0345938,	-0.0417059,	-0.186984,	-0.0222689,	0.150317,	0.0884397,	0.175712,	0.164281,	0.055811,	-0.114958,	0.142725,	0.103231,	0.154693,	0.155602,	0.0862284,	0.0621746,	0.0379536,	-0.0960433,	0.0522428,	-0.0563557,	-0.0897705,	-0.084548,	-0.112517,	-0.180799,	-0.0625948,	-0.115487,	-0.0505915,	0.13595,	0.0225334,	-0.0168296,	0.0177578,	-0.131753,	-0.0458332,	0.0741264,	0.0820807,	0.162726,	0.0360042,	0.100904,	0.143865,	0.20245,	0.204691,	0.249407,	0.0737593,	0.0350724,	0.00623649,	-0.0200533,	-0.0202254,	0.0440746,	-0.061354,	-0.12625,	-0.111252,	-0.192536,	-0.0539258,	0.0568778,	0.0708659,	0.0581722,	0.0512911,	0.0725451,	-0.0238322,	0.145166,	-0.0375421,	0.0183229,	-0.0515636,	0.0801627,	0.102236,	0.141676,	0.105422,	0.172169,	0.325593,	0.433372,	0.320996,	0.167801,	0.0391118,	-0.081261,	-0.0745693,	-0.0301055,	-0.0439273,	-0.215596,	-0.138833,	-0.157325,	-0.0346408,	0.193456,	0.0372401,	0.20092,	0.118003,	0.118735,	0.0281421,	0.0553564,	-0.0644314,	0.097362,	0.0858294,	0.0987828,	0.138981,	0.139634,	0.13093,	0.162451,	0.437295,	0.618388,	0.416941,	0.033591,	0.067399,	0.0223981,	-0.0733668,	0.00567731,	-0.102106,	-0.303797,	-0.165402,	-0.138447,	-0.00119226,	0.151247,	0.134043,	0.106276,	0.162658,	0.0432769,	-0.0953258,	-0.0593954,	-0.163755,	-0.0268838,	-0.0337393,	0.0320692,	0.094201,	0.167907,	0.131732,	0.17229,	0.451739,	0.825219,	0.532727,	0.101971,	-0.0124701,	0.0497715,	0.12046,	-0.0149055,	-0.0473328,	-0.228809,	-0.183562,	-0.137837,	0.00280548,	0.125226,	0.140429,	0.162996,	0.330435,	0.196915,	0.00188885,	-0.209763,	-0.378653,	-0.385217,	-0.173916,	-0.07375,	-0.0676477,	0.00471857,	0.250016,	0.203111,	0.573701,	1.00741,	0.813333,	0.139917,	-0.00969058,	0.0454259,	-0.0332382,	0.00911845,	-0.138889,	-0.050791,	-0.087547,	0.0408344,	0.217737,	0.229909,	0.335018,	0.284708,	0.294932,	0.305714,	0.202399,	0.0928355,	-0.157881,	-0.234895,	-0.331266,	-0.337281,	-0.177361,	-0.177444,	-0.0476189,	0.0953408,	0.503456,	0.773684,	0.754209,	0.14274,	-0.0415441,	-0.00121091,	-0.0259649,	-0.058904,	-0.0292905,	-0.0506335,	-0.0131108,	0.152552,	0.144542,	0.181607,	0.150364,	0.173398,	0.184625,	0.171502,	0.388467,	0.051973,	-0.134427,	-0.201321,	-0.213611,	-0.273873,	-0.455023,	-0.567709,	-0.559461,	-0.557689,	-0.330253,	0.0681447,	0.2371,	0.0189973,	-0.088963,	0.0575473,	0.00500087,	-0.0576539,	-0.113796,	-0.0461442,	0.0409918,	0.225147,	0.144014,	0.110905,	-0.00352394,	0.18162,	0.234294,	0.306736,	0.281443,	-0.0552642,	-0.122875,	-0.408976,	-0.308704,	-0.284957,	-0.261417,	-0.385044,	-0.615773,	-0.70149,	-0.633911,	-0.411,	-0.0676505,	0.0892106,	-0.0827437,	0.0538963,	7.31143e-05,	-0.0245459,	0.0451628,	0.0437778,	0.0589741,	0.0538736,	0.0906945,	0.0688088,	0.051345,	0.245749,	0.261709,	0.313261,	0.168352,	0.00969958,	-0.208241,	-0.412552,	-0.302699,	-0.124522,	-0.0473881,	-0.109395,	-0.294056,	-0.315207,	-0.350664,	-0.305849,	0.00319676,	0.0662621,	0.0144458,	0.000105878,	-0.0435216,	-0.139933,	-0.0906564,	0.0198014,	0.00111224,	-0.0811962,	0.0801901,	0.029715,	0.261174,	0.283936,	0.0877079,	0.157554,	0.0597566,	-0.0354594,	-0.162237,	-0.239011,	-0.25149,	-0.18451,	-0.154283,	-0.0827602,	-0.0625815,	-0.14918,	-0.164843,	-0.129111,	-0.115098,	-0.00799515,	0.0259657,	-0.0426605,	-0.00974979,	0.00371705,	-0.00432676,	-0.0654354,	-0.237813,	-0.191135,	-0.220282,	-0.00753531,	0.110831,	0.127114,	0.0873426,	0.0820596,	-0.0110818,	-0.228627,	-0.189122,	-0.257998,	-0.174563,	0.0681548,	-0.0709097,	-0.0529311,	-0.0431014,	-0.0419852,	-0.0136599,	-0.0514979,	-0.0487247,	-0.0202155,	0.12909,	-0.0869639,	-0.0373055,	0.0432943,	0.00194392,	-0.0620021,	-0.257912,	-0.27972,	-0.368246,	-0.298419,	-0.248959,	-0.0575056,	-0.0546089,	0.0441326,	-0.0940139,	-0.235666,	-0.187681,	-0.225044,	-0.163532,	-0.0936489,	-0.00586137,	-0.0294057,	-0.0190781,	0.0832798,	0.0278763,	0.0180489,	-0.0846151,	-0.0424458,	0.00927839,	0.044148,	-0.00318534,	-0.0273377,	0.0699324,	-0.00105168,	0.069712,	0.0727064,	-0.266812,	-0.353036,	-0.335956,	-0.35689,	-0.323692,	-0.360766,	-0.398813,	-0.272286,	-0.0692704,	-0.0145195,	0.159618,	0.126393,	0.0736959,	0.0786493,	0.104415,	0.0129432,	0.110831,	0.0246399,	0.0460588,	0.0322253,	0.102541,	-0.00570772,	0.0518648,	-0.064721,	0.0204574,	0.0820972,	0.228673,	0.350945,	-0.019481,	-0.146913,	-0.305827,	-0.270842,	-0.254756,	-0.233155,	-0.12813,	-0.0322028,	-0.0945236,	0.0357398,	0.0841686,	0.035674,	0.00564254,	0.0953318,	-0.0304027,	0.0699914,	0.047298,	0.0848466,	-0.0643488,	0.000168645,	0.0609086,	-0.0765268,	0.0471702,	-0.00186233,	0.0815153,	0.172331,	0.215364,	0.420406,	0.332464,	0.290076,	0.133717,	-0.00415453,	0.0291051,	0.113038,	0.0546857,	0.0528696,	0.102825,	-0.0126394,	-0.0293945,	0.0593504,	0.0891146,	0.0889738,	0.202608,	0.0825954,	0.235757,	0.0996852,	0.0252974,	-0.0236251,	0.0474696,	-0.0597769,	0.0172352,	-0.0646287,	-0.00716295,	0.063948,	0.185874,	0.15089,	0.0828731,	0.203859,	0.208007,	0.120162,	0.216946,	0.0966942,	0.0838275,	-0.0268542,	-0.0287813,	0.0442592,	0.156214,	0.0484001,	0.0163876,	0.0145267,	0.0248385,	0.271745,	0.201499,	0.114305,	0.0289541,	-0.00940627,	-0.00491905,	0.0815522,	-0.0137682,	-0.0562497,	0.0377102,	0.143696,	0.101778,	0.1105,	0.101936,	0.247772,	0.152713,	0.07075,	0.154851,	0.106227,	0.0738806,	0.0857809,	0.120849,	0.0646417,	0.0159225,	0.0316263,	0.0301405,	0.160225,	0.262198,	0.206985,	0.0738988,	0.0601717,	0.113519,	-0.0524806,	-0.0114793,	-0.0213235,	-0.00422432,	-0.0306868,	-0.0524879,	0.0901384,	0.0141163,	-0.0656932,	0.0539926,	0.014357,	0.0881754,	0.0153547,	0.0993897,	0.193377,	0.236906,	0.17504,	0.106593,	0.168743,	0.0928275,	0.104745,	0.152372,	0.0744066,	0.0986801,	0.140794,	0.135689,	0.0177022,	0.0769654,	-0.0460132,	-0.0029308,	0.0218228,	0.00410913,	0.00402681,	0.0288576,	0.017307,	0.032462,	-0.105221,	-0.0892097,	0.0916377,	0.144439,	0.173295,	0.248788,	0.133148,	0.0490274,	0.118578,	0.140867,	0.11047,	0.0485947,	-0.12816,	0.0133353,	0.10406,	-0.028771,	0.0154676,	0.0837381,	-0.0529396,	-0.00656186,	0.000715746,	0.0341768,	0.0135568,	-0.0510255,	0.077541,	-0.011685,	-0.0166308,	0.0372535,	0.000826094,	0.0720889,	0.0855249,	0.236543,	0.17813,	0.139778,	0.289833,	0.202436,	0.295929,	0.184941,	0.144381,	0.153803,	0.0920123,	0.0667858,	0.000200496,	0.0779508,	0.0751927,	0.121095,	-0.0524768,	-0.0182194,	0.0439845,	-0.0171205,	0.00240667,	0.000826349,	-0.0685053,	-0.083094,	-0.000228445,	0.0235871,	0.0217217,	-0.0435093,	-0.0675146,	0.0776154,	0.0365679,	0.180487,	0.209749,	0.168351,	0.0692392,	0.181436,	0.117964,	0.169442,	0.124257,	0.0415814,	-0.0139852,	0.106756,	0.0619781,	0.078573,	-0.0425258,	-0.00545281,	0.0111809,	-0.00787678,	0.00235648,	-0.000902846,	-0.0367161,	-0.0360859,	0.0188712,	-0.0829637,	0.00104077,	0.0292373,	0.0308748,	0.0234253,	-0.0385494,	0.00780405,	-0.0204506,	0.0508532,	-0.0107886,	0.00247648,	-0.035041,	-0.0892715,	-0.0413763,	-0.0601617,	-0.0297695,	0.0342569,	-0.0534993,	-0.0184368,	-0.00960481,	0.033653,	0.0393773,	-0.0765246,	-0.099572,	-0.0351062,	-0.00708694,	-0.116095,	-0.0190635,	0.0124949,	-0.0208541,	-0.0693197,	0.0144055,	-0.0680663,	-0.0266171,	0.0364467,	-0.0210203,	0.0385458,	0.0744369,	-0.0703844,	-0.110565,	-0.0781847,	0.0692957,	-0.0330334,	0.067984,	0.0689895,	-0.0799081,	0.0205037,	0.0666541,	-0.056671,	0.0353625,	-0.0229115,
	0.0134557,	-0.010855,	0.00324696,	0.0527169,	0.00751977,	-0.0127234,	0.0267078,	0.0689036,	-0.000915708,	0.00426044,	0.00322016,	0.0535351,	0.0742159,	-0.0778746,	0.0873071,	-0.0868045,	0.00238387,	-0.0418033,	-0.0603179,	0.00589836,	0.0128984,	-0.0305414,	-0.0142161,	-0.000289703,	0.00220193,	0.0280712,	-0.0223943,	-0.00412006,	0.049507,	-0.0272727,	-0.0358231,	0.0204346,	0.0498748,	-0.0601389,	0.00781617,	0.0626468,	0.00235345,	-0.0252714,	0.0823391,	0.0355913,	0.0544082,	0.031115,	0.031777,	0.084176,	-0.0405157,	0.0366764,	0.0788762,	-0.0261894,	0.0295331,	5.04462e-05,	0.0285107,	0.0309434,	-0.0063099,	0.0352761,	0.00919964,	-0.00715005,	0.0286782,	0.00786541,	-0.112807,	-0.0453866,	0.0190938,	0.000756057,	-0.00680946,	-0.000188951,	0.152151,	0.0292502,	0.115039,	0.207014,	0.245714,	0.178562,	0.168965,	0.153951,	0.14227,	0.137799,	0.0542925,	0.114355,	0.124514,	0.0426269,	0.1498,	0.0779571,	-0.0526792,	0.00443196,	-0.0736637,	-0.0861616,	0.0655012,	-0.041166,	-0.00290762,	-0.0255541,	-0.0350206,	0.0159301,	0.0503416,	0.01829,	0.112284,	0.144752,	0.172505,	0.203418,	0.166368,	0.142404,	0.217806,	0.175057,	0.0536605,	0.105448,	0.211899,	0.161624,	0.137736,	0.153992,	0.103988,	0.0312132,	0.177603,	-0.016327,	-0.0669837,	0.0522184,	-0.0499707,	-0.0772655,	0.133812,	0.0588611,	0.0549674,	0.0399324,	0.0128207,	0.00120835,	0.0423374,	0.0628052,	0.0645856,	-0.0191335,	-0.101376,	-0.104944,	-0.055807,	-0.0998128,	0.042016,	0.0941892,	0.159081,	0.159635,	0.182999,	0.171993,	0.250002,	0.127988,	0.00488326,	0.014101,	0.0246613,	0.0517008,	0.0397436,	-0.122264,	-0.125627,	0.0865289,	0.00616673,	-0.0627685,	0.000304498,	-0.0486862,	0.108081,	0.0408319,	0.00465009,	-0.230206,	-0.102923,	-0.163961,	-0.0382624,	-0.0760791,	-0.0946771,	-0.0179336,	0.0711534,	0.130398,	0.187398,	0.131838,	0.148295,	0.207824,	0.0613176,	-0.070006,	0.0880062,	0.0396763,	-0.0522351,	0.0162032,	-0.0341953,	0.0492733,	0.00313054,	-0.0277633,	-0.0987661,	-0.0733365,	-0.125322,	-0.126093,	-0.152732,	-0.143248,	-0.174343,	-0.202537,	-0.110511,	-0.139995,	-0.146273,	-0.0795064,	-0.12079,	-0.128622,	-0.0367129,	-0.0150983,	0.0266608,	-0.0438401,	0.0480711,	0.0361369,	-0.0321198,	-0.056236,	-0.0368984,	-0.00111529,	0.00809686,	-0.0374743,	-0.020713,	-0.103484,	-0.10902,	-0.0536663,	-0.113266,	-0.0974875,	-0.191013,	-0.0337009,	-0.173206,	-0.206983,	-0.0984401,	-0.0870581,	-0.197439,	-0.284604,	-0.249208,	-0.254793,	-0.208977,	-0.241112,	-0.190962,	-0.113738,	0.0399343,	-0.0516952,	-0.0438761,	-0.00387617,	-0.0419876,	0.00341935,	0.0401109,	-0.0454678,	-0.0693213,	-0.0887808,	-0.09913,	-0.0781538,	-0.129318,	-0.0650315,	-0.242844,	-0.107163,	-0.140388,	-0.181922,	-0.277289,	-0.31343,	-0.307638,	-0.31186,	-0.449877,	-0.382289,	-0.445328,	-0.271014,	-0.308718,	-0.303997,	-0.102004,	-0.106231,	-0.0119679,	-0.0183016,	-0.0396088,	-0.00179376,	0.0169223,	-0.0441756,	0.0507808,	-0.17661,	-0.201905,	-0.0299996,	0.00283597,	-0.139955,	-0.175956,	-0.168214,	-0.140378,	-0.208993,	-0.247721,	-0.282284,	-0.495804,	-0.560843,	-0.557096,	-0.432328,	-0.324848,	-0.269382,	-0.389075,	-0.253146,	-0.214869,	-0.123635,	-0.0353075,	-0.0612746,	0.0808884,	0.00822327,	0.025026,	-0.0527401,	0.00504916,	-0.0486652,	-0.0992907,	-0.0462883,	0.0387549,	-0.123501,	0.00427962,	-0.0806639,	-0.224298,	-0.0475021,	-0.180443,	-0.460044,	-0.464297,	-0.390803,	-0.492119,	-0.314159,	-0.272247,	-0.211364,	-0.258513,	-0.105528,	-0.166838,	-0.16304,	-0.0417193,	-0.030959,	0.0369278,	0.0799445,	-0.0376894,	-0.0574115,	-0.0221572,	-0.0476362,	-0.00885085,	0.0706907,	0.0730448,	0.107422,	0.0451569,	0.0195406,	-0.0332436,	0.00800927,	-0.16396,	-0.419062,	-0.304874,	-0.228681,	-0.269446,	-0.172251,	-0.185686,	-0.0534903,	-0.00424039,	0.138295,	0.0200665,	-0.220544,	-0.135651,	0.13227,	0.118159,	-0.00385381,	-0.0331899,	-0.0518779,	0.00392364,	-0.0312795,	0.0769034,	0.164268,	0.0976985,	0.145288,	0.0565049,	0.133657,	0.209789,	0.118454,	-0.214658,	-0.263842,	-0.109205,	-0.181227,	-0.0779134,	-0.256934,	-0.0261732,	0.110551,	0.228354,	0.349726,	0.207976,	-0.0889876,	-0.0473491,	0.0307256,	0.021626,	0.107213,	0.0169342,	-0.0234884,	-0.0875368,	0.0710517,	0.187266,	0.0326919,	0.102476,	0.113527,	0.170513,	0.167364,	0.106431,	-0.0604172,	-0.153972,	0.133952,	0.0168034,	-0.0721651,	-0.208923,	0.017094,	-0.0841202,	0.168092,	0.270473,	0.474343,	0.269983,	-0.0307635,	-0.140783,	-0.0543694,	0.0905222,	0.00877126,	0.045317,	-0.0122073,	-0.135749,	0.0741357,	0.148839,	0.133954,	0.136001,	0.213545,	0.149997,	0.311801,	0.198286,	-0.0306687,	-0.00741468,	0.0838318,	0.0310136,	-0.24006,	-0.146501,	-0.0469398,	-0.0200219,	0.174043,	0.238863,	0.348956,	0.133344,	-0.0584932,	-0.0892335,	-0.0567889,	-0.00644085,	-0.0895709,	-0.000794892,	0.00404256,	-0.184924,	-0.0335921,	0.117816,	0.169024,	0.233295,	0.1956,	0.334401,	0.409811,	0.174409,	0.0806634,	0.200073,	0.146118,	0.014447,	-0.134082,	-0.0631187,	-0.0254844,	0.0766834,	0.0878067,	0.107718,	0.218798,	0.0502083,	-0.0704609,	-0.0958117,	0.0246896,	0.0045643,	-0.0927808,	0.0473422,	-0.00974911,	-0.223469,	-0.0212697,	0.191235,	0.228333,	0.171208,	0.309501,	0.378817,	0.445847,	0.0885413,	0.0540359,	0.141009,	0.060891,	-0.0804647,	-0.172516,	0.0824585,	0.0802994,	0.000850953,	0.00367462,	0.0405765,	0.0356489,	0.0165677,	-0.0405055,	-0.105949,	-0.0299959,	-0.0434196,	0.0894874,	0.0671505,	-0.110351,	-0.231486,	-0.159404,	0.0181563,	0.14464,	0.184097,	0.239416,	0.360044,	0.426294,	0.195416,	0.0851971,	0.0522179,	0.0392969,	-0.101049,	0.0304492,	0.0769352,	0.174084,	-0.0123832,	-0.00537236,	0.0439233,	0.0190674,	-0.0148937,	-0.0902912,	-0.022171,	0.0189166,	-0.00123486,	-0.01677,	-0.0790768,	-0.0311175,	-0.13325,	-0.205123,	-0.106796,	0.0731814,	0.18171,	0.18523,	0.267253,	0.294626,	0.394693,	0.215131,	0.043302,	0.0611288,	0.140211,	0.228212,	0.238289,	0.0501968,	0.0272251,	0.0709685,	0.000709827,	0.0443886,	-0.0975916,	-0.0134663,	0.0292398,	-0.0523284,	0.00209233,	-0.000668675,	-0.0144047,	-0.100091,	-0.189631,	-0.238302,	-0.136045,	0.0859222,	0.0490977,	0.178859,	0.282124,	0.481477,	0.442552,	0.209388,	0.147199,	0.247645,	0.198653,	0.212906,	0.0569345,	0.0279494,	0.11122,	0.0713292,	0.0815368,	-0.0165324,	-0.0402621,	-0.057713,	-0.0490998,	-0.0412891,	-0.0305544,	0.0160686,	-0.0111769,	0.0364992,	-0.163333,	-0.164167,	-0.183761,	-0.0231774,	0.007756,	0.143174,	0.311665,	0.274249,	0.43992,	0.38147,	0.430897,	0.301458,	0.200857,	0.0842411,	0.195061,	0.123248,	0.0732564,	-0.104797,	-0.0608954,	-0.0768386,	-0.0213866,	0.00733564,	-0.0386648,	0.031507,	-0.0439784,	-0.00789491,	-0.0492814,	0.0154827,	-0.072752,	-0.138389,	-0.312844,	-0.090308,	0.0339841,	0.153753,	0.240428,	0.328594,	0.458578,	0.4271,	0.517671,	0.221501,	0.24999,	0.253397,	0.32281,	0.0197719,	0.0435165,	-0.117703,	-0.115096,	-0.170931,	-0.0301829,	-0.005402,	-0.0468688,	0.0572448,	0.0969158,	-0.0457469,	-0.048001,	-0.0311589,	-0.00987813,	-0.13275,	-0.272668,	-0.166594,	-0.170916,	-0.053235,	0.12365,	0.0359378,	0.163775,	0.212419,	0.0820913,	0.168272,	0.128481,	-0.00742733,	-0.00542201,	0.0154497,	-0.0138997,	-0.0134659,	0.007249,	-0.0397401,	-0.0410357,	-0.0437847,	-0.12178,	0.0144772,	-0.0492674,	-0.0144383,	-0.0766042,	-0.00738537,	0.0725112,	-0.0385404,	-0.0634664,	-0.144538,	-0.182812,	-0.268867,	-0.265847,	-0.239405,	-0.110187,	-0.105573,	-0.0370838,	0.00140517,	-0.171154,	-0.122233,	-0.0706164,	-0.107453,	0.0246396,	-0.0777333,	-0.0587838,	0.0125863,	0.0211476,	0.0755199,	-0.0573177,	-0.0106459,	-0.0563616,	-0.0134551,	0.0712823,	0.013913,	0.0052038,	-0.115743,	-0.0280344,	-0.0342605,	-0.122363,	-0.157486,	-0.268268,	-0.206186,	-0.202861,	-0.314879,	-0.224245,	-0.171918,	-0.0959611,	-0.0251113,	-0.0628643,	-0.111332,	-0.115774,	0.0504017,	-0.0662083,	-0.0617343,	-0.0500505,	0.00174355,	0.0641768,	0.0272666,	0.0792915,	0.0994449,	0.0797282,	0.0382889,	0.0654054,	0.0277677,	-0.0686209,	-0.0358417,	-0.00168739,	-0.0321094,	-0.0660661,	-0.074967,	-0.0617284,	0.0215784,	-0.0932446,	0.0166491,	-0.0285378,	-0.119814,	-0.0246705,	0.00152142,	0.0740674,	-0.0416403,	-0.0228039,	-0.0598612,	0.0503472,	-0.00247496,	0.0109752,	-0.068051,	-0.0236304,	0.0410355,	0.0719571,	-0.0153249,	-0.0692638,	-0.0485716,	-0.101466,	-0.0784547,	0.0120218,	0.0322535,	0.00111839,	-0.0388368,	-0.0557498,	-0.0387638,	-0.0174699,	0.128978,	0.0198187,	-0.0321587,	0.0900729,	0.0280995,	-0.0429259,	-0.0190162,	-0.0771101,	0.0683883,	-0.017173,	0.000744931,	0.0104553,	0.0149819,	-0.0698839,	-0.000530207,	-0.0407902,	-0.0353534,	-0.0225321,	0.0308595,	-0.0223727,	0.0371979,	0.057526,	0.0167952,	-0.000722803,	-0.00620127,	-0.0150195,	0.00469838,	0.0389913,	0.0519501,	-0.101429,	0.00398401,	0.00845344,	-0.0288019,	-0.0078993,	-0.0573946,	0.000521177,	-0.0853866,	-0.076758,	0.0286751,	0.0579305,	-0.0640167,
	-0.0326534,	-0.0749571,	-0.0203904,	-0.0591274,	-0.0247195,	0.0937399,	0.152384,	-0.0178172,	-0.00893561,	0.0210047,	-0.0309912,	0.00534143,	0.0660685,	-0.0265666,	-0.0423269,	-0.00350243,	0.0282935,	0.024756,	-0.0221286,	0.0446518,	-0.00126823,	-0.0406797,	-0.0316722,	0.0480292,	-0.0801621,	-0.0029044,	0.00284461,	-0.00217636,	-0.0793306,	0.0430842,	0.0106014,	0.0535961,	-0.0386176,	0.0252416,	-0.00029973,	-0.0404256,	-0.0251791,	0.0307277,	0.012432,	-0.055856,	0.0289798,	0.00499981,	0.0159764,	-0.0693446,	-0.018727,	0.0803771,	-0.0453992,	0.0345178,	0.0566748,	-0.0623009,	0.0237711,	0.0176962,	0.0481441,	-0.07735,	0.0287104,	-0.00451722,	0.131759,	0.00686334,	0.0418698,	-0.10533,	-0.0510311,	0.037015,	-0.0122363,	0.0211613,	-0.0118108,	-0.00546583,	0.0329716,	-0.107824,	0.0694483,	-0.0864255,	-0.0116032,	-0.0268483,	-0.0615853,	0.0640793,	-0.0863808,	0.0153014,	-0.0726313,	-0.0171045,	0.0332588,	0.0351133,	0.0129539,	-0.0714569,	-0.0230717,	-0.0374368,	0.00530424,	0.0159755,	-0.084878,	-0.0245106,	0.0301875,	0.0122716,	0.0739343,	0.0273925,	0.00824024,	-0.0246696,	-0.0100579,	-0.0652689,	-0.041843,	-0.0652265,	-0.0418705,	-0.0205039,	0.0409425,	-0.0500667,	-0.0387077,	-0.0581628,	0.0487986,	-0.0354545,	-0.0618927,	-0.0336829,	-0.00780781,	-0.0154731,	-0.045372,	-0.0595222,	-0.0578264,	-0.062609,	0.0420243,	-0.0553506,	0.0308227,	-0.00860984,	-0.00169909,	-0.0368219,	-0.11729,	-0.0525585,	-0.186057,	-0.114645,	-0.142927,	-0.175396,	-0.180662,	-0.195111,	-0.20026,	-0.161377,	-0.0450833,	-0.143551,	-0.0347438,	-0.0371052,	0.00880901,	0.0943709,	0.016179,	0.0780521,	0.0599649,	-0.0625606,	-0.0303349,	-0.00339538,	-0.00162719,	0.0212919,	0.0146412,	-0.0213628,	0.00359821,	-0.0648993,	0.0191854,	-0.0615433,	-0.0631006,	-0.264729,	-0.220038,	-0.222288,	-0.426535,	-0.305737,	-0.307143,	-0.326006,	-0.35844,	-0.234958,	-0.229248,	-0.208088,	-0.17155,	-0.0924671,	-0.109402,	0.0568218,	0.0252611,	0.109995,	-0.0187454,	0.00325215,	0.0142824,	0.0207372,	0.00573017,	0.0747175,	0.0519144,	0.155927,	0.169238,	0.0671594,	0.102572,	0.215738,	0.0496231,	-0.0995331,	-0.0767338,	-0.00930689,	-0.0207425,	-0.0737915,	-0.11714,	-0.126783,	-0.137273,	-0.0468452,	-0.0186715,	-0.00850745,	-0.0788021,	-0.0193324,	0.0552458,	-0.0128804,	-0.0225799,	-0.024552,	-0.0196529,	0.0245112,	0.0811542,	0.0701058,	0.177933,	0.199977,	0.193659,	0.26084,	0.24579,	0.264836,	0.236164,	0.186264,	0.00108866,	0.107832,	0.0800628,	0.335995,	0.244303,	0.139484,	0.168584,	0.0243537,	0.00499402,	-0.0389774,	-0.0387561,	0.0364991,	-0.11527,	0.0787358,	-0.142762,	0.0823658,	0.108193,	0.0745709,	0.177142,	0.17013,	0.209685,	0.293915,	0.188272,	0.199841,	0.134398,	0.232921,	0.287211,	0.106942,	0.168493,	0.297133,	0.294477,	0.274866,	0.329248,	0.155911,	0.165545,	0.0145178,	0.0129374,	0.0249965,	-0.109958,	-0.0983495,	-0.0164339,	-0.0418754,	0.0159355,	0.0401709,	-0.00404068,	0.238368,	0.30303,	0.146628,	0.12917,	0.0501503,	0.0983368,	0.0417188,	0.0592004,	0.123419,	0.251043,	0.18058,	0.358131,	0.312056,	0.322432,	0.363346,	0.249828,	0.192876,	0.176429,	0.21096,	0.1168,	-0.0237109,	-0.152007,	-0.0367374,	-0.0175818,	-0.0217652,	0.0393313,	0.0376703,	0.113216,	0.214821,	0.194717,	0.21661,	0.058408,	0.0351669,	-0.00334903,	-0.0006924,	0.0336443,	0.0342001,	-0.0657308,	0.126327,	0.267931,	0.439201,	0.569472,	0.333206,	0.319715,	0.356637,	0.274304,	0.207251,	0.134304,	-0.175038,	-0.145945,	-0.0231337,	0.0033672,	-0.0467452,	-0.0530239,	0.0332868,	0.15593,	0.344564,	0.295509,	0.126286,	0.0513907,	0.0584234,	-0.0319085,	0.11994,	0.0813873,	0.0928223,	-0.0110057,	0.0281771,	0.208395,	0.367217,	0.471414,	0.380429,	0.372774,	0.177957,	0.311072,	0.195733,	0.0591242,	-0.051541,	-0.084577,	0.0559495,	0.0111475,	-0.0115142,	0.0925623,	0.0148858,	0.112449,	0.18322,	0.328598,	0.0499492,	0.0121272,	0.134917,	0.191658,	0.0579423,	-0.0417083,	-0.159019,	-0.527222,	-0.571323,	-0.192549,	0.202736,	0.441421,	0.341114,	0.284996,	0.00679045,	0.114785,	0.119156,	0.100569,	-0.152331,	-0.251374,	0.0458304,	-0.0434811,	0.0337829,	-0.0418386,	0.0747137,	0.0102026,	0.14689,	0.161978,	0.0452604,	0.0566208,	-0.00656256,	0.0234801,	-0.0938641,	-0.110648,	-0.3838,	-0.799265,	-0.806234,	-0.27006,	-0.0265367,	0.277797,	0.103509,	0.108513,	-0.00939385,	0.0380952,	0.0513689,	-0.0615686,	-0.0392292,	-0.0556577,	0.0440363,	0.0663249,	-0.0895659,	-0.0375616,	-0.0890304,	0.191209,	0.0688096,	-0.00862347,	0.047651,	0.0256261,	-0.0414889,	-0.145245,	-0.206505,	-0.215224,	-0.581708,	-0.749416,	-0.599834,	-0.099386,	-0.0843439,	0.0612241,	0.198525,	0.316261,	0.455927,	0.347056,	0.146324,	0.204397,	0.176419,	-0.0640963,	-0.020012,	0.08106,	-0.0657777,	0.0536081,	0.0340128,	0.0349204,	0.156585,	-0.0634741,	-0.00629282,	-0.121287,	-0.168945,	-0.194924,	-0.308876,	-0.363755,	-0.425013,	-0.661974,	-0.364455,	-0.19128,	0.0169548,	-0.083122,	0.468385,	0.452832,	0.41585,	0.279482,	0.243668,	0.232124,	0.0739089,	0.0532099,	-0.00942402,	0.00489636,	0.0343723,	0.0682128,	-0.00687086,	0.045069,	0.0863845,	-0.0236326,	-0.164089,	-0.00659288,	-0.0786914,	-0.179669,	-0.232937,	-0.38362,	-0.501553,	-0.227631,	-0.138425,	-0.189135,	0.165128,	0.0692173,	0.310816,	0.164152,	0.242013,	0.189684,	0.217398,	0.0468255,	-0.0444439,	0.00836454,	-0.0327939,	0.0625606,	0.0300649,	-0.000807034,	0.024438,	-0.0173025,	0.115517,	-0.0528908,	-0.0818814,	-0.18778,	-0.231004,	-0.223407,	-0.249757,	-0.422343,	-0.279463,	-0.0799083,	0.000730439,	0.0654845,	0.122337,	0.0223476,	0.000590011,	0.110515,	-0.0204188,	0.0195598,	-0.0669429,	-0.0219436,	-0.0798174,	-0.0460157,	0.0127293,	-0.0295883,	-0.13647,	0.0631932,	-0.0241513,	0.0109768,	-0.136125,	-0.0939775,	-0.170769,	-0.295573,	-0.324438,	-0.272447,	-0.307137,	-0.387826,	-0.192093,	-0.160668,	0.0645318,	0.0409279,	-0.0742813,	-0.190979,	-0.0603736,	-0.130157,	-0.0529832,	-0.0701956,	-0.122434,	-0.164322,	-0.226258,	-0.100817,	-0.0100079,	-0.0356557,	-0.0104826,	-0.116649,	0.03173,	-0.0254581,	-0.0908275,	-0.0434127,	-0.116271,	-0.283267,	-0.301534,	-0.382799,	-0.315495,	-0.314865,	-0.222142,	-0.0546265,	-0.00979581,	0.114489,	-0.0393685,	-0.260934,	-0.222248,	-0.282252,	-0.170439,	-0.284498,	-0.332801,	-0.350389,	-0.186259,	-0.108725,	-0.0424018,	-0.011861,	0.0208652,	-0.00301681,	0.0851425,	0.00306758,	0.000891773,	-0.116706,	-0.289646,	-0.322081,	-0.316188,	-0.320731,	-0.30971,	-0.191549,	-0.265116,	-0.0104835,	-0.0592241,	0.0560069,	-0.0923334,	-0.240331,	-0.271348,	-0.363991,	-0.362772,	-0.385397,	-0.322154,	-0.300922,	-0.0516333,	0.0528497,	-0.00936587,	-0.0141716,	-0.0134943,	-0.0463525,	0.0665416,	0.00367464,	-0.0679401,	-0.189039,	-0.172159,	-0.303428,	-0.278622,	-0.175096,	-0.228436,	-0.247853,	-0.0710398,	-0.0722431,	-0.0312798,	-0.129562,	-0.155431,	-0.119576,	-0.26934,	-0.362838,	-0.361242,	-0.393018,	-0.282139,	-0.324779,	-0.266981,	-0.022406,	-0.0345269,	-0.00434236,	-0.0499998,	-0.0717449,	0.0253558,	-0.0396104,	-0.0118446,	-0.0858732,	-0.0525072,	-0.111717,	0.010004,	-0.0663743,	-0.0697757,	-0.0508277,	-0.219893,	-0.0727188,	-0.19624,	-0.0361308,	0.0916597,	-0.0924282,	-0.180399,	-0.245319,	-0.32601,	-0.31986,	-0.256878,	-0.233901,	-0.0957791,	-0.0667289,	-0.0345407,	0.0545205,	0.0129451,	0.0266928,	0.0221024,	0.0621259,	-0.00475743,	-0.0360417,	0.0509498,	0.226348,	0.305513,	-0.00582174,	-0.0143784,	-0.0877717,	-0.123618,	-0.15903,	-0.033924,	0.0316965,	0.0678146,	-0.0388247,	-0.0485236,	-0.0799235,	-0.267637,	-0.300644,	-0.221252,	-0.145018,	-0.159298,	-0.0670155,	-0.00349822,	0.00422316,	0.0359814,	0.0561881,	0.0265031,	0.0798995,	0.0384268,	0.137765,	0.236968,	0.205209,	0.225828,	0.180746,	0.133605,	0.126126,	0.156807,	0.0148451,	0.104297,	0.127295,	0.171816,	0.029989,	0.0466081,	-0.150088,	-0.111872,	-0.125837,	-0.148831,	-0.173523,	-0.0774291,	-0.0563291,	-0.0576228,	0.059368,	0.130004,	-0.0381392,	-0.0346435,	-0.0694795,	-0.0648167,	-0.0262732,	0.0705993,	0.0886141,	0.19644,	0.264649,	0.215359,	0.260124,	0.355217,	0.281315,	0.221399,	0.210195,	0.241054,	0.187903,	0.103668,	0.193811,	0.0491746,	-0.0443208,	-0.0508945,	-0.0543392,	0.0256835,	-0.0154062,	-0.0846486,	0.00961551,	-0.0132094,	0.0481956,	-0.036533,	-0.000492254,	0.0131127,	-0.028356,	-0.0741273,	0.0543285,	0.0646976,	0.0490574,	0.146965,	0.0630902,	0.0880871,	0.136016,	0.220192,	0.18227,	0.183594,	0.238002,	0.206299,	0.133414,	0.213928,	-0.0848246,	-0.0875858,	0.000551061,	-0.104135,	-0.0515715,	-0.0884882,	0.0527766,	0.0476326,	0.00312693,	0.0377636,	0.0110781,	0.00963281,	-0.055539,	0.0348224,	0.0460817,	0.0181288,	0.0356363,	-0.0158711,	0.0483073,	-0.0522067,	-0.0531295,	0.06367,	0.103867,	0.00842434,	0.0916909,	0.0321416,	0.0250205,	-0.0215859,	-0.0484915,	0.016227,	-0.0177324,	0.00716057,	0.0186416,	0.019552,	0.0147503,	0.0248011,
	0.00136922,	-0.0366718,	-0.0288752,	0.0206659,	0.0244546,	0.0163912,	-0.0982277,	-0.120589,	0.0361162,	0.047951,	0.0219914,	-0.004624,	-0.0337121,	0.0070802,	0.0476525,	0.00785578,	-0.0536572,	0.0115407,	-0.0185227,	-0.0628919,	-0.0137542,	-0.0944276,	0.0134093,	-0.139957,	0.00888304,	0.0220474,	0.0511913,	-0.0231134,	-0.0290001,	-0.0214556,	0.0364578,	-0.0160019,	-0.00765546,	0.0629322,	-0.0224529,	-0.040688,	0.0316205,	-0.00654177,	-0.0474056,	0.0304809,	0.0174724,	0.0420884,	0.0195807,	0.0192788,	-0.0148069,	-0.075904,	-0.0825302,	-0.0534461,	-0.00117089,	-0.0787328,	-0.0171468,	-0.0887042,	-0.0359455,	0.0886799,	0.0148292,	-0.0175971,	-0.0580133,	-0.0513513,	-0.0117533,	0.0323629,	0.0228184,	-0.0147909,	0.0961698,	-0.0390096,	-0.0547605,	-0.070805,	-0.0688955,	-0.0440128,	0.0935615,	-0.094,	-0.00960862,	0.00153389,	0.0442566,	-0.00792303,	0.0703412,	-0.000419626,	0.0201193,	-0.056306,	-0.107683,	0.0860731,	0.0161568,	-0.0173471,	-0.0744737,	0.0251424,	-0.058744,	0.00293501,	0.0227447,	0.122877,	0.0205809,	0.0987204,	0.0360198,	0.0356699,	0.0493635,	-0.00645274,	-0.0517391,	-0.0518148,	-0.0614186,	-0.132852,	-0.22125,	-0.119459,	-0.0722867,	-0.118049,	-0.105204,	-0.0840827,	0.0155465,	-0.0789826,	0.0325677,	0.0750882,	0.00484581,	0.0249506,	-0.03928,	-0.0139895,	0.00363557,	0.0594423,	0.129729,	0.0466843,	0.00187651,	-0.0903444,	-0.0307119,	-0.0460978,	-0.0448988,	0.0493329,	0.0154386,	0.00865129,	0.133281,	0.162415,	0.244944,	0.21201,	0.0900525,	0.107391,	0.0662006,	-0.0459685,	-0.0214336,	-0.0465207,	-0.0899107,	-0.0391123,	-0.0796017,	0.0314117,	0.00999162,	-0.041642,	0.0249279,	0.0195808,	0.0209196,	0.03184,	0.0181488,	-0.0454265,	-0.10047,	-0.0545619,	-0.0567329,	0.028369,	-0.0332086,	0.0753493,	0.271443,	0.174429,	0.172668,	0.178441,	0.23244,	0.16715,	0.0634305,	0.0719259,	0.144241,	0.0277221,	-0.130484,	-0.0263624,	-0.0685703,	0.116882,	0.065107,	-0.0519809,	0.0340299,	-0.0763354,	-0.0720585,	-0.109767,	0.0311546,	-0.108024,	-0.0340787,	-0.0980926,	0.00511156,	-0.000716055,	0.0294205,	0.0289972,	0.145448,	0.0975078,	0.100417,	0.243605,	0.156614,	0.144625,	-0.0131241,	0.0944407,	0.070025,	0.076841,	0.0103816,	0.110824,	0.0352651,	0.0121068,	0.0299386,	0.069193,	-0.0456284,	0.0135588,	-0.0967315,	-0.0407578,	-0.0597112,	-0.0328223,	-0.0593452,	0.00567424,	0.137353,	0.0434961,	0.0226851,	-0.029467,	-0.0475959,	0.0262272,	-0.14776,	0.160758,	0.044001,	-0.0681475,	0.0633929,	0.0118568,	0.0487901,	0.0962748,	0.0253664,	0.291263,	0.126435,	0.0596692,	-0.023136,	-0.0432245,	-0.0194037,	-0.0617617,	-0.036471,	-0.0410217,	0.000926897,	0.0875901,	0.0416085,	0.169638,	0.0394499,	0.21377,	0.0806341,	0.0255112,	0.177996,	0.13799,	-0.0718476,	-0.107486,	0.152604,	-0.0183072,	0.124363,	0.171877,	0.0317879,	0.074043,	0.181251,	0.242144,	0.373809,	-0.0104455,	-0.037071,	-0.067479,	-0.0958456,	-0.0579163,	0.00816302,	-0.0316515,	-0.0265938,	0.0864375,	0.109559,	0.202304,	0.132899,	0.167569,	0.136027,	0.108908,	0.0875856,	0.0462658,	-0.155399,	-0.436007,	-0.183898,	0.0407272,	-0.0242114,	0.134416,	0.11575,	0.123443,	0.148722,	0.280338,	0.0567946,	0.0698828,	0.033442,	-0.0446373,	-0.0740831,	-0.109379,	-0.0259879,	0.00948633,	0.105046,	0.0806591,	0.212342,	0.162184,	0.121929,	0.303011,	0.282531,	0.190988,	0.109955,	-0.0390714,	-0.0183559,	-0.273425,	-0.272128,	-0.10556,	0.0562405,	0.13479,	0.0869744,	0.129686,	0.173052,	0.294596,	-0.0797489,	-0.124905,	-0.0226346,	0.0158017,	-0.0173445,	-0.167644,	-0.0107934,	0.0424174,	0.0976192,	0.126437,	0.185187,	0.295279,	0.303675,	0.278598,	0.24187,	0.135789,	0.204545,	0.325254,	0.301542,	-0.044026,	-0.211084,	-0.127611,	-0.0943818,	0.0479589,	0.0350314,	0.0918437,	0.25007,	0.293236,	0.173107,	0.127884,	0.0308572,	-0.017314,	0.00340684,	-0.0199423,	-0.00439378,	-0.0909846,	0.0752848,	0.171991,	0.100774,	0.193474,	0.0287215,	0.116903,	-0.0785417,	-0.0259151,	0.0424331,	0.380758,	0.286913,	0.0803007,	-0.045414,	0.0042584,	-0.0436042,	0.0958211,	0.219629,	0.177945,	0.307028,	0.258551,	0.139891,	0.108053,	-0.128877,	0.00745141,	0.0274065,	-0.00575441,	0.0144262,	-0.0299688,	0.0313657,	0.0662414,	-0.0491146,	-0.0431916,	-0.0475018,	0.0252203,	-0.11129,	0.0480197,	0.253281,	0.345394,	0.0778179,	0.312056,	0.0884615,	-0.111299,	-0.0427062,	0.0551973,	0.163466,	0.0406615,	0.0337142,	0.122378,	0.0145435,	-0.0248243,	0.000254047,	-0.0400596,	0.0444396,	-0.0406408,	-0.010777,	-0.00303715,	-0.0664952,	-0.127904,	-0.154561,	-0.301665,	-0.280403,	-0.184339,	-0.027895,	0.231169,	0.231981,	0.36738,	0.233636,	0.185008,	0.0208527,	-0.00553855,	-0.0333737,	-0.0459396,	-0.192521,	-0.25959,	-0.197167,	-0.153189,	-0.0854483,	-0.0723612,	-0.0406147,	0.0299427,	-0.0307857,	0.0133979,	0.0726184,	-0.0936018,	-0.170502,	-0.156431,	-0.268357,	-0.159474,	-0.137347,	-0.0799069,	-0.045136,	-0.0352995,	0.2372,	0.413105,	0.33407,	0.137815,	-0.0676503,	-0.128394,	-0.0650124,	-0.200727,	-0.314893,	-0.282577,	-0.178952,	-0.238139,	-0.141537,	-0.0382736,	-0.0546453,	0.0915161,	0.0893633,	-0.0517523,	-0.0403086,	0.0301338,	-0.128938,	-0.234215,	-0.186456,	-0.118944,	0.0475506,	0.0515515,	0.0909455,	0.0567957,	0.291449,	0.243294,	0.104002,	0.0164069,	0.114841,	-0.0934387,	-0.238862,	-0.298729,	-0.329998,	-0.339248,	-0.27428,	-0.0927355,	-0.137654,	0.000515564,	-0.114477,	-0.0472323,	-0.00672305,	-0.0128356,	-0.0854694,	-0.0434813,	-0.0932683,	-0.178073,	-0.144023,	-0.0223879,	0.163039,	0.119167,	0.0479786,	0.232026,	0.343126,	0.185298,	0.0169481,	-0.00916339,	0.0392562,	-0.100789,	-0.234666,	-0.136649,	-0.104273,	-0.108104,	-0.197911,	-0.0701179,	-0.0339303,	-0.0591544,	-0.0684364,	-0.0861401,	-0.0156188,	-0.00945809,	-0.0572867,	-0.0387343,	-0.151805,	-0.0796399,	-0.0861039,	0.0461313,	0.0822634,	0.31773,	0.169396,	0.170835,	0.207947,	-0.0116224,	-0.145382,	-0.080475,	-0.118343,	-0.102922,	-0.0149581,	-0.107791,	-0.0481093,	-0.116694,	0.0342943,	-0.0630464,	-0.0642868,	-0.0791799,	-0.0281626,	-0.0129186,	0.0436591,	0.0509253,	0.0461454,	-0.0427728,	-0.190698,	-0.0696722,	0.0681423,	-0.0170536,	0.044603,	0.0404219,	0.192522,	-0.0848152,	0.0192737,	-0.123859,	-0.155327,	-0.0610227,	0.00165414,	-0.055887,	-0.0186346,	-0.0184674,	0.0350919,	0.0128784,	0.133988,	0.124931,	-0.165135,	0.0205146,	0.0341094,	0.0639306,	-0.0484497,	0.088306,	-0.0704764,	-0.0901306,	-0.17986,	0.0751905,	0.114818,	0.264734,	0.125217,	0.0259598,	0.0737443,	-0.107617,	0.0315447,	0.0142591,	-0.0878808,	0.0339888,	-0.1691,	0.0102594,	0.0183783,	0.0479479,	0.0352002,	0.0733483,	0.118615,	0.0452806,	0.0127694,	-0.00335374,	0.0578869,	-0.154022,	0.00812081,	-0.0264531,	-0.0613281,	-0.0428424,	-0.0951811,	-0.0820698,	0.195055,	0.150552,	0.0665298,	0.0433535,	-0.00576897,	-0.033405,	-0.054207,	0.0479521,	-0.0374593,	0.0175993,	-0.113719,	0.0151811,	-0.0777071,	-0.00843003,	0.06251,	0.0759514,	0.0991822,	-0.0984702,	-0.102835,	-0.0602379,	-0.0476661,	-0.0302069,	0.0955432,	-0.0595513,	-0.0511453,	-0.00620247,	-0.113856,	-0.139532,	-0.013572,	0.135641,	0.0086679,	-0.0116864,	0.0648052,	0.0979169,	0.211898,	0.209621,	0.197152,	0.116975,	-0.0058087,	0.0522856,	0.0275097,	-0.028477,	-0.0292698,	0.136542,	-0.0163343,	0.0241777,	-0.0114574,	-0.0032122,	-0.00677626,	-0.00648688,	0.0175512,	0.0662413,	0.00440674,	-0.0201916,	-0.130976,	-0.297943,	-0.272235,	-0.0945781,	0.152007,	0.0565907,	0.0104124,	0.173816,	0.226288,	0.325283,	0.303132,	0.237009,	0.259256,	0.109611,	0.169236,	0.115026,	0.134624,	0.0298346,	-0.0472977,	0.0214063,	-0.115923,	-0.0358945,	-0.00281349,	-0.0034603,	0.0100541,	0.0390435,	-0.00135511,	-0.0779234,	-0.0956627,	-0.148005,	-0.337121,	-0.281099,	0.0536699,	-0.0401172,	0.12117,	0.170292,	0.220352,	0.13137,	0.158477,	0.324286,	0.307585,	0.0200732,	0.23238,	0.227173,	0.0139959,	0.00739609,	-0.0595359,	0.056292,	-0.0329455,	0.0929023,	-0.019381,	0.00848273,	-0.00883632,	0.0573511,	-0.0228236,	-0.00436374,	-0.117881,	-0.00818469,	-0.107951,	-0.146214,	-0.189029,	-0.176817,	-0.0706954,	-0.0982119,	-0.0997347,	-0.0348568,	-0.0106417,	0.133749,	0.0507425,	0.15579,	0.0624505,	0.0600714,	0.0398483,	-0.109896,	-0.00104648,	0.00819162,	-0.0388993,	0.0614204,	-0.0717196,	-0.034595,	0.102152,	0.0804774,	0.0712817,	0.0201756,	-0.00037781,	-0.0720448,	0.0256487,	0.000406485,	-0.0315925,	-0.0586289,	-0.119641,	-0.0894237,	-0.0137787,	-0.014311,	-0.0513111,	-0.0960519,	-0.0615847,	-0.0746832,	-0.14621,	-0.0912597,	-0.108214,	-0.0464023,	0.0255951,	-0.0621185,	-0.00554883,	0.0182431,	0.0684172,	0.00676089,	0.0190224,	0.0125811,	0.100993,	-0.0761438,	0.00375904,	0.0289625,	0.0553818,	0.0588768,	0.0695496,	-0.0394676,	0.00567582,	0.0432649,	-0.00548711,	-0.0811479,	0.0370532,	-0.0521327,	0.129239,	0.0284404,	-0.0735516,	-0.078788,	0.0829028,	-0.0307487,	0.0404291,	0.0214036,	-0.0483531,	-0.0940924,	-0.0216083,	-0.00368798,
	-0.0995986,	0.022314,	0.0429551,	-0.0870391,	-0.0161631,	-0.0335076,	-0.00931762,	0.0319647,	-0.0343643,	-0.0559821,	-0.0970111,	-0.0270803,	-0.0618299,	-0.0224959,	0.116185,	-0.00496854,	0.0548928,	0.00419818,	-0.0215024,	0.00105299,	0.0424616,	-0.0635817,	0.00780041,	0.050757,	-0.0345525,	0.0410825,	-0.00554815,	0.0397227,	0.0246223,	-0.0877539,	0.200708,	-0.029738,	-0.0330544,	-0.128042,	-0.0491634,	0.00672901,	0.00896927,	0.037977,	0.137877,	-0.00339955,	0.102001,	-0.0715976,	0.00228048,	-0.0966373,	-0.0410582,	0.0725,	0.00251584,	0.0223813,	0.013424,	0.0410989,	0.0111367,	0.0786128,	0.0207044,	-0.024969,	0.0121218,	-0.0298979,	-0.0299084,	0.0464726,	-0.0217655,	0.048961,	-0.112875,	0.0380654,	-0.0725084,	-0.0169536,	-0.0195674,	-0.0039783,	0.0972006,	-0.0522205,	-0.0164468,	0.0309164,	-0.0539006,	-0.0884776,	-0.0244751,	-0.105387,	-0.0842601,	0.00941039,	0.0462844,	0.104046,	0.0344572,	-0.0102573,	0.023637,	-0.0690089,	-0.0212238,	0.061556,	-0.0856134,	0.0651636,	0.0237434,	-0.0327888,	0.0613334,	-0.093649,	-0.00465263,	-0.0504737,	-0.0359921,	0.0454411,	-0.112848,	0.0537738,	-0.0586938,	-0.174686,	-0.0978581,	-0.0312709,	-0.102775,	-0.112875,	-0.0285102,	-0.0281022,	0.056781,	-0.0182105,	-0.046287,	-0.0323476,	-0.0772873,	0.0115936,	0.0109654,	0.0223096,	-0.00162028,	-0.0286303,	0.069454,	0.009155,	-0.00837161,	0.00554888,	-0.0638896,	-0.0991596,	0.143866,	-0.137795,	-0.183668,	0.00572719,	-0.148882,	-0.202759,	-0.270096,	-0.267497,	-0.2888,	-0.275147,	-0.108165,	-0.194553,	-0.139127,	-0.0606067,	0.00643014,	-0.059948,	-0.0259067,	-0.0592214,	0.0262791,	0.0345938,	-0.0239646,	-0.0325027,	0.0649655,	0.0134598,	-0.0542682,	-0.020638,	0.0476665,	-0.0769619,	-0.138112,	-0.138371,	-0.152951,	0.0142489,	-0.0437154,	0.0759116,	-0.113667,	-0.144369,	-0.317592,	-0.306386,	-0.286492,	-0.214308,	-0.239452,	-0.289227,	-0.231727,	-0.125744,	-0.045638,	0.0568592,	0.0716621,	0.0020197,	-0.0295968,	-0.0169379,	-0.0476764,	-0.0649631,	-0.111066,	-0.0993113,	-0.151562,	-0.236064,	-0.258734,	-0.1216,	-0.014325,	0.157039,	0.0954204,	0.240673,	0.31971,	0.208362,	0.221487,	0.218995,	0.098155,	0.0127134,	-0.0726692,	-0.148072,	-0.215918,	-0.245747,	-0.157006,	-0.0575455,	-0.0573067,	-0.067842,	-0.0417963,	0.000577476,	0.0604218,	0.0550077,	-0.0604425,	-0.190212,	-0.262725,	-0.27738,	-0.148048,	-0.0487787,	0.0466738,	0.074997,	0.314508,	0.468208,	0.521879,	0.560841,	0.471912,	0.325389,	0.180478,	0.113996,	0.123153,	-0.0005369,	-0.192468,	-0.192404,	-0.128836,	-0.157091,	-0.0186007,	-0.0836071,	-0.0123859,	-0.00339834,	0.0578869,	-0.124542,	-0.12744,	-0.292362,	-0.157025,	-0.242362,	0.041069,	-0.0220103,	-0.0137904,	-0.0747766,	-0.0895498,	0.0723746,	0.324703,	0.278295,	0.188687,	0.0692139,	0.0849227,	0.0394311,	-0.0332785,	0.0103656,	-0.18113,	-0.172717,	-0.145099,	-0.0725701,	0.0448368,	0.0321344,	-0.11035,	0.0123793,	-0.040989,	-0.100647,	-0.194228,	-0.119428,	0.0628118,	-0.0128565,	0.082067,	-0.0253647,	0.138702,	0.0537132,	0.0230977,	0.106453,	0.191086,	0.15264,	-0.0753205,	0.06085,	0.0534753,	-0.0367379,	0.0121565,	-0.0756385,	-0.146452,	-0.300176,	-0.0971221,	-0.0663274,	-0.0148602,	0.0970223,	-0.0185876,	-0.0779169,	0.0232582,	-0.097619,	-0.122697,	0.0337991,	0.100836,	0.0654631,	0.149498,	0.112631,	0.129835,	0.25128,	0.0805904,	-0.126325,	-0.0394633,	-0.0466373,	0.0289456,	0.0254893,	0.0843997,	0.0716155,	0.0526077,	-0.0482968,	-0.0396488,	-0.0998619,	-0.10439,	-0.130222,	-0.0599919,	0.0127725,	0.0535305,	-0.0204481,	-0.208972,	-0.080731,	-0.0398856,	0.136309,	0.31898,	0.153734,	0.300745,	0.236857,	0.212449,	0.251058,	0.0026387,	-0.0864853,	0.00208278,	0.0994969,	0.208216,	0.282766,	0.2019,	0.188533,	0.418003,	0.221282,	0.175284,	-0.113428,	-0.164293,	-0.0727031,	-0.0730441,	-0.0819081,	0.151303,	-0.00950913,	-0.0120689,	-0.0956425,	0.146839,	0.238605,	0.279764,	0.122759,	0.243798,	0.257686,	0.280412,	0.0708416,	-0.134291,	9.36811e-05,	0.262966,	0.252024,	0.283327,	0.383021,	0.329844,	0.397471,	0.399561,	0.381983,	0.142428,	0.0604741,	-0.0702716,	-0.0568959,	-0.103484,	0.0540008,	0.00219402,	-0.0671522,	-0.0940042,	-0.0953088,	0.191725,	0.291934,	0.227302,	0.197137,	0.135121,	0.0664508,	0.123677,	-0.0879586,	-0.147606,	0.0595026,	0.264566,	0.247953,	0.386486,	0.304887,	0.199263,	0.367163,	0.280036,	0.290565,	0.0522511,	-0.0995585,	-0.205039,	-0.0252368,	0.044672,	-0.052336,	0.098313,	0.00928169,	-0.0431572,	0.0453337,	0.105808,	0.156137,	0.106788,	0.152851,	0.0845769,	0.0570489,	-0.00674767,	-0.0189935,	-0.0531902,	0.0653363,	0.0448903,	0.195082,	0.37582,	0.270476,	0.174448,	0.119054,	0.222401,	0.0321772,	-0.0520376,	-0.313929,	-0.269715,	-0.0388261,	-0.0324165,	-0.102697,	-0.0646282,	-0.0102409,	0.0523324,	0.0405022,	0.0351999,	0.0960023,	-0.0468405,	0.0158292,	0.0812597,	-0.125952,	0.0884217,	-0.00456377,	-0.0553429,	-0.13134,	-0.128654,	0.171821,	0.306794,	0.275319,	0.113463,	0.0139024,	0.0484283,	-0.133789,	-0.314737,	-0.259821,	-0.125304,	-0.0523244,	0.0322602,	0.00120176,	-0.0111449,	-0.106489,	-0.0180805,	-0.0178932,	0.126398,	0.0378232,	-0.0485469,	-0.102642,	0.154272,	-0.112455,	0.0402334,	0.0999996,	-0.0477129,	-0.135258,	-0.146522,	0.0920423,	0.219628,	0.170809,	0.107054,	-0.140371,	-0.199285,	-0.279546,	-0.296635,	-0.208559,	-0.222767,	0.0320092,	0.00229372,	0.0589514,	0.0185547,	-0.104402,	0.00237078,	-0.0305815,	-0.128625,	-0.186542,	-0.200457,	-0.0385199,	0.0493145,	-0.0688978,	0.0902006,	-0.038964,	0.0262711,	-0.102531,	-0.0194716,	0.157471,	0.0857997,	0.0552799,	-0.0748631,	-0.0972912,	-0.208487,	-0.148194,	-0.298971,	-0.252142,	-0.223696,	-0.0182727,	0.00591808,	-0.0242896,	0.0109047,	0.00103157,	-0.0288783,	-0.104873,	-0.118214,	-0.301921,	-0.109069,	-0.233948,	-0.0433344,	-0.08947,	0.0904758,	0.0284931,	-0.196959,	-0.160763,	0.0937535,	0.0833585,	-0.0723212,	-0.105933,	-0.103157,	-0.0733499,	-0.0542978,	-0.207979,	-0.183061,	-0.269769,	-0.21387,	-0.14267,	0.111397,	0.0705867,	0.0727377,	0.0131909,	0.0504398,	-0.0791303,	-0.116617,	-0.12629,	-0.187812,	-0.122111,	-0.20921,	-0.347127,	-0.238182,	-0.270257,	-0.221631,	-0.190379,	-0.186557,	-0.163804,	-0.150402,	-0.151589,	-0.158717,	-0.0446628,	-0.063853,	-0.182126,	-0.208509,	-0.221614,	-0.148934,	-0.0598716,	0.0132241,	-0.0285616,	0.0300487,	0.0599672,	0.00836879,	-0.0533252,	-0.0861264,	-0.0340298,	-0.256421,	-0.248757,	-0.324389,	-0.405933,	-0.442275,	-0.518205,	-0.438505,	-0.178085,	-0.246463,	-0.24805,	-0.153899,	-0.11923,	-0.148817,	-0.105218,	-0.0923428,	-0.200782,	-0.182259,	-0.118138,	-0.00526947,	-0.00722214,	0.0387395,	0.023605,	0.00964402,	-0.0281591,	-0.0361982,	-0.0955689,	0.0280338,	-0.127217,	-0.10904,	-0.250792,	-0.16945,	-0.190055,	-0.242825,	-0.3221,	-0.322396,	-0.226154,	-0.234272,	-0.18489,	-0.159794,	-0.247367,	-0.197791,	-0.194709,	-0.114728,	-0.116308,	0.00576339,	-0.02963,	0.0864242,	-0.0494688,	-0.0490302,	-0.0815595,	-0.023201,	0.102266,	-0.0336739,	-0.0525782,	-0.0984212,	-0.0927126,	-0.153933,	-0.150873,	-0.127602,	-0.0522846,	-0.0104872,	-0.0961339,	-0.017643,	-0.102861,	-0.0244425,	-0.106255,	-0.140788,	-0.217913,	-0.23617,	-0.226191,	-0.0531921,	-0.0288824,	0.0804446,	-0.0391851,	0.0608675,	-0.125091,	-0.0188719,	0.0968507,	0.00555113,	-0.0178481,	-0.0351755,	-0.038837,	-0.0361305,	-0.111951,	-0.11106,	-0.0609085,	-0.00585382,	-0.0511491,	0.00682059,	-0.00354077,	-0.0268575,	-0.074904,	0.00569335,	-0.128551,	-0.1635,	-0.0721341,	-0.164836,	-0.119717,	0.0172485,	0.17548,	0.136912,	0.192285,	-0.0201108,	-0.102793,	-0.0203642,	0.0189571,	0.0448481,	0.0217851,	-0.00701155,	-0.0684544,	0.0178022,	-0.0426907,	-0.0516205,	0.14422,	0.126289,	-0.0575277,	0.0508821,	-0.0304505,	-0.122113,	-0.182906,	-0.206573,	-0.0107105,	0.041755,	0.0788362,	0.121929,	0.109715,	0.286619,	0.256534,	0.165931,	0.0571415,	0.0329898,	-0.0578164,	0.0703333,	0.0490909,	-0.0189469,	-0.0644494,	-0.0400687,	0.0823934,	-0.0272354,	0.12535,	0.162421,	0.0906383,	0.176821,	0.161573,	0.11588,	0.127292,	0.0612732,	0.168528,	0.11827,	0.120621,	0.168179,	0.169829,	0.276485,	0.289397,	0.416163,	0.234331,	0.14201,	-0.0657139,	0.055936,	0.0312211,	0.0135549,	0.0524151,	0.0288256,	-0.10353,	0.0183937,	0.091781,	-0.0329641,	-0.0252063,	0.0703844,	0.0905585,	0.18589,	0.182302,	0.254308,	0.270911,	0.309437,	0.410301,	0.364659,	0.246629,	0.193833,	0.169392,	0.206536,	0.19488,	0.0921112,	0.0801283,	-0.0171745,	0.0113585,	0.024778,	-0.0248224,	-0.072798,	-0.0449634,	-0.0153953,	0.0656602,	-0.0160436,	-0.0284762,	0.00955182,	-0.0314305,	0.0389487,	0.0827525,	-0.00424949,	0.0420248,	0.0163725,	0.0391961,	-0.111813,	-0.0745136,	-0.0467119,	-0.0635387,	-0.0442998,	-0.0143054,	-0.0496125,	0.0346104,	0.072971,	-0.0254577,	0.0593703,	0.0503205,	-0.0203553,	-0.0154603,	0.0330428,	0.0238535
};

const float b_784x10[10] = {
	-0.418656,
	0.37338,
	0.17573,
	-0.298825,
	0.0419013,
	1.68477,
	-0.0323781,
	0.922667,
	-1.66115,
	-0.225748,
};

const float A1_784_50_100_10[28*28*50] = {-0.0061602, -0.0548926, 0.0345619, -0.0543053, 0.00168036, 0.0376199, 0.00169737, -0.0265992, 0.0233614, 0.0101369, 0.107312, 0.0187748, 0.0514976, 0.0394174, -0.00645699, -0.0170116, -0.0347066, 0.0660852, -0.0472672, -0.134975, -0.0390292, -0.026343, -0.101787, 0.00254396, -0.0451854, -0.000665008, -0.00932776, -0.0343176, -0.00372435, 0.043348, 0.0190369, -0.037844, 0.0708272, 0.0407041, -0.0169117, 0.0339994, 0.0609628, 0.0022081, -0.0149172, 0.0571454, -0.0664683, -0.00664221, -0.0237625, 0.0034063, 0.0842125, -0.000617287, -0.0205658, -0.0160932, -0.0307352, -0.0806267, -0.0500062, 0.0401239, 0.0557958, -0.0669514, -0.0490807, 0.0792753, 0.00801752, -0.0081774, 0.0716286, -0.100694, 0.0286846, -0.000716771, 0.0260564, -0.0604947, 0.0558951, 0.0271876, -0.075989, -0.0110914, 0.0371587, -0.0199014, -0.0275846, -0.0574972, -0.0104674, -0.0577068, 0.0364066, -0.00451001, 0.00481012, -0.00346067, 0.000839211, 0.0575496, -0.0079756, 0.0670584, 0.0556675, 0.0227573, 0.00669776, 0.00298038, 0.00641027, -0.0935622, 0.0500317, -0.0115315, 0.0467514, 0.0492422, -0.0385058, -0.00115853, 0.0955704, -0.0178181, 0.0197046, -0.164628, 0.040397, -0.0368614, -0.0531012, -0.0212603, -0.00292662, 0.103922, 0.055701, 0.0328057, -0.058454, 0.0266429, 0.0269186, -0.00700426, -0.0467427, -0.0499504, -0.0355726, 0.0463534, -0.0193615, 0.0618554, -0.0144416, -0.0231058, -0.0397636, -0.0453699, 0.0255484, 0.00204117, 0.0166279, 0.075213, -0.0809352, 0.0291966, -0.0141293, -0.0242923, 0.0784895, 0.0318213, 0.0237289, 0.00286598, -0.0892591, 0.0567385, 0.15133, 0.0744877, -0.065035, -0.0393914, -0.106295, -0.051208, -0.00784085, -0.0134379, 0.0592625, 0.0447585, 0.104474, -0.0326142, -0.0751301, -0.0105996, 0.0186586, -0.0715895, 0.0238159, 0.129537, 0.100185, -0.0280458, -0.0109144, -0.0269846, -0.0128242, 0.0405211, -0.0280754, -0.0412565, -0.0744175, -0.000131479, 0.0939101, 0.0742163, -0.00872451, -0.0528411, -0.0306542, -0.0156914, -0.0250541, -0.0419333, 0.118914, -0.12564, 0.0134632, -0.0181308, -0.11245, 0.0345678, 0.0471936, 0.0252622, 0.110223, 0.155457, 0.0140447, -0.00621486, -0.0584627, -0.0907916, -0.141089, -0.154192, -0.0768874, -0.0212343, 0.104732, -0.0180841, 0.0371807, -0.0606952, -0.0100973, -0.0534484, 0.0472133, 0.0255246, -0.0381199, 0.0410233, 0.0413929, 0.0352223, -0.0114626, -0.00258778, 0.0211326, -0.0410548, 0.00843022, 0.0498304, 0.112595, 0.0884775, 0.10019, 0.0714317, 0.015435, -0.0188654, -0.0443129, -0.0650301, -0.0683157, 0.00142094, 0.0549104, 0.0851535, -0.0215599, -0.0618784, 0.0446648, -0.0782039, 0.0461773, 0.0537011, -0.00919833, 0.0284723, -0.0806625, -0.0196256, -0.0844454, -0.0480207, -0.0102339, -0.083282, 0.0545214, 0.114447, 0.0414438, -0.00767613, -0.00976742, -0.00251665, -0.0692944, 0.0900492, 0.0559968, -0.00549085, 0.0226295, -0.0674452, -0.0467647, 0.0138443, -0.00695328, -0.0652408, 0.000980444, -0.0554209, 0.000846497, -0.0782116, 0.00418127, -0.0221811, 0.0209764, -0.0985771, 0.0367949, -0.046942, -0.000899754, 0.00640504, 0.0598532, 0.148573, 0.0969407, 0.10109, 0.0624535, -0.0366341, -0.0451501, -0.0545718, -0.0360945, 0.0242176, 0.02225, -0.0438289, -0.0630018, -0.0367769, -0.0266174, 0.0114581, 0.0461917, 0.0906503, -0.0533878, 0.00744892, 0.0530938, 0.124785, 0.0240381, 0.102603, 0.0293461, 0.0499103, -0.0206819, -0.148841, 0.0427706, 0.047504, 0.229716, 0.122051, -0.0366206, 0.0049325, -0.0952451, 0.0101039, -0.0137054, 0.120584, 0.111916, -0.0766759, 0.0171224, -0.0465515, -0.0078592, -0.0307607, 0.0322456, 0.0303222, 0.0299441, -0.0337092, 0.0616263, -0.0380054, 0.0380737, -0.103849, 0.0472806, -0.0020011, 0.0214511, 0.00644355, -0.0461717, -0.0195113, 0.0295242, 0.0745651, -0.0768215, -0.0146641, -0.00914874, -0.038288, 0.0153301, 0.0828571, 0.0541259, 0.0137189, -0.0335047, 0.0108166, 0.114715, -0.0186123, 0.0646971, 0.0568973, 0.0111309, 0.0487204, 0.0757739, -0.0494731, -0.00319156, -0.0553105, -0.061798, 0.0859871, 0.0528345, -0.0856757, 0.0941722, 0.115463, 0.129237, 0.0171007, -0.0581779, 0.050228, -0.105514, -0.0757188, -0.100265, 0.0238719, 0.111635, -0.0396443, -0.00395119, 0.0799739, -0.000848465, 0.0283963, -0.0320658, 0.0756824, 0.0434726, 0.00866307, -0.0872032, 0.00615147, 0.0217069, 0.0421725, -0.0617456, 0.0156436, 0.0888077, 0.185926, 0.0201189, 0.0330229, 0.0124099, 0.0345077, -0.0474637, -0.000165442, -0.115114, -0.00503651, -0.0816926, -0.118174, -0.163088, 0.00841129, 0.0429134, 0.00742718, -0.00798765, -0.0393789, 0.0774364, 0.0220239, 0.0570055, 0.0676397, -0.100073, 0.0198348, -0.0854078, -0.0253837, 0.0219089, -0.10151, -0.00475401, 0.143797, 0.0791398, 0.0142385, 0.0913249, -0.0212783, -0.133924, 0.0108001, -0.109386, -0.0395102, 0.00503547, -0.0453602, -0.0449792, -0.119914, -0.057911, 0.0604798, -0.048696, 0.0345189, -0.0265146, -0.0623896, 0.0239518, -0.00182863, 0.0338441, -0.0337867, 0.0039982, 0.0216477, 0.0124574, -0.0208618, 0.0272012, 0.0511653, -0.000574853, 0.170001, 0.176692, 0.115842, 0.00677336, -0.0897673, -0.0642611, -0.0368824, -0.0689334, -0.0156169, 0.00650603, -0.0912812, -0.100077, -0.0765018, -0.0482851, 0.109473, -0.0281313, 0.0108679, -0.0225043, 0.0711796, -0.00729955, 0.0133665, -0.0320038, 0.0254988, 0.0271578, 0.0243312, -0.0835499, 0.0349155, 0.0438735, 0.0548457, 0.103172, 0.247798, 0.183359, 0.0633802, 0.00133476, -0.146197, -0.0334589, -0.12333, -0.0147009, -0.0469486, -0.228279, -0.0706236, 0.0191047, 0.0616715, 0.0165171, 0.043247, -0.0560069, -0.0261755, -0.013459, 0.0820719, 0.0361666, -0.062122, -0.0500343, -0.0271823, -0.0588772, -0.000804529, -0.00688095, -0.0981093, 0.0962647, 0.000735546, 0.131562, 0.146177, 0.0168802, 0.00873278, 0.0277602, 0.0607409, -0.0702207, -0.0405725, -0.0435109, -0.0785145, -0.0356733, 0.0327845, -0.0176983, -0.0399978, 0.0839038, -0.0269426, -0.120658, -0.0389677, 0.0756407, -0.150033, -0.0617325, 0.0461851, -0.0414705, -0.104556, -0.1058, -0.137044, -0.249127, -0.0520572, -0.0141916, -0.0838251, -0.0833801, -0.0736992, -0.0631986, 0.0264916, 0.049765, -0.0253863, -0.0117975, -0.0173302, 0.0954939, 0.0220223, -0.00144073, 0.00772089, 0.0571833, -0.042749, 0.0612335, -0.107371, 0.0746386, 0.0680696, -0.0259955, -0.044161, -0.0150174, -0.114442, -0.189475, -0.161534, -0.249452, -0.275206, -0.153372, -0.249498, -0.133218, -0.0130192, 0.0790317, -0.0313632, 0.0154142, 0.012295, -0.00485451, -0.0789541, -0.0526976, -0.00596298, -0.00800753, -0.00210164, -0.0713619, -0.0156669, 0.0385385, 0.0577982, -0.0489247, 0.0158674, 0.00724548, -0.0528454, -0.164578, 0.0391871, -0.0572465, -0.193382, -0.174777, -0.231967, -0.208729, -0.171986, -0.0621915, 0.0996009, 0.0899608, -0.00277336, 0.0709616, 0.0921732, -0.0108195, -0.065862, -0.00622212, 0.039462, 0.0506443, 0.0544236, -0.0473158, 0.00290451, -0.0351306, 0.0515637, 0.0351069, 0.043437, -0.0263306, -0.0367951, -0.0340415, -0.106849, -0.0714472, -0.0755626, 0.00597395, -0.104605, -0.107045, -0.00057949, -0.0403787, 0.0339483, 0.0767359, -0.0586973, 0.0605234, 0.0385212, 0.048417, -0.0240206, 0.033012, 0.0788065, 0.0132855, 0.0336945, 0.030005, -0.0345779, -0.00469384, -0.0176256, 0.0189115, -0.0559641, 0.0656179, 0.00819235, 0.0245009, 0.0330352, 0.0302669, 0.0394134, 0.098244, 0.143233, 0.0286637, -0.00762407, 0.00656915, 0.0762438, -0.0512232, 0.0075595, -0.0349803, -0.0840806, -0.039677, -0.122559, 0.0526756, 0.0432021, 0.0949649, 0.0210399, -0.0216175, 0.0734788, 0.0511083, 0.0641415, -0.0484233, -0.0408176, 0.0105362, 0.0317853, 0.0388696, 0.104905, -0.0185627, -0.00566628, 0.163709, 0.132653, 0.0222652, 0.0964815, 0.1437, -0.019055, 0.0180515, -0.0678625, 0.00558989, -0.00952414, -0.0559265, -0.00103557, 0.0539499, 0.00457849, 0.034159, 0.00357799, -0.0525444, 0.0119074, -0.0429234, -1.69506e-05, -0.0215701, -0.0564291, -0.0318418, -0.0394073, -0.0279272, 0.0483122, 0.0566002, 0.190391, -0.00640791, -0.0474982, 0.0717083, 0.119081, 0.128951, 0.0342925, 0.0342849, 0.106972, 0.0600823, 0.0112353, 0.00131601, 0.0110126, 0.0444794, 0.00478705, -0.00324093, -0.0216945, 0.0545533, 0.00258701, 0.00860537, 0.0017097, 0.0273858, 0.0473254, -0.0696782, 0.0791998, -0.0345929, -0.129244, -0.0229406, 0.0462084, -0.133761, -0.0409722, 0.158072, 0.0295193, 0.130215, 0.0719456, -0.00233774, 0.113761, 0.0402824, 0.0319131, 0.0378309, 0.114515, 0.0178408, 0.0296064, -0.00331155, 0.0985253, 0.0625917, 0.00121833, 0.0696922, -0.0266575, 0.106984, -0.0356783, -0.0436898, 0.0490126, 0.0267818, -0.0317759, 0.059382, -0.0178102, 0.0337178, 0.00461752, 0.00977878, 0.101034, -0.0602915, -0.011949, 0.100582, 0.0276589, 0.0336215, 0.066919, -2.44712e-05, 0.0933841, 0.00937267, 0.00176917, -0.0561751, 0.0380982, -0.0430968, 0.0220037, -0.0359346, 0.00832679, 0.0330518, -0.0422082, 0.0269936, -0.0226482, -0.0356115, -0.0414447, 0.0226375, 0.0555728, 0.0874441, -0.0166829, -0.0442796, -0.0114372, -0.075562, -0.020809, 0.0357453, 0.107315, -0.100806, -0.0176711, -0.0237032, 0.0307933, 0.0336882, 0.0200876, 0.00654357, -0.067279, -0.0463851, 0.0394833,
-0.0274286, -0.111903, -0.00949961, -0.0254555, -0.0449538, 0.0600028, 0.01671, 0.0197478, -0.0694173, -0.0238967, -0.0428261, -0.0179525, -0.00479104, -0.0412155, 0.0239469, 0.0913683, 0.0316478, -0.062107, -0.00755842, 0.0985425, 0.0826995, 0.00422862, 0.0892373, 0.127315, -0.0377019, -0.0443744, 0.0646712, -0.0583746, -0.0157164, 0.0485795, -0.0242672, -0.0342382, -0.00455041, -0.0594185, 0.0297931, -0.0504564, 0.0740389, 0.0104662, -0.0090188, 0.089583, 0.0203883, 0.0936112, 0.0227239, 0.0541323, 0.0622033, -0.0534917, -0.019465, 0.0604447, -0.0492819, -0.0664747, -0.00169265, 0.00874802, -0.0794789, 0.0640543, -0.0590856, 0.0611611, 0.0184354, -0.0256587, -0.0143652, 0.032983, 0.0311306, 0.00111587, -0.00302717, -0.0534387, -0.0519149, 0.0429887, 0.0565513, -0.00166184, -0.0268077, 0.0365936, 0.0258115, 0.0638008, -0.0140872, -0.00887949, 0.0218183, -0.0753888, 0.055916, -0.0268858, -0.0213227, -0.0148314, 0.0110939, -0.0291742, -0.0347332, -0.0278666, -0.0550235, 0.027799, 0.0103366, 0.032168, 0.0619451, -0.0274808, -0.0308561, -0.000120123, -0.0255429, 0.0209943, 0.0531942, 0.0563244, -0.0211725, -0.0742821, 0.00362271, -0.0317595, -0.0034912, 0.0656927, -0.0424114, -0.0803564, 0.0438983, -0.0604798, -0.074573, 0.0183322, 0.0124015, -0.0283631, 0.0515007, -0.021203, -0.00851806, 0.00368658, -0.0714216, 0.112165, 0.0450668, -0.0170159, -0.0105181, -0.0122407, 0.02343, 0.0368435, -0.0491438, -0.0243792, 0.0140905, 0.0101562, 0.00269951, -0.1249, -0.0279835, 0.00473102, -0.0416446, 0.110499, -0.0210928, -0.0787176, 0.0240488, -0.00615117, -0.0066243, -0.0474091, -0.0188689, 0.031266, 0.0839514, -0.00872962, -0.0306716, 0.0170156, -0.043777, -0.0619894, 0.063258, -0.0433966, 0.0619403, 0.030075, -0.0519344, -0.0488075, -0.163434, -0.050456, -0.0568631, -0.057449, -0.0106422, -0.0286068, -0.0323438, -0.0520354, 0.0100019, -0.0547606, 0.0161893, -0.0693178, 0.0747805, 0.0152058, -0.0663771, -0.052175, 0.0453061, -0.0465625, -0.0363815, -0.0921867, 0.0118842, 0.0650367, 0.0644923, 0.0206193, 0.0831621, 0.00172465, 0.0269423, -0.0179372, 0.0414921, -0.00840628, 0.00508194, -0.0103506, -0.0348832, -0.0798552, -0.0973862, -0.103207, -0.115996, -0.0248746, -0.0520433, -0.0559335, 0.0966153, 0.00571947, -0.0371922, 0.06974, -0.0414287, 0.0251829, 0.0258749, 0.103348, -0.0367225, 0.03333, 0.175018, 0.0415519, 0.121447, 0.0715793, -0.0189927, 0.0541664, 0.0278339, 0.0810641, 0.0311029, -0.0195913, 0.0635424, 0.0646504, 0.0176458, -0.046277, 0.00828048, -0.0307163, -0.0355503, -0.0170707, 0.103569, -0.0377666, -0.0377959, 0.001755, 0.0447201, -0.18751, 0.0719302, 0.0287478, -0.00115339, -0.0172466, 0.0775912, 0.0957603, 0.0442424, 0.10177, 0.196592, -0.0149571, 0.0768204, -0.0244933, 0.151559, 0.0586681, -0.048331, 0.0137541, 0.000575149, 0.00311714, 0.0672032, 0.0866166, 0.0958529, 0.0849613, 0.0872031, 0.0645225, -0.0186831, -0.0302349, -0.0383566, -0.0393253, 0.148836, 0.0524475, 0.0458756, 0.000781901, 0.0652383, 0.0121904, 0.0182885, -0.00578979, -0.00593348, 0.0341508, 0.193523, 0.0516356, 0.0985783, 0.0719307, -0.00932964, -0.0323262, 0.0410318, 0.066984, -0.0174003, -0.0502565, 0.0651419, -0.0622489, 0.100846, -0.0156658, -0.020109, 0.0580756, 0.00344326, -0.0118732, 0.0564667, 0.0466606, 0.00781957, 0.0291323, -0.00564664, 0.000334626, -0.00723026, -0.0756388, 0.0283299, -0.0879519, 0.103624, 0.129965, 0.0178063, 0.0466488, 0.0985227, 0.0391121, -0.0690298, -0.0597048, 0.0666583, -0.0185698, -0.132466, -0.122279, -0.0727496, -0.11924, 0.0248023, -0.0183543, -0.0873208, 0.0407216, 0.00820749, -0.0914411, -0.0555335, -0.0238761, -0.0671043, -0.103347, -0.0517015, -0.0673745, 0.0783382, 0.15394, 0.0372943, -0.0826557, -0.136728, -0.060629, -0.0126573, 0.0102221, -0.0432228, -0.0631301, -0.0377053, -0.092275, -0.152169, -0.160634, -0.110142, -0.0872081, 0.0262972, 0.0208288, 0.039996, -0.0431311, -0.0280881, -0.0130407, -0.0212592, -0.0402347, 0.00889056, -0.0548441, 0.0176543, -0.0566015, 0.037482, 0.0227226, -0.143467, -0.248644, -0.137798, -0.133193, 0.0211652, -0.0526434, 0.00713689, -0.13424, -0.191241, -0.0492223, -0.115221, -0.0426721, -0.140781, 0.0754861, -0.0727862, -0.0788549, -0.00730959, 0.0213594, 0.00493659, 0.070618, -0.105729, -0.0220587, -0.0381278, -0.0705594, -0.094917, 0.0122664, 0.151498, -0.0935828, -0.140002, -0.124593, 0.000616808, -0.0966347, -0.0823596, -0.0185228, 0.0591251, -0.0939306, -0.101644, -0.0773373, -0.0372016, -0.071364, 0.0328573, 0.140328, -0.0677767, -0.0560735, 0.0158138, -0.0339788, 0.0837729, 0.0318498, -0.0667474, -0.102124, -0.112426, -0.116827, 0.0234495, 0.0321967, 0.0872635, -0.0132233, -0.131571, -0.115952, -0.0515595, -0.0476865, 0.014942, -0.0498787, 0.094371, -0.0330546, 0.0428864, 0.0203187, -0.0437036, -0.028896, -0.0897254, -0.0232722, 0.078218, -0.000361426, -0.00823432, 0.0953954, -0.0332362, -0.129438, 0.0132886, -0.121064, -0.0303668, -0.064256, -0.0406538, -0.00165001, -0.0494916, -0.0984635, -0.0929498, -0.0153271, -0.0988045, -0.0565239, -0.124116, 0.0260845, 0.0485928, 0.0294576, 0.0784567, 0.137857, 0.0691924, 0.00891008, 0.052138, 0.0360291, -0.162623, -0.0161936, 0.000497525, 0.0347718, -0.00701228, -0.0196338, 0.0166577, -0.0321581, 0.0131183, -0.0111447, 0.0502505, 0.0105352, -0.043317, 0.00506556, -0.152426, -0.0663046, -0.137726, -0.00804465, -0.0147812, 0.069318, -0.108651, -0.038426, -0.00879265, 0.0496931, 0.0835395, -0.0477263, 0.0715571, 0.00330688, 0.003183, 0.0168059, 0.0362692, -0.00198287, 0.00526324, 0.0486079, -0.0186636, -0.0640518, -0.143543, -0.00323124, 0.0586722, 0.182173, 0.0889017, 0.019966, -0.065195, -0.02024, -0.0608392, 0.058562, -0.0203845, 0.0603167, 0.123344, -0.0180863, 0.11049, -0.0360586, 0.108732, 0.0799996, 0.00846498, 0.0518358, -0.00984875, -0.0116623, -0.049435, -0.105036, -0.0118488, -0.0358726, -0.0882748, -0.110492, 0.0932068, 0.031036, 0.0867946, 0.19545, -0.00213969, -0.0110414, -0.0806969, -0.118485, -0.077706, -0.111638, -0.052295, 0.132838, 0.119137, 0.00217953, 0.0230466, -0.0495506, -0.0223068, 0.0701078, 0.0649401, 0.0415708, 0.0312743, -0.0197475, -0.0711927, -0.00478547, -0.0406681, 0.0295016, 0.0213132, -0.0681846, 0.034656, 0.028763, -0.0301827, 0.0733845, 0.0962947, 0.184848, 0.183396, -0.00211861, 0.055553, -0.027122, 0.0995685, 0.16992, 0.0539373, 0.0745375, 0.0985752, 0.078648, 0.0340227, 0.0780606, 0.108954, -0.0136247, 0.0327908, -0.0149894, 0.03091, -0.0201942, -0.0932766, 0.0180254, 0.00191174, 0.0137838, -0.0824215, -0.0750605, 0.0252146, 0.0698053, 0.0621533, 0.2052, 0.187166, 0.152937, 0.136279, 0.188911, 0.0424329, 0.055596, 0.0583456, -0.00536822, 0.0254175, 0.0757242, 0.0159929, 0.0233742, 0.0440671, 0.00515151, 0.00114834, 0.016699, -0.0211933, -0.0729663, -0.00284031, 0.0666464, -0.00614583, -0.118846, -0.178527, -0.0976011, -0.129076, -0.0633321, 0.0834147, 0.153531, 0.054718, 0.131404, 0.108306, -0.0301332, 0.103568, 0.04424, -0.075032, 0.0335605, -0.123201, -0.0863102, 0.0696156, -0.00961532, 0.0666522, 0.0634563, 0.0173046, -0.0275456, 0.095579, 0.013919, -0.0592578, -0.067333, -0.129106, -0.105974, -0.0858899, -0.0424711, -0.0725194, 0.040939, 0.131406, 0.0225252, 0.0636517, 0.10572, 0.0560733, 0.0354909, 0.043711, 0.012045, -0.0510977, -0.0783097, -0.139206, 0.0434883, 0.00200617, 0.120996, -0.0599466, -0.00819823, 0.0758875, -0.0878399, 0.0286171, 0.0027986, -0.0357517, 0.00584093, 0.0623178, 0.0329781, 0.0137438, 0.0330068, 0.0169367, 0.0289024, 0.059435, 0.0953174, 0.00108642, -0.0135939, -0.0327571, -0.0768193, -0.0635608, -0.12791, -0.11377, -0.0523689, -0.0658049, -0.0551341, 0.0205404, 0.0142108, -0.00640339, 0.0428223, -0.0213742, -0.00345016, -0.0429921, -0.00010433, 0.0209502, -0.0534117, 0.0859897, 0.0226473, -0.0241747, -0.0865095, 0.0064508, 0.0296756, -0.0881619, -0.0906468, -0.0478067, 0.00284214, -0.0258521, 0.112641, -0.0217849, 0.117205, 0.0197918, -0.000962977, -0.0801827, -0.0292943, 0.00291337, -0.035607, 0.0713053, -0.0141537, 0.0251373, 0.0215609, -0.04928, 0.0868643, -0.0191712, -0.00980162, -0.0880197, 0.0561993, 0.0599097, 0.0289879, -0.00319065, 0.0582662, -0.0030976, -0.00487106, 0.123043, -0.0900833, -0.0282252, 0.0488507, -0.0885589, -0.0398412, 0.0396739, -0.00338256, -0.112894, -0.0252734, -0.0377713, -0.0104974, 0.0567561, 0.084967, -0.0793767, 0.0174134, -0.0207987, 0.0197188, -0.05982, 0.0165806, 0.0245119, -0.0320881, -0.0571881, -0.0186242, 0.0270539, -0.0936548, -0.106724, 0.0115795, -0.0151431, 0.041195, -0.0287075, 0.0321685, 0.0817686, 0.0132347, 0.081512, 0.108408, -0.0163792, -0.0229462, -0.00904234, -0.0696147, -0.0624572, -0.0770615, -0.00854178, 0.0562052, -0.0385061, 0.0121918, 0.0787479, 0.0213336, -0.0786857, 0.0366594, -0.0676078, -0.0648602, -0.00261416, 0.031553, 0.0507687, -0.0502776, 0.0196953, 0.0392022, -0.00284216, -0.0580042, -0.0664065, 0.0431327, 0.0642497, -0.040682, 0.0365921, 0.0300356, -0.0047175, -0.0247394, 0.00605646, 0.0300957, 0.0439212, -0.116181,
0.0339426, -0.0220702, 0.0272975, -0.0388139, 0.0247358, -0.0148556, -0.0135904, -0.0405522, 0.0255647, 0.0373811, 0.0312114, 0.00759, -0.0260169, -0.0141238, -0.0873337, 0.03179, -0.0118772, -0.050542, -0.0111688, -0.0587419, -0.0415364, -0.0266549, -0.0475519, 0.00297951, -0.043901, 0.0273485, 0.0373821, 0.0413998, -0.0742432, -0.066093, -0.0640258, -0.0264377, -0.0498292, -0.0404023, -0.0391088, 0.00137186, 0.0372037, 0.00477386, 0.0474412, -0.0228248, -0.0715083, 0.0744263, 0.0406976, -0.0291002, 0.0399894, 0.048241, 0.0145957, 0.0466511, 0.00225542, -0.0122891, 0.0448964, 0.000871261, 0.0304944, -0.0160317, -0.0583279, 0.0662377, -0.062472, 0.00928213, -0.0331989, 0.0615777, 0.0206183, -0.0532928, 0.00115097, 0.0298974, 0.0179338, 0.0124197, -0.0285394, -0.0044466, 0.0985567, -0.0874108, -0.0709962, 0.0648079, -0.0460041, -0.0197363, 0.102294, -0.0171471, 0.0254674, 0.0589914, -0.084924, 0.0750284, -0.00379315, 0.0285026, -0.0307163, 0.030606, -0.0044196, -0.0872085, 0.000978811, 0.0134074, 0.0053001, 0.0778584, 0.0294438, -0.043788, -0.0718306, -0.0242568, 0.143417, -0.0597994, -0.0287364, 0.0783933, 0.114649, 0.0456063, 0.0531743, 0.023015, -0.0193021, -0.0401464, -0.0465911, 0.0805676, 0.0444764, 0.00381453, 0.00162014, 0.00033964, -0.0239282, -0.0704196, 0.0221196, -0.0161972, -0.0213217, 0.0180867, 0.0312249, -0.0261041, -0.122941, -0.0955398, -0.0394111, -0.104401, -0.0234302, 0.063075, 0.0884927, 0.146168, 0.0843618, 0.051704, 0.150956, 0.100158, 0.0682597, 0.023355, -0.0409661, 0.0107846, -0.101484, 0.0385222, -0.0417653, -0.0381048, 0.0344152, 0.0545987, -0.0377842, 0.0334775, 0.0764111, -0.00430554, 0.0211465, -0.0780722, -0.0231278, -0.109683, -0.0921771, -0.0322165, 0.00949698, 0.151328, -0.111219, 0.08605, 0.0334493, -0.00666548, 0.00236998, 0.0440373, 0.0820272, 0.00664667, 0.0219561, 0.0678363, 0.0328535, -0.0494974, -0.0354757, -0.00341663, -0.0156908, -0.0100078, -0.0525421, 0.0375878, 0.09151, -0.0962177, -0.119205, -0.0161948, -0.164146, -0.0881302, -0.118233, -0.120049, -0.0593529, 0.0751276, -0.00830363, 0.0244636, 0.0911107, -0.00206613, -0.099218, -0.100021, -0.140443, 0.0514329, 0.000580408, -0.0689502, 0.0801128, -0.0292724, 0.0395475, 0.0600093, -0.0245629, 0.00883293, -0.0707518, -0.119761, -0.0627452, -0.0966063, -0.0478485, -0.023269, -0.0615046, -0.179129, -0.0648035, -0.0370237, 0.0308741, 0.0516249, 0.00976952, -0.0477883, -0.0928468, -0.0548907, -0.146429, -0.102499, 0.0178111, -0.0566753, -0.0253487, 0.0397819, 0.0822657, 0.115633, 0.0408817, 0.018069, -0.0715763, -0.00322858, 0.0259506, 0.0326219, 0.0315435, 0.00551711, -0.128678, -0.105435, -0.146078, -0.02871, -0.114296, 0.0369606, -0.0218185, 0.0500003, 0.151026, 0.147872, 0.196754, 0.035842, 0.0607946, 0.0641928, 0.079983, 0.0719683, 0.0665323, -0.0214458, -0.0109671, 0.120533, 0.0574269, 0.0787801, 0.0472929, -0.0522897, 0.0415442, -0.0319712, -0.0388292, 0.0371751, -0.121094, -0.118393, 0.00346323, -0.0336187, -0.0737104, 0.0143799, 0.120174, 0.0272831, -0.0297561, 0.111725, 0.195634, 0.0595074, 0.118158, 0.0464121, -0.000505511, 0.0932298, 0.0878069, 0.0821642, 0.00368891, 0.17319, 0.113787, 0.114011, 0.00423894, -0.0538881, -0.0346489, 0.036093, -0.0449795, -0.0929519, 0.0328646, 0.0487133, -0.0571178, 0.0523914, 0.0294062, 0.0724719, 0.118373, 0.0576765, 0.0549197, 0.22175, 0.331742, 0.124262, -0.000468931, 0.0917901, 0.0938255, -0.0479499, 0.00105336, 0.0453215, 0.156601, 0.165526, 0.183278, 0.0470965, -0.0166028, -0.00499043, -0.019498, -0.018548, -0.0915914, -0.0599977, -0.0206499, -0.0669104, 0.103066, 0.0998611, -0.0575889, 0.0118457, 0.0709592, 0.0157424, 0.130908, 0.162721, 0.143454, -0.0401096, -0.025932, -0.0362468, 0.0373592, 0.047399, -0.057732, 0.140197, 0.0949781, 0.127446, 0.0854723, 0.0664662, 0.0974736, 0.0393945, 0.0324894, -0.0464716, -0.0480478, 0.0293875, 0.00686676, -0.152214, 0.081743, 0.0710643, 0.0142891, 0.128707, 0.0395746, 0.121023, 0.0218703, 0.0886364, 0.0735169, -0.0579846, -0.182467, 0.103195, -0.0172514, -0.017395, -0.00652244, 0.0623024, 0.0633756, -0.000826363, 0.0902912, 0.0241734, 0.00698939, -0.00826623, 0.074958, 0.0673344, -0.0368377, -0.0369762, -0.00841104, -0.0599553, -0.020677, -0.022873, -0.0753453, 0.153756, 0.0908504, -0.0103276, 0.153662, 0.112612, 0.0292042, 0.0492284, -0.107442, -0.0416274, -0.112847, -0.0571663, -0.0494875, -0.0253901, 0.0436962, 0.0320881, -0.00232147, -0.0167407, 0.0358179, 0.00492865, 0.0440559, -0.0717714, -0.050077, 0.0816769, 0.00136483, -0.00620469, -0.052303, -0.0282702, -0.0886427, -0.0890156, 0.0675043, 0.124224, 0.0873833, 0.1586, -0.0102445, -0.140533, -0.0643459, -0.0937645, -0.0964011, -0.133652, -0.107404, -0.0119682, -0.0254022, 0.00619874, 0.0907796, -0.0124198, -0.100158, -0.0641917, -0.00921107, -0.168763, -0.0184614, 0.00805037, -0.0501018, 0.00451065, 0.00442936, -0.114781, -0.068608, 0.0204094, -0.0468968, 0.00829191, 0.150852, 0.131352, -0.00727934, -0.174701, -0.109589, -0.232217, -0.0409266, -0.152267, -0.116988, -0.0411562, -0.101421, -0.0178995, 0.0665792, 0.0631384, -0.0538935, -0.0233229, 0.0294798, -0.0267465, -0.0250921, 0.0200885, 0.0158346, 0.0346801, -0.0907268, -0.0406688, -0.0205908, -0.0308523, -0.00738936, 0.179429, 0.104816, 0.131492, -0.0576726, -0.142667, -0.101088, -0.149447, -0.152836, -0.111425, -0.107354, -0.199126, -0.0693107, 0.0513797, -0.0328195, -0.0018499, 0.016886, 0.0641248, -0.00137341, 0.0595069, -0.0176895, 0.0262136, 0.00922395, 0.0185372, 0.00720052, -0.0363632, 0.0109743, 0.0640566, 0.199085, 0.0848354, 0.121846, -0.0240423, -0.0239616, -0.109273, -0.121982, -0.11053, -0.12041, -0.0534196, -0.0357201, -0.0248752, 0.00251667, -0.010778, 0.00669959, -0.0251704, 0.0308057, -0.0278058, -0.0259249, 0.00850606, 0.0153051, 0.0258264, 0.0587521, 0.0468264, 0.0555125, 0.0433239, 0.0764025, 0.191149, 0.133516, 0.0713297, -0.0132407, 0.0216724, 0.0200479, -0.0612601, -0.0373907, -0.0789501, -0.0697029, -0.0506666, -0.0251782, -0.0303307, 0.00817833, 0.0165062, -0.0369367, 0.0105313, 0.0261554, 0.01354, 0.0285593, 0.0458343, 0.0232886, -0.0232045, 0.0181873, 0.0628024, 0.0584799, 0.0756186, 0.158503, 0.0434229, 0.092369, -0.0371054, 0.0438731, 0.0128749, 0.0305015, -0.0373343, 0.0627372, 0.0166874, -0.0257342, -0.0747084, -0.00371792, 0.0443748, 0.107908, 0.0655145, 0.0845598, -0.0486558, 0.00676961, -0.101445, -0.0511828, 0.0835714, 0.0516596, -0.0830243, -0.0179095, 0.0292841, 0.124806, 0.00620758, 0.124988, 0.0986747, 0.11907, 0.0949075, 0.249613, 0.0265686, -0.040735, 0.183798, 0.0462189, 0.162208, 0.19982, 0.0724463, 0.0807625, 0.0803928, 0.0938893, -0.0410214, -0.0345421, 0.0174115, -0.0349491, -0.0312117, 0.0542769, -0.00361696, 0.03199, 0.0152778, -0.13417, -0.0159212, 0.0977988, 0.140629, 0.0555064, 0.0499071, -0.0110325, 0.070121, -0.0623145, 0.034351, 0.0732231, 0.0921928, -0.060701, 0.0450907, 0.126725, 0.021073, -0.0355594, 0.0160645, 0.0108691, 0.0166453, -0.0245513, 0.0578891, 0.0333164, 0.0366011, -0.0118232, 0.0143636, 0.0930366, 0.0831228, -0.0482141, 0.0993954, 0.113219, 0.0682183, 0.000118583, 0.0507083, -0.0037431, 0.00821876, 0.0514702, 0.121678, 0.0429777, 0.147845, -0.0903754, -0.058394, 0.0208626, -0.0346134, -0.0530088, -0.098023, 0.0643894, 0.0170759, -0.062348, 0.0140858, -0.073541, 0.0551521, 0.076774, 0.10443, -0.105101, -0.0400576, -0.0170041, -0.018866, -0.0273608, 0.0786388, 0.0721559, 0.0945752, 0.042047, 0.0406372, 0.0680944, 0.124137, 0.0704155, 0.0822881, 0.0596715, 0.0888428, 0.017997, -0.0863488, -0.0838511, 0.0319665, -0.0265609, -0.0150005, 0.0547957, 0.0544199, 0.0137797, 0.0115921, -0.017062, -0.0643019, -0.0567329, 0.0477275, -0.0220641, -0.0271974, -0.146611, -0.138465, 0.0145626, 0.108265, 0.108228, 0.0999302, 0.204162, 0.18375, 0.107016, -0.054878, 0.0603517, -0.0660093, -0.0251902, -0.0201186, -0.123451, -0.0356083, -0.0885352, -0.0231209, 0.0736769, -0.0940663, -0.0357361, -0.037184, 0.00419057, 0.029006, -0.00196591, -0.0621472, -0.0130778, -0.00150422, -0.141958, -0.0396962, -0.0422527, -0.0240373, -0.0159374, -0.0134606, -0.100096, 0.0134219, -0.118769, 0.0119191, -0.00349144, -0.0414735, -0.0721567, -0.0943712, -0.00583206, -0.0117966, -0.0618959, -0.10786, 0.000191138, 0.0319638, 0.0396181, 0.150942, -0.0185095, -0.00158846, 0.0187126, 0.0657779, 0.00266452, -0.0592651, 0.0234149, -0.0676925, 0.0355551, -0.0836716, -0.0308567, -0.0311116, -0.0804818, -0.0389795, -0.0655216, -0.0232151, -0.0156389, -0.0411416, 0.00382901, -0.0361248, -0.0835682, -0.0700388, -0.11623, -0.0451081, -0.00868584, -0.00767017, -0.0655188, 0.00652695, -0.00401389, -0.0346803, -0.0424506, -0.114132, -0.0135311, 0.00153574, -0.024528, 0.0956548, 0.0172988, 0.0236621, 0.0404278, -0.0126408, 0.0726613, -0.162628, -0.0628656, 0.00453643, -0.0127189, 0.0775036, -0.0821339, 0.0525115, -0.00172952, -0.119187, -0.0752335, -0.0745176, 0.0254562, -0.0411193, 0.0318445, -0.0227563,
-0.0521651, 0.0835123, -0.0361058, -0.0227401, 0.0433177, 0.062938, -0.062438, -0.0351793, -0.13069, -0.0850534, 0.00699429, -0.0744093, 0.0588154, -0.0413277, 0.0124028, 0.0341777, -0.0275844, -0.0621318, -0.0369046, 0.0186996, 0.0132867, 0.0943874, -0.001167, 0.0399912, 0.000831897, -0.00514653, 0.032994, 0.0123315, 0.122329, 0.0305791, -0.0299328, -0.0203632, 0.0448933, -0.0737157, 0.0544949, 0.0523814, -0.0417358, -0.00154911, 0.0609242, 0.0825183, 0.0435419, 0.0906297, 0.00645318, 0.0303908, 0.00187132, 0.0334179, -0.0197402, 0.0146018, -0.0453724, 0.00676158, -0.0216536, -0.0278436, 0.0977457, -0.0131784, 0.0503244, 0.0491331, 0.000490578, 0.0255656, -0.0309301, 0.0390798, 0.0278855, -0.103223, -0.00914681, -0.0457807, 0.00313998, 0.0717798, 0.0267418, -0.00591991, -0.0134874, -0.0219139, 0.0188452, 0.0482924, -0.11937, 0.0373129, -0.045663, -0.00279323, 0.0188867, -0.0685266, -0.00818598, -0.0633098, 0.0139853, 0.0553753, -0.0137773, -0.0928841, 0.0202117, 0.0425312, 0.000171045, -0.0175697, -0.0501256, 0.0196288, 0.0136262, 0.025449, -0.0626936, 0.114385, -0.00834813, -0.0487608, 0.0125667, 0.110593, 0.0917282, 0.0987271, 0.0298497, 0.000168998, -0.00802524, -0.0471447, -0.0364735, 0.0430045, 0.0574881, 0.0131381, -0.00550275, 0.0443273, -0.0167058, 0.00327174, 0.0727984, -0.0688636, -0.0177074, -0.109522, -0.12504, -0.0295752, -0.0628405, 0.0550586, -0.118269, 0.013853, -0.0893487, -0.0653838, 0.00332841, -0.0639749, 0.104288, 0.0750965, -0.0192737, -0.0345471, -0.0163751, 0.0793415, -0.0104421, -0.0595288, 0.0808657, 0.00847093, 0.0247161, -0.0481572, 0.0351418, -0.02206, 0.0418225, -0.00925373, 0.029837, -0.00307648, 0.0528277, -0.0969772, -0.065645, -0.105986, -0.00404872, -0.0946176, -0.13892, -0.050816, 0.0734998, 0.0794764, 0.135077, 0.0437814, 0.146262, 0.0644931, 0.0063059, -0.0737057, -0.00250651, -0.00246194, 0.0483272, -0.00682377, 0.0625433, 0.0393666, 0.0238444, -0.0713679, -0.0651918, 0.0435735, 0.0152854, 0.00877282, -0.077699, -0.0166667, -0.0677172, -0.0557535, -0.0744298, -0.143154, -0.125871, -0.0611805, 0.135841, 0.00658583, 0.0131785, 0.0790988, 0.0635843, 0.0790105, -0.0177368, 0.012285, -0.0112823, 0.0355416, 0.115886, 0.0108029, 0.0422943, -0.0471192, 0.0358373, 0.0700763, 0.0599099, 0.0334392, -0.0439789, 0.00590651, 0.053011, -0.102086, -0.00877082, -0.0249315, -0.0345299, -0.0186853, -0.0159897, 0.0309624, 0.0806644, 0.0847552, -0.0772936, 0.113164, 0.0519908, 0.0556087, 0.0347019, 0.0880407, -0.00380578, 0.0618175, -0.119405, -0.0174197, 0.0325398, -0.005557, 0.0673402, -0.102807, -0.0131899, 0.0410712, 0.0226028, 0.034939, -0.00865141, -0.102093, 0.0279541, -0.012834, -0.0249521, -0.158418, 0.0568369, 0.185245, 0.064429, 0.0190721, -0.133192, -0.0304843, -0.0434171, 0.0351258, 0.105212, 0.0344059, 0.088221, -2.39805e-05, -0.172059, -0.00520272, -0.0167015, 0.0601656, 0.00653929, 0.00477517, -0.00960138, -0.0628781, -0.111836, -0.0477896, -0.0523491, -0.139623, 0.0793265, -0.0271048, -0.059532, -0.0749, 0.0654635, 0.0474241, 0.0868252, -0.0504032, -0.11225, -0.170371, -0.0193294, 0.0502706, -0.0392016, -0.130536, -0.0425191, -0.0845672, -0.0790333, -0.103409, 0.0436839, -0.0131036, 0.0765463, 0.0220652, 0.0949373, 0.0234648, -0.018043, -0.0679603, 0.0116693, -0.060047, -0.0374797, -0.0414357, -0.0459641, -0.102337, 0.112817, 0.106357, 0.167614, -0.0204096, -0.126803, -0.174334, 0.0153913, 0.00365879, -0.138358, -0.0831828, 0.0476583, -0.0761383, -0.126208, 0.0287536, 0.0162237, -0.00369814, -0.108353, -0.0322454, 0.0385816, -0.0785564, -0.00932199, 0.00434002, 0.0328344, 0.0334906, 0.0591729, -0.0490808, -0.0581271, -0.0460402, -0.00874002, 0.0751418, 0.0751233, 0.0176358, -0.165222, -0.101076, 0.0854416, 0.0591056, 0.0182035, 0.0340247, -0.0847889, -0.0571963, -0.0204332, -0.0866741, 0.0248251, -0.0188171, -0.0432587, -0.00567052, 0.0562855, 0.033582, 0.0624386, -0.0210094, 0.045513, 0.0206997, 0.0629698, -0.036775, 0.048443, -0.0362547, -0.151312, 0.0637137, 0.0111861, -0.0831515, -0.0800128, 0.00028642, 0.0927411, 0.0150363, 0.00139945, -0.0138675, 0.000515487, -0.0343819, -0.0046716, 0.0211094, 0.0207364, 0.048518, -0.0515029, -0.0335831, -0.0136299, -0.0554067, -0.0385206, 0.0092129, -0.0243116, -0.00390864, 0.000701689, 0.0757642, -0.047012, 0.0323826, 0.0587656, 0.0948972, -0.0222345, -0.0907901, -0.127371, 0.0102443, -0.0981112, -0.116109, 0.0426615, 0.086559, 0.0264028, -0.0788052, -0.0527048, 0.0692528, 0.0389432, -0.0198811, -0.00614851, -0.0817, -0.00117989, 0.00781672, -0.0175467, 0.0100317, 0.111495, 0.0835852, 0.0140495, 0.113843, 0.0669721, -0.0410969, 0.00951172, 0.0353445, -0.0818216, -0.0493434, 0.0748742, 0.0212479, -0.0807599, 0.0106751, 0.0939172, 0.135409, 0.0354561, -0.0170379, 0.00501488, 0.0707563, 0.00873907, 0.0972131, -0.00159041, -0.0190954, -0.0736572, -0.056062, -0.0126776, 0.0291933, -0.0199068, 0.0339394, 0.19142, 0.17418, 0.160888, -0.0365605, 0.0497835, 0.0288542, -0.0085276, -0.0774152, 0.0136392, 0.00611844, -0.030633, 0.037089, 0.127243, 0.0189684, 0.00150537, -0.0358907, -0.0234219, -0.0135404, 0.012427, 0.0948068, 0.0500109, -0.0848186, -0.0123642, -0.032171, -0.00116631, -0.0401542, -0.0590264, 0.0787985, 0.017985, 0.191463, 0.0859938, -0.0767427, -0.11461, -0.0661554, -0.0394962, 0.0188711, -0.016326, 0.0618651, 0.0236634, 0.0230436, -0.00185934, 0.0771527, -0.011173, -0.0828042, 0.0254682, 0.0283842, 0.042123, -0.0415687, 0.0082264, -0.0606038, -0.106218, -0.0568698, 0.00188345, 0.0530427, 0.0292648, 0.0329809, 0.138624, -0.0877283, -0.015308, -0.0104565, -0.246551, -0.124527, 0.0604842, 0.180503, 0.0865257, 0.174693, 0.108129, -0.0121524, -0.0816509, 0.0307477, 0.0681767, 0.00739663, 0.0784012, 0.0328202, 0.0422916, 0.0922028, -0.0643216, 0.0393325, -0.0022228, 0.0679569, -0.05928, 0.0124317, 0.0384226, 0.120084, 0.101851, 0.0412762, -0.0553325, -0.156213, -0.0602093, 0.0121814, 0.113006, 0.0899753, -0.0889047, -0.0934305, 0.0131567, -0.0143226, 0.122341, -0.0142365, 0.0514162, -0.0179281, 0.0907536, 0.105061, 0.0556649, -0.0328283, 0.0169202, 0.00411541, 0.00113103, 0.0383774, 0.0234127, -0.00960127, 0.00555698, 0.0596763, 0.120379, -0.0524543, -0.0848246, -0.134714, -0.0702863, 0.0591338, 0.079365, 0.0177923, -0.0634994, -0.164547, -0.0419076, 0.102797, -0.0517227, 0.0302304, -0.0102001, 0.0911618, 0.0084927, 0.0106631, -0.0726812, 0.024355, 0.0116333, 0.0464358, 0.0686931, -0.0126742, 0.0258733, -0.0500842, 0.050001, 0.0569121, -0.0138749, -0.0387838, -0.0494717, -0.154106, -0.0939454, 0.155117, 0.0473953, 0.0120477, -0.0769121, -0.0540069, 0.0838782, 0.023255, 0.0173847, 0.0132545, 0.0227915, 0.0599662, 0.0697226, -0.0120365, 0.0433377, 0.0259839, 0.0521137, -0.0551165, -0.0433975, -0.0535812, 0.0117197, -0.0518848, 0.0126841, 0.0100795, 0.0754992, -0.0901766, 0.0078651, -0.00572259, -0.0253599, 0.01299, 0.0767785, -0.000776189, 0.0238687, -0.0808489, -0.0331181, 0.0731589, 0.0380285, -0.0596918, -0.0683359, -0.0607706, 0.0173616, -0.0613332, 0.0773055, -0.0921126, -0.122125, -0.0617059, 0.00254066, 0.0420068, 0.0693966, 0.0608327, 0.0220888, -0.022626, -0.00722451, 0.0473733, -0.07197, -0.12332, 0.0805882, -0.0959678, -0.114531, -0.0286139, 0.0780572, 0.079025, -0.0115852, 0.0200269, 0.0210249, -0.0503924, -0.0572829, 0.0843985, -0.0611547, -0.0137248, 0.0110529, -0.0290473, 0.00923679, 0.0199804, -0.0179225, -0.000242617, 0.0749101, 0.0355559, 0.00975276, 0.00976958, 0.0433188, -0.109197, -0.0152397, -0.0572528, -0.0697403, -0.0990834, -0.00375004, -0.0639689, 0.0137648, 0.0275424, 0.038874, -0.070847, -0.0821652, -0.0258425, -0.0369124, -0.121348, -0.00439599, -0.0778328, -0.0473711, -0.00153088, 0.0331644, 0.033293, 0.0183418, 0.0600686, -0.082867, 0.0137533, -0.135301, -0.0421784, -0.00258718, -0.000385152, -0.101857, -0.016998, -0.0572501, -0.0309442, -0.0102435, 0.00860739, 0.0214521, 0.0243536, -0.00462007, -0.0153922, -0.0313575, -0.0334005, 0.00645122, -0.0109944, 0.0140514, 0.0643604, -0.0651313, -0.040571, -0.0270513, 0.00366394, 0.0270825, 0.0324837, -0.0386944, 0.116234, -0.0270689, 0.0756309, -0.0674474, 0.0568174, -0.0737501, -0.0161481, -0.0291912, -0.0653842, 0.0156681, -0.0869415, 0.115144, 0.0544055, -0.0563796, -0.0379593, -0.0171185, -0.0231338, -0.0624633, -0.00490492, 0.0548439, -0.0735644, 0.109981, -0.0260185, 0.0671846, -0.0360604, -0.0351702, -0.0614504, -0.000365893, 0.0582365, -0.110369, 0.087078, 0.0828393, 0.0196059, 0.0715375, -0.114119, 0.0202743, 0.0329686, -0.109793, -0.175255, -0.0331496, -0.0196983, -0.0104685, -0.0545124, 0.018624, -0.0113833, -0.0211395, 0.0373632, 0.0173571, 0.0235935, -0.0744376, 0.00539871, 0.117364, 0.0433728, -0.0107341, 0.0319997, 0.0392983, -0.0342277, -0.0449286, -0.0258223, 0.0243069, -0.0499366, 0.0257123, -0.047946, 0.0126272, 0.0425171, -0.095527, -0.0602626, 0.080646, -0.0558246, -0.00825404, 0.00559512, -0.0311588, -0.00230045, -0.0144575, 0.0140201, -0.0244292, 0.0741679, -0.0278335, 0.0192616, -0.00223761, 0.0357612,
0.00810522, 0.0376298, -0.0773115, -0.00465878, -0.03028, 0.0174031, 0.0266376, 0.0972302, 0.0610118, 0.0435455, 0.0685281, 0.0240434, -0.0466492, 0.0165957, 0.0451091, -0.0179496, -0.0577621, -0.0581955, 0.0902097, 0.0505512, 0.0154557, 0.0362094, 0.0037, -0.0403026, -0.0402204, 0.00888765, -0.0509706, 0.0141882, 0.0163483, 0.0964324, -0.022617, -0.0582059, -0.10524, -0.0853524, 0.101095, 0.0261695, 0.000843759, 0.0119929, 0.0516842, -0.0194244, -0.00127212, 0.0761307, 0.0692659, 0.0168946, -0.02551, -0.0317721, -0.0210735, -0.0271343, -0.0200072, -0.0902289, 0.0396318, -8.65195e-05, 0.0281371, -0.0731537, 0.0104032, -0.0121664, 0.030337, -0.00701909, -0.0469424, 0.0822924, 0.0293902, 0.00695772, 0.0199554, -0.118352, -0.0503263, -0.0750957, 0.0367372, -0.0155409, 0.0435621, -0.0592478, 0.0266432, -0.0193158, 0.0205818, -0.040504, -0.0817088, 0.00940887, -0.00851566, 0.00968699, -0.0791964, -0.0982697, 0.010645, -0.065839, -0.00618115, 0.07694, 0.0419623, 0.0360757, 0.0251889, -0.0178401, 0.0214112, -0.123382, -0.00619627, -0.040826, -0.0294288, 0.106997, -0.101866, 0.108787, 0.080151, 0.0181174, 0.0620956, 0.0982957, 0.0377776, 0.000107623, 0.0992207, 0.0499226, 0.0252707, 0.0648303, 0.076241, 0.0213601, -0.0723514, 0.110403, -0.0459672, 0.0597766, -0.0816421, -0.0314767, 0.0128103, -0.109941, 0.0411978, 0.0249555, -0.0946843, 0.0590391, 0.00661759, -0.0405656, -0.0844784, 0.0275549, -0.0257446, -0.0139589, -0.0574729, -0.0539702, -0.130956, -0.0469841, -0.0903931, -0.112808, -0.0300064, -0.00736341, -0.00438705, 0.0710225, 0.116429, 0.0758155, -0.0782121, -0.0185434, -0.146311, -0.0253652, 0.0147524, -0.010859, -0.00912785, -0.0365873, 0.115072, -0.022403, -0.0571871, -0.00744608, -0.0302615, 0.0151075, -0.0437756, -0.084986, -0.0762716, -0.0977772, -0.0671508, -0.0647181, -0.171608, -0.145349, -0.0198497, -0.0872933, -0.0304468, 0.0267804, -0.00493044, 0.0213362, -0.0695435, 0.0679082, 0.0621372, -0.0287082, 0.00616669, 0.0157831, -0.0393944, -0.0506924, -0.0878691, -0.147624, -0.0296216, -0.0561485, -0.0553998, -0.0304625, -0.0915493, -0.0955177, -0.0912313, 0.0291647, 0.0191239, -0.0784064, -0.10922, -0.010871, -0.0553528, -0.132455, -0.0393938, 0.0745149, 0.100287, 0.0274886, 0.0617339, 0.0500073, 0.0278003, 0.0709072, -0.0360167, 0.0340766, -0.0762557, -0.0463079, -0.0561423, -0.10799, 0.013692, -0.0514341, -0.0117896, -0.0632636, -0.151759, -0.116954, -0.112982, -0.0791575, -0.0493186, -0.0226573, -0.140305, 0.0241774, 0.013, -0.00305922, -0.0845577, -0.0704195, 0.0842603, 0.0858698, 0.0388674, 0.00934759, -0.00289677, 0.0234416, 0.010016, 0.039858, 0.0138376, -0.117378, -0.196078, -0.117536, -0.185122, -0.152567, -0.0341277, 0.023285, -0.149234, -0.011858, -0.0628263, -0.0197237, 0.038539, 0.0637602, 0.0586705, 0.036492, 0.0691861, 0.148588, 0.0578933, 0.12417, 0.0986979, 0.117244, 0.100582, 0.0423938, -0.024507, -0.0546682, 0.0640521, -0.0236531, -0.038274, -0.0178968, -0.121375, -0.0784681, -0.127521, -0.0249312, -0.00494746, -0.00581037, -0.0405531, 0.0668293, -0.0683643, -0.0537657, 0.0186785, 0.042333, 0.0638442, 0.106636, 0.0622638, -0.0582922, -0.0720423, -0.107843, 0.103981, 0.114518, 0.122138, -0.0441537, 0.0719421, -0.0335328, 0.0157933, -0.0577225, -0.0257507, -0.00474732, -0.0460235, -0.0467524, -0.0457192, 0.022699, 0.0984372, -0.0400041, -0.0578638, -0.0846099, -0.0158094, 0.0182799, -0.10909, -0.113769, -0.116884, -0.0189839, -0.0497526, -0.0264109, -0.0321923, 0.0190952, 0.118835, 0.127801, 0.0900448, -0.0136779, -0.0175843, 0.0358459, 0.0097639, 0.0334319, -0.0740483, -0.0403003, 0.0111387, 0.00532018, 0.0834745, 0.1309, -0.0119967, -0.022185, -0.0927323, -0.145122, -0.13071, -0.190666, -0.0884184, -0.174583, -0.22834, -0.0809413, 0.00654064, -0.00864435, -0.116879, 0.0244879, -0.0330602, 0.0884392, 0.00751332, 0.0515325, 0.0283356, 0.0720184, -0.00835424, -0.0986179, -0.0551222, -0.0369971, -0.0538794, 0.0354974, 0.088724, 0.0593858, 0.149839, -0.103258, 0.0263847, 0.0367939, 0.0498917, -0.0167726, -0.0871201, -0.225654, -0.27541, -0.204564, -0.227689, -0.112345, -0.0465728, -0.0431376, 0.0155616, 0.0627008, 0.1003, -0.022334, -0.0378844, -0.0247086, -0.0050854, 0.00493339, -0.0399274, 0.0530221, 0.0316191, 0.111632, 0.121218, 0.120075, 0.0326573, 0.0620589, 0.179347, 0.0816963, 0.132651, 0.1123, 0.0604485, 0.125774, -0.17662, -0.0727688, -0.0718715, -0.0765529, -0.0823857, 0.0546226, -0.0659372, 0.066102, -0.0595485, -0.00622562, -0.0518951, -0.0247415, 0.0345193, 0.009344, -0.0292218, 0.0764994, -0.0106449, 0.116561, -0.000697529, 0.1591, 0.10832, 0.113165, 0.207661, 0.096927, 0.13233, 0.0716667, 0.133301, 0.160304, 0.077741, 0.00415879, -0.0727242, -0.0645481, -0.0307897, 0.0172989, -0.0585074, 0.0877698, -0.0108007, -0.0284054, -0.0376761, 0.0530063, -0.0258808, 0.129741, 0.127837, 0.0487089, 0.12903, 0.0512716, 0.144881, 0.178965, 0.243692, 0.19864, 0.19299, 0.128321, 0.0492754, 0.0414472, -0.0070558, 0.0425809, 0.077014, 0.0108259, -0.00543788, -0.0451306, -0.0740231, 0.00228085, 0.000227094, 0.0949103, 0.040099, 0.0853463, -0.00664499, 0.00355184, -0.0158595, 0.0333583, 0.0905324, 0.0811873, 0.0902216, 0.0511217, 0.082708, 0.0879495, 0.109251, 0.191792, 0.125486, 0.166674, 0.0434912, -0.0177477, 0.0494187, 0.16755, 0.0356496, 0.0253829, 0.161987, 0.0134032, 0.0223824, 0.095521, -0.0901448, 0.0538009, -0.0297549, -0.0939071, 0.0466706, -0.00454852, -0.0184868, 0.013475, -0.0150163, -0.0377615, 0.141154, 0.0591064, 0.105139, 0.102591, 0.186363, 0.164054, 0.0587991, 0.0160266, -0.0664146, -0.0280455, 0.10569, -0.0430855, 0.0962451, -0.0121382, 0.00695045, 0.00547965, -0.0215015, -0.0269204, -0.108956, 0.0341927, 0.0577295, -0.110757, 0.046607, 0.0686351, 0.122186, -0.0125271, -0.0932385, 0.134937, 0.137166, 0.140241, 0.0629899, 0.11126, -0.00998081, 0.0180193, 0.117256, 0.0467897, -0.0422208, -0.0295193, -0.020615, -0.00977294, -0.0425559, 0.107296, 0.0752458, 0.0138499, 0.0167866, 0.0120661, -0.00232425, 0.115403, -0.046651, -0.0231913, -0.0303538, 0.041367, 0.0948952, -0.0618979, 0.0306608, 0.0301524, 0.0680977, 0.129704, -0.134399, -0.00756209, 0.00441168, 0.036425, -0.0750627, -0.0573459, 0.0810605, 0.0197944, 0.0606938, -0.0181016, -0.0286064, 0.0410548, 0.0875935, 0.034768, -0.000597775, 0.114662, -0.0192105, 0.100417, -0.0365085, -0.103175, 0.0338213, -0.0264396, -0.0406831, 0.0933556, 0.0828479, -0.0236489, 0.0260089, -1.09813e-05, -0.0545958, 0.015099, -0.0826169, -0.0863621, -0.0702549, -0.158293, -0.135121, -0.0651909, -0.0939129, 0.0598693, 0.0847441, 0.0902849, -0.0188662, 0.167755, 0.106401, 0.0250199, 0.119911, 0.0296329, 0.0443139, -0.022674, 0.0129063, -0.059705, -0.0388156, -0.0346907, -0.0464647, -0.0349526, 0.0578199, 0.0469165, 0.0345703, -0.0301029, -0.0459362, 0.109387, -0.0960737, 0.0415132, 0.009607, -0.00176998, -0.0702106, 0.0591682, -0.156607, -0.0185998, 0.0569459, -0.00554131, 0.0384909, 0.135144, 0.0312202, 0.102135, 0.000549865, -0.0886686, -0.023953, -0.0972492, 0.0272881, -0.044684, 0.00859377, -0.149926, -0.0568916, -0.0960224, -0.0816549, -0.0195265, -0.0486646, 0.0643974, 0.0351359, -0.0529491, 0.00354763, -0.1085, -0.0864862, -0.115869, -0.0858545, -0.0954786, 1.58884e-05, 0.0167648, -0.105288, 0.00462629, -0.0289373, -0.00259645, -0.0271407, 0.044911, 0.0526196, 0.0402985, 0.0202379, 0.0107364, -0.0613879, -0.0282875, -0.0222577, -0.0302443, -0.0504661, 0.0171464, 0.0167439, 0.0381289, -0.0425616, -0.0235897, -0.00308808, -0.0978424, -0.0834867, -0.105291, -0.0972105, -0.0201797, -0.0601814, 0.0558296, 0.0398475, 0.0435008, -0.000793483, -0.0821035, -0.0115097, -0.0316943, 0.0221982, 0.0653129, 0.0115009, 0.0324141, 0.0600854, -0.0587126, -0.0467562, -0.017106, -0.0381263, 0.0331284, -0.0294613, 0.00559295, 0.0635536, 0.0180852, 0.0386283, 0.0242498, -0.0577357, -0.0418378, -0.0431058, -0.0845427, -0.0378712, 0.0193537, 0.0305959, 0.11433, -0.020242, 0.0256218, -0.0804507, -0.0305151, -0.0183844, -0.0655865, -0.0455461, -0.00026883, -0.00447826, 0.00722129, -0.0220714, -0.00323123, -0.0166741, -0.0630786, -0.0048446, -0.0323794, -0.161126, -0.00451778, -0.133513, -0.095948, -0.0698429, -0.0678087, -0.0809962, 0.0218225, -0.0533383, -0.0412615, -0.00228361, -0.00166108, 0.0214117, -0.0225102, -0.0153927, 0.0824034, 0.0578601, 0.00229967, -0.073975, 0.0127652, 0.0135358, -0.0323315, -0.0789288, -0.0461585, -0.0694183, -0.0757647, -0.15733, -0.0387398, 0.121937, 0.0220873, 0.017381, 0.036712, -0.091889, -0.0424973, -0.0331032, -0.0170102, 0.0230863, -0.00290337, 0.00112444, -0.066398, 0.015352, 0.0578753, 0.0137389, 0.0981498, -0.0175453, -0.0128407, -0.00484436, -0.0754952, 0.0358364, 0.0836438, -0.0325656, 0.0213493, 0.0857723, -0.0577033, -0.031701, -0.0113013, 0.00388525, -0.0629811, -0.0787352, 0.0541743, -0.0586097, 0.00288974, 0.0162886, -0.0662493, 0.0076482, -0.00818119, -0.0318232, -0.0318929, -0.0935637, 0.0413735, -0.0280847, -0.138519,
-0.0312161, -0.0568717, -0.0111107, 0.0175864, -0.0669672, -0.0254296, -0.0316038, -0.022584, -0.0137423, -0.0740081, -0.0542606, 0.0452458, 0.0783982, 0.0800628, -0.0109688, 0.120765, 0.0164015, -0.0441799, 0.029221, -0.0457771, 0.0549584, -0.0294358, 0.0512685, 0.0253495, -0.0378746, -0.0134728, -0.0156087, -0.0230668, -0.0257586, -0.0539007, 0.00948976, 0.0268113, -0.0371893, -0.0533168, 0.0463897, -0.052118, 0.0175384, 0.0317738, -0.062429, -0.0537451, 0.0599169, -0.0654384, 0.00925905, -0.0361462, -0.0777885, -0.0126767, 0.00706396, 0.0390177, -0.0580849, 0.0740838, -0.0127733, -0.0360918, 0.017568, 0.0762144, -0.116617, 0.0350076, 0.0707783, -0.0270884, 0.0125292, -0.0138929, 0.050797, -0.0139107, 0.00225855, 0.0487179, -0.0468467, -0.0309005, -0.0606318, 0.0562368, -0.00661945, 0.060924, -0.0085977, -0.00439122, -0.0470565, 0.0179696, 0.0176163, -0.00892776, 0.0145925, -0.0896322, -0.125256, 0.0154054, -0.0420797, 0.162022, 0.00722246, 0.00349582, -0.0682099, 0.0057339, 0.0602204, -0.0709747, -0.0265059, -0.0511064, -0.16317, -0.0938384, 0.00492763, 0.019813, 0.0273203, -0.0360774, 0.0551259, 0.0787574, 0.0511994, 0.0466306, -0.0252111, 0.0214249, 0.0306528, -0.0316206, 0.00556738, -0.0206332, -0.0113909, -0.0201374, 0.0646417, 0.119815, 0.01502, 0.0195778, 0.027223, -0.0407217, 0.0123601, 0.0132786, -0.0611656, 0.0200709, -0.104142, -0.0465935, -0.0633789, 0.0269398, 0.0105266, 0.0279927, 0.0752238, 0.0049953, -0.00651181, 0.0618706, 0.00518536, 0.0952782, 0.0925832, 0.0382517, -0.0625006, 0.053052, -0.0130603, 0.00847527, 0.0301687, 0.0459107, 0.057583, 0.0387407, -0.0960433, 0.0522428, -0.0552366, -0.0756455, -0.0343702, -0.00679987, 0.000191814, 0.0643974, -0.00274378, -0.021599, 0.154122, 0.000374514, -0.0306205, 0.0834759, -0.136392, 0.0812487, 0.0912786, 0.0502489, 0.181433, 0.0569749, -0.035487, 0.039434, -0.0176336, -0.0997903, -0.00733797, -0.0629818, 0.000465372, 0.00354719, -0.0200533, -0.0202151, 0.0435272, -0.0291541, -0.0469731, 0.0383411, -0.0285403, 0.0262439, 0.0618119, -0.00945507, 0.0308146, 0.0554712, -0.00119753, -0.0104229, 0.0283148, -0.0553976, 0.0703002, -0.167698, -0.00203852, -0.0499067, -0.0200252, -0.0178366, -0.109298, -0.0635562, -0.0421161, 0.0498425, 0.0759575, 0.0270659, -0.0812603, -0.07371, -0.02405, -0.00865738, -0.0505634, 0.0439494, -0.0058531, -0.0376917, 0.0286527, -0.0475526, 0.128818, 0.057415, -0.0141996, 0.0385216, 0.0188157, 0.0457124, 0.00239842, -0.119886, -0.0179395, 0.0531426, -0.030683, -0.0857663, -0.141788, -0.0652449, 0.0180371, -0.0297116, -0.0780333, 0.0573734, 0.0224007, -0.0731268, 0.0144545, -0.0435376, -0.0733659, 0.0250191, 0.010673, 0.00624728, 0.0426372, 0.0875262, -0.0275979, 0.0333897, -0.0248484, -0.146581, 0.138166, 0.086785, 0.0603969, -0.114267, -0.0348833, -0.00749086, -0.0135165, -0.0534388, -0.114125, -0.2163, -0.000976251, -0.0275358, -0.0552997, -0.0231716, 0.0497929, 0.119987, -0.00997163, 0.0284766, -0.0401627, 0.000354781, -0.041067, 0.0378984, -0.0303246, 0.0750852, 0.037393, 0.144502, -0.0710943, -0.109804, -0.0437151, 0.051267, 0.0125023, 0.0166455, 0.0284836, -0.0179551, -0.0333094, 0.0124708, -0.245929, -0.183122, -0.0533502, 0.0851028, -0.0270233, -0.0220195, 0.0454402, -0.0339825, 0.0169169, -0.0754008, 0.0549756, 0.0508848, 0.10005, 0.102141, 0.0140911, 0.148633, 0.0102645, 0.000548661, -0.0939576, -0.211281, -0.0510938, 0.0562339, 0.0837458, -0.0227998, -0.0244804, 0.0773864, -0.0586176, -0.0240494, -0.0092862, 0.074088, -0.0380813, 0.0383522, -0.0554365, -0.0467986, -0.00117849, -0.025478, -0.062277, 0.00782002, -0.0446307, 0.0175875, 0.119184, 0.043641, 0.0276743, 0.0153007, 0.0135063, -0.0396406, -0.201802, -0.15746, -0.136381, 0.0928873, 0.15114, 0.150064, 0.0191953, -0.0505518, 0.0913478, 0.0550699, 0.0308589, -0.0488432, -0.0288721, -0.0878887, -0.0988315, -0.0923149, 0.0575463, 0.00762611, -0.0594435, -0.0798022, -0.0124044, 0.0470402, 0.124553, -0.017891, 0.0578462, -0.0182002, -0.042594, -0.0793321, -0.0611502, -0.157612, -0.117973, 0.00916557, 0.0269619, 0.0400499, -0.0573638, 0.121255, 0.139472, 0.123944, 0.151666, 0.10487, -0.00248944, -0.0598608, 0.0544338, -0.0820885, 0.0538964, 0.000614731, -0.0262803, 0.0730592, 0.12597, 0.0542417, 0.0272062, 0.0243102, 0.01173, -0.0155762, 0.0285304, -0.109105, -0.0601096, -0.203788, -0.205454, -0.116864, -0.0301071, -0.0437634, -0.0688135, 0.029069, 0.00746402, 0.101976, 0.187175, 0.208792, 0.0669645, 0.0858182, 0.0694255, 0.0199635, -0.00304933, -0.0434104, -0.142649, -0.0505486, 0.133818, 0.140373, 0.0652703, 0.209336, 0.0161984, 0.0961877, -0.0313387, -0.0223192, 0.0181715, -0.144703, -0.170373, -0.105269, 0.0167727, 0.0123044, 0.00637552, 0.00599359, 0.0708664, 0.110835, 0.0547534, 0.0641856, 0.0640716, -0.0301675, 0.00296114, 0.026482, -0.0436824, -0.00969629, 0.00105541, 0.0445275, 0.054833, 0.0599476, 0.101243, 0.0775835, 0.0496085, -0.0161623, -0.0399527, -0.0469975, -0.0726338, -0.0271876, -0.170787, -0.160476, -0.164295, 0.0184063, 0.0358736, 0.0276514, 0.115455, 0.11044, 0.0532049, -0.00649337, -0.0104309, 0.023467, 0.00411644, 0.132128, -0.0869003, -0.0372436, 0.0452165, 0.0177339, 0.0443392, 0.0470581, 0.0589854, 0.0657028, 0.0830158, -0.0287923, -0.0419715, -0.0147808, 0.0550244, -0.0506205, -0.197307, -0.226922, -0.112833, -0.0356371, 0.0192518, 0.000639891, 0.0350912, 0.056921, 0.0764701, -0.0847641, 0.0244355, -0.0226274, -0.0179545, 0.0142912, 0.044148, -0.00307708, -0.0202486, 0.0574081, 0.00670486, 0.0892041, 0.0608236, 0.0452001, 0.0832267, 0.100999, 0.0633305, 0.00661254, 0.00509895, -0.126295, -0.0987531, -0.235882, -0.149756, 0.163962, 0.106663, 0.0318442, 0.00733677, 0.0440215, 0.0242987, -0.0465892, -0.0444753, 0.102872, 0.0442801, 0.102828, -0.00560983, 0.0519575, -0.0605472, 0.0079173, -0.0434054, 0.00813166, -0.0577819, -0.138265, 0.0165163, 0.0337417, 0.163073, 0.0936702, 0.0678758, -0.0745659, -0.14454, -0.296001, -0.0400082, 0.158422, 0.231586, -0.00101307, -0.0127176, -0.0408822, 0.00484281, -0.143841, -0.0340761, -0.0373675, 0.00384049, 0.0615865, -0.0765244, 0.0474125, -0.00787455, 0.0914686, 0.000310146, -0.0220563, 0.0701469, 0.141516, 0.0723762, 0.137358, 0.145413, 0.154225, 0.140905, -0.120724, -0.123685, -0.048075, 0.0572811, 0.168951, 0.270334, 0.0841503, 0.0167356, 0.168161, 0.00089009, 0.0521077, -0.00433772, 0.0304441, -0.0116898, 0.0489824, -0.0597769, 0.0173996, -0.0686969, -0.00138953, 0.00592056, 0.0246366, -0.0267957, -0.0630459, -0.0720427, 0.0463996, 0.0555222, 0.127151, 0.0584397, -0.0156523, -0.0402963, -0.0578698, 0.0825899, 0.210482, 0.0480292, -0.0315488, -0.0351285, -0.0615765, 0.056918, 0.0674837, 0.104031, 0.0231819, -0.00619897, -0.00500657, 0.0815524, -0.0136949, -0.0515695, 0.0278486, 0.0809706, 0.117214, -0.0231682, -0.0957526, 0.045498, 0.011864, -0.0380455, 0.0409496, 0.000685254, -0.0972738, -0.0885465, -0.00318607, 0.0957908, -0.0353854, 0.0554997, -0.0103494, 0.0801555, 0.079566, 0.011941, -0.0339266, 0.0535748, 0.106437, -0.052883, -0.0114742, -0.0213233, -0.00422246, -0.0261456, -0.0586822, 0.0139824, 0.0919388, -0.033383, 0.0201104, -0.0357052, -0.0998607, -0.204877, -0.106681, -0.0537839, -0.00778832, 0.0219068, 0.056481, 0.142421, 0.0655693, 0.0659323, 0.0517238, -0.0021578, 0.0329956, 0.0671579, 0.0421833, -0.0195072, 0.0664598, -0.0512047, -0.00292133, 0.0218228, 0.00410913, 0.00418564, 0.0203666, -0.0411985, -0.0121537, -0.0720593, -0.089768, -0.0461591, -0.0550341, -0.187464, 0.0218036, -0.0643281, -0.0219247, 0.0469343, 0.160999, 0.0428951, 0.13566, -0.0568372, -0.0348395, 0.099632, 0.0395862, -0.00488226, 0.0295178, -0.0750912, -0.0198084, -0.00246381, 0.0341768, 0.0135568, -0.0510255, 0.0778638, -0.0171657, -0.0104428, 0.0541031, 0.0247314, 0.0421231, 0.0105185, 0.0360798, 0.024526, -0.0230847, 0.0841631, -0.132921, -0.0263068, 0.0410914, 0.150241, 0.10918, -0.00745948, 0.0469984, 0.0676356, 0.166695, 0.0873948, 0.0972101, -0.0801698, -0.0158828, 0.0437697, -0.0171205, 0.00240667, 0.000826349, -0.0684295, -0.0914498, -0.009445, 0.0359268, 0.0385919, -0.0286534, -0.0690112, -0.0127243, -0.0920065, 0.0424859, 0.0172466, -0.0958556, -0.11823, -0.032108, -0.0368734, 0.0538418, 0.1286, 0.0783155, 0.0658264, 0.153432, 0.0446425, 0.0335791, -0.0705458, -0.00556984, 0.010849, -0.00787678, 0.00235648, -0.000902846, -0.0367161, -0.0360599, 0.0208226, -0.0780255, 0.0106682, 0.0572408, 0.0741127, 0.0643222, -0.0317147, -0.0315664, -0.0475338, 0.0322263, -0.00584778, -0.0230538, -0.00947471, -0.0313889, 0.0367939, -0.00126909, 0.0322314, 0.0806096, -0.0332419, -0.0174292, -0.00933697, 0.0336535, 0.0393773, -0.0765246, -0.099572, -0.0351062, -0.00708694, -0.116095, -0.019063, 0.0125093, -0.0208067, -0.0690692, 0.0148665, -0.0677233, -0.0286173, 0.0311139, -0.0245337, 0.0418265, 0.0823674, -0.0688429, -0.11155, -0.0779136, 0.077818, -0.0228171, 0.0814518, 0.0743697, -0.0732736, 0.0201238, 0.0666541, -0.056671, 0.0353625, -0.0229115,
0.0134557, -0.010855, 0.00324696, 0.0527169, 0.00751977, -0.0127234, 0.0267078, 0.0689036, -0.000915708, 0.00426044, 0.00322016, 0.0535351, 0.0717344, -0.0832703, 0.0873495, -0.0868028, 0.00238387, -0.0418033, -0.0603179, 0.00589836, 0.0128984, -0.0305414, -0.0142161, -0.000289703, 0.00220193, 0.0280712, -0.0223943, -0.00412006, 0.049507, -0.0272727, -0.0358231, 0.0204346, 0.0498515, -0.060275, -0.00210037, 0.0386582, -0.0278338, -0.0465529, 0.0532867, -0.00321015, -0.0136814, -0.0148417, 0.0198478, 0.0749122, -0.0456503, 0.02318, 0.0610308, -0.0463769, 0.0155976, -0.0105806, 0.0191023, 0.0285517, -0.0063099, 0.0352761, 0.00919964, -0.00715005, 0.0286782, 0.00786541, -0.112814, -0.0450211, 0.0187418, 8.05181e-05, -0.0236763, -0.0510215, 0.0738885, -0.057006, -0.0304524, 0.0381577, 0.0328921, -0.00606811, -0.00435317, 0.0392939, 0.0296392, 0.00701743, -0.0928297, -0.0148989, 0.0267922, -0.0393377, 0.0913059, 0.0475658, -0.0534214, 0.00945901, -0.0736637, -0.0861616, 0.0655012, -0.041166, -0.00280847, -0.0256489, -0.0394892, 0.00530449, 0.0228356, -0.031011, 0.0352876, 0.0386446, 0.0260198, 0.0170183, -0.00999371, -0.0138204, 0.114297, 0.114927, 0.0233768, -0.0141008, 0.00905574, -0.0632743, -0.0236598, -0.0185553, -0.0457882, -0.0915396, 0.118823, -0.0126981, -0.0734079, 0.0522184, -0.0499707, -0.0772644, 0.134209, 0.0593136, 0.0531594, 0.0388685, 0.0125952, -0.00687438, 0.0044373, 0.0619772, 0.0264915, -0.00256041, -0.0681004, -0.0257133, 0.111551, 0.0876497, 0.0508627, -0.0299162, 0.0853716, 0.0227998, -0.0499009, -0.0691933, 0.0183711, -0.0476005, -0.0452461, 0.00117529, 0.0214946, 0.0479049, 0.0397436, -0.122264, -0.125662, 0.0938453, 0.0127473, -0.00509305, 0.0419986, -0.0168721, 0.0940742, 0.0488176, -0.0241935, -0.1469, -0.0360375, 0.0179324, 0.109788, 0.0554918, -0.0195602, -0.0712586, -0.0712406, -0.0272994, 0.0643202, 0.00781548, 0.0538744, 0.0850353, -0.00449528, -0.0734542, 0.078976, 0.0338864, -0.0522351, 0.0162032, -0.0317462, 0.0630521, 0.0271068, 0.0554237, -0.000873386, -0.0096444, -0.0665398, -0.132598, -0.0385994, 0.00283038, -0.110929, -0.031052, 0.0175833, -0.0643897, -0.0525603, -0.0381441, -0.0417117, -0.049215, 0.0164301, 0.10472, 0.085022, -0.0293061, 0.0502099, 0.0570974, -0.0275426, -0.0632288, -0.0368964, -0.000510341, 0.0145084, -0.00293459, 0.00845348, -0.00951535, 0.081944, 0.102693, -0.00980304, 0.0515638, 0.0668082, 0.119081, -0.0926976, -0.0399498, 0.0406341, 0.0528659, 0.045089, -0.032791, 0.0890679, 0.149262, 0.016631, -0.104248, -0.0258697, 0.0533591, 0.0914094, 0.00313964, -0.033107, -0.00624766, -0.0386441, 0.00725455, 0.0504324, -0.011111, -0.0440484, 0.0395535, 0.0961164, 0.127021, 0.0172735, 0.142966, 0.0108906, 0.0443998, 0.015155, -0.003072, 0.0465342, 0.118313, 0.0723151, 0.055629, 0.0712884, 0.107667, -0.060619, -0.0206501, 0.00117255, 0.00969845, 0.0738668, -0.0185165, 0.0276047, -0.0111434, -0.0395935, 0.00180216, 0.0214391, -0.00600507, 0.105567, -0.0404379, -0.0394623, 0.101891, 0.0577629, 0.0738432, 0.116953, -0.0630985, -0.0708955, 0.0820424, -0.0107083, 0.116701, -0.0806129, -0.0833504, -0.0181381, 0.0828469, 0.0644705, 0.0465807, -0.0263496, 0.0628861, 0.0192258, 0.00747546, 0.0148636, -0.0577596, 0.0809222, 0.0114329, 0.0432938, -0.00871303, 0.0872091, 0.0455802, 0.0192779, -0.00401838, 0.0448089, 0.0548486, 0.128291, -0.0274834, -0.140766, -0.00835101, 0.0366764, 0.015081, -0.0201087, 0.0231943, -0.102641, 0.0528075, 0.028565, 0.0707557, 0.0855874, 0.114978, 0.0225274, -0.000134223, 0.00399277, -0.0231295, 0.0369851, 0.0866308, -0.0237383, -0.00774878, 0.00699174, -0.0681636, 0.0304963, 0.0338406, 0.0864253, 0.176759, 0.0225428, -0.0805724, -0.286816, -0.166552, 0.0785885, 0.0764187, 0.00046363, 0.0215429, -0.000882225, -0.0228853, -0.0691164, -0.037103, 0.0485655, 0.100281, 0.120395, -0.084691, -0.0737947, 0.137057, 0.118185, -0.000401685, -0.0208709, -0.0168434, 0.0281788, -0.0647048, 0.0227048, 0.102624, 0.14294, 0.21768, 0.00357494, -0.134748, -0.128659, -0.0527204, 0.0710153, 0.0282721, -0.0448408, -0.121135, 0.0922752, -0.133289, 0.0794375, 0.0170966, 0.0383406, 0.100838, 0.16161, 0.00327723, 0.0221648, 0.0350432, 0.0216277, 0.109024, 0.0240748, 0.0105551, -0.0324995, 0.00504463, 0.117953, 0.0794566, 0.184113, 0.146016, 0.00202432, -0.130262, -0.270352, -0.141674, 0.0044309, 0.125145, -0.0883897, 0.00494716, -0.120705, 0.000685491, -0.0622516, 0.0509833, 0.00905986, 0.146468, 0.138464, 0.0779763, -0.0702939, -0.0439518, 0.0906471, 0.00996865, 0.0509632, 0.00698041, -0.0519699, 0.097436, 0.103749, 0.143829, 0.111084, 0.0859886, -0.182907, -0.192822, -0.11984, -0.0271082, 0.067578, -0.0017946, -0.127796, -0.0757279, -0.0483972, -0.105779, -0.0897236, -0.00196832, -0.0227836, 0.142998, 0.102759, 0.0172238, -0.0226411, -0.0555482, -0.00639902, -0.0888983, 0.00244635, 0.0425802, -0.0571542, 0.0348679, 0.0551329, 0.074052, 0.0794733, -0.00287288, -0.121903, -0.148968, -0.153997, 0.00337007, 0.0720316, 0.0299561, -0.0599784, -0.0808526, -0.0318018, -0.0845796, -0.0373014, -0.0628897, 0.00579083, 0.135049, 0.070824, 0.0153649, -0.0160319, 0.0296931, 0.00456965, -0.0925407, 0.0481739, 0.0383644, -0.0394526, 0.109126, 0.125848, 0.136952, 0.0200163, 0.0801677, -0.140258, -0.242539, -0.195342, -0.0773534, 0.0141077, 0.0481872, -0.0776844, -0.093342, -0.0734292, -0.0559857, 0.0190067, 0.0379401, 0.0045079, 0.0117094, 0.0680059, 0.0602175, -0.0386783, -0.0196983, -0.0434196, 0.089624, 0.0686857, -0.0651207, 0.00179758, 0.0686197, 0.070774, 0.0457519, -0.0569879, 0.0237061, -0.0720987, -0.0890971, -0.110767, -0.0722468, -0.00797602, 0.0681512, -0.122595, -0.130447, -0.185957, -0.0392463, -0.0088959, -0.0015864, 0.0666737, 0.0364024, 0.0845989, -0.00695908, 0.0198312, 0.0231618, -0.00123428, -0.0167675, -0.0771508, -0.00389596, 0.0375516, 0.0869444, 0.037542, 0.0407648, 0.0263364, 0.068497, 0.0506313, -0.0851754, -0.144791, -0.0328699, 0.00739452, -0.0269358, -0.110812, -0.0247184, 0.00930887, -0.0380883, 0.103554, 0.124579, 0.069263, 0.0857031, 0.0145219, 0.0390037, 0.0431903, -0.0465633, 0.00209522, -0.000207491, -0.00788825, -0.061508, -0.0184395, 0.0237499, 0.0559196, 0.13747, 0.0254371, 0.0869446, 0.0181623, -0.050398, -0.103609, -0.114924, -0.125735, -0.0232004, -0.0574386, 0.0911192, -0.0806192, -0.0454497, 0.15098, 0.0928708, 0.109768, -0.0212863, 0.0385409, 0.00761085, -0.0259589, -0.036811, -0.0305544, 0.0171876, -0.00714985, 0.0728934, -0.00718926, 0.0685556, 0.0414527, 0.0569575, 0.0311728, -0.00595208, 0.0522013, -0.0338131, 0.05575, 0.0428061, -0.056042, -0.043366, -0.000972223, 0.0317044, 0.0496773, 0.0661175, -0.0292454, -0.0966704, 0.00666575, -0.0957471, -0.00713575, 0.015654, -0.033579, 0.0315107, -0.0439761, -0.00789201, -0.049535, 0.0374761, 0.0476418, 0.0570873, -0.0536019, 0.032339, -0.0504639, -0.00956679, 0.0685863, 0.0942904, 0.0741957, -0.0423693, 0.131262, 0.0141598, 0.101447, 0.0456958, 0.124244, -0.0529722, 0.040356, -0.0188788, -0.0998638, -0.175821, -0.0340802, 0.00930456, -0.0457943, 0.0572491, 0.096918, -0.0457479, -0.0509932, -0.0342256, 0.0278954, 0.00828113, -0.0393169, 0.0418078, -0.0174779, -0.0135644, 0.0598911, -0.0195624, -0.0473778, 0.0566338, 0.0558569, 0.0495735, 0.0355341, -0.00480707, 0.0479823, 0.0623005, 0.00294534, 0.03327, 0.0681581, -0.034031, -0.057133, -0.0430489, -0.122255, 0.014485, -0.0492674, -0.0144383, -0.0770093, -0.00953006, 0.0667068, -0.0164379, 0.0326754, 0.000987671, -0.0103538, -0.0524595, 0.038034, -0.0388916, 0.00411225, -0.0149488, 0.0960783, 0.109737, -0.0548272, -0.00317776, -0.0543762, -0.075509, 0.00824322, -0.0500194, -0.0659173, -0.0185281, -0.0074325, 0.0718385, -0.0575585, -0.0106459, -0.0563616, -0.0134551, 0.0708396, 0.0133015, 0.00623804, -0.110048, -0.0136583, 0.00442656, -0.0601709, -0.0527787, -0.102233, -0.0185175, -0.00340101, -0.091314, -0.00273005, 0.0219422, -0.0468307, 0.0238967, 0.00340731, -0.0585455, -0.13195, 0.0659865, -0.0574541, -0.0651123, -0.0533124, -0.000293301, 0.0642197, 0.0272666, 0.0792915, 0.0994449, 0.0796113, 0.0383186, 0.0655105, 0.0228608, -0.0724127, -0.0291943, 0.018298, -0.0118538, -0.0962668, -0.0815582, -0.0498356, 0.0518018, -0.0144956, 0.0732905, 0.00491727, -0.0919575, 0.017195, 0.0133549, 0.0544556, -0.0326416, -0.0119636, -0.05515, 0.0539654, -0.00256236, 0.0110055, -0.068051, -0.0236304, 0.0410355, 0.0719571, -0.0153171, -0.0696523, -0.052108, -0.108332, -0.0811893, 0.019546, 0.0352499, -0.0101115, -0.0332809, -0.0675125, -0.00903715, 0.00190795, 0.138106, 0.00909496, -0.0331684, 0.116014, 0.0611691, -0.0399767, -0.0133237, -0.0675975, 0.068478, -0.0171826, 0.000745026, 0.0104553, 0.0149819, -0.0698839, -0.000530207, -0.0407902, -0.0353534, -0.022532, 0.030875, -0.0222883, 0.0376126, 0.0584341, 0.0180612, 0.00178936, -0.00367403, -0.0106997, 0.0133386, 0.050367, 0.0490486, -0.100697, 0.0146377, 0.0214347, -0.0267509, -0.0052973, -0.051394, 0.00652903, -0.0843749, -0.076758, 0.0286751, 0.0579305, -0.0640167,
-0.0326534, -0.0749571, -0.0203904, -0.0591274, -0.0247195, 0.0937399, 0.152384, -0.0178172, -0.00893561, 0.0210047, -0.0309912, 0.00534143, 0.0659727, -0.0267753, -0.0423257, -0.00350238, 0.0282935, 0.024756, -0.0221286, 0.0446518, -0.00126823, -0.0406797, -0.0316722, 0.0480292, -0.0801621, -0.0029044, 0.00284461, -0.00217636, -0.0793306, 0.0430842, 0.0106014, 0.0535961, -0.0386176, 0.0252399, -0.000547173, -0.0406956, -0.0250265, 0.0307335, 0.0109123, -0.0558573, 0.0237258, -0.000588028, 0.013322, -0.0688458, -0.0160134, 0.0798852, -0.0481477, 0.0320178, 0.0548378, -0.064002, 0.0217108, 0.0172975, 0.0481441, -0.07735, 0.0287104, -0.00451722, 0.131759, 0.00686334, 0.0418696, -0.104953, -0.0509038, 0.0369422, -0.0131419, 0.0195677, -0.0144603, -0.0106259, 0.0207807, -0.120613, 0.0579812, -0.0955013, -0.0262548, -0.0360313, -0.0672019, 0.0517057, -0.103642, 0.00573295, -0.0841007, -0.0303806, 0.0241077, 0.0316882, 0.0128663, -0.0707719, -0.0230717, -0.0374368, 0.00530424, 0.0159755, -0.0848784, -0.0234537, 0.0305825, 0.00827966, 0.0668167, 0.0222668, 0.00230379, -0.0305231, -0.00810409, -0.061591, -0.055267, -0.087827, -0.0399776, -0.0151617, 0.0414401, -0.0449671, -0.0646899, -0.0863336, 0.0294597, -0.0561528, -0.0789934, -0.0511222, -0.017175, -0.0153893, -0.0451991, -0.0595222, -0.0578264, -0.062609, 0.0420466, -0.0556355, 0.0294659, -0.0162737, -0.0137868, -0.0559413, -0.110102, -0.0195591, -0.139216, -0.0574938, -0.0607166, -0.0452075, 0.0041583, -0.0215187, -0.0477023, 0.00139052, 0.0395912, -0.135447, -0.0419084, -0.0701953, -0.0319109, 0.0580414, 0.00128638, 0.0750628, 0.0602628, -0.0628061, -0.0303349, -0.00339538, -0.00154556, 0.0213355, 0.0183792, -0.0639749, -0.0761022, -0.155187, -0.0591019, -0.126701, -0.0823828, -0.21286, -0.0633618, 0.042561, -0.138329, 0.0079845, 0.0675605, 0.137636, 0.0237274, -0.014358, -0.0706117, -0.112489, -0.116758, -0.0629193, -0.117545, 0.0541339, 0.0267721, 0.110114, -0.0187454, 0.0032557, 0.00938496, 0.00663735, -0.00506158, 0.00240719, -0.13199, -0.115414, -0.100384, -0.149815, -0.177579, -0.0180594, -0.0480736, -0.0707369, -0.00142405, -0.00317472, -0.0387584, -0.0574072, 0.0334191, 0.0552411, -0.066762, 0.0446817, 0.0781198, 0.0434349, -0.0573677, 0.00328563, 0.0671484, -0.0113089, -0.0226072, -0.0242376, -0.0335378, -0.0333652, 0.0243041, -0.0418609, -0.0283862, -0.0648848, -0.0375615, -0.0348479, -0.0864503, -0.102702, -0.149584, -0.0717257, -0.133349, -0.0651371, -0.176675, 0.000956369, -0.0467178, -0.0752307, -0.00772905, -0.000352295, -0.0430618, -0.0338775, -0.0195609, 0.0863751, -0.0963374, 0.0817254, -0.141238, 0.0703604, 0.0742768, -0.046137, 0.0439587, 0.0463488, 0.0801625, 0.118664, -0.0185028, -0.0308309, -0.0288906, -0.0806706, -0.0804166, -0.17099, -0.17828, -0.0438324, 0.0129475, -0.0352077, -0.0303439, -0.107277, 0.0165069, -0.0457343, -0.128254, 0.0593951, 0.00285112, -0.0276984, -0.00131066, -0.0396878, 0.0151834, 0.0213385, -0.0516048, 0.0781946, 0.104098, 0.0566869, 0.102371, 0.0492613, 0.0965085, 0.0230851, 0.132534, 0.134843, 0.13893, 0.0480516, 0.0963881, -0.0154637, -0.0465023, -0.0379477, 0.0753809, -0.0224377, 0.120937, 0.108565, 0.0508717, 0.0736042, -0.0155622, 0.0540472, 0.0203688, 0.00666201, 0.0382961, 0.0182676, 0.0462313, 0.0350703, -0.0442117, 0.0847706, 0.149753, 0.083054, 0.152125, 0.0864027, 0.134357, 0.284165, 0.143371, 0.0613867, 0.0769885, 0.123205, 0.083684, -0.055331, 0.115954, 0.204025, 0.144353, 0.0494967, 0.0605544, 2.66281e-05, 0.0700352, 0.099475, 0.0480386, -0.0395392, -0.0536655, 0.00303244, 0.0558882, 0.147688, 0.0578875, 0.0981647, 0.147117, 0.11045, 0.120435, 0.155867, 0.0724868, 0.217391, 0.271062, 0.284751, 0.171634, 0.0442412, -0.0396251, -0.0213958, 0.0472634, 0.129704, 0.194306, 0.0366924, 0.0653493, 0.131698, 0.075601, 0.126673, 0.039665, -0.00894625, 0.0918854, -0.0185566, 0.00634508, -0.0366425, 0.112622, 0.020631, 0.051932, 0.116104, 0.261469, 0.189782, 0.0913489, 0.155444, 0.126957, 0.141935, 0.0970253, -0.0398562, -0.0298772, -0.0682939, 0.0684533, 0.0436267, 0.0620179, 0.061728, 0.140294, -0.108565, -0.141116, 0.101506, -0.0082581, 0.0410871, -0.0418361, 0.0554741, -0.0579488, -0.00866717, 0.029787, 0.037572, 0.0226859, -0.0204406, 0.0801784, 0.0206848, 0.0736279, 0.155364, 0.0170009, 0.00633225, 0.0241811, -0.196362, 0.0120037, -0.0325945, 0.0680184, -0.067672, -0.00303772, 0.0361763, -0.0472101, -0.0532385, 0.0229568, 0.0568698, 0.0905332, -0.0837831, -0.0375328, -0.0988417, 0.157291, -0.0112027, -0.0685356, 0.00324226, 0.0295046, 0.0133175, -0.025986, -0.0841798, 0.0079381, -0.0648593, 0.0182586, -0.0431186, 0.0180207, -0.107171, -0.0876976, -0.042661, -0.128784, 0.0867137, 0.108507, 0.00152347, 0.0720332, 0.137424, -0.0149492, -0.0279056, 0.0888978, -0.0653244, 0.0535816, 0.0326666, 0.0204899, 0.10967, -0.0951252, 0.0760453, -0.0851781, -0.102232, -0.0339577, -0.0639122, -0.106808, 0.0426287, -0.100614, -0.0706015, -0.0318108, 0.0345244, -0.230126, -0.0160469, -0.0107622, 0.12284, -0.017506, -0.0437876, 0.000587559, -0.015198, 0.075843, -0.00520858, 0.0224745, 0.0370942, 0.0680076, -0.00724924, 0.0389263, 0.0944997, -0.0227279, -0.0805733, -0.0288726, -0.0742563, -0.0811703, -0.0148755, -0.0615413, -0.0905154, 0.0461544, -0.0186881, 0.0155141, 0.101237, -0.09179, 0.110962, 0.0229487, 0.0655947, 0.0176993, 0.10241, 0.0438702, -0.0343605, 0.0293635, 0.00119357, 0.103427, 0.035416, -0.000807034, 0.0242417, -0.0198215, 0.125302, -0.044457, 0.00501593, -0.0828082, -0.0915432, -0.102107, -0.0479377, -0.0608431, -0.0480418, 0.00777797, 0.0246708, -0.0182845, 0.048211, -0.0265672, 0.0205694, 0.140245, 0.00333006, 0.0824873, 0.0295747, 0.0807064, 0.00099693, -0.021603, 0.0342149, -0.0208464, -0.136176, 0.0618905, -0.0241571, 0.00583923, -0.154518, -0.0482166, 0.00932869, -0.0292436, -0.0578075, -0.037692, -0.0377553, -0.100972, -0.0227715, -0.111803, 0.0035796, -0.0834728, -0.0569001, -0.028421, 0.108162, 0.0232887, 0.0218105, 0.0621982, 0.0960261, 0.0730727, -0.0344385, -0.00534656, -0.00156032, -0.0333939, -0.00992156, -0.116648, 0.0289429, -0.0281285, -0.0981281, 0.0416706, 0.0941692, -0.000522845, 0.0496376, -0.0978431, -0.100752, -0.137115, -0.105402, -0.0322643, -0.0288591, 0.0449342, 0.0285019, -0.0191372, 0.047695, 0.000364185, 0.0355535, -0.0201349, -0.00458734, -0.0445624, 0.0107082, -0.0506701, -0.038881, -0.00672001, 0.0208873, -0.00301681, 0.0849429, 0.0167147, 0.0458152, 0.000326109, -0.0968508, -0.00216285, 0.0305592, -0.0539121, -0.10945, -0.0190653, -0.105963, -0.0448758, -0.0869991, -0.0511736, -0.0575295, -0.0480366, -0.0191567, -0.0306582, -0.0532385, -0.0770918, 0.0116373, 0.0211294, 0.124491, 0.0985985, 0.00393456, -0.0121267, -0.0134887, -0.0463494, 0.0666227, 0.0314804, -0.00839523, -0.0806246, -0.0150368, -0.0549016, -0.0496401, -0.0424296, -0.0811513, -0.0760519, 0.0274591, -0.0901388, -0.0317709, -0.0796461, -0.0819483, 0.0337274, -0.0359344, -0.0814534, -0.044577, -0.164955, -0.0564149, -0.114384, -0.132257, 0.0134924, -0.0283472, -0.00351868, -0.0499992, -0.0717419, 0.0253608, -0.0227005, 0.0265871, -0.0403077, -0.0896158, -0.0473591, 0.0271599, 0.0391263, 0.100684, 0.0340268, -0.0598963, 0.048511, -0.0459991, 0.0131563, 0.0644658, -0.0583008, 0.0367728, 0.00301362, -0.127268, -0.129496, -0.0734399, -0.0837817, 0.0235344, -0.0227891, -0.022463, 0.0548929, 0.0129462, 0.0266928, 0.0221024, 0.0737543, 0.0229189, -0.0665256, -0.0629537, 0.0620831, 0.114011, 0.102257, 0.138167, 0.0368726, 0.0334861, 0.0338855, 0.0721924, 0.0314262, -0.0435151, 0.0121649, 0.111254, 0.0910972, -0.0262509, -0.11479, -0.0530945, -0.015899, -0.0384791, -0.0151339, 0.0139015, 0.00465624, 0.0359814, 0.0561881, 0.0265031, 0.0855815, 0.0349855, 0.0628157, 0.0367403, -0.0602731, -0.0535171, -0.0068402, 0.0362442, 0.0151554, 0.0367312, -0.0778885, -0.0585716, -0.0190766, -0.0302578, -0.00552654, 0.0856527, -0.00198096, 0.102101, 0.00277476, -0.0462295, -0.0632278, 0.0359937, -0.00677407, -0.0430374, 0.0593433, 0.130004, -0.0381392, -0.0346435, -0.0682892, -0.0767907, -0.0815229, -0.0522598, -0.0805512, -0.0286753, -0.0349782, -0.14142, -0.0365984, 0.049844, -0.00427174, -0.0159367, -0.0150528, 0.00708898, 0.0434114, -0.0321357, 0.136608, 0.117042, 0.0580646, 0.00667598, -0.00603409, 0.0758675, -0.0010774, -0.0843877, 0.00962633, -0.0132094, 0.0481956, -0.036533, -0.000492254, 0.013233, -0.030045, -0.103975, 0.0335038, 0.0383198, -0.00508512, 0.0412563, -0.0823161, -0.0282503, 0.0682117, 0.0764624, -0.0192863, -0.0364543, 0.0288339, -0.010471, -0.0407806, 0.176673, -0.0576598, -0.0945326, -0.0080547, -0.0991991, -0.0513285, -0.0885024, 0.0527766, 0.0476326, 0.00312693, 0.0377636, 0.0110781, 0.00963281, -0.0556034, 0.0343387, 0.0448636, 0.0137179, 0.0280506, -0.0210847, 0.0370753, -0.0678817, -0.0685938, 0.0164952, 0.0492028, -0.0514517, 0.0414673, -0.0232151, -0.0324562, -0.0600929, -0.0629604, 0.0075097, -0.0319444, 0.00417309, 0.0186416, 0.019552, 0.0147503, 0.0248011,
0.00136922, -0.0366718, -0.0288752, 0.0206659, 0.0244546, 0.0163912, -0.0982277, -0.120589, 0.0361162, 0.047951, 0.0219914, -0.004624, -0.0336635, 0.0072431, 0.0477155, 0.0078584, -0.0536572, 0.0115407, -0.0185227, -0.0628919, -0.0137542, -0.0944276, 0.0134093, -0.139957, 0.00888304, 0.0220474, 0.0511913, -0.0231134, -0.0290001, -0.0214556, 0.0364578, -0.0160019, -0.00765558, 0.0629299, -0.0226659, -0.0410681, 0.0308801, -0.0065967, -0.0484867, 0.0275499, 0.0126989, 0.0406686, 0.0182514, 0.0160119, -0.0134449, -0.0772211, -0.0855469, -0.0558742, -0.00625297, -0.0853095, -0.0235762, -0.0896269, -0.0359455, 0.0886799, 0.0148292, -0.0175971, -0.0580133, -0.0513513, -0.0117527, 0.0322908, 0.0227901, -0.0147842, 0.0959213, -0.0398724, -0.0561929, -0.0698218, -0.0675476, -0.0368524, 0.107941, -0.0645849, 0.0344103, 0.0374339, 0.0679358, -1.72711e-05, 0.0559828, -0.00707653, 0.0198221, -0.0557728, -0.114361, 0.0827204, 0.0166697, -0.016919, -0.0744737, 0.0251424, -0.058744, 0.00293501, 0.0227792, 0.122854, 0.0199219, 0.100996, 0.0389178, 0.0438057, 0.0541513, -0.00697226, -0.0471899, -0.00363733, -0.0070558, -0.0717859, -0.133085, -0.0479065, 0.0387048, -0.0144322, -0.0710853, -0.0248011, 0.0506989, -0.0686202, 0.0383551, 0.0566607, -0.00188443, 0.022781, -0.0386197, -0.0139895, 0.00363557, 0.0594449, 0.129713, 0.0471169, 0.00291632, -0.0749844, 0.00286784, 0.0160447, -0.0176014, 0.0217884, -0.0489356, -0.130058, -0.0785285, 0.0251075, 0.0226636, 0.0240088, -0.0028763, 0.131631, 0.0830536, -0.101943, -0.0723618, -0.0510921, -0.0161948, 0.0117889, -0.112547, -0.000566979, -0.00176752, -0.0439675, 0.0249279, 0.0195808, 0.021213, 0.0374193, 0.0328808, 0.00841619, -0.0200164, 0.0202763, 0.00564569, -0.00852389, -0.0739873, -0.183602, -0.0301109, -0.0323138, -0.0175082, 0.0750336, 0.135522, -0.0148666, 0.117368, 0.0263203, 0.033017, 0.0180603, -0.00542694, 0.133464, -0.0465093, 0.0666478, 0.0567271, -0.0529327, 0.0340299, -0.0763223, -0.0773722, -0.0937981, 0.0648139, -0.0505536, 0.0588457, -0.0199001, -0.0306707, 0.000380806, 0.00359303, -0.121164, -0.0953371, -0.0238864, 0.0532133, 0.0462193, 0.164619, 0.15528, 0.189247, 0.0643672, 0.05253, 0.0821637, 0.0188021, -0.000678324, -0.0669643, -0.0532703, 0.0385518, 0.0718761, -0.0456276, 0.0150593, -0.0954379, -0.0219967, -0.0258888, 0.0115471, -0.00439076, -0.0484621, -0.0639938, -0.0672785, -0.0987748, -0.021856, 0.00467875, 0.0674365, 0.00109615, 0.00747244, 0.057605, 0.0416765, 0.0313277, 0.121098, 0.164421, 0.0499268, -0.0508972, -0.0488417, -0.132138, 0.0134574, -0.029298, -0.0429443, -0.0198675, -0.0607513, -0.0322074, -0.0369031, 0.00742635, 0.0666091, -0.0938653, -0.0354385, -0.116213, -0.013712, 0.0184497, 0.0683908, 0.0366207, -0.00588926, -0.0308404, -0.115379, 0.103431, 0.0618007, 0.118726, 0.263196, 0.00892469, 0.00609431, 0.0646169, -0.0777085, 0.0254496, -0.10613, -0.0883614, -0.0835528, -0.0958403, -0.0566506, 0.0192201, -0.0163834, -0.0523976, -0.0101743, -0.0563235, 0.0324052, -0.00404633, 0.0310457, 0.108852, 0.0360345, 0.106355, 0.035532, -0.100593, -0.127289, 0.0441102, 0.120927, 0.124063, 0.0830297, 0.0692763, 0.0654709, 0.0283777, 0.00363932, -0.132489, 0.013383, -0.0235215, -0.0343345, -0.0740838, -0.109056, -0.0196872, 0.0506452, -0.0121398, -0.0381213, 0.0671472, -0.0566395, -0.0303502, 0.118005, 0.0198529, 0.0385827, -0.0447172, -0.154805, -0.159301, -0.00487891, 0.055955, 0.157986, 0.0950599, 0.0764916, 0.122713, 0.128064, 0.00775177, 0.0418669, -0.14995, -0.0793867, -0.0389064, 0.00987455, -0.0173466, -0.167814, -0.00891362, 0.0593578, 0.0220746, -0.0153742, 0.00574459, -0.0148795, 0.0543792, -0.00692087, 0.00109692, -0.0210283, -0.00711951, -0.106476, -0.247206, -0.0582883, -0.0400899, 0.111255, 0.0733644, 0.18388, 0.154555, 0.0594218, -0.0230184, -0.0656924, -0.10197, 0.0761188, -0.0277183, -0.0207238, 0.00340513, -0.0204086, -0.00381898, -0.105179, 0.0566325, 0.0291859, -0.0154518, 0.011266, -0.0534057, -0.102441, -0.0609448, -0.132949, -0.172707, -0.175369, -0.0391614, 0.00665659, -0.0791764, 0.0671638, 0.0855862, 0.102092, 0.124031, -0.0858438, 0.018466, -0.115997, -0.152344, 0.0441533, -0.151613, 0.00955003, 0.027411, -0.0062864, 0.0135404, -0.0389039, 0.0716974, 0.0567567, -0.0744826, -0.0219092, 0.0435851, -0.0285844, -0.0770623, -0.121226, -0.11775, -0.0644584, 0.0365295, 0.0570441, -0.0214376, 0.10305, 0.0647421, 0.0722372, 0.0999207, 0.0944469, -0.0195822, -0.0313312, -0.0651809, -0.036626, 0.00783424, -0.0387695, 0.0445341, -0.0403027, -0.0117501, 0.0110811, 0.0328503, -0.0835264, 0.0199275, 0.0217556, -0.0494327, -0.0793931, -0.18303, -0.124889, -0.104533, -0.00736203, -0.0310548, 0.0169806, 0.109145, 0.105572, 0.0948441, 0.117866, 0.0663166, -0.0867937, -0.0555662, -0.0707646, -0.0269794, -0.0709753, -0.0337765, 0.0302006, -0.0307502, 0.0148371, 0.0746792, -0.0773825, -0.0738662, 0.0112958, -0.00775041, 0.0739442, 0.0340124, -0.0928414, -0.148228, -0.145906, -0.0442243, 0.0501284, -0.0275333, 0.0794404, 0.0452791, 0.0022461, 0.0669076, 0.0576766, -0.0321053, -0.065719, -0.0539034, -0.164125, -0.0441617, -0.0228059, -0.0383437, 0.0934503, 0.0893889, -0.0508057, -0.034919, 0.0421322, -0.0240684, -0.0326774, 0.0139731, -0.0181471, -0.0546351, -0.0852046, -0.00546952, -0.0713825, 0.0217041, 0.077168, 0.0243102, -0.0331676, 0.0502167, -0.0266504, 0.0488039, 0.0442661, -0.0623452, -0.093894, -0.0845011, -0.0719093, -0.0542973, 0.0807107, -0.0928928, -0.0453595, -0.00672305, -0.012451, -0.0789274, -0.00911024, 0.0272534, 0.0231964, 0.0254359, -0.0666516, -0.0157882, -0.144277, -0.0104909, 0.0670011, 0.111732, 0.0487348, -0.0704471, 0.0454851, -0.0684855, 0.00939652, 0.00670844, 0.000327842, -0.0162779, 0.0143287, -0.157347, -0.122277, -0.0138703, 0.0698286, -0.0467397, -0.0854585, -0.015546, -0.00940021, -0.0525592, 0.0291572, 0.0401491, 0.0488979, -0.0172824, 0.064117, 0.00404698, 0.132047, 0.0981956, 0.082336, 0.123037, -0.016647, -0.101743, -0.0855614, -0.0448624, 0.0844413, 0.0509891, -0.0532499, 0.0140299, 0.0273203, 0.0145506, -0.124689, 0.00154133, 0.0238252, -0.0165333, -0.0112442, 0.0436685, 0.0512345, 0.046456, 0.0204827, -0.0125273, -0.0849881, 0.0252882, 0.0605366, 0.0963599, -0.00857075, 0.273563, 0.158547, 0.0729047, 0.0566124, -0.0913759, -0.058749, 0.0723529, -0.0785769, -0.103536, -0.0320508, 0.0816553, 0.0198621, -0.000559476, 0.0285439, -0.111836, 0.103709, 0.0484122, 0.0646223, -0.0484497, 0.0888635, -0.0629134, 0.0267409, -7.63761e-05, 0.103214, 0.0680826, 0.125718, 0.132825, 0.110653, 0.149609, 0.072256, 0.0106438, -0.0460111, -0.0529402, -0.0584926, 0.0129152, 0.0915497, 0.0408684, 0.0702164, -0.000695486, -0.0603567, -0.00875693, 0.0178339, 0.103838, 0.0565439, 0.0715423, -0.15395, 0.00813338, -0.0263047, -0.045741, 0.0674473, 0.0813615, 0.0328421, 0.114874, -0.0187159, 0.0349026, -0.0280774, 0.128424, -0.00866369, -0.0220836, -0.114207, -0.118976, -0.0316122, 0.0529564, 0.0540203, 0.0166338, 0.100196, 0.0598325, 0.0953851, 0.0762827, -0.028879, 0.0166771, -0.0254269, -0.0446337, -0.0301995, 0.0955553, -0.059542, -0.0400763, 0.0392405, 0.0511336, 0.0840809, 0.033847, 0.0849274, 0.0501563, 0.0700934, 0.0664727, -0.00630767, 0.0698833, -0.0473966, -0.0447243, -0.0335571, -0.0154049, 0.106648, 0.0832078, 0.0781061, 0.0916479, 0.196006, 0.0613931, 0.0882971, 0.0514195, 0.0111881, -0.00861722, -0.00647319, 0.0175512, 0.0662413, 0.00602321, -0.00837272, -0.0226613, -0.00336263, 0.0347966, 0.0899468, 0.10173, 0.020389, -0.00329965, 0.089879, -0.0736696, -0.197672, -0.0447805, -0.12627, -0.0377037, -0.0121749, 0.109644, 0.058952, 0.0943387, 0.0627478, 0.043378, 0.0595466, -0.0671561, -0.0248322, -3.63889e-05, -0.0034603, 0.0100541, 0.0390435, -0.00127249, -0.0597252, 0.0198097, 0.0886896, -0.0292751, -0.0712771, 0.0398775, -0.126767, 0.036631, -0.0724795, -0.111006, -0.21062, -0.0872709, 0.0699265, -0.0365483, -0.118941, 0.0195984, 0.003416, -0.00808318, 0.0954651, 0.0440883, 0.114734, 0.00548005, 0.106276, -0.017079, 0.00848273, -0.00883632, 0.0573511, -0.022766, -0.00122087, -0.0481921, 0.129132, 0.0264524, 0.00463103, 0.0356563, 0.00375191, -0.0313707, -0.0757069, -0.0543803, -0.0445652, -0.0969964, -0.0548699, -0.128782, -0.0116494, -0.0255712, 0.0196545, 0.0777254, -0.0507209, 0.0378131, 0.0307912, -0.0316032, 0.0635692, -0.0695904, -0.034595, 0.102152, 0.0804774, 0.0712817, 0.0205895, 0.0106871, -0.040998, 0.0712858, 0.0370459, 0.000920846, -0.00121227, -0.0442213, -0.0428045, 0.0355647, 0.0451264, 0.0242505, -0.0392122, -0.00810871, -0.05479, -0.124339, -0.0635567, -0.0708347, -0.0144114, 0.0399272, -0.0616957, -0.00539231, 0.0182442, 0.0684172, 0.00676089, 0.0190224, 0.0125811, 0.100993, -0.0761438, 0.00378836, 0.0290389, 0.0559349, 0.0616864, 0.072224, -0.0383704, 0.0112526, 0.0459123, -0.000455174, -0.0766469, 0.0426831, -0.0479662, 0.132016, 0.0302464, -0.0718996, -0.0754544, 0.0853809, -0.0283933, 0.0401495, 0.0212256, -0.0483531, -0.0940924, -0.0216083, -0.00368798,
-0.0995986, 0.022314, 0.0429551, -0.0870391, -0.0161631, -0.0335076, -0.00931762, 0.0319647, -0.0343643, -0.0559821, -0.0970111, -0.0270803, -0.0619089, -0.0226567, 0.116198, -0.00496797, 0.0548928, 0.00419818, -0.0215024, 0.00105299, 0.0424616, -0.0635817, 0.00780041, 0.050757, -0.0345525, 0.0410825, -0.00554815, 0.0397227, 0.0246223, -0.0877539, 0.200708, -0.029738, -0.0330535, -0.128041, -0.049311, 0.00646331, 0.00859929, 0.0384585, 0.138273, -0.00111418, 0.104821, -0.0702581, -0.00514827, -0.107294, -0.0321204, 0.0769457, 0.00054072, 0.0185864, 0.00876696, 0.037893, 0.00903498, 0.0782841, 0.0207044, -0.024969, 0.0121218, -0.0298979, -0.0299084, 0.0464726, -0.0217649, 0.0491554, -0.112811, 0.0380209, -0.0737419, -0.0187218, -0.020802, -0.00142237, 0.104707, -0.0440834, 0.00496435, 0.045425, -0.0440616, -0.113934, -0.0437457, -0.119193, -0.104986, -0.0155966, 0.0396996, 0.10196, 0.0312978, -0.0130988, 0.0231182, -0.0685299, -0.0212238, 0.061556, -0.0856134, 0.0651636, 0.0229637, -0.0324933, 0.0593763, -0.0996056, -0.0175859, -0.0589947, -0.0265296, 0.0747611, -0.0403179, 0.155022, 0.0287039, -0.0679534, 0.0325633, 0.0786755, -0.00425481, 0.00448679, 0.00529201, -0.0314687, 0.0764749, -0.010894, -0.0513859, -0.0409239, -0.0796362, 0.0107627, 0.0114943, 0.0223096, -0.00162028, -0.0286302, 0.0667828, 0.00831839, -0.0121553, -0.014111, -0.0825649, -0.0883451, 0.185658, -0.019113, -0.0277651, 0.210288, 0.105975, 0.0734036, 0.0450295, 0.0627381, 0.0225782, -0.0043563, 0.0609402, -0.104841, -0.0823292, -0.0259648, 0.0269023, -0.0652049, -0.0166324, -0.054012, 0.02873, 0.0363802, -0.0239646, -0.0325027, 0.0645314, 0.0121897, -0.060116, -0.0355292, 0.0730609, -0.00531267, 0.00664172, 0.0688889, 0.0297415, 0.105698, 0.0500794, 0.15415, 0.0912778, 0.0126772, 0.052661, 0.0415482, 0.0231953, -0.0131675, -0.0452855, -0.137476, -0.103094, -0.0609916, -0.00985862, 0.0789334, 0.0801893, 0.00435745, -0.0295968, -0.0168456, -0.0449283, -0.0569434, -0.0925017, -0.020195, -0.0203868, -0.0476169, -0.0435374, 0.0124857, 0.0840176, 0.108627, 0.00255298, 0.0483666, 0.135771, 0.123943, 0.105532, 0.15297, -0.0267939, 0.0150964, -0.0490516, 0.00767192, 0.0443814, -0.0477332, -0.042906, -0.010735, -0.0476069, -0.0665966, -0.0417775, 0.00400512, 0.0672895, 0.0889715, 0.0164218, -0.0325202, -0.0387588, -0.0544504, -0.0488365, -0.0202076, 0.0484384, -0.00374679, 0.063542, 0.0860001, -0.00559176, 0.118661, 0.0669107, 0.0880503, 0.0882837, 0.0590952, 0.0723163, 0.0695387, 0.0799004, 0.0914287, 0.0820027, -0.0660095, 0.00301228, -0.081778, -0.0105864, 0.00689021, 0.0844829, -0.0335099, -0.0123887, -0.159056, -0.0110047, -0.083753, 0.0153731, 0.113696, 0.0730883, 0.0734271, -0.00218505, -0.102989, 0.0723832, 0.140603, 0.141225, 0.0492387, 0.103848, 0.133993, 0.0617514, -0.00504958, 0.0449235, 0.107842, 0.0432721, 0.0148973, 0.0654096, 0.0343091, -0.109827, 0.0266962, -0.000112675, 0.0046375, -0.0397127, -0.0424265, 0.0152445, -0.0512496, 0.0386948, -0.0208581, 0.07306, -0.0276858, -0.115609, 0.0915155, 0.11099, -0.00620097, -0.105064, 0.143328, 0.0461325, 0.0640293, 0.11309, 0.0264237, 0.0449917, -0.0380224, 0.0642676, 0.0116864, 0.00352531, 0.0938467, -0.017834, -0.0631892, 0.077565, 0.0118396, 0.00894755, 0.0189976, -0.0502404, -0.0323472, -0.0690383, -0.00482257, -0.122229, -0.0920601, -0.0840428, 0.0446751, 0.066807, -0.0731327, -0.00593764, 1.24598e-05, 0.0616705, 0.0758431, 0.0261291, -0.0600137, 0.0543303, 0.0350641, 0.0154227, -0.089519, -0.0451386, 0.0141218, 0.0539811, 0.000631567, -0.137278, 0.0237075, -0.039808, -0.076146, 0.108372, -0.0101901, -0.0281937, -0.0376014, -0.186108, -0.187874, -0.0408308, 0.0281289, 0.0047368, -0.110621, -0.0536607, -0.0302659, 0.0199357, 0.0329579, 0.0217367, -0.0846962, 0.0350749, -0.0686114, -0.0452786, -0.0167334, -0.0520265, -0.0813951, 0.151971, 0.00625145, 0.0535956, -0.0144482, 0.00776026, -0.0707298, -0.00215418, -0.069185, -0.0456636, -0.0776174, -0.0599948, -0.152826, 0.0654616, 0.0818646, 0.0133061, -0.0585735, -0.0469352, 0.122467, 0.0741409, 0.108904, -0.023904, -0.0313538, -0.0769191, 0.0226383, 0.0782609, 0.00607663, -0.0833314, 0.054234, 0.00219481, -0.0594017, -0.0510901, -0.0548094, -0.0539827, -0.0106449, 0.0587994, 0.0294086, -0.148119, -0.0818471, 0.0159839, -0.0885568, 0.115056, 0.0779437, -0.000938954, -0.0508782, -0.0207172, 0.064077, -0.0134797, 0.00477893, -0.126675, -0.0325351, 0.0111268, 0.0737562, 0.0240642, 0.075216, 0.0709817, -0.0499576, 0.100294, 0.0124091, -0.0193279, 0.0430572, -0.0468615, 0.0536996, -0.0315163, 0.0817595, 0.00177278, 0.0212545, -0.0279316, 0.0296605, 0.107494, 0.101635, -0.0457699, 0.0027282, -0.0129148, 0.0307821, 0.041568, -0.0667226, 0.00681585, -0.0181517, 0.087198, -0.0129529, -0.00557329, 0.0430099, -0.0198116, -0.102544, -0.0639727, -0.0108289, 0.068433, 0.0491973, 0.00514359, 0.115251, -0.0386074, 0.000389139, 0.0343032, -0.0552403, 0.103291, -0.025654, -0.00687339, -0.0784108, -0.0169236, 0.0497713, -0.0230273, 0.0210711, 0.0444286, 0.0261873, 0.0950044, 0.0823495, -0.0309469, 0.0747266, 0.125792, 0.0219061, 0.0581794, 0.00346386, -0.0111064, -0.106255, -0.0052548, 0.00440998, 0.147055, 0.124021, 0.12241, -0.00156857, 0.17361, 0.0766713, 0.10781, 0.0717827, -0.0439828, -0.034999, -0.0698323, 0.0210265, -0.00556014, 0.129427, 0.160041, 0.0690603, 0.080976, 0.120119, 0.0165938, 0.156577, 0.00726009, 0.0997213, 0.0247644, 0.0609571, 0.0185547, -0.104466, 0.0234632, 0.0307503, 0.0314224, 0.0489468, 0.04325, 0.119603, 0.213169, 0.169015, 0.180197, -0.0262306, -0.0365444, -0.110609, -0.00468712, -0.0289403, 0.130489, 0.127813, 0.0393078, 0.153642, 0.0529487, 0.193087, 0.050201, 0.0911733, -0.0151896, 0.0229958, 0.0202219, -0.021399, 0.010635, 0.00105756, -0.0150662, -0.0220991, 0.0878953, 0.0200776, 0.200091, 0.0402184, 0.196943, 0.066518, 0.0636963, 0.0027843, -0.0574207, -0.0530257, -0.0087128, 0.0244997, 0.102361, 0.0132055, 0.0901414, 0.0846673, 0.0667056, 0.0431164, 0.104854, 0.0144132, -0.0821894, -0.13434, 0.115259, 0.0704314, 0.0727346, 0.0167542, 0.0650244, -0.0160777, 0.0604265, 0.181584, 0.118346, 0.0946166, -0.00338136, -0.105619, -0.0119447, -0.0377419, 0.0242084, -0.0450701, -0.119649, -0.0229289, -0.0667059, 0.0455533, 0.0483537, 0.0376051, 0.0409285, 0.0335197, -0.0275625, -0.0820738, -0.119868, -0.0705203, 0.0179182, -0.0289419, 0.0300487, 0.0622722, 0.0217477, 0.0084595, 0.0439384, 0.174249, -0.0386886, 0.044893, -0.0272794, -0.0668107, 0.0097221, -0.0341564, -0.0528859, 0.0511996, -0.122877, -0.129482, -0.108404, -0.0714433, -0.0309294, -0.054268, 0.0292062, -0.0452933, -0.103688, -0.0699618, -0.0256962, -0.0226621, 0.0421199, 0.023605, 0.00963678, -0.0280464, -0.0260646, -0.0527989, 0.103357, -0.0275923, 0.0831374, -0.00207856, 0.011068, -0.0156491, 0.00725067, 0.0557217, -0.0192726, 0.0316765, -0.126461, -0.066171, -0.0748002, -0.0833881, -0.0691836, -0.104434, -0.0567247, -0.0856185, 0.0570536, -0.0368154, 0.037014, -0.0560377, -0.0411412, -0.0815581, -0.023208, 0.102269, -0.0206179, -0.0187089, -0.0559786, -0.0153817, 0.0316732, 0.115748, -0.0528415, 0.00946549, 0.089215, 0.0118419, 0.0834163, -0.00452808, -0.0725113, 0.0319325, -0.00428156, -0.0502573, -0.0954275, -0.0672387, -0.0430917, -0.0456603, 0.0408295, -0.110262, 0.035841, -0.130726, -0.0178459, 0.0968533, 0.00555113, -0.0178481, -0.029951, -0.0221378, 0.0226968, -0.0436538, 0.0229711, 0.114962, 0.044163, 0.0424347, 0.0723726, 0.083665, 0.0365077, -0.0355946, -0.0365078, -0.0517941, -0.120486, -0.00207642, -0.114728, -0.156686, -0.0548244, 0.0162093, -0.039588, 0.0819098, -0.0314093, -0.101183, -0.0197667, 0.0189571, 0.0448481, 0.0217851, -0.00606155, -0.0526229, 0.0383029, -0.0272307, -0.0375239, 0.134287, 0.115831, 0.0153705, 0.0367863, 0.0459606, 0.0643684, 0.0497274, -0.0387992, 0.0399084, 0.0570567, -0.0690335, 0.0132374, -0.0514575, 0.049424, 0.002137, -0.0346087, -0.0609005, 0.0335555, -0.0487478, 0.0746265, 0.0490909, -0.0189469, -0.0644494, -0.0406014, 0.0862823, -0.0323531, 0.0462432, 0.0563211, -0.0578478, -0.00278364, 0.0591117, 0.00790449, 0.0948963, -0.0316488, 0.0821029, -0.0832427, 0.043584, 0.0220541, -0.0916531, -0.0304569, -0.0070749, 0.0999059, -0.0124732, 0.033474, -0.117345, 0.0531075, 0.0339476, 0.0183168, 0.0524151, 0.0288256, -0.10353, 0.0183937, 0.0909569, -0.0583277, -0.0813006, -0.0394298, -0.0279697, 0.0513931, 0.00715866, 0.0804555, 0.0812162, 0.052698, 0.17563, 0.0211778, 0.0474165, 0.04553, 0.030247, 0.0661061, 0.0806317, -0.0199042, 0.0154796, -0.038877, 0.00345087, 0.0243581, -0.0248133, -0.072798, -0.0449634, -0.0153953, 0.0656602, -0.0160436, -0.0284762, 0.00957881, -0.0311336, 0.0376416, 0.0738485, -0.010271, 0.0396612, 0.0188003, 0.0452214, -0.115711, -0.0498262, -0.0496848, -0.053106, -0.0221224, 0.019856, -0.0226586, 0.0513739, 0.0747889, -0.0248627, 0.0567062, 0.0509295, -0.0203553, -0.0154603, 0.0330428, 0.0238535,
0.0501794, 0.0286382, -0.0984318, -0.0661867, -0.0701221, 0.0589204, -0.031757, 0.00547555, 0.0455104, -0.0182883, -0.0111174, 0.0211192, -0.124248, 0.0267608, 0.0385928, -0.0154984, -0.043571, -0.0560759, -0.158145, -0.0200986, -0.0296982, -0.0280159, 0.0240958, 0.0221105, 0.0409212, 0.0101714, -0.049451, 0.00186057, -0.0804721, 0.0592949, -0.0753296, 0.0163679, -0.074532, 0.0125455, 0.0170683, 0.0379332, -0.0138922, -0.0258648, -0.0209513, 0.134032, 0.0108788, -0.00670076, 0.0651639, -0.0826776, 0.00647882, 0.0182778, 0.0125885, -0.00505326, 0.044507, 0.0148111, -0.00273929, -0.0883687, -0.0555376, 0.057276, 0.0200045, -0.0268266, -0.0936786, 0.00969242, 0.00891223, -0.0191533, -0.0369675, 0.0337241, -0.1033, 0.0276802, -0.017667, 0.0191166, 0.136927, 0.0208733, 0.0618488, -0.00881758, 0.0339855, 0.133214, -0.0422024, 0.00387773, 0.0658309, 0.101201, 0.00913753, 0.0420044, -0.0101223, -0.00291999, -0.0123385, 0.0795959, -0.00449621, 0.053242, -0.023705, -0.00131299, 0.0395976, -0.061583, 0.026613, 0.0335308, 0.0591138, 0.0217467, 0.102757, 0.0752425, 0.00559216, 0.119398, 0.049988, 0.0847144, 0.0229837, 0.0472464, 0.087067, 0.0197637, 0.050584, -0.00885075, 0.0649326, 0.0440425, 0.0436882, 0.0768957, 0.00824666, 0.000593148, 0.0604085, -0.0316165, -0.00422566, -0.00597048, -0.122902, 0.0311271, -0.00049782, 0.0560472, 0.0590227, 0.059657, 0.197973, 0.1383, 0.13423, 0.155234, 0.120103, -0.036638, 0.0332374, 0.0201689, 0.0862877, 0.0244682, 0.0577393, 0.0438536, -0.032766, 0.0148187, -0.00531772, 0.0350824, 0.0369811, -0.0405973, -0.0550373, 0.00249054, -0.0324863, 0.0129866, 0.0221686, 0.0873949, 0.0470237, -0.0209492, -0.0368523, 0.0806121, -0.00144871, -0.012371, -0.0933836, 0.00845449, 0.0279386, 0.0478962, -0.00284729, -0.00876532, 0.0882885, 0.0653477, 0.0507605, 0.0636771, 0.0291592, 0.132643, 0.085531, -0.0116466, 0.00370608, 0.0427688, -0.0575933, -0.0127663, 0.0186108, 0.074905, 0.0422181, 0.00526969, -0.056482, -0.0116575, -0.0187833, -0.0240498, -0.078479, 0.00420161, 0.0622892, -0.0628805, 0.0749793, -0.0689532, -0.032041, 0.0818999, 0.0461445, 0.0330073, 0.0179796, 0.0526749, 0.0279082, -0.0467019, -0.0635375, -0.00335098, 0.119444, 0.109201, -0.0834651, -0.017834, -0.0244849, 0.0228479, -0.0459766, 0.0307938, -0.0101632, -0.0257987, -0.155673, -0.090261, 0.085022, 0.00612114, 0.0203027, -0.00310438, 0.0252088, 0.0722762, 0.0208614, 0.0409638, 0.0733789, -0.0522194, 0.0851267, -0.0551555, -0.0596465, 0.0381287, 0.015379, -0.0790714, -0.0728016, -0.0727176, 0.00457496, -0.084594, -0.0105603, -0.0809339, -0.00613225, -0.0555638, -0.0892376, -0.158129, -0.00132129, 0.0555299, 0.0208452, -0.00296453, -0.0218489, 0.044033, 0.0402774, 0.0304787, 0.0585474, 0.0507645, 0.0976534, -0.0696392, -0.182158, 0.0472596, -0.0776111, 0.00703657, 0.0584388, -0.075295, -0.00670444, -0.00544458, -0.00544528, 0.0707386, -0.0069079, -0.0542169, -0.0047753, -0.0286099, -0.107197, -0.124963, -0.0533055, -3.0006e-05, 0.111763, 0.0551647, -0.0102737, -0.0364913, -0.0710404, 0.034939, 0.0726487, 0.0400294, 0.0888884, -0.0556524, -0.0644793, -0.148286, -0.165231, -0.0569413, -0.0632866, -0.0264483, 0.0477513, -0.0589065, -0.00504428, -0.0265553, -0.0603353, 0.000463235, 0.0372453, 0.000318656, 0.0808892, -0.0921267, 0.139889, 0.0474872, 0.0777375, -0.0182062, 0.039538, 0.00593435, 0.0147317, -0.0254048, 0.126728, 0.000550686, -0.0807991, -0.202946, -0.126624, -0.0994413, -0.0438144, -0.00273134, -0.112884, 0.046348, 0.0587285, -0.00443592, -0.00540031, -0.0908704, -0.0132243, -0.0939359, -0.0146615, -0.0160987, 0.0787629, 0.0704621, 0.0708589, 0.0174699, 0.128487, 0.0238561, -0.144764, -0.089984, -0.0590632, 0.0224483, 0.153767, -0.026624, -0.123674, -0.149202, -0.0621404, -0.110936, 0.0212257, 0.00305687, 0.00171024, 0.0129326, 0.0827537, 0.039522, 0.0776931, -0.0471486, -0.0582466, -0.0478503, 0.00748403, -0.0899063, 0.0305273, -0.0126196, 0.0794534, 0.0556671, 0.133244, 0.138089, -0.00576271, 0.0648294, -0.0991739, 0.00963011, 0.150583, -0.0156969, -0.106655, -0.0828849, -0.180793, -0.12029, 0.0535096, 0.0249996, 0.0106385, 0.0412961, 0.0771558, 0.0189392, 0.0690647, -0.0097918, -0.0281034, -0.00571414, 0.0184178, 0.00224154, -0.0522703, -0.0252093, 0.0200364, -0.0207337, 0.0995555, 0.0469996, -0.045051, -0.0867938, 0.0221405, 0.138609, 0.146845, 0.123981, 0.0203981, -0.0226082, -0.0972415, 0.0125761, -0.0820322, -0.0075434, 0.0819317, 0.00850278, 0.0428472, 0.0616175, 0.0103398, -0.0211176, 0.0230967, -0.0275681, 0.0272511, -0.0269371, -0.0502319, -0.0995832, -0.0329967, -0.0222655, -0.0226345, 0.0498327, 0.0367879, -0.00226967, 0.0184507, 0.125883, 0.156637, 0.0888117, 0.113203, -0.128212, -0.0496472, -0.0903866, -0.0882719, -0.0675427, -0.0252815, -0.0696008, -0.0364853, 0.0228148, -0.0164834, -0.0171881, 0.0180458, -0.00175609, 0.0290067, 0.0619919, -0.129973, -0.0284973, 0.0593586, -0.0587873, 0.10407, 0.0142904, -0.0324409, 0.0540524, 0.0430927, 0.268018, 0.290268, 0.0650446, 0.0111309, -0.0279289, 0.0875911, -0.0293649, -0.0475186, -0.0139491, -0.0950777, -0.109446, 0.0164988, 0.00412729, 0.016756, -0.0905823, 0.0701698, -0.00577787, 0.0299909, -0.0696206, -0.0116809, -0.0460908, 0.00423156, -0.0862431, -0.0880741, 0.058126, 0.110236, 0.0646209, 0.0388286, 0.135411, 0.209578, 0.0488926, 0.00363522, 0.00732549, -0.00611964, -0.0288897, -0.0865828, -0.00062114, -0.019796, -0.0185432, 0.0844058, 0.161873, 0.0603832, 0.0219183, 0.0556478, 0.00639174, -0.0280591, -0.00385211, 0.0100565, -0.0950271, 0.0251491, -0.0119649, 0.0691152, -0.0740821, -0.0716723, -0.0748747, 0.000504408, 0.0650123, 0.0261498, -0.0231434, -0.0464098, -0.0340313, -0.0139312, -0.00245398, -0.0390422, 0.104889, 0.00975431, 0.00581425, 0.0295728, 0.0696806, 0.04656, -0.034787, -0.0445124, 0.0214815, 0.110883, 0.0716183, -0.0313297, -0.0307541, 0.0474559, 0.0495746, -0.0964616, -0.0296308, -0.0721723, -0.164685, 0.0361536, 0.0471768, -0.0170208, 0.0544414, -0.0631579, 0.0405062, -0.0617794, -0.0431799, -0.0472267, -0.0561107, 0.0845083, -0.0114515, 0.0310606, 0.0138369, -0.0354341, 0.0343479, 0.0389945, 0.0349403, -0.05695, 0.0593822, 0.0166458, -0.0566252, -0.00897176, 0.0780496, -0.0200356, 0.0369164, 0.0983453, -0.00922858, -0.0149363, 0.0837368, 0.00509283, 0.0788793, -0.00304519, 0.0985782, -0.105864, 0.0427694, -0.0389692, 0.0688437, 0.12589, 0.0998257, 0.0359601, -0.00739638, -0.0503013, -0.0495264, -0.00401848, -0.110964, 0.00541981, -0.0412978, 0.0413687, 0.130569, 0.0526009, 0.0970741, 0.108394, 0.043827, 0.135008, 0.108396, -0.0118879, 0.0788114, 0.127036, 0.15896, 0.040165, 0.0398172, 0.0541127, -0.0358007, 0.0785645, 0.101733, -0.00402292, 0.0801295, 0.0433023, 0.0518181, -6.41425e-05, -0.0375737, -0.0672471, 0.00108141, 0.0231236, 0.0453557, 0.0888739, -0.0315484, 0.0224573, 0.171802, 0.108816, 0.133944, 0.0644865, 0.146403, 0.131215, 0.125048, 0.113164, -0.0139198, 0.0275702, 0.0747699, -0.0170092, 0.0660017, 0.0837499, 0.0557682, 0.0641277, 0.0316617, -0.042237, -0.000142009, -0.0144846, -0.0845948, -0.0116619, 0.0855738, -0.0122533, -0.03825, 0.0596546, 0.0247341, 0.0976851, 0.0729308, -0.0328528, 0.0721182, 0.0447915, 0.0982803, 0.0782001, 0.0577959, -0.00835258, 0.00730003, -0.0613333, 0.141274, 0.0271604, 0.0374556, 0.035964, 0.143051, 0.0511458, 0.0969678, 0.0245146, -0.00891961, 0.0132884, 0.0659264, 0.104604, 0.0163347, 0.0479972, 0.0637178, 0.0604215, 0.0900584, -0.152298, -0.084484, 0.0079341, -0.0225084, -0.0396299, -0.0477572, 0.0011788, -0.00275885, -0.0273579, -0.00774273, 0.0738571, -0.0536078, 0.0531503, 0.189358, 0.0297375, -0.00384996, 0.087197, -0.00319133, 0.00679353, -0.0552617, -0.0341292, -0.0588092, 0.0863132, 0.018924, 0.0193004, 0.0143493, -0.00278316, -0.0673217, -0.0652402, -0.0825648, 0.0681742, -0.0566182, -0.0166579, -0.0368435, 0.0495668, -0.0519792, 0.00777838, -0.0456393, 0.0731275, -0.0202927, 0.106705, 0.0797314, 0.123155, 0.0474379, 0.0609906, 0.0449838, -0.0356577, -0.073695, 0.0200232, 0.0605931, 0.0220648, -0.0367197, -0.0125822, 0.0355628, 0.0283661, 0.0369474, -0.0231998, 0.0746398, 0.00591985, 0.0138, 0.0599623, 0.020458, 0.0317382, -0.0114396, 0.0190486, 0.046019, -0.0248414, 0.0796136, 0.0733617, 0.0558895, -0.0220438, 0.0555135, 0.0613848, 0.0305022, -0.0539253, 0.0195516, -0.0784387, 0.0526907, -0.0180456, -0.0242818, -0.00246967, 0.0326113, 0.0306983, -0.0498189, 0.064063, -0.000392009, 0.0327821, 0.00482107, 0.0214845, -0.0596674, 0.0761764, -0.00214071, 0.0501897, 0.00724153, -0.0609602, 0.00960735, -0.0702373, 0.0700677, -0.0361918, -0.0115459, -0.0840059, -0.0668065, -0.00282497, 0.0798127, -0.034828, -0.114537, -0.0298807, 0.0369809, 0.0369012, 0.071552, 0.00283279, -0.0310516, -0.0742742, -0.0873937, -0.00852555, -0.0976996, -0.000497724, 0.0671276, 0.0285949, 0.000889438, -0.03479, -0.0501891, -0.00472944, -0.0268432, -0.0151395, 0.0808354, -0.0145241, -0.0501435, 0.0287039, 0.0447071, 0.0394785, -0.0375094, 0.0454114, 0.0400959,
-0.0121014, 0.0455641, 0.0389619, -0.0490878, -0.0799727, -0.00875075, 0.0210858, 0.00535973, 0.00712056, 0.0471706, 0.0239126, -0.00793615, -0.0481059, -0.0711554, 0.08217, -0.0490528, 0.0242283, -0.0193724, -0.0175642, 0.0212973, -0.0187154, -0.0305603, 0.0346784, -0.0912673, -0.0340085, 0.0272936, -0.0614635, 0.0392034, 0.0317746, 0.0263121, -0.0659921, -0.0649207, -0.0124143, -0.0207425, -0.0193338, -0.0102889, -0.0369022, -0.0784213, 0.0321703, 0.0164893, -0.0166585, -0.0773982, 0.0744936, -0.0567235, -0.0501595, -0.0643251, 0.0061682, 0.0209228, -0.00545022, -0.0621062, -0.118068, -0.0994752, 0.0273291, -0.0146909, 0.0416254, 0.00183403, -0.0407561, 0.069213, -0.0305659, 0.0323024, 0.00545681, -0.0238037, 0.076863, -0.00815598, -0.0439645, -0.102685, 0.0534114, -0.000863258, 0.00320231, -0.025833, 0.160778, -0.060797, -0.0028037, 0.0120575, -0.0239256, 0.0154871, -0.0510697, 0.0751678, 0.00387929, -0.137334, -0.0784043, -0.067339, 0.0619523, -0.0714784, -0.0270942, 0.0288886, -0.0806059, -0.0494681, -0.0806958, -0.0255641, -0.0369501, 0.00780911, -0.036543, 0.0291107, 0.0447379, 0.0101768, -0.0263404, -0.0267807, 0.117991, -0.0376652, -0.029372, 0.0634433, 0.00448159, 0.0241251, -0.025975, 0.0179461, -0.113711, 0.0437855, 0.0647775, -0.0730471, 0.0147017, -0.011526, -0.00489318, -0.00454154, 0.00169666, 0.0133353, 0.00985355, -0.0168264, -0.0476351, -0.0298399, -0.0395166, 0.0205671, -0.023462, 0.051904, 0.00307415, 0.0575195, 0.00499273, 0.0471325, 0.0120135, -0.0714021, -0.0593899, 0.00814444, -0.0467871, -0.121759, -0.0136389, 0.00732229, 0.0332903, -0.00866047, 0.0286227, -0.0957175, -0.0462781, -0.0782251, 0.0633215, -0.00233018, -0.00271392, 0.0478742, 0.0137788, -0.0945997, 0.0348666, -0.0699988, 0.0618625, -0.0374532, -0.00310361, 0.0539064, 0.0561336, 0.00538361, -0.0537598, 0.0253014, -0.0350064, 0.0200214, -0.0651454, -0.00416051, -0.0282949, 0.0216501, -0.0407473, -0.0761683, 0.00645789, -0.0040865, -0.0149354, -0.0206544, -0.0701204, 0.00506208, -0.0116209, 0.0214184, 0.00266661, 0.0360262, -0.035536, -0.113133, 0.0442081, 0.0116635, 0.0167611, 0.0562951, -0.00945239, -0.0186943, -0.0663559, 0.016984, -0.0916114, 0.021453, -0.040123, -0.00185613, 0.0715649, -0.0884997, 0.0488701, -0.0971504, -0.0389366, 0.0377801, -0.0168513, -0.0755828, -0.0177834, 0.00141831, -0.0778587, 0.0907282, 0.0521947, -0.0491251, -0.0376496, -0.0523939, -0.0035267, 0.0683882, 0.0437371, 0.0164267, -0.0178882, 0.0267031, -0.145352, -0.0823928, -0.168058, -0.114143, -0.124563, -0.0573981, -0.0394564, 0.0506115, -0.0666001, -0.0945823, 0.0474514, -0.0327467, 0.0867829, 0.0204304, 0.0230279, -0.0160955, -0.00425175, 0.0364936, -0.0506975, -0.0444527, -0.0404952, -0.124762, 0.0178503, 0.0673112, 0.0429095, -0.0696033, -0.00959337, -0.156737, -0.0057052, -0.0521995, 0.00511348, -0.0746884, 0.0334526, -0.0473375, -0.0295314, 0.00413132, 0.00470829, 0.0407009, 0.0772963, -0.063795, -0.0220207, -0.0708283, 0.0222776, -0.0639221, 0.0501732, 0.0410948, -0.0443715, 0.010917, -0.0783171, -0.118297, 0.0208667, 0.0457285, -0.0655702, -0.0672655, 0.0266976, 0.0187558, 0.0613129, 0.0265239, -0.00270636, -0.0176752, 0.00227594, -0.0400789, 0.0446751, 0.0347464, 0.102574, 0.0998655, -0.0321912, -0.00475382, -0.0263148, -0.0535525, -0.0630461, -0.0456296, 0.0294158, -0.0455102, -0.0131217, 0.0793665, -0.0855176, -0.0298432, -0.0217662, 0.0213178, -0.0251721, 0.00131749, 0.00651894, 0.00706441, 0.0512915, 0.0249532, 0.0509047, 0.0156827, 0.00521476, 0.094377, 0.0647187, 0.0813239, 0.128476, 0.0778839, 0.0680044, -0.0461783, 0.0398309, 0.0260733, 0.0540399, -0.00146245, -0.0205402, -0.0282905, 0.0516796, -0.0412477, -0.0307618, -0.0827654, -0.0220829, 0.12603, -0.0200888, -0.0527443, -0.0668197, -0.0446583, 0.0050392, 0.025356, 0.00245474, 0.0491882, 0.00699899, 0.0492739, 0.0392069, 0.0732385, 0.0389277, 0.0493987, 0.00264441, 0.0166396, -0.0228115, 0.0740841, 0.0087869, 0.0986361, -0.00223284, -0.139961, 0.0605839, -0.0245706, -0.00295792, -0.0389284, 0.0376671, 0.0927515, -0.00525823, 0.0341611, 0.0404665, -0.0608167, 0.0406079, 0.0612223, -0.00519088, 0.0283668, 0.0435781, -0.0126576, 0.0471554, -0.0446115, 0.0271839, -0.0419131, 0.0243062, -0.0624579, -0.0410169, 0.0218356, 0.0289207, -0.00627884, 0.0395385, 0.0317625, -0.0292586, 0.0158865, -0.0205222, -0.022011, -0.0239194, -0.00476738, 0.0954082, 0.00586325, 0.0611293, -0.022366, -0.0293714, -0.0211158, -0.0326221, -0.0526551, -0.046331, -0.0503674, 0.00568984, 0.0293022, 0.0659058, 0.00168351, 0.0497134, -0.0221866, 0.0955995, -0.0102362, 0.0777963, -0.0565433, 0.00659507, 0.0524441, -0.010455, 0.0504647, 0.02946, 0.0310835, 0.0619384, -0.0954115, 0.0555181, -0.0122905, 0.0096881, -0.00376469, -0.0604165, -0.0583697, 0.00887165, -0.0747287, -0.0093838, -0.0371076, 0.0903818, -0.0166906, 0.0501698, 0.0380022, -0.0372595, 0.00738889, -0.101961, -0.0191856, 0.00683141, -0.0960304, 0.0787854, 0.0461503, 0.0248763, 0.0199725, -0.050538, -0.0436233, 0.0188887, -0.0603013, 0.0433297, -0.0634912, -0.0801433, 0.0254574, -0.0181967, 0.004096, 0.061135, -0.0101765, -0.0444263, -0.033861, -0.0865195, 0.0283894, -0.0247982, 0.0324783, -0.0229813, 0.0297707, -0.0205704, -0.0719275, 0.105395, 0.0169617, 0.0257052, -0.0450788, 0.0561085, -0.0110443, -0.112581, -0.143266, -0.0931764, -0.0616158, -0.0176433, -0.0213413, 0.0555795, 0.105697, 0.104928, 0.130564, 0.0813613, -0.0395863, -0.0546713, -0.0353367, 0.000734213, -0.00146891, 0.0142982, 0.0496601, -0.0686294, 0.12042, -0.0676265, -0.0158183, -0.0606539, -0.0342469, 0.00649366, 0.106943, 0.0347069, 0.0672021, 0.0103662, 0.0374227, -0.073788, -0.0327201, -0.0140338, 0.0398839, 0.0806515, 0.00221483, 0.0676022, 0.022813, -0.0457947, 0.0241171, 0.0125499, 0.0199496, -0.0349613, 0.0689341, -0.00970591, -0.0264932, -0.000979173, 0.00594777, -0.0281874, 0.0147129, 0.0264186, 0.0202587, -0.014722, -0.00171312, 0.153241, 0.158857, 0.0657086, -0.0271182, -0.000645549, -0.00313882, -0.00549685, 0.186645, 0.0441581, 0.070725, 0.0590532, 0.0180431, -0.110662, -0.0486365, 0.0647717, 0.0482738, -0.127055, -0.00303359, -0.0653243, 0.0141252, -0.00394904, -0.00861304, -0.0543788, 0.0498616, 0.0495871, -0.000715205, 0.031474, 0.0759862, -0.00546084, 0.0929522, 0.0710357, 0.0177899, 0.0883707, 0.00949378, 0.0207764, 0.0183349, 0.134886, 0.0698493, -0.095627, -0.0592899, -0.0705367, -0.0112107, -0.103707, -0.0221815, -0.0914827, 0.0186929, 0.0588269, -0.0159665, 0.0101883, -0.00455336, -0.0235013, -0.0100241, 0.0209422, -0.0308749, 0.0943301, 0.0612391, 0.0196323, 0.0468794, 0.0291363, 0.0586877, 0.0549815, -0.00770474, 0.0455973, -0.0332718, 0.040399, 0.0254887, 0.00446959, -0.0842243, -0.0249943, -0.0499647, 0.0554106, -0.034025, 0.040974, -0.080908, 0.0626803, -0.0459867, 0.0271476, 0.0089968, -0.0700228, -0.0438059, -0.0137235, -0.0674665, -0.0452703, 0.0376678, -0.0438575, -0.00480927, -0.0139524, 0.0926299, 0.104737, 0.0192534, -0.0432229, 0.00138729, 0.0382053, -0.0806497, -0.0364872, -0.0289672, 0.0471787, -0.0666361, 0.0816306, 0.0126228, -0.023684, 0.0877081, -0.0201533, 0.0462991, 0.0598378, 0.019388, -0.0719645, 0.033145, -0.0101251, -0.0747867, -0.0148704, -0.12954, -0.0816287, -0.0249279, 0.027049, 0.00262439, -0.00403322, 0.0140398, 0.0500803, 0.0563308, -0.0258882, 0.0695246, -0.0111736, 0.0934456, 0.0203435, 0.0023526, 0.0390032, 0.0194302, -0.0343687, 0.0898517, -0.00260622, 0.0744625, 0.0257617, -0.0459193, -0.0197923, -0.0594475, -0.00424455, -0.0313108, 0.0560163, 0.00637656, 0.0164547, -0.0930862, -0.106141, -0.0395106, -0.0692236, -0.0206595, -0.0772285, 0.0040898, 0.0522035, 0.0419343, 0.0590511, 0.00305685, 0.0863478, -0.0137116, 0.0602239, -0.00187327, 0.031789, 0.0374749, -0.045992, -0.0200035, 0.0484759, 0.0890633, -0.0434301, 0.0870801, 0.109559, -0.0986686, -0.0975799, -0.00685561, -0.119458, 0.0246801, -0.0299075, -0.00322505, 0.00182554, -0.149723, -0.0623025, -0.0571984, -0.120074, -0.00457874, 0.00153971, 0.0605484, 0.096978, 0.0155862, 0.0365703, -0.0224354, -0.048004, 0.0372551, -0.00347119, -0.100334, -0.0748435, -0.00171081, 0.0329376, -0.0343013, -0.0258625, -0.0674672, 0.0408599, 0.030572, -0.00909317, 0.0354743, 0.0826831, -0.0129956, 0.0153297, 0.0164529, -0.0456106, 0.0522434, -0.0561362, -0.0740005, -0.100546, 0.0260956, -0.0348957, -0.0175407, -0.0101225, -0.0740662, 0.081005, -0.0485874, -0.0678515, 0.0602943, 0.0257798, -0.119565, 0.00848567, -0.0813875, 0.0460493, -0.0214209, 0.0792253, 0.0425244, -0.00797613, -0.107699, 0.0437205, -0.0121087, -0.00811043, 0.0437625, 0.0318355, -0.0786889, 0.107995, -0.0456067, -0.132367, 0.0603294, -0.0331555, -0.0842181, -0.0622879, 0.0263218, 0.0544919, -0.0381577, -0.104509, -0.0640831, -0.0245901, -0.089687, 0.00563448, -0.0484469, -0.0722642, 0.0348895, -0.0528413, -0.00575432, 0.0449846, -0.0190828, -0.143297, -0.0117982, -0.0347448, -0.0030607, 0.0227163, 0.0277694, 0.0273003, -0.00524669, 0.0848538, -0.0773532, -0.0241854, -0.00446805, 0.0316303, -0.0705998, 0.0440543, 0.018753, 0.0380512, 0.0126735, 0.026755, -0.0385927,
0.0896037, 0.00663965, 0.0361015, 0.089829, -0.0226326, -0.0107079, -0.0903472, -0.0357874, -0.0166629, 0.0123097, 0.0175739, -0.0658676, -0.00170735, 0.0207016, -0.0195957, -0.0674675, -0.0108431, -0.031248, -0.029606, -0.0241573, -0.0256877, 0.00276715, -0.0365978, 0.0158711, -0.036388, -0.0654446, -0.0287987, 0.0473827, 0.0259244, -0.0870065, -0.0132592, 0.0256627, 0.0825527, -0.0186366, -0.0305879, 0.0430106, 0.00344099, 0.0687498, 0.00613191, -0.0291218, 0.0953379, -0.00934887, 0.0123428, 0.0176153, 0.0609041, -0.0637639, -0.0242946, 0.0766858, -0.0635082, 0.0328116, 0.038062, -0.146197, -0.0412795, -0.089479, -0.0346036, 0.0371956, -0.0433136, -0.0233723, -0.0612533, -0.117235, 0.0161715, 0.0989669, 0.00401841, -0.0301103, 0.0333039, 0.0162085, -0.0319035, 0.0413638, 0.0245595, 0.12948, -0.00203668, 0.0899632, 0.0760704, 0.0273534, -0.0317164, 0.0459544, -0.0508784, -0.0215507, 0.017582, -0.0639017, -0.0370594, -0.0228797, 0.00120783, 0.0165159, 0.0560361, 0.0270331, 0.0404734, -0.00712745, 0.0131503, -0.0356519, 0.126067, -0.0401656, -0.0294463, 0.0482115, 0.0444618, 0.0380231, 0.0659449, 0.0519329, 0.0274908, 0.154739, 0.0479852, -0.0275058, 0.0472905, 0.121998, -0.0133928, 0.0332715, 0.0599648, -0.0266142, -0.0551137, -0.014094, -0.0934648, -0.00913133, -0.00371354, 0.00534546, 0.0530366, -0.0389188, -0.0750326, 0.0291463, 0.0250361, 0.00896764, 0.0134849, -0.0037597, 0.0798301, 0.011753, 0.0151661, -0.0622838, 0.040206, 0.0656165, 0.045609, -0.101099, -0.00130117, 0.030593, -0.0480305, -0.0168983, -0.0823617, 0.0555946, -0.0808849, -0.0348293, -0.0164017, 0.0205521, -0.0380499, -0.0201382, -0.0617065, -0.00220773, 0.0466611, -0.0221119, 0.0805582, -0.0362486, 0.0509615, 0.080859, 0.0955271, -0.0271313, -0.048641, -0.0101749, -0.152096, -0.0752522, -0.0199831, -0.0317518, -0.0759784, 0.0363239, -0.0914895, -0.0560898, 0.0690145, -0.0126323, -0.00365918, 0.0706746, -0.0436582, 0.0265327, 0.0104108, -0.0741252, -0.0723513, 0.0277478, 0.0282738, 0.0214848, 0.091537, 0.0368362, 0.0262115, -0.0214385, -0.00298465, -0.098181, -0.161618, -0.198898, -0.132685, -0.10795, -0.102175, -0.109501, -0.0876862, -0.0506244, 0.0666961, -0.0636934, -0.060374, 0.0243132, -0.114589, 0.0300141, 0.028688, 0.0681902, -0.0354001, 0.0346108, -0.120988, -0.0205277, 0.0623601, 0.00495797, -0.0246639, 0.0655369, 0.0445363, -0.0712595, -0.0146135, -0.0455412, -0.126125, -0.181814, -0.127165, -0.131464, -0.0817958, -0.0250255, -0.0966185, -0.0919237, -0.0458754, -0.104905, -0.109105, -0.0857026, -0.037078, -0.0503217, 0.0430734, -0.00738434, -0.0126031, 0.0322897, -0.00522094, -0.0298451, -0.0285001, -0.00504725, -0.0221072, 0.0693534, -0.109296, -0.0584301, -0.0938005, -0.0479092, -0.0483455, -0.0229154, -0.0713364, -0.0210511, -0.0599259, -0.0120557, -0.0198504, -0.0655079, -0.0626726, -0.0466997, -0.0310815, 0.00923799, -0.0927388, -0.0457686, -0.0873774, -0.0314022, -0.0975725, 0.0274261, 0.00565628, 0.0469633, 0.0262764, 0.0764688, 0.0245155, 0.0227009, -0.0755235, -0.147396, -0.0530717, -0.0116177, 0.188139, 0.070753, 0.0381153, 0.0235533, 0.0651389, 0.0331663, -0.0670882, -0.0673392, 0.0404053, -0.0678271, 0.0748326, 0.0313267, -0.00509502, -0.00202717, -0.0197276, 0.0265354, 0.0757312, -0.05176, 0.0178167, 0.0599538, 0.128955, 0.0360001, 0.0432791, 0.0242543, -0.0273906, 0.0740762, 0.227936, 0.177245, 0.127698, 0.099223, 0.130239, -0.0201794, 0.0168766, -0.0535697, 0.058985, 0.0285581, 0.0264233, 0.0581135, 0.114439, 0.129295, 0.0596327, 0.0714113, 0.00765321, 0.0239228, -0.0330422, -0.00350629, -0.133859, -0.0228472, 0.086022, -0.0263851, -0.0481891, 0.00441402, 0.102639, 0.0843039, 0.202105, 0.222729, 0.0318317, -0.0329682, 0.0486957, -0.00761516, 0.00471863, 0.0489484, 0.145484, 0.0869993, 0.0666635, 0.0577458, -0.0150231, 0.0580942, 0.0745697, 0.0448194, -0.00758097, -0.0319198, 0.0318174, 0.0870866, -0.0150064, -0.0694413, -0.027259, 0.0919463, 0.0653883, 0.0640614, 0.109904, 0.238896, 0.143689, 0.0886875, 0.00322107, -0.00886209, -0.0385634, 0.0339062, 0.0913612, 0.143436, 0.0519591, 0.029524, 0.0839859, 0.0132933, 0.149931, 0.128148, 0.0558918, 0.01564, 0.000704024, -0.0297199, -0.0033737, 0.0500545, -0.00808055, -0.0629607, 0.0468516, 0.129734, 0.09146, 0.0709901, -0.0277139, 0.0922204, 0.0628837, -0.00421453, -0.00296704, -0.113799, 0.00961967, 0.0196014, 0.109389, 0.00596406, 0.063607, 0.060982, 0.0556322, 0.0606398, 0.0424649, 0.0588592, 0.0852012, -0.0132066, 0.075447, -0.0386617, -0.0174473, -0.00709641, -0.0539214, 0.0678204, -0.0460253, -0.00850705, 0.0159774, 0.00667376, -0.0434571, -0.00831976, 0.0197163, -0.0998889, 0.0519555, 0.0359197, 0.0235218, -0.0106439, 0.0193285, 0.0604966, -0.00316858, 0.141057, 0.0904085, 0.0730158, 0.0840456, 0.0267844, -0.000807473, 0.0349845, 0.0317751, -0.0575619, 0.0362848, 0.0181722, -0.106761, -0.103762, -0.0502008, 0.0379187, -0.0310272, -0.0252475, -0.0463931, -0.105578, -0.103559, -0.107653, 0.0174333, -0.0548595, 0.0892557, -0.0279173, -0.130864, -0.0194112, 0.0837433, 0.0620882, 0.169557, 0.0420247, 0.0150367, 0.085561, 0.0730039, -0.026456, -0.0671291, 0.0337996, 0.0376674, 0.0666183, 0.0276559, 0.0154477, 0.0273305, 0.09189, -0.137478, 0.014725, 0.0548172, -0.0636906, -0.131226, -0.00753172, -0.0821603, 0.0326318, 0.0213878, -0.097064, -0.197532, -0.114063, -0.0226144, 0.0771829, 0.025562, -0.0690017, -0.0202641, -0.122799, -0.0385125, -0.00655018, 0.0063193, 0.0226671, 0.0330732, -0.10532, -0.0532454, -0.0958088, -0.0202783, 0.0567504, 0.0779232, 0.109489, -0.0256603, -0.00362735, -0.0162635, -0.0476794, 0.0817749, 0.0208754, 0.0826092, -0.0782611, -0.119898, -0.0837817, -0.0402315, 0.1225, -0.0848082, 0.0781649, -0.0392721, -0.0389225, -0.0152531, 0.08259, -0.0279926, -0.127724, -0.0197306, 0.00153544, 0.137649, -0.0511313, -0.0239764, -0.00802001, 0.0418618, 0.0841729, 0.0738771, -0.0439062, 0.0873325, 0.0868498, 0.083177, 0.130362, -0.016186, -0.0336986, -0.0489012, 0.00653509, -0.0230099, -0.00882346, 0.00705809, 0.0499135, -0.068106, 0.0315249, -0.00526988, -0.00627769, -0.0376804, -0.0274097, 0.0822339, 0.034268, 0.0471311, -0.063208, -0.0102566, 0.0293457, 0.0942785, 0.0820456, -0.00618336, 0.136392, 0.14479, 0.127047, 0.104562, 0.0248457, 0.10141, 0.120778, 0.03182, -0.0304043, 0.0134456, 0.053731, -0.0318975, -0.078892, -0.0962823, -0.070861, -0.0561479, -0.0432704, 0.00991011, 0.00232655, -0.0706033, -0.0252997, -0.0337198, -0.0498709, -0.0674307, -0.00699176, -0.0248949, 0.0755464, 0.0480694, 0.0996059, 0.131031, -0.00646004, 0.0532162, 0.0983828, 0.13993, 0.0768722, 0.0241667, 0.0393028, -0.0552339, -0.126555, -0.1058, 0.00543317, -0.0441663, -0.0832412, 0.0339505, -0.0221421, -0.0631645, -0.033192, 0.00362149, 0.00327253, 0.0235283, -0.0238191, -0.0558186, -0.00436641, -0.101693, -0.00448795, 0.0257895, 0.0412368, -0.0102867, -0.0680753, 0.0812992, 0.0625099, 0.076849, 0.0103587, 0.0564469, 0.0627482, 0.122301, -0.077086, -0.0504602, -0.162819, -0.0558859, -0.0497218, -0.0639109, 0.0226961, -0.0614961, 0.0243218, -0.041697, -0.035216, -0.0704755, 0.0388414, -0.0537898, -0.150993, -0.0406924, -0.0857161, -0.056262, -0.0130077, -0.0352812, 0.0382183, -0.0282518, 0.0322973, 0.00661496, 0.0133834, 0.0528538, 0.0701653, 0.0248597, -0.100492, -0.111055, -0.00384645, -0.128731, -0.0577125, 0.0200472, -0.0134503, -0.0319114, 0.0304751, 0.0422808, -0.00226372, 0.00922317, 0.00808951, -0.00778285, -0.000805778, -0.170866, -0.0244633, -0.0675975, -0.159857, -0.0811565, -0.0971987, -0.157942, 0.00485824, -0.104583, -0.0321904, -0.0933037, -0.0495189, -0.0162932, -0.0393345, -0.0730279, -0.0864759, -0.013155, -0.0566388, -0.047936, -0.0699465, -0.0546591, -0.0313766, 0.038054, 0.0407225, -0.0213688, -0.00744403, -0.0130018, -0.0340084, -0.0249265, -0.0853294, -0.00189799, -0.0222038, -0.0374884, -0.0524022, 0.0245092, -0.10262, 0.0239804, 0.0131953, -0.0372583, 0.125746, 0.00934759, -0.0339645, 0.0268859, 0.0516404, -0.0883979, -0.00184576, 0.0644379, -0.0695657, -0.0391752, 0.0705435, 0.0241746, -0.137131, 0.0377747, 0.000870115, -0.0150322, 0.0992546, 0.0350409, 0.0775302, -0.0240628, 0.0293174, -0.0224079, 0.00637222, 0.0755274, 0.033329, -0.0508085, -0.0455766, 0.00725658, 0.0583272, -0.0452802, 0.048891, 0.0134342, -0.1078, -0.01559, -0.0244584, -0.000594553, 0.0487997, 0.0183376, 0.000231557, 0.00582778, 0.00366483, -0.000879677, 0.0401224, 0.0623847, -0.00789401, -0.0592274, -0.0410878, 0.0265822, 0.00863503, -0.0337465, -0.00978435, -0.0344017, 0.073783, -0.0165583, -0.01232, 0.0738734, 0.0158093, 0.0336536, -0.020143, -0.0831966, -0.0208092, 0.0172223, -0.0637394, 0.031275, -0.089459, -0.0564773, 0.0177903, 0.017113, -0.0316545, 0.0429466, -0.0396296, -0.0189497, -0.0450042, -0.00733181, -0.0769898, -0.0773341, 0.0405268, -0.0222872, 0.0151494, -0.0273485, 0.00628627, -0.0277951, 0.0137451, 0.00195327, -0.00664187, 0.0638023, 0.0661228, -0.029216, 0.00801995, 0.0431443, -0.0482818, 0.0219554, -0.0280278, 0.0343582, -0.00525515,
-0.0308303, 0.068071, -0.0777482, 0.0207558, -0.0617611, -0.00134699, 0.0200617, 0.0818675, 0.015234, -0.00788198, -0.023489, 0.000903645, 0.0182908, 0.0365243, 0.00404208, -0.0406543, 0.0360673, -0.0184067, 0.0732641, -0.00619725, -0.0218329, -0.0530814, -0.0456357, -0.00817067, 0.0673786, 0.0118674, 0.0258256, 0.0231953, -0.00195765, -0.0232075, -0.0416637, 0.0331728, -0.0600788, -0.0838774, -0.0358668, -0.00866718, -0.070623, -0.0345407, 0.096819, 0.0422059, 0.0714237, -0.0016716, 0.0564705, -0.0927094, 0.0950902, -0.056118, -0.0174444, 0.0671832, -0.0431297, 0.0171186, -0.0873967, -0.0329593, -0.0249064, -0.0515882, 0.00692676, 0.0323721, -0.0221865, 0.0469863, -0.0530182, -0.0469535, 0.0486555, -0.00482587, 0.0393105, -0.0237894, 0.0603762, 0.0359377, 0.0279304, 0.0245663, -0.0328259, -0.0115066, 0.0482217, -0.0698433, -0.0939935, 0.00396187, 0.101106, 0.0188476, 0.00500119, 0.00337426, -0.0490745, -0.00379407, 0.0213039, 0.065863, 0.0375466, 0.0883647, -0.0635882, 0.0083436, -0.107514, 0.017992, 0.048431, -0.0469271, -0.00433424, 0.0164904, 0.0111634, -0.067793, -0.0230292, 0.000719935, -0.0781065, 0.0354729, 0.00899735, -0.0745985, 0.114258, -0.0596753, 0.0650668, 0.0360615, 0.0515715, 0.0311496, -0.0420771, -0.0694927, -0.00376132, 0.047065, 0.0147552, -0.0171831, 0.0474061, 0.0778331, -0.0120362, 0.0716864, 0.0526351, 0.0250729, 0.0444239, 0.01084, -0.00708865, -0.123399, -0.0372806, -0.00789244, -0.0652007, 0.0315346, -0.00690674, 0.022825, -0.00914152, 0.0207389, 0.00823876, -0.0193416, 0.015174, -0.0144057, -0.0133325, 0.00641992, 0.119711, 0.00755291, -0.0321657, 0.00271962, 0.0242798, 0.000127385, -0.00908232, 0.049781, 0.0187365, -0.00380986, 0.126571, -0.0991188, -0.048895, 0.0143266, -0.00938673, -0.0771443, -0.0545482, -0.0413219, 0.0639277, -0.0605133, 0.00660005, -0.0679487, -0.0363268, 0.0110012, 0.0510538, 0.0374353, 0.0164986, -0.0524399, -0.0291112, -0.0673517, 0.0814767, -0.0606304, 0.00792011, 0.00400978, 0.03436, -0.0309509, 0.0634675, 0.00602465, 0.0213884, 0.00125428, 0.0457997, -0.069964, -0.0429681, 0.0110792, -0.0541839, -0.0439219, -0.0509355, -0.0762353, 0.0286139, -0.0559577, -0.00828542, 0.107613, -0.0602625, -0.0117594, -0.0673755, -0.0346589, 0.0257469, 0.0252385, 0.0213529, 0.0255798, 0.0438622, 0.0771512, 0.132489, -0.0107059, -0.0942431, -0.0101519, 0.0971957, -0.0258074, -0.0544977, -0.0259259, -0.0585075, -0.114201, -0.0566046, -0.125384, -0.0320364, 0.0327217, 0.0445314, 0.044981, 0.00332293, 0.0538358, -0.0224686, 0.045605, 0.0243823, -0.043783, 0.0766375, 0.0124259, 0.0310569, -0.0537706, -0.0491424, 0.0444035, 0.00689036, 0.0396576, -0.0134031, -0.0748105, -0.0249051, -0.0769767, -0.0627156, -0.123383, -0.131116, 0.0676192, 0.0641441, 0.0087839, 0.0123986, 0.0404309, 0.0170962, 0.122107, 0.0461117, -0.0180413, 0.0374784, 0.01126, 0.0943654, 0.0900352, 0.0668649, 0.129268, 0.0441728, 0.00269693, 0.00066259, -0.0476013, -0.0648971, -0.0173952, 0.0599402, 0.0107143, -0.0506566, -0.0856718, -0.0416031, -0.012123, 0.0239869, -0.0269473, -0.0111342, -0.00252438, 0.111859, -0.020297, 0.000262246, -0.000338527, 0.0512974, 0.0111128, 0.0824273, -0.0488234, 0.117013, 0.0632038, 0.121922, 0.0892267, -0.0679025, 0.0151747, 0.00583532, -0.00268069, 0.00285402, 0.0471444, -0.127768, 0.00787449, 0.0135074, 0.100917, -0.0127535, 0.0327849, 0.088979, 0.144497, 0.130676, 0.0935266, -0.0203765, 0.0216461, -0.0188431, -0.132588, -0.0270515, 0.0871459, 0.165454, 0.136635, 0.126222, -0.0111285, 0.0498148, 0.013933, -0.0471985, 0.0513527, 0.0340072, -0.0201845, -0.0130393, -0.032142, 0.0111429, -0.0180499, -0.0165284, 0.042366, 0.125693, 0.0630635, 0.0267393, -0.0279761, 0.0027273, 0.0429128, 0.043292, 0.00970059, -0.0829075, -0.0607651, 0.0561519, 0.196787, 0.135061, -0.00857867, 0.0462671, -0.00322218, 0.0044967, -0.0473595, -0.0101272, 0.0032265, -0.0749493, 0.0709941, -0.0414872, -0.00615857, -0.0146759, -0.0360449, -0.0289466, -0.00671384, 0.0288882, -0.0729546, -0.10405, -0.110264, -0.0877678, -0.0543744, 0.0629266, 0.09097, -0.0313563, 0.0205544, -0.0432879, 0.05408, -0.0703909, 0.0451012, -0.0613718, -0.0703088, 0.00951118, -0.0140439, -0.0152469, 0.0908396, -0.0520273, 0.0387321, 0.025868, -0.0126482, 0.0739755, -0.0455524, 0.0651194, 0.00784259, -0.123714, -0.0414656, -0.0951165, -0.0379367, -0.117724, -0.0616441, 0.0179126, 0.0199172, 0.00966941, 0.0669875, 0.0415294, -0.0430174, 0.0120244, -0.0504315, -0.0482694, -0.0546284, 0.0124578, 0.0274505, 0.0343064, 0.0121842, -0.0454786, -0.0166632, -0.00710407, -0.0929176, -0.0379167, -0.0142637, 0.0603296, 0.0320486, -0.0568405, -0.0935878, -0.07482, -0.0716899, -0.0227465, -0.0117664, -0.0921018, 0.0316238, -0.0259375, -0.0493818, -0.0444445, -0.0885901, -0.0365461, -0.200096, -0.0564026, -0.0579557, -0.0698191, 0.00449295, -0.0169403, 0.0201515, -0.0516388, -0.00555655, -0.0950468, 0.0187391, -0.0221967, 0.0948128, 0.0710857, 0.116797, 0.0537579, 0.0250063, 0.0239571, 0.0497818, -0.0405567, -0.0190201, -0.0405005, -0.0677047, 0.0587315, 0.00741866, -0.00305112, 0.0500538, -0.00931538, -0.12974, -0.0755891, -0.00350722, -0.0389089, 0.0118889, 0.0128496, 0.0770896, -0.0446585, 0.0152981, 0.0617753, 0.126964, -0.03947, 0.0758139, -0.0331793, 0.00601315, 0.0205851, -0.0815581, -0.0275944, -0.0783522, 0.00690115, -0.0738089, -0.0971355, 0.0121746, 0.000684315, 0.0204366, -0.00870224, -0.0244568, 0.120364, -0.0282118, -0.00784584, -0.120642, -0.0586479, -0.0676362, 0.0405712, 0.0370398, 0.041088, 0.0304557, 0.0696093, -0.0281734, 0.0326859, -0.0296138, 0.0222828, -0.0250959, -0.0176965, 0.0249119, 0.0416055, 0.0311955, -0.040914, 0.00245453, -0.00564506, -0.00113698, 0.0103693, -0.0282925, -0.0878719, -0.136533, 0.0470575, -0.0451514, -0.129246, 0.0335387, -0.0335048, -0.00014923, 0.0273423, -0.0147083, -0.094239, 0.0585049, -0.0655223, 0.00107721, -0.0359934, -0.0176847, 0.103709, 0.0722966, 0.058903, 0.0126067, -0.0142828, 0.059751, 0.048281, 0.00352501, 0.0284362, 0.0552647, 0.0768463, -0.016722, 0.00890961, -9.67071e-05, -0.0176544, -0.147151, -0.0410244, -0.0368881, -0.0106952, -0.0348736, 0.0461372, 0.0528957, 0.0551163, 0.0604849, -0.0188284, -0.0576848, 0.0292781, 0.0899326, -0.00971244, 0.0168291, 0.0472455, -0.015742, 0.00970794, 0.0801317, 0.0999626, 0.0450553, 0.0623551, 0.106777, 0.0372635, -0.0113272, 0.00186087, -0.0130444, -0.078059, -0.0850751, -0.116835, -0.108249, 0.0339255, 0.0326177, -0.123175, 0.0552796, 0.0142773, -0.0178458, 0.0650476, 0.0115366, -0.097545, 0.0365068, 0.00850008, -0.0826102, -0.0511757, 0.00683736, 0.0723595, -0.0347752, -0.0883653, 0.0393991, 0.105867, -0.00575979, 0.0832864, -0.00133637, -0.0369962, -0.057684, 0.0232005, -0.0718121, 0.0199153, -0.0209627, -0.0978309, 0.0317392, 0.0340876, -0.0649211, -0.0133172, 0.0209716, 0.101545, 0.084813, 0.018591, 0.109126, -0.00398094, -0.0886524, 0.0309733, -0.0168217, -0.0315799, 0.010711, -0.0217205, 0.0548691, 0.00531591, -0.0162572, 0.152966, 0.0726432, 0.141789, -0.0118512, -0.00688838, 0.00786022, 0.0938637, 0.059542, -0.0578218, -0.000476541, -0.0893226, -0.0280353, -0.0231469, 0.0201558, 0.0432722, 0.0346079, -0.055686, -0.115407, -0.0286941, -0.122041, -0.0345431, -0.0344716, -0.104218, -0.0941244, -0.0934923, -0.0690128, -0.00843877, -0.0872131, -0.0165773, 0.00965409, 0.0823975, -0.0221046, -0.0116821, -0.0527317, -0.00903063, 0.0838759, -0.00657982, -0.0182043, 0.0695814, 0.0818187, -0.102738, -0.0254998, -0.0131764, 0.004162, 0.00729117, 0.0473752, 0.0673466, -0.108514, -0.08756, -0.157992, -0.116829, 0.0911386, -0.00657407, 0.0419432, 0.0177432, -0.00328093, 0.0175591, 0.0771627, 0.0277121, 0.0158637, 0.0616679, 0.0273831, -0.00327444, 0.0810173, 0.0759832, -0.0463759, 0.0390619, 0.0988671, -0.108633, -0.0387644, -0.0626774, -0.00694655, 0.0706845, 0.0824402, -0.00168174, -0.111466, 0.00824285, -0.0200969, 0.0136527, 0.0396957, 0.0113423, -0.0932312, -0.0405487, 0.00269112, 0.0165218, 0.0619531, 0.0293151, 0.0319192, 0.0295742, -0.041711, -0.0138392, 0.0551811, 0.0114768, -0.00625663, 0.00792845, -0.0327745, 0.0292749, -0.00944456, -0.00282744, 0.0234543, 0.0515957, 0.0137917, 0.023355, 0.0336353, 0.0132813, 0.0318027, 0.105918, 0.0139628, 0.083703, -0.000753181, 0.0471997, -0.0262378, 0.1145, -0.0401905, 0.00124601, -0.0214565, 0.046007, -0.0271544, -0.0572013, 0.0282306, -0.00509438, 0.0720601, -0.115137, 0.0148995, 0.040179, 0.0226634, -0.0335562, -0.104056, -0.0371744, -0.00204665, -0.0473687, 0.0312119, -0.0240433, -0.0463318, -3.83118e-06, -0.0328934, -0.0535928, 0.0533934, 0.0288348, 0.108829, -0.0168143, 0.0513287, -0.0247255, -0.0182302, -0.0012449, 0.0352402, -0.0252595, 0.0163837, 0.0246931, -0.0741455, 0.00077429, -0.0143664, 0.0210769, -0.0280251, 0.00575299, -0.0357361, -0.0239107, -0.0211134, 0.0467461, 0.0663627, -0.0240256, -0.0524685, 0.0618759, -0.021917, 0.0598651, -0.0159886, -0.0292742, -0.0149138, 0.0353757, 0.0807124, 0.0107413, -0.0249299, -0.135245, 0.0183598, 0.053897, 0.0109147, 0.0949182, -0.000183719, 0.044135, 0.0715714,
-0.0242981, -0.0113286, -0.046564, -0.0188999, 0.0337332, -0.0102734, 0.0548082, -0.00308677, 0.0688842, 0.0141908, 0.00336216, 0.0446604, 0.0583698, -0.00373496, -0.0131683, 0.0240139, 0.011217, 0.0386501, 0.0502751, 0.0628343, -0.0530952, 0.0430317, 0.0538658, 0.068803, 0.0437664, -0.0294231, -0.0185773, 0.155447, 0.0830977, -0.0407403, 0.0171773, 0.0336478, -0.0327446, -0.0241731, -0.010513, -0.121719, 0.0813714, -0.122905, 0.0615275, -0.0174405, 0.00661263, -0.0172246, 0.0231419, -0.0231884, 0.00177983, -0.0383306, 0.0187806, 0.00731902, -0.0411322, 0.00487085, 0.00981994, -0.00346271, -0.0130334, 0.030341, 0.00738709, 0.0513526, -0.0195029, -0.0192482, 0.0401314, 0.0422783, -0.00851074, 0.0122774, -0.0475495, -0.0244255, 0.00203846, 0.0201449, 0.0878322, -0.0313615, -0.087064, -0.0908485, -0.047492, -0.0726468, -0.0252802, -0.0205673, -0.0835246, 0.00550131, -0.0707818, 0.0160477, 0.047037, 0.0685161, 0.0315783, 0.0564625, 0.0833588, 0.0395999, -0.0298682, -0.0280324, -0.0784672, 0.0367915, 0.0419742, 0.0279117, -0.033506, -0.0364553, -0.0296312, -0.113872, -0.199921, -0.163984, -0.0811175, -0.0644974, -0.104673, -0.038134, -0.0107617, 0.0516766, -0.0152572, 0.053283, -0.0286204, 0.0151827, 0.0576887, 0.0161335, 0.00962067, 0.0871555, 0.00831388, 0.0195161, 0.0705456, 0.00725594, -0.0360267, -0.0393698, -0.01674, 0.0211168, 0.0305465, -0.00970072, -0.0803287, -0.182569, -0.0558827, -0.0815643, -0.0941075, -0.133643, -0.0913193, 0.000634132, -0.0681958, 0.0491492, -0.0566422, -0.0295107, -0.0642456, -0.101896, -0.0314249, -0.0373423, 0.0164807, 0.00812195, -0.0134593, -0.0411788, 0.0393829, 0.040108, -0.00899287, -0.0166421, 0.0753479, -0.0361225, -0.0188514, -0.0855027, -0.0108218, -0.131926, -0.160541, -0.0655634, -0.0252853, -0.0850567, -0.100865, 0.00994863, -0.0744368, -0.109046, -0.157695, -0.0503905, -0.112821, -0.191009, -0.162735, -0.0138776, -0.0396406, 0.0844006, -0.068033, 0.0797823, -0.0806059, 0.0432065, 0.0410568, -0.0909608, 0.0120297, 0.0608935, 0.10806, 0.0341567, 0.0263253, 0.160832, 0.153319, 0.0584147, 0.0125331, -0.0302651, 0.0442367, -0.0277751, 0.0671848, 0.0530218, -0.0748271, 0.0753354, 0.119528, 0.109188, -0.0108287, 0.0202209, -0.0106402, -0.0426433, -0.00793774, 0.0578669, -0.0173563, 0.0210552, 0.0054524, 0.00542655, 0.0608872, 0.0409763, 0.0966801, 0.0925889, 0.0716134, 0.146275, 0.141028, -0.0236555, 0.180374, 0.222716, 0.0529261, 0.10043, 0.0339908, 0.0732941, 0.0971663, 0.00806899, 0.0466939, -0.066946, -0.0511271, 0.0193778, 0.12438, 0.0374153, -0.0798502, -0.0562363, 0.16146, 0.00323026, 0.0495222, -0.0414686, 0.0303257, 0.041655, 0.0969646, 0.0558926, 0.0829276, 0.00589142, 0.151933, 0.0645445, 0.147358, 0.0953215, 0.126945, 0.124182, 0.234596, 0.222376, 0.187343, 0.0777991, 0.122559, 0.0507954, 0.0136774, 0.10789, 0.0459339, 0.040373, -0.00670172, 0.0598842, -0.0795948, 0.0344459, -0.0334136, -0.0215834, 0.11287, -0.0204476, 0.158693, 0.0471791, 0.0577415, 0.0257115, 0.0465846, 0.10941, 0.141165, 0.200749, 0.161886, 0.152685, 0.211107, 0.189366, 0.111199, -0.0257546, 0.0812066, 0.0255501, 0.152717, 0.176608, 0.0436005, -0.0401154, 0.0159773, -0.00621578, -0.0230516, 0.0398384, 0.0221453, -0.0454787, 0.0613615, 0.102906, 0.00777109, 0.0115584, -0.00828579, -0.0284881, 0.110934, 0.0004397, -0.000821278, 0.00929855, 0.0643447, 0.0359186, 0.123967, -0.0114045, 0.025103, -0.0229632, 0.119064, -0.0178509, 0.156367, 0.104634, 0.0829811, 0.0925515, 0.14652, 0.0470252, -0.00746228, -0.0375569, 0.0165314, -0.0333303, 0.102772, 0.102154, 0.130455, -0.0592355, 0.0572258, -0.0830512, -0.0629331, 0.0461399, -0.110427, 0.0277511, -0.114935, -0.00831483, -0.123847, -0.0152937, -0.0332641, -0.0531527, 0.00199711, 0.0102997, 0.0541772, 0.0333709, 0.104225, 0.0303411, -0.0139419, -0.0162344, 0.0625397, 0.0085667, -0.0308449, -0.00644809, 0.01823, 0.0745201, 0.0638613, 0.0745, 0.0555851, -0.0285257, 0.00581834, -0.0999525, -0.00981421, -0.116372, -0.123197, -0.0686895, -0.106329, -0.00989866, 0.091874, -0.0103236, -0.0525143, -0.0310074, -0.0172066, 0.076526, 0.0223485, 0.121589, 0.0672691, -0.0329723, -0.095114, -0.0122504, -0.0652052, 0.0423691, 0.0813383, -0.0629205, -0.0340823, 0.0469347, 0.0354178, -0.113649, -0.0301848, -0.00588277, -0.103747, -0.0261488, 0.0755971, -0.0761928, -0.068208, 0.0335612, 0.0498122, 0.0501884, 0.0238114, 0.03069, -0.136095, -0.0609315, 0.0322115, -0.0725889, -0.0022121, 0.0112191, 0.0405533, -0.0256742, 0.0839213, -0.00434909, -0.0149734, -0.0736032, -0.0323344, -0.0515556, 0.0397334, -0.0457667, 0.0704689, 0.00219164, 0.00953772, 0.0194569, -0.0418829, 0.0917585, -0.0307013, 0.0758132, 0.0323868, 0.0290105, -0.00565947, 0.057258, 0.00342852, -0.0386736, -0.105385, -0.00179376, 0.0250876, 0.026175, 0.0189499, -0.0423433, 0.0371201, -0.0128021, -0.0312549, 0.0371126, -0.0654457, -0.0282215, 0.0119089, 0.0329178, 0.0624095, 0.00862229, -0.0247715, -0.0150797, 0.00540053, -0.0253129, -0.0891562, 0.0613896, 0.0502434, -0.0135006, 0.141253, -0.000384363, -0.0197143, -0.0410171, -0.121226, -0.0476034, -0.00521093, -0.064153, -0.0587046, -0.0245288, 0.00421778, -0.0190668, -0.0709084, -0.105494, -0.0535399, 0.0186221, 0.11606, 0.0715825, 0.107571, 0.0021048, 0.125654, 0.0698918, 0.0116071, -0.133037, -0.0290212, 0.0699897, 0.0530323, 0.0352453, -0.00313615, -0.0303898, -0.0016236, -0.0477431, -0.0514325, -0.0162101, 0.0555658, -0.00774317, 0.0571567, -0.0631649, 0.00889694, -0.0377507, -0.0352708, -0.0255439, -0.159022, -0.0034021, -0.00612376, 0.103796, -0.101064, -0.0476812, -0.00667836, 0.0907629, -0.0452997, -0.0512689, -0.0435076, 0.0852987, 0.0534325, -0.0315376, 0.028039, -0.0112322, 0.0586378, -0.060177, 0.0826567, 0.0360695, -0.0740664, 0.0760994, -0.0101323, 0.0326193, 0.0502235, -0.0192434, -0.0733966, -0.125192, -0.0154557, -0.0649097, -0.0867222, -0.035922, -0.0609803, -0.0441914, -0.00566273, 0.0326508, -0.0413943, -0.103919, -0.0896126, 0.098152, -0.0528151, -0.0167695, -0.0726979, -0.0161175, 0.0687854, -0.113303, -0.0594583, 0.0184951, -0.00784787, -0.0446736, -0.00121361, 0.0266574, 0.0494876, -0.0384021, -0.0620947, -0.112417, -0.026425, 0.0829342, -0.0307537, -0.0251517, -0.0909558, -0.167642, -0.113978, -0.0360652, -0.0586647, -0.0576496, -0.0161231, 0.0380752, 0.0602464, 0.057512, 0.0530578, 0.0265776, 0.01458, -0.134228, -0.0654773, -0.00827992, 0.024569, 0.044423, 0.0253142, -0.0349013, -0.0529132, -0.0524572, -0.117733, -0.0741582, 0.0108187, -0.0111219, -0.0523031, -0.10967, -0.102071, -0.163923, -0.202449, 0.0059314, 0.0168731, 0.0734636, 0.00821791, 0.0731128, -0.0740258, 0.00531622, 0.00296763, 0.0466545, -0.031015, -0.0841157, 0.00499291, 0.00256908, 0.00821939, 0.00356882, 0.0569331, -0.00833272, -0.0298289, -0.041331, -0.0465679, -0.0178084, -0.00598553, 0.0119036, -0.159121, -0.165545, -0.154581, -0.114966, -0.0747587, -0.0215134, -0.0174042, -0.0398347, -0.0416558, 0.0662997, 0.0978754, 0.0412304, 0.0324363, -0.029541, -0.0575245, -0.0677371, -0.029261, 0.12469, -0.0428829, -0.0286093, -0.0429431, -0.0874781, -0.0623196, -0.050651, -0.125285, -0.103282, 0.00608428, -0.0226923, -0.0827285, -0.10731, -0.151665, -0.0800718, -0.0988466, 0.00275524, 0.0751672, 0.0160995, -0.0302012, 0.0279227, 0.070864, -0.0222188, -0.0770382, 0.018033, 0.0237207, 0.0195989, -0.0263358, 0.0351115, -0.0863805, 0.0624621, -0.0129559, -0.0509162, -0.021709, -0.040766, 0.0565075, 0.106682, 0.0387418, 0.0653475, -0.0443837, -0.011774, 0.00174729, -0.0361368, -0.0294237, -0.0668966, -0.0792447, 0.0355888, 0.0484325, -0.0124026, 0.0206151, -0.160633, -0.0351212, -0.0149365, -0.016957, -0.0768814, 0.0297305, -0.0396604, -0.0416586, -0.0545926, 0.0062391, 0.0171323, 0.0128097, 0.0388884, 0.148689, 0.0954278, 0.169329, 0.174558, 0.0229387, -0.0443502, 0.133846, -0.0394272, 0.125455, 0.166386, 0.0966639, 0.0982055, 0.059812, 0.0374862, -0.0285842, 0.000989624, 0.00378146, 0.0661544, 0.0313858, 0.0770335, -0.0893723, -0.0875715, -0.0597351, -0.00245036, 0.0399071, 0.0271238, -0.0365439, 0.0351362, -0.0213554, -0.00751581, 0.0312481, 0.0876589, 0.108567, 0.0673369, 0.238869, 0.0374997, 0.125416, 0.178122, 0.0628557, 0.0717504, 0.14512, 0.0373108, 0.0431625, 0.124405, 0.128226, -0.0214964, 0.052015, 0.00771627, -0.000961992, -0.085375, -0.0289462, 0.00692733, 0.0476574, -0.109243, -0.0423146, -0.0167627, -0.020709, 0.0215694, -0.0513382, 0.0337652, -0.016082, -0.0564177, 0.0780211, 0.163339, 0.00669126, 0.0748762, 0.112566, 0.0182689, 0.0730648, 0.133765, 0.00363343, 0.0432714, 0.0231968, -0.028572, 0.0426501, -0.0225309, 0.0916888, -0.062703, 0.0115751, 0.0103206, 0.0684317, 0.0384239, 0.0522475, 0.0287641, 0.0229044, -0.0471094, -0.0959283, 0.0261014, 0.0286904, 0.0426483, 0.0738449, -0.0361922, -0.0248867, -0.08746, 0.112029, -0.0402928, -0.0135812, -0.017437, 0.0960539, -0.00157149, 0.0329002, 0.041828, -0.0346741, -0.00364768, 0.025796, -0.0135054, -0.0165601, -0.0574817,
-0.0280422, 0.089794, -0.00273061, -0.0429845, 0.0343935, -0.139157, 0.0373895, -0.035002, -0.0674233, -0.0285248, 0.0676976, 0.0177135, -0.00403593, -0.00607669, -0.0658739, -0.0711619, -0.0325813, -0.0142377, -0.0729076, 0.091516, -0.0153766, 0.0437989, -0.0384535, 0.0704231, -0.00964085, -0.0106629, -0.0185314, 0.11084, -0.02044, 0.0816713, 0.0316018, 0.0277449, -0.0379089, 0.0434147, 0.031477, 0.0152214, 0.0425886, 0.0348216, -0.0216803, 0.0275375, 0.0453258, -0.0745174, 0.0504499, 0.0553695, -0.00532209, -0.119943, 0.0174102, -0.0146938, 0.00383259, -0.0686156, -0.0170846, 0.0405586, 0.0515613, -0.0329442, 0.0835311, 0.0516886, -0.0518655, 0.0138324, 0.0162984, -0.033747, 0.0449931, -0.0180961, -0.00478464, -0.142328, -0.00284058, 0.0171289, 0.0663081, -0.0176863, -0.0561839, -0.0716255, -0.0104617, -0.0286078, -0.0272504, 0.00215898, -0.00987405, 0.103313, -0.113104, -0.0789001, -0.0141567, -0.0651579, 0.0290923, 0.0010133, -0.0546158, 0.048959, -0.0363907, 0.0137049, -0.0101642, 0.0408039, -0.0233033, -0.00450913, -0.12971, -0.065919, 0.0296941, -0.075301, -0.110296, -0.108711, -0.00852918, -0.0943943, 0.0655717, -0.0601452, -0.00572519, 0.0955596, 0.0125753, 0.0424701, 0.0484898, -0.00176102, -0.0598032, 0.0637681, -0.0363756, -0.019431, -0.0345399, -0.0146406, -0.000685208, 0.10181, 0.0348449, -0.0346017, -0.0176854, -0.0514348, -0.0157116, 0.0561344, -0.0280768, -0.0598002, -0.115775, -0.0512136, 0.0903469, -0.0130616, 0.0170898, 0.00152972, -0.0677431, -0.0601607, -0.0707823, -0.0137706, 0.00198278, -0.0415735, -0.0592967, -0.010695, -0.000974876, 0.0611442, -0.0453051, 0.0479756, -0.00622644, -0.128679, -0.0602506, 0.00202361, -0.0322351, 0.0815295, -0.0303279, 0.0105105, -0.0232306, 0.00104942, -0.0203644, 0.0028031, 0.0445414, 0.097758, 0.0669, 0.0879309, -0.0129027, 0.117025, 0.00222051, -0.0987275, -0.0404684, -0.0332423, 0.0694182, -0.145852, -0.0334785, -0.0470842, 0.1026, -0.053267, -0.0558637, 0.0173725, 0.0498436, 0.0106526, -0.143401, -0.0608812, 0.108542, 0.0777696, -0.00592608, -0.0481909, 0.0754515, 0.0734244, -0.0183433, -0.0579503, -0.114747, -0.0298043, -0.0140585, -0.0460578, -0.045348, 0.0425044, 0.0504746, 0.0302902, -0.00827852, -0.0197679, 0.0406508, -0.0939088, -0.0559597, 0.001891, 0.00743525, 0.0629058, -0.0569534, -0.0218091, 0.0853737, -0.114114, 0.06386, -0.0227868, -0.00166096, 0.0999418, -0.0411835, -0.100564, -0.0607658, -0.138024, -0.169068, -0.0585319, -0.0599193, 0.0559918, 0.0358981, 0.00252073, 0.0873717, -0.0362082, 0.0783901, 0.117282, 0.0520069, 0.0212299, -0.0188779, 0.0379932, -0.0731325, 0.0172281, 0.015667, -0.0278557, 0.0355173, 0.0273056, -0.000104231, -0.032346, 0.0053765, -0.0127678, 0.0561221, -0.0259018, -0.0167883, -0.107503, -0.0481186, 0.0361061, 0.0852131, 0.078393, 0.173062, 0.0430037, -0.0150667, 0.0939083, 0.0487925, 0.0518189, 0.0218416, 0.0869872, 0.0832264, 0.0487698, -0.0144368, 0.019764, -0.0592895, 0.017389, 0.0436139, -0.0199386, 0.0276534, -0.0301271, 0.00148066, 0.0683814, -0.00300811, 0.104885, 0.0242077, 0.0122831, 0.0264214, 0.094826, 0.139083, 0.127175, 0.0721765, 0.119536, 0.0882286, 0.0654366, 0.103091, 0.0630514, 0.10184, 0.169745, 0.0267033, -0.0296422, 0.0225097, -0.0980671, 0.0622289, 0.0279111, 0.0940541, -0.0606814, 0.00192213, 0.0410127, 0.0560166, 0.0583394, 0.113192, 0.0844799, -0.00376484, 0.100662, 0.207549, 0.126231, 0.0556609, 0.0935471, 0.106553, 0.0259457, 0.108534, 0.00626247, 0.0396365, 0.0810791, 0.0673043, 0.0997592, 0.0187034, -0.0584338, -0.00623485, 0.00719856, 0.0496933, 0.0604698, 0.109548, -0.104433, -0.0324759, 0.0193971, 0.132322, 0.159775, 0.0310452, 0.0532389, 0.0985367, 0.211463, 0.210418, 0.181215, 0.118083, 0.0468756, 0.0219478, -0.00766366, 0.110377, 0.0892167, -0.0210149, 0.0550728, 0.115491, 0.0167236, 0.0637624, 0.00539681, 0.0302398, 0.148402, 0.0690567, -0.028713, 0.0529568, -0.0464246, 0.0816584, 0.0345063, 0.0583001, 0.0295504, -0.0161943, 0.129902, 0.0230948, 0.194483, 0.158385, 0.0492687, -0.0295406, -0.0266943, -0.0120032, 0.106445, 0.0120939, -0.0784026, 0.0715071, 0.0211235, -0.00133133, 0.0408907, 0.0614097, -0.0180515, -0.0346751, -0.00185936, -0.0394394, -0.0124189, 0.0125305, -0.00744675, -0.00856555, 0.0352132, -0.00392488, 0.0263474, -0.0610768, -0.0327155, 0.0587993, 0.053211, 0.121346, 0.00230495, 0.0645619, -0.079426, 0.0435066, 0.0785226, -0.0748193, 0.00574098, 0.00930659, 0.0355639, -0.0578593, 0.0181651, -0.000621248, -0.0398276, -0.0496238, -0.025468, 0.0408858, -0.0280807, -0.0799601, 0.0874368, -0.115879, 0.000208085, -0.10691, -0.0539841, 0.0599077, 0.0603353, 0.10497, 0.0845115, -0.0911613, -0.0458761, -0.164827, 0.00647727, 0.106015, 0.0453716, 0.0251663, -0.0730479, -0.000584859, -0.04731, -0.00108299, 0.013846, -0.0457282, -0.0409704, 0.0222682, -0.060615, -0.0132598, -0.0244812, -0.00385428, -0.00744674, 0.0150352, 0.0246495, -0.0183134, 0.0263494, 0.0490528, 0.0338417, 0.0150472, -0.0435375, 0.0174899, 0.0568139, -0.0386081, 0.0646976, 0.179791, 0.0224439, 0.0551522, -0.02716, -0.0110377, 0.0396641, -0.0282387, -0.129874, -0.0516622, -0.0580436, 0.0119324, 0.0469544, 0.0484835, 0.0084445, -0.0209594, -0.0884621, -0.0021347, 0.0480835, 0.0310779, 0.0706133, 0.02237, -0.117821, -0.0242765, -0.0131035, -0.0451653, 0.0603552, -0.0998082, 0.0560194, 0.0680982, 0.0712272, 0.0977799, -0.0424799, -0.118191, -0.0992812, 0.104068, -0.0261881, -0.0122653, -0.0586259, 0.0385093, -0.0628276, 0.0570426, -0.0393428, -0.0271593, 0.0350636, -0.054439, -0.0172413, -0.0121065, 0.0084981, -0.0880049, -0.0116732, 0.044545, -0.0230245, -0.0632567, 0.0180363, 0.181031, 0.0634141, 0.00693763, 0.000931583, -0.0165037, -0.019043, -0.0868228, -0.0897111, -0.030253, -0.0838323, -0.0523398, -0.00154121, -0.00985366, 0.07101, -0.000649744, -0.0572075, -0.000782783, 0.109533, 0.0157496, 0.00595277, 0.0302398, -0.0997863, -0.053159, 0.0534354, -5.126e-06, -0.0366631, -0.0507853, 0.0551688, 0.0857094, 0.0871492, -0.0219051, -0.0789218, -0.0812895, -0.12682, -0.103174, -0.0222886, -0.0444735, -0.0132878, 0.0336787, 0.00684993, 0.0616561, -0.0250235, 0.0493859, 0.0121933, -0.0249847, -0.0267479, 0.00178714, 0.0621605, -0.0704879, -0.0691237, -0.0205151, -0.049795, 0.0482628, -0.0297049, 0.0339776, 0.0271444, 0.00158296, -0.0177787, -0.106793, 0.00917037, -0.127418, -0.168683, -0.12141, -0.0322184, 0.00227068, -0.0197463, -0.0436099, -0.112995, 0.0100076, 0.0256534, -0.0594353, 0.0402228, 0.0126162, 0.0408624, 0.105789, -0.0147473, 0.00724177, -0.037839, 0.0236255, -0.0312324, -0.0118086, 0.0936724, -0.0799732, -0.082616, -0.00724287, -0.146404, -0.187044, -0.140394, -0.165785, -0.170576, -0.145125, -0.0495884, -0.0601422, -0.0753102, 0.000785051, 0.0243611, -0.0618248, 0.0592202, -0.0173952, -0.136333, 0.0564134, 0.0510489, 0.0540972, 0.0409291, 0.0531079, 0.0367762, 0.0672414, 0.0619071, 0.0287023, -0.0389604, -0.101571, -0.0516474, -0.0571064, -0.155204, -0.0893754, -0.185437, -0.114544, -0.081281, -0.0686147, -0.143661, -9.9897e-05, -0.0567569, -0.00673377, 0.00185691, -0.0066442, -0.0094036, 0.0662943, -0.0374024, -0.0428257, -0.00212753, -0.0355256, 0.0701971, 0.0177906, 0.0504189, 0.0244142, 0.107028, 0.163627, -0.00551317, 0.0943349, 0.0297268, -0.0652183, -0.0995952, -0.0912181, -0.106764, -0.142799, -0.0360158, -0.0568537, -0.0242021, -0.0587609, -0.00700046, -0.0371148, -0.0127878, -0.0208163, -0.00503677, -0.0152519, 0.000623384, 0.0215309, -0.0205422, -0.028376, -0.0480064, -0.048668, -0.0476379, 0.00485632, -0.0248116, -0.00436323, 0.0270831, 0.0533274, 0.148319, 0.0540443, -0.0586449, -0.0551487, -0.198297, -0.176612, -0.0921633, -0.0546562, 0.0209979, 0.0685854, 0.0506489, -0.0476542, -0.0690318, -0.00702824, -0.0356159, -0.0643226, 0.0578156, -0.0573405, -0.0217882, 0.0764206, -0.0240331, -0.00203993, -0.0308344, 0.0586184, -0.0120917, 0.0752772, 0.0750486, 0.0401956, -0.00361746, 0.0748604, -0.0662872, -0.119941, -0.119124, -0.066866, -0.0903235, 0.0482682, -0.013362, -0.0178514, 0.0140476, 0.0870256, -0.00445605, 0.0683271, -0.0280508, -0.0535953, 0.0254053, -0.0283275, -0.0448923, 0.0459019, -0.0306628, -0.0831645, -0.0166127, 0.0744107, 0.119889, 0.0363178, 0.0190869, 0.0298855, 0.0770742, 0.0377252, 0.0321076, -0.0564911, -0.0266366, 0.0242292, 0.0765845, -0.0128045, -0.0893747, -0.0092172, -0.0126151, 0.0110832, -0.105481, 0.0167709, 0.0539474, 0.0106391, -0.0991771, 0.0405499, -0.0945369, 0.0193568, 0.0915907, 0.0234273, 0.0862945, -0.0216516, -0.0763681, 0.0491101, 0.076527, 0.0314967, 0.0351054, 0.0989032, 0.0931487, -0.0128516, 0.12766, -0.115906, 0.0698878, 0.0107257, -0.00744443, 0.00887008, -0.0179544, 0.0367401, 0.0809304, -0.0198731, -0.0777006, 0.00290937, -0.00280367, -0.062466, -0.0244113, 0.00522845, 0.0469361, 0.0467481, 0.0519393, 0.115238, 0.0703669, -0.0665291, 0.00308414, -0.0307002, -0.0676431, -0.0528294, -0.0185131, 0.0404887, 0.129067, -0.00582558, 0.070416, 0.0775541, 0.00332943, 0.00858577, 0.0172168, 0.0464837, 0.023529,
0.136064, -0.0487065, -0.026267, 0.0730331, -0.0565769, 0.0132047, -0.00738423, 0.0126916, -0.0883195, 0.00589892, -0.00344174, -0.0843665, 0.00879859, -0.049241, -0.0210534, -0.0186555, -0.0401159, -0.0174607, 0.0100369, 0.0532124, -0.033278, 0.0388785, 0.0848123, -0.0250846, 0.159614, 0.0436579, -0.043976, 0.0504713, -0.0542285, -0.0108105, 0.0107129, 0.00250971, 0.015534, -0.0589822, 0.0056111, -0.0503545, 0.0102631, -0.0271153, -0.0504695, -0.0496094, 0.00859749, 0.0562616, 0.0165618, -0.0318264, 0.00100237, 0.0260107, 0.0615564, 0.0259513, -0.0403671, 0.00736671, 0.0369817, 0.0328744, -0.0672257, 0.00870114, 0.0289486, -0.0512719, -0.0216811, 0.0804906, 0.0409616, -0.019117, -0.00532784, 0.0259891, 0.00649757, -0.10397, 0.0366162, -0.0356513, 0.0422524, 0.0455008, 0.0170211, -0.0579185, 0.031217, -0.0389371, -0.00349673, 0.048841, 0.0380245, 0.0114724, -0.00372, -0.0208334, -0.0606537, 0.0349958, -0.00327143, -0.0862102, -0.0380057, 0.0746666, 0.0235281, 0.0445752, 0.00825728, -0.0541747, 0.0127809, 0.0039627, 0.0884785, 0.019468, 0.0830426, -0.0831303, 0.0212513, -0.0936347, 0.0273892, 0.0310448, -0.0426692, -0.0361124, 0.0159383, 0.0273287, -0.00438587, 0.040073, 0.0957146, 0.00365037, 0.0443293, 0.0258258, -0.0799055, 0.0537014, 0.0553125, -0.0104379, 0.0739636, 0.0213173, 0.0824542, -0.0116504, -0.0343408, 0.0894106, 0.091227, 0.0722813, 0.112155, 0.0106424, -0.0445523, 0.00991654, -0.0698353, 0.0158104, -0.0135838, 0.00451788, 0.0210656, -0.0229123, 0.0842215, 0.0439782, 0.047424, -0.0157587, 0.0111162, -0.0620479, 0.0362138, -0.00443017, -0.0252759, 0.0736855, 0.0243892, 0.0211521, 0.083864, -0.0869582, -0.0300578, -0.0252501, 0.0992607, -0.0678502, -0.12097, -0.00838622, 0.0550905, 0.0976034, -0.0316802, 0.0955806, 0.0680404, 0.0267237, 0.0476499, 0.00912027, 0.0278135, 0.0390762, -0.0406189, 0.0454963, 0.110744, -0.0955113, -0.0734278, -0.010327, 0.0448352, -0.0466308, -0.0709845, 0.0641127, -0.123, 0.0393713, -0.0285971, -0.0502388, 0.023524, -0.0635269, 0.0622222, -0.0934553, -0.0440585, -0.0249988, 0.00257707, 0.0368095, -0.128384, -0.0678228, -0.0436747, -0.0343409, -0.0637933, -0.0631271, -0.084744, -0.0876734, 0.0154949, -0.0961256, -0.0174357, -0.0295211, -0.0483532, -0.00321505, -0.0287965, -0.0536502, 0.0629532, -0.039094, 0.0331386, 0.0111976, 0.0461829, 0.0361602, -0.117074, -0.0312986, -0.0395684, 0.0052636, 0.0188197, -0.0420207, -0.0677524, -0.0057752, -0.0721129, -0.0277044, -0.137194, -0.0270964, -0.00868715, -0.17188, -0.0734838, 0.0420608, 0.00676305, -0.0313535, 0.0761798, 0.0615289, -0.00908698, 0.0419566, -0.0543054, -0.0318261, -0.0145361, 0.0386901, -0.0332494, 0.0272889, -0.000414984, -0.0638041, -0.0345284, -0.054197, -0.0528494, -0.000702508, -0.0953699, -0.0296825, -0.0339934, -0.0838996, -0.00576765, -0.0645608, -0.120649, -0.0987306, -0.0549344, -0.017983, 0.0508136, -0.0730356, -0.00191689, 0.0143031, -0.0365105, 0.000966295, -0.0380958, 0.0364091, -0.039143, -0.0390299, -0.0877879, -0.0884848, -0.0501949, -0.0657479, -0.0950542, -0.0627876, -0.0794432, -0.0358159, -0.090169, -0.0689453, -0.0772816, -0.0774388, -0.101254, 0.0363421, -0.113577, 0.0105556, -0.0235992, -0.0577287, 0.0394447, -0.0364256, 0.0126439, 0.00432673, 0.10392, 0.0359212, 0.0548278, -0.0435441, 0.0820004, -0.0494765, -0.0345476, 0.0160603, -0.0533388, -8.78921e-05, -0.0569711, -0.0719748, -0.0163144, -0.0570588, -0.115245, -0.086111, 0.00215991, 0.125882, -0.0602343, 0.0197224, 0.0137297, 0.0453633, 0.0767126, 0.0269676, -0.0518179, -0.0410239, -0.0261805, 0.00271437, -0.0103045, 0.0144939, -0.00781799, 0.091453, 0.00882508, -0.048465, -0.0288822, 0.019855, 0.0240093, 0.0228284, 0.00842587, 0.0729348, 0.0454924, -0.0283723, -0.0344592, 0.0553241, 0.112795, 0.133763, 0.152217, 0.0883003, 0.0797521, 0.057635, 0.0414339, 0.0568913, -0.073388, -0.00689092, -0.00452601, -0.00257885, -0.0363489, 0.0162994, -0.00830018, 0.0219621, 0.00443454, 0.0301365, 0.0147276, 0.0730565, -0.0155388, -0.00946199, 0.0106759, -0.00297816, 0.0333591, -0.0261534, 0.100934, 0.0978765, 0.0700425, 0.119167, 0.137978, 0.155762, 0.160234, 0.0742685, 0.0852571, 0.102511, 0.0547144, -0.0575176, -0.0247389, -0.00594745, 0.0632954, -0.0434433, 0.0382928, 0.0535704, 0.0705912, 0.0431407, -0.0123356, 0.0213222, 0.0036288, 0.041551, 0.0721763, 0.0329547, 0.0161662, 0.0748445, -0.00539636, 0.113589, -0.0166617, 0.0163469, -0.0117328, 0.0464528, 0.0730257, 0.064727, 0.0806557, -0.0188659, 0.0911279, -0.0640982, 0.0866365, -0.0312847, 0.0715783, 0.0405096, -0.0586504, -0.0228347, -0.0427965, 0.080389, -0.0671657, -0.028573, -0.0541196, 0.0442457, -0.100765, 0.0611403, 0.0564221, -0.0350475, 0.0318257, -0.0732769, 0.00845962, -0.00469051, 0.10624, -0.0305389, 0.0109413, 0.0108237, 0.0397915, -0.072185, -0.0179938, 0.104006, -0.0595601, -0.013593, 0.0237466, 0.133843, 0.0353225, -0.0500324, 0.035606, 0.00779399, -0.0249091, -0.0140056, -0.0330723, -0.0930979, -0.00878083, 0.0597359, 0.0371306, -0.0491203, 0.00354528, -0.00893144, -0.083292, 0.0104991, 0.026975, 0.0338719, 0.00931724, -0.00213378, 0.0673524, -0.0506788, -0.0292063, 0.0801845, 0.0146832, -0.105452, -0.027652, 0.0151841, -0.0341808, 0.0579564, 0.0459108, 0.0920267, 0.0948063, 0.0217799, 0.00148277, -0.0733584, -0.0433117, -0.0861387, -0.121327, 0.0326213, 0.0594118, 0.0961714, -0.0380917, 0.0138159, -0.0400715, 0.121917, -0.0168703, -0.0199202, 0.00493215, -0.0258929, -0.00462011, -0.0398479, -0.019478, 0.00747761, 0.0138694, -0.0224482, 0.00790141, -0.0787804, 0.0191799, 0.0381343, 0.0218962, -0.0314543, -0.00534186, 0.0206338, -0.00819187, 0.0407967, -0.0169818, -0.0647885, -0.127182, -0.0507555, -0.0867416, 0.0482872, 0.111572, 0.0287153, 0.094321, 0.0432007, -0.0873704, 0.0192922, -0.00161702, 0.0123529, 0.0301633, 0.017088, 0.00557698, 0.0433604, 0.0422281, -0.129644, 0.0113965, -0.0178517, 0.036947, 0.086194, -0.0442887, 0.0277037, 0.108638, 0.0452575, -0.00767192, 0.0454192, -0.00963218, -0.0457334, -0.0645676, 0.0132429, 0.0759706, -0.0114216, 0.0509173, -0.119103, -0.05085, 0.0139439, -0.00162907, 0.0510978, -0.0110394, 0.00994608, 0.0284165, 0.0795597, -0.039673, -0.0806793, -0.000366293, -0.0385787, 0.03736, -0.0549387, -0.0444678, -0.0275659, -0.043843, 0.0218428, -0.0702584, -0.0100182, -0.0256788, -0.000267362, -0.0801375, 0.0492549, -0.0461003, 0.0164422, -0.152142, -0.000844647, -0.030726, -0.00626587, 0.0448489, 0.146761, 0.0127024, -0.125083, 0.0913206, 0.09619, 0.0672684, -0.0418446, 0.0180658, -0.0577261, -0.0699086, -0.0441971, -0.0674118, 0.0102706, 0.0714612, 0.0182787, -0.00373351, 0.0280036, -0.0445555, 0.00338958, 0.0505145, 0.0607015, -0.0711915, -0.185418, -0.0058112, -0.0413283, 0.0475211, 0.0803717, 0.0529408, -0.0225866, -0.0269254, -0.0153831, 0.0936242, 0.113538, 0.0395038, 0.0307595, -0.033468, -0.0531134, 0.00293343, 0.0374688, 0.0228355, 0.036469, 0.0415358, -0.0111733, 0.0249689, -0.0738708, -0.0177395, -0.00589247, 0.0175739, 0.0708684, 0.000678264, 0.0318856, -0.00177181, -0.0238087, 0.0129712, -0.00305486, -0.0301527, -0.0451314, -0.0460008, -0.00665139, 0.0688527, 0.0356462, 0.0594835, 0.0233043, -0.0672396, 0.0621263, -0.0331331, -0.0393755, -0.0138683, -0.00258209, 0.0521265, -0.106122, -0.079316, -0.0819805, 0.0316241, -0.0188459, 0.068426, 0.0266636, 0.0222838, 0.0055261, -0.0650988, 0.00716436, -0.0776554, 0.0185327, -0.0466268, 0.0320534, 0.10656, 0.0494687, 0.102931, -0.0599757, -0.092772, -0.0282118, -0.0885938, -0.00542823, 0.0113427, 0.0235076, 0.117479, -0.0350611, 0.0569553, 0.0395106, 0.0147611, -0.108255, -0.0519406, 0.00259761, -0.0835368, 0.0685571, 0.0946758, -0.057276, -0.0939371, 0.0402803, -0.0892795, 0.0828336, 0.0338193, 0.0193599, -0.0637539, -0.0388056, 0.0798684, -0.00893852, 0.0860232, 0.0073419, 0.0216765, 0.0167337, 0.0856613, 0.0185675, 0.0778289, -0.079388, -0.0639831, -0.0946926, -0.0409054, -0.0325524, -0.0617884, -0.118689, 0.102273, -0.0639488, -0.0424055, -0.0282648, 0.0277548, 0.0908358, 0.0877476, 0.0615523, 0.0025195, 0.0025727, 0.0665878, -0.0778578, -0.0370576, -0.0438007, 0.046651, 0.0251749, -0.0420384, -0.0205682, 0.0813703, 0.0294732, -0.0824113, -0.0166559, 0.0326134, -0.0684659, -0.0332959, -0.0292295, 0.0850529, 0.00260462, 0.0630177, 0.0350045, -0.133423, -0.028194, -0.0405172, 0.0588652, -0.0554169, -0.0932573, -0.0574924, -0.0891943, 0.0457044, -0.0546338, -0.0372393, 0.00161141, 0.019088, 0.123027, -0.0487025, 0.101464, 0.00717788, -0.0674718, 0.0206964, -0.02564, 0.0327935, 0.12539, -0.0473408, 0.0988886, -0.0232705, -0.0544241, 0.00135658, 0.0370752, -0.00892126, 0.106862, -0.0957772, -0.000179466, -0.105912, -0.0116301, 0.0547215, 0.0390085, -0.0888818, 0.0128418, 0.0511151, 0.0699805, -0.00127193, -0.00581607, 0.0187559, 0.0498345, 0.0547514, -0.0465041, -0.0395708, -0.100389, 0.0230073, 0.000865685, 0.0194113, 0.0407875, -0.0666051, 0.0536494, 0.0679031, -0.0599226, 0.115565, 0.0517541, 0.0544349, 0.0349403, -6.15069e-05, -0.0362988, -0.114815, -0.0194375, 0.0254978, -0.0160029,
-0.0200315, -0.0440619, -0.0247982, 0.0158505, -0.126323, 0.0453625, -0.0695104, 0.120468, 0.050324, 0.0740757, -0.134018, 0.056784, 0.0124492, 0.0469096, 0.0780314, -0.0345928, -0.0879209, 0.0317746, -0.0369995, -0.009088, -0.0250748, 0.0792511, -0.010179, -0.0484766, -0.0299601, 0.0666307, -0.0945426, 0.026653, 0.00210374, -0.0496867, -0.0381333, -0.0745711, -0.0232325, 0.0970445, 0.0385823, 0.129465, 0.00119929, 0.0581131, -0.0349767, 0.0530767, -0.0648827, 0.0772687, -0.0301228, -0.0424152, 0.0490672, 0.106506, -0.0355559, 0.00663679, -0.0521716, 0.0304294, 0.00443027, 0.043494, 0.0217903, 0.0364837, -0.0382691, -0.131087, -0.0133786, 0.0909818, -0.0704794, 0.05389, -0.000131549, 0.0204028, 0.0512347, 0.017293, 0.112786, -0.0390155, -0.00442858, 0.0224531, -0.0525125, 0.0268521, 0.0431494, 0.00497115, 0.025898, 0.0256989, 0.0134667, -0.0319339, -0.0096251, -0.0659977, -0.00942403, 0.00504751, 0.0359172, 0.00738834, 0.0753267, 0.0231329, -0.00471901, -0.0962667, 0.0700568, 0.00139727, -0.0173956, -0.00565407, 0.0821237, -0.00935587, 0.007418, 0.0347362, 0.207731, 0.170212, 0.116596, -0.0165747, 0.043946, 0.0417208, 0.115313, 0.0197377, -0.0511178, -0.0205765, -0.00518822, 0.0259869, -0.00149445, 0.113971, 0.011456, -0.0358558, 0.0326977, -0.012384, -0.0984804, -0.0165582, 0.00676654, -0.0127762, -0.00679741, 0.0539476, -0.0299749, 0.00937717, 0.0838177, 0.0783112, 0.0422525, 0.0870238, -0.035999, -0.0554194, 0.00883675, 0.008071, 0.0488787, -0.0184491, -0.0136184, -0.0790112, 0.00159017, -0.101498, -0.05673, -0.000602114, 0.00955119, 0.0287568, -0.0311576, 0.0415572, -0.0676948, 0.0285506, -0.0502467, 0.0315193, 0.0264849, 0.0970775, 0.0102878, -0.009359, -0.00973644, -0.0605066, -0.00861315, -0.0407379, -0.159257, -0.0788874, -0.00616909, 0.0796636, 0.0965515, 0.0700205, 0.171115, 0.0362322, -0.00567711, -0.0523331, -0.00586929, 0.0172193, -0.0105815, -0.0562509, 0.00327556, -0.00199259, -0.0927572, 0.0922404, 0.0315676, 0.0315777, -0.0786462, -0.0743003, -0.125305, -0.107793, -0.125342, -0.161543, -0.153482, -0.169681, -0.107176, -0.0763257, -0.0726023, -0.0527512, 0.0321472, -0.0230929, -0.167736, -0.0956389, -0.120111, -0.0584435, -0.030594, -0.00704066, -0.101276, 0.0532175, -0.0122944, 0.0322346, -0.0254854, -0.065991, 0.0302864, 0.0135928, 0.124055, -0.0267579, -0.0926743, -0.137594, -0.0221001, -0.0468531, -0.126738, -0.0321011, -0.121143, -0.0455813, -0.0891055, -0.101427, -0.041368, 0.00666074, 0.0266062, 0.000749401, 0.0442231, 0.0243284, -0.0161851, -0.0940385, 0.0207752, -0.0096032, 0.00644795, -0.00309154, -0.0734503, 0.027397, -0.113598, -0.0703919, -0.00904383, 0.013471, 0.0142056, -0.131698, -0.0784659, 0.0488767, -0.0567108, -0.0667249, -0.118245, -0.102925, -0.0184124, 0.167776, -0.016014, -0.0452585, -0.0088373, -0.0484575, 0.00946531, -0.0849877, -0.0468628, -0.0405544, -0.0211077, -0.00464728, -0.0630616, 0.00341816, -0.0730941, -0.043992, 0.00166757, 0.0694919, -0.0512524, -0.0446151, -0.0533527, -0.0884377, -0.15064, -0.130774, -0.0325811, -0.138078, -0.00583011, -0.0594025, 0.0262444, 0.0725407, 0.00796347, -0.0937256, -0.0114075, 0.0638643, 0.015727, -0.0017111, -0.130002, -0.105417, -0.100275, -0.0999572, -0.0538878, 0.0442104, 0.100534, 0.00739199, 0.022821, -0.0360587, -0.0701314, 0.0647784, -0.0280908, -0.133746, -0.167259, -0.127684, -0.106368, -0.0905796, 0.049489, -0.0112762, 0.0596553, 0.0408269, -0.0773003, -0.0577429, -0.0350262, 0.0463232, -0.0908371, -0.0653276, -0.141079, -0.0427263, -0.17828, -0.0972756, -0.0333374, 0.00205057, -0.0763231, 0.0572093, -0.116205, -0.0147651, -0.053979, -0.0830816, -0.0825269, -0.090359, -0.0438799, 0.0251003, -0.0186793, 0.132573, 0.0958459, 0.081661, 0.0576406, -0.0616988, 0.0162595, 0.0357857, -0.0305715, 0.102618, -0.040506, -0.184068, -0.0442066, -0.066843, -0.0600116, -0.0893114, 0.115164, 0.0135197, 0.0180122, 0.0230624, -0.0235767, -0.00442288, -0.15775, -0.0299459, -0.0154275, -0.0458975, 0.002424, -0.0372157, 0.0469009, 0.154471, 0.102711, 0.066206, 0.0184199, -0.078178, -0.0300275, 0.0427384, 0.131125, 0.0763965, 0.0198314, 0.0149288, 0.0271011, 0.0104691, -0.0204455, -0.0813868, -0.0309396, -0.000877759, 0.0634681, 0.0363473, 0.0117712, -0.0390208, -0.04854, -0.0476565, 0.052679, -0.0220437, 0.0392416, -0.0318574, 0.105909, 0.0877628, 0.124536, 0.0888944, -0.0454261, -0.0611714, -0.0270875, -0.0724339, 0.189857, 0.0777493, -0.0288499, 0.0114575, 0.0124803, 0.0294933, 0.0704635, 0.0924936, -0.129399, 0.0292996, -0.0557582, 0.0361759, 0.00168263, -0.0139695, 0.0220174, 0.141818, 0.095366, 0.101984, 0.0726782, 0.0157633, 0.141414, 0.126246, 0.0950456, 0.0290015, -0.117109, -0.0224905, -0.0060675, 0.0130015, 0.149675, 0.088651, -0.0882575, 0.010324, 0.177798, 0.0301233, 0.0940176, 0.0722286, 0.055486, 0.037954, 0.00148134, -0.0155835, 0.0171959, -0.0472269, 0.00893169, 0.086477, 0.10196, 0.041312, 0.0254091, 0.135316, 0.02589, -0.0802239, 0.0729747, 0.0147972, -0.0348806, -0.00580959, 0.142221, 0.120567, 0.197692, 0.13774, 0.0199733, 0.105193, 0.0439611, 0.170787, 0.0573506, 0.111901, -0.0589937, -0.0220339, -0.042981, 0.0241808, 0.0667312, -0.0362862, 0.0887412, 0.0685349, 0.0273375, 0.117099, -0.09616, 0.0760008, 0.0101985, 0.141717, -0.0532958, -0.160559, 0.0303954, 0.0322278, 0.146356, 0.212007, 0.129192, 0.130469, -0.059996, 0.054835, 0.171116, 0.0923827, 0.104328, -0.0393654, 0.0393891, 0.0371554, 0.0767067, -0.0247479, 0.0629187, 0.0830206, -0.0348238, -0.0525593, -0.0520875, 0.024494, 0.1157, -0.0521647, 0.123497, 0.0627487, 0.0942172, -0.0450861, 0.111499, 0.0908361, 0.172262, 0.194447, 0.0387054, 0.0352571, 0.0123485, 0.0799182, 0.0412526, 0.131092, -0.021631, 0.144375, 0.0527566, -0.00629354, 0.047851, 0.0451253, -0.0418527, -0.0460513, 0.0947748, 0.0112602, 0.0570402, 0.0207896, 0.0148472, 0.00298355, -0.00414186, -0.0535321, -0.0681293, -0.0206435, 0.00380724, 0.289587, 0.305211, 0.0712472, 0.0890091, -0.0917541, -0.12442, 0.0441083, 0.0458145, 0.0220015, -0.00583251, -0.00903751, 0.0714851, 0.0416642, 0.018475, 0.0926515, -0.0700468, 0.043521, 0.0930648, 0.111761, 0.083535, -0.0648494, 0.0514093, -0.0170668, 0.0835256, -0.0122061, -0.00739968, 0.0459546, 0.231097, 0.233501, 0.0893959, 0.0241994, -0.053989, -0.0309509, 0.051721, 0.0238608, -0.03899, -0.00121819, -0.0659811, 0.0183726, 0.0191168, 0.0153279, -0.0509189, 0.0210318, 0.0353394, 0.0721725, 0.102866, 0.0116554, -0.082015, 0.107731, 0.00192475, 0.0481078, 0.14438, 0.077659, -0.0204314, 0.0436824, 0.00634737, -0.0129641, -0.0690605, -0.0706924, -0.0266417, -0.0501423, 0.020853, 0.126262, 0.0563318, 0.0638697, -0.0257742, -0.0637996, 0.0567736, 0.0557957, 0.0449656, 0.0394502, 0.00800253, 0.0056992, 0.0325357, 0.0934605, -0.0408042, 0.00442529, 0.0684114, 0.149624, 0.102313, -0.00717528, 0.105836, -0.0397239, -0.101403, -0.0789761, -0.013936, -0.045, 0.0844893, 0.0215169, -0.0442209, -0.0195434, -0.012452, -0.082196, -0.124799, -0.0291111, 0.0127192, -0.0338854, -0.049247, 0.0510069, -0.0233712, 0.055898, 0.0515825, -0.0146866, -0.0653049, -0.0153857, -0.0047986, -0.0146413, 0.0486791, 0.00908609, 0.0564977, -0.0425422, -0.15458, -0.090297, -0.0442561, -0.105039, -0.0426506, 0.0171518, -0.0493959, 0.00324928, 0.0322721, -0.0833222, 0.0412761, 0.128661, -0.030796, -0.0625568, -0.00584883, -0.00286627, 0.0953317, -0.017662, -0.0794499, 0.00722336, 0.00493559, 0.0259155, 0.00611526, -0.0308916, 0.0785466, -0.0686475, -0.115749, -0.102798, -0.0948344, -0.151264, -0.211274, -0.1735, -0.109362, -0.0852335, -0.0683047, -0.0587946, 0.0478886, -0.0839836, -0.00264295, 0.0187774, -0.000213032, -0.0219927, 0.0287215, -0.0817048, -0.00133494, -0.0147417, -0.0366227, -0.0692201, -0.0574146, -0.141063, -0.0972535, -0.0488456, -0.167581, -0.160302, -0.197571, -0.180307, -0.0702078, -0.153465, -0.0789382, -0.031188, -0.0717252, -0.0728392, -0.0222184, 0.00365259, -0.040002, 0.0494121, 0.0454603, -0.0152525, -0.0697265, -0.061452, -0.03958, 0.132522, -0.0380846, 0.00744957, 0.0479652, 0.00421356, 0.00494391, -0.0599767, -0.147551, -0.139584, -0.0882635, -0.0143765, -0.0919367, -0.0780414, -0.0988232, 0.00813497, -0.102847, -0.041877, -0.0584594, 0.0248868, -0.0714073, -0.0313411, -0.0570979, -0.0755977, 0.0197045, -0.0258033, -0.0694594, 0.00621855, 0.0355768, -0.0659432, 0.00547319, 0.032585, 0.0728228, 0.0551308, -0.000379984, 0.0353368, -0.0176022, -0.0582874, 0.0595012, 0.0755479, -0.0537104, -0.0222656, -0.0511074, 0.0371177, -0.0844969, 0.0597143, 0.025922, 0.0109508, 0.0198668, 0.0280071, 0.0154464, 0.0477075, -0.00336127, 0.0234068, -0.0235369, 0.0804567, 0.0517711, 0.0280465, 0.0711958, 0.063244, -0.0318879, 0.0260668, -0.0576121, 0.0909742, 0.0290055, 0.0522223, 0.0377759, 0.109393, -0.0133615, 0.143542, -0.0832714, 0.0376933, -0.018628, -0.0229769, 0.109389, 0.00922283, 0.0558358, -0.000853403, -0.0171014, 0.0160205, 0.0453957, -0.0378116, -0.0185712, 0.0433153,
-0.107575, 0.0834591, -0.035705, -0.0399856, -0.0879417, 0.00699155, 0.176683, 0.0892413, 0.00542422, -0.00657515, 0.0465088, 0.0645496, -0.0101788, -0.0455654, 0.0296529, 0.0376313, -0.0456173, 0.00205175, 0.0632586, -0.0200713, 0.0557628, -0.0356576, 0.0290332, 0.0570405, -0.00101699, 0.0396177, 0.052796, -0.0303906, -0.0226058, -0.00437229, -0.042288, 0.00505526, -0.0245703, 0.021919, 0.110535, -0.0189804, 0.0062899, 0.0717235, 0.0735292, 0.0573686, 0.0184102, -0.0276039, -0.0191776, 0.00867415, -0.0172119, -0.125435, 0.0562559, -0.0211525, 0.0216711, 0.0194452, 0.0554166, 0.010999, -0.12539, 0.0335823, -0.0614592, 0.0640392, -0.011617, -0.04598, -0.0606357, -0.060872, -0.06464, -0.0738107, -0.0351968, -0.0310048, 0.0908562, 0.101627, -0.0123351, 0.0212508, -0.0022561, 0.00323775, -0.0760098, 0.047942, -8.64631e-05, -0.0415078, -0.0634982, -0.0271214, 0.0058621, -0.0464788, -0.0258392, -0.07485, 0.00743536, -0.0410755, -0.00547885, -0.0554652, 0.0145568, -0.084995, 0.091579, -0.126712, -0.0574083, 0.0410042, 0.0167185, 0.0690226, 0.0262965, 0.0361057, -0.135417, -0.05894, 0.0283011, -0.0132897, 0.0689307, -0.0347672, 0.0235924, -0.00976234, -0.124836, -0.0797842, 0.0288701, 0.00763774, -0.0192007, -0.0700104, 0.0381553, 0.0386365, 0.0787032, -0.0202087, -0.00269116, 0.0558997, 0.0397262, 0.0638935, -0.0521901, 0.0304219, 0.0327234, -0.0117713, -0.00590624, -0.0190436, 0.00809156, 0.0160507, 0.00823795, 0.0823883, -0.0677853, -0.13692, -0.0661218, -0.215378, -0.131892, -0.0758953, -0.166942, -0.055058, -0.0535357, -0.0962517, -0.0495309, -0.0659253, -0.0410824, 0.00362347, -0.071448, 0.0515276, -0.00177192, 0.0224911, 0.0952009, 0.103301, -0.0480356, 0.0551927, 0.0419482, 0.00674785, -0.0208352, -0.018794, 0.0177383, -0.0170055, -0.0818538, -0.0577157, -0.0447744, -0.0607448, -0.00802945, -0.0158764, -0.0363967, -0.156042, -0.073549, -0.00894799, -0.0671787, 0.0360482, -0.0972311, 0.00227322, 0.125073, -0.00289741, 0.000289049, -0.133663, -0.0352245, -0.00230561, 0.0267241, -0.00682366, 0.122496, 0.12038, -0.00266353, 0.0757187, 0.0695659, 0.0388565, -0.0900344, 0.0335895, -0.0641965, -0.0348305, -0.0286554, -0.152856, -0.0935514, -0.000558155, -0.142122, -0.119567, -0.0302268, -0.0521709, 0.123702, 0.000678656, 0.00161062, 0.085209, -0.00820152, 0.158927, -0.106705, -0.00136118, -0.0951435, 0.0322235, 0.0655936, 0.10548, -0.0156207, 0.0535407, 0.0829644, 0.0792622, -0.0441255, -0.108016, -0.068356, -0.0120093, -0.0588175, -0.0951518, -0.13434, -0.14513, 0.0149324, -0.162347, -0.00801255, 0.00977233, 0.0180862, 0.00185399, 0.0393251, 0.0552313, -0.0224447, 0.0352088, 0.0582211, 0.0276906, 0.000696404, 0.0356142, 0.174222, 0.0836081, 0.178251, 0.0480463, 0.025335, -0.0786605, -0.0274857, -0.116597, -0.0597618, 0.033616, 0.0570644, -0.175714, -0.181397, -0.0628894, -0.0485082, -0.0530421, -0.0732874, -0.136306, -0.0620569, -0.030074, -0.0334161, -0.0288721, 0.0784949, -0.00037303, 0.0275462, 0.00835844, 0.00935487, 0.0184427, 0.0987676, 0.0615781, 0.102416, -0.00518528, 0.0475535, 0.0464418, -0.00993888, -0.0173458, -0.0359915, 0.111306, -0.0231368, -0.0244693, -0.0783623, -0.104588, -0.0175417, -0.0903059, -0.0917634, -0.114183, 0.0718028, 0.0846942, -0.0233608, 0.125845, 0.0428551, -0.00665462, 0.0238039, 0.0866819, 0.0717715, 0.14991, 0.0931687, 0.0222109, -0.0507407, 0.0502757, 0.00570248, 0.0356604, -0.0165668, -0.0629114, 0.0244885, -0.014597, -0.0422075, -0.0732529, -0.0793288, -0.0342046, 0.101509, -0.0517558, -0.0622848, -0.075884, -0.0868143, 0.0512483, 0.0198211, 0.0473336, 0.0564896, 0.010448, 0.0489273, 0.00963008, 0.04467, -0.00668913, 0.0556554, -0.0478186, -0.0554326, -0.00107888, -0.114993, -0.120226, -0.0685157, -0.0280345, 0.0317885, 0.181908, 0.211442, 0.124141, 0.0524552, -0.0206568, 0.0378071, -0.118933, -0.169641, -0.0565127, -0.0435496, 0.0772268, 0.0606744, 0.0885882, 0.0261338, -0.0380064, 0.0392144, 0.0189166, -0.0229205, 0.011752, 0.0253314, -0.135682, 0.0284302, -0.0438423, -0.0707403, -0.0432628, -0.0331695, 0.109513, 0.176884, 0.228129, 0.159048, 0.0745538, 0.0125475, 0.0843756, 0.0916797, -0.0140605, -0.0317481, 0.0171345, -0.00438249, -0.056209, -0.014075, 0.0805354, 0.0156563, 0.0665194, -0.0311571, -0.0874472, -0.022345, -0.000662284, -0.0515979, -0.120998, 0.0342755, -0.0485158, -0.0570013, -0.0504075, -0.0469329, 0.109308, 0.140523, 0.200912, 0.170672, 0.222411, 0.0915189, 0.0592915, -0.0435429, -0.00722642, -0.0519322, -0.085713, -0.0385368, 0.0759033, -0.111333, 0.0217147, 0.0905523, 0.114699, 0.060674, -0.0547496, -0.120544, -0.0311536, -0.107865, 0.0900475, 0.103007, 0.0307207, 0.0557176, 0.101522, 0.0253279, 0.0958892, 0.161469, 0.167316, 0.155048, 0.0371874, 0.0992838, 0.0325289, 0.0834642, 0.0968926, 0.0168053, 0.0337267, -0.0521413, -0.0141512, 0.0473136, 0.0247404, 0.0629032, 0.0212085, -0.0210574, 0.0582346, -0.0489947, -0.0337696, -0.114161, -0.0221037, 0.25672, 0.0185753, -0.0221628, -0.0759897, -0.0500688, 0.157523, -0.0185871, 0.0604792, 0.147743, 0.093333, 0.0229407, 0.0440697, 0.0864168, -0.0230537, -0.0488122, 0.0371572, -0.0625684, 0.0014798, 0.0589969, 0.0319824, 0.0465054, -0.0591506, -0.0262115, 0.0171858, -0.051003, 0.102805, 0.103318, -0.0736124, 0.0859648, 0.0667219, -0.0959236, -0.0488285, 0.0444873, 0.172967, 0.10015, 0.084326, 0.0101262, 0.143291, -0.0173563, 0.0351838, 0.0504116, -0.0236073, -0.025012, 0.0178217, 0.011948, 0.0115656, 0.0527476, 0.00716345, -0.0542783, 0.0301803, 0.0257629, 0.0365417, -0.0469369, -0.0652872, -0.0813868, -0.00765219, 0.0990844, 0.0693657, -0.16718, -0.0624572, 0.10514, 0.17759, 0.0168264, 0.1107, 0.0825761, -0.0985842, -0.116855, -0.00346382, 0.0900057, -0.0420463, -0.0507853, 0.0239274, -0.00600069, -0.0907351, 0.0926777, -0.096224, -0.0516201, 0.0270344, -0.0436988, -0.0664631, -0.106516, -0.165939, -0.0970306, -0.212423, -0.0488278, -0.123586, -0.124802, -0.0108769, 0.214863, 0.144323, 0.0234787, 0.0555691, 0.0311885, 0.000632555, -0.176791, 0.0102478, -0.0431482, -0.15277, 0.0574216, 0.0565392, 0.0490427, 0.0360918, -0.0559289, 0.0192778, -0.00762343, -0.022794, -0.0952745, -0.0977853, -0.140852, -0.0532756, -0.129327, -0.126982, -0.121464, -0.109762, -0.0901929, 0.0455135, 0.1692, 0.0616177, 0.0352827, -0.0579783, -0.133289, -0.0235697, -0.0169403, -0.165785, -0.100628, -0.0743317, -0.0494955, 0.0928924, 1.27928e-06, -0.0463738, 0.091994, -0.0137501, -0.0276686, -0.0138678, -0.0606852, -0.0704852, -0.0763868, -0.113933, -0.144573, -0.235122, -0.285641, -0.227524, 0.0624162, 0.154033, 0.249056, -0.000373275, -0.0234059, 0.00753832, -0.0844434, -0.156146, -0.130131, -0.104747, -0.0857292, -0.106248, -0.0228046, 0.0376435, 0.0319031, -0.0612287, -0.0147077, 0.0250626, -0.0960101, -0.00555846, 0.0354839, -0.0467783, -0.175379, -0.0868177, -0.147298, -0.127289, -0.13264, -0.0710459, 0.0600654, 0.0374824, 0.0186573, -0.00446936, 0.0690859, -0.0791654, -0.0983656, -0.00247244, -0.129585, -0.087591, -0.140508, 0.0338328, -0.017393, -0.0120605, -0.0141966, -0.0285954, -0.0467302, -0.0343076, -0.0220895, -0.0421874, -0.0962052, -0.0641463, -0.0196329, -0.147555, -0.0714764, -0.0782701, -0.00576676, -0.0361024, -0.0198972, 0.0574383, 0.0629216, 0.100169, -0.0423884, -0.0584152, 0.026432, -0.129633, -0.124514, -0.154764, -0.0753455, -0.0835539, 0.044699, 0.0182076, -0.0535912, -0.0574532, -0.0332363, 0.099804, 0.0236116, 0.00767718, -0.0232534, -0.0863068, -0.0258059, -0.0174419, -0.0808348, -0.0654198, -0.00838457, -0.00659316, -0.0118562, 0.0577048, 0.0760084, 0.0239121, -0.0875694, -0.0522493, -0.0708476, -0.0465133, -0.101596, -0.0570164, -0.0817194, -0.00446949, -0.0281086, -0.125812, -0.0375396, 0.0222216, 0.142294, 0.0128823, -0.0452001, -0.0549012, -0.0829308, 0.0327474, -0.0775474, -0.0313191, -0.032465, -0.0324924, -0.0321638, 0.0314999, -0.0926753, 0.0257342, 0.0168506, -0.0335623, 0.0987912, 0.038699, -0.00647332, -0.00948218, 0.0790182, -0.0401165, 0.0195736, -0.0512324, 0.0727114, -0.0897353, 0.0297398, 0.0371109, 0.0190052, 0.0747013, 0.0401677, -0.0046518, -0.0502547, -0.00844704, -0.0181866, 0.0147465, 0.050374, -0.0194119, -0.111346, 0.058509, 0.0144023, -0.017826, 0.095995, 0.0122018, 0.0489421, 0.0556717, 0.0645171, 0.0856588, 0.0281354, -0.0197153, -0.0279345, -0.119608, -0.0087395, 0.0440028, 0.0554436, -0.0611061, -0.0662915, -0.0719396, 0.00278854, -0.0393674, -0.067776, -0.0135152, -0.021022, 0.0699149, 0.0314165, 0.128325, 0.00970567, 0.0477716, -0.00295028, 0.112136, 0.133541, 0.0630883, -0.0298358, 0.00201527, 0.00328586, 0.0428552, -0.0291015, -0.0564085, -0.0136625, -0.0477623, -0.00127669, -0.0335977, -0.00220331, 0.0540393, 0.038336, -0.0667332, -0.0158171, -0.0641626, -0.0207976, 0.0224921, 0.0411476, -0.0447766, 0.00457102, -0.0379557, -0.0170342, 0.0225884, 0.0792341, 0.0129852, -0.0244675, 0.0896103, -0.015966, -0.0194658, 0.0273034, -0.00388243, -0.00346432, 0.0061405, 0.0589158, -0.0530975, 0.0318286, 0.019125, -0.0644951, -0.0440371,
0.00743727, 0.0291999, -0.0029558, -0.0436081, 0.0678636, -0.0383706, -0.0398255, 0.0874994, 0.0327253, 0.0312281, 0.0911952, -0.0124893, -0.0578612, 0.0126946, 0.0587617, -0.10608, 0.0161955, -0.00824136, 0.0287524, -0.00294562, -0.00390048, 0.0600048, 0.02947, -0.0151182, -0.0592123, -0.0134865, 0.0793193, 0.0470448, -0.0763153, 0.00978971, -0.0865965, -0.0331252, 0.0537788, -0.0280175, 0.0380966, 0.0627872, -0.0123128, 0.0615509, -0.120172, -0.0463876, -0.000366009, -0.0775565, -0.0295052, -0.0514829, -0.0658507, 0.0123177, 0.0370407, -0.0385541, -0.0225334, -0.0899203, -0.0696399, 0.0798474, 0.0726791, -0.0138633, -0.0908672, -0.0142923, -0.0481776, 0.115797, -0.0254475, -0.00219534, 0.102203, -0.0381218, 0.0649614, -0.0388323, -0.12006, -0.00769802, -0.0838937, -0.0375021, -0.00784253, -0.0464113, 0.00401536, -0.0101795, 0.00797469, 0.0728443, -0.0196903, 0.0117786, 0.0440833, -0.0614461, 0.00302865, 0.117712, 0.00620789, 0.000620966, -0.000847627, 0.0150926, 0.000458362, -0.00197487, 0.0122455, -0.0162554, -0.0939887, 0.0522934, -0.013268, -0.109994, -0.0686727, -0.0745632, -0.000226781, 0.00129362, -0.0080961, -0.104179, -0.095439, -0.0557977, -0.0962694, -0.0447199, -0.0460224, -0.010969, -0.00962019, 0.0154416, -0.0337233, 0.0727915, 0.0855536, 0.0647567, 0.0696117, -0.0196192, -0.120275, -0.00392622, 0.00421352, -0.0311917, -0.013446, 0.00587061, 0.0822106, 0.0851688, -0.0672072, -0.0733432, 0.0115426, -0.00125403, -0.0643443, 0.0125507, -0.138281, -0.0322531, 0.010886, -0.0384336, 0.0221415, 0.0628737, 0.033031, 0.00392782, 0.0522804, 0.0409196, -0.107821, 0.0125617, -0.131377, 0.0376548, -0.0195894, -0.0541841, -0.00482286, 0.0796117, 0.0417002, -0.0572632, -0.00049794, -0.0316019, 0.0546543, 0.0317625, 0.0399329, 0.0917219, 0.0800554, -0.107363, -0.17584, -0.18217, 0.0132773, 0.0745, 0.0264554, 0.0213286, 0.0654061, 0.0212513, 0.0991945, -0.0187617, -0.022568, -0.0477977, -0.0343118, -0.0598302, 0.041906, -0.0820693, 0.0120889, -0.098236, -0.0675152, 0.0676448, 0.00602903, 0.0268697, 0.0160988, 0.179233, -0.0324924, 0.0215138, -0.0447257, -0.180381, -0.30793, -0.227583, 0.00176688, 0.0464526, 0.167014, 0.0815348, 0.0538058, 0.105038, 0.112627, 0.136388, -0.149451, -0.0293086, 0.0100921, -0.0812155, 0.00840007, -0.094602, -0.0410343, -0.0590459, -0.0111032, 0.0678241, 0.0951546, -0.0977472, -0.00293068, 0.162593, 0.0234491, -0.0316126, -0.0236121, -0.172623, -0.314495, -0.180482, -0.0773678, 0.194814, 0.134437, 0.136533, 0.0222391, 0.00166754, 0.0319351, -0.0289754, 0.101208, 0.0164934, 0.094957, -0.0200408, -0.0355975, 0.0366417, -0.1187, -0.0116805, -0.0498793, 0.0311634, -0.0822618, -0.0677784, 0.0170496, 0.144188, 0.0731871, 0.032394, 0.0117014, -0.239976, -0.235323, -0.201998, 0.185972, 0.165938, 0.0531469, -0.057344, -0.0103245, -0.00568755, -0.0167318, -0.000965599, -0.0461669, 2.70838e-05, 0.00449223, 0.0367562, -0.128736, -0.00843083, -0.052554, -0.0123269, -0.032358, -0.00848258, -0.0548308, -0.0319812, 0.108711, 0.128126, 0.131339, -0.0161362, -0.0575445, -0.256786, -0.316967, -0.0561085, 0.201099, 0.0775968, -0.0322706, -0.0865811, 0.0633152, -0.111617, -0.0210778, -0.0975253, 0.0293198, 0.00573885, -0.0182604, -0.0373914, -0.00528496, 0.00978867, 0.0600207, 0.107377, 0.00155427, -0.0503168, -0.17873, -0.0986356, 0.0304157, 0.185698, 0.0296635, 0.0997237, 0.00500321, -0.30521, -0.255585, -0.0288548, 0.0953961, 0.0230543, -0.0539894, -0.0806433, -0.0491088, -0.0735987, -0.0787959, -0.100005, 0.034233, -0.033983, -0.0330297, 0.153437, 0.00371184, -0.0568947, 0.0259707, 0.00467932, 0.0533865, -0.00646955, -0.0514293, 0.0251332, 0.0913584, 0.154794, 0.117014, 0.129054, 0.0131168, -0.302834, -0.180545, -0.06269, 0.147623, -0.0187503, -0.0503484, -0.133873, -0.0680162, -0.104252, -0.0312359, -0.134541, -0.135641, 0.0159041, -0.0374887, 0.028907, 0.0767944, 0.0495387, -0.0350126, -0.00943321, -0.00128895, 0.00435871, 0.0289316, 0.0594723, 0.0137019, -0.0155847, 0.167035, 0.236789, 0.00939533, -0.0611037, -0.133744, 0.0709785, 0.119544, 0.0579824, 0.0410399, -0.0654783, -0.0492568, 0.0295662, 0.0425309, -0.121224, -0.0122564, -0.0655466, 0.0586199, -0.0109746, -0.0839165, 0.117806, 0.108797, 0.00752967, -0.000325957, 0.0157784, 0.02978, 0.0361605, 0.037204, -0.046476, 0.113834, 0.0808375, 0.0106586, -0.176686, -0.183722, -0.00790962, 0.0189784, 0.0996313, 0.165628, 0.00447448, 0.00497293, 0.0797679, 0.112688, 0.0591595, -0.095088, 0.0143742, -0.0654019, -0.0095846, 0.0516211, 0.00567632, -0.045396, -0.0104567, 0.0489468, 0.0574761, 0.118905, 0.0229407, -0.0152006, 0.109231, 0.124784, 0.0288421, -0.0194447, -0.0944085, -0.0199816, 0.0579968, 0.0575231, 0.0432687, 0.0596895, 0.157282, 0.0669131, 0.0375353, 0.0622355, -0.101684, 0.0253426, 0.0738025, -0.0877585, 0.0634863, 0.10445, -0.00677346, 0.0129619, 0.054433, 0.0386293, 0.0728727, 0.00688348, 0.00204136, 0.0401027, 0.0310618, 0.0623387, 0.0474615, -0.00896761, 0.0222114, 0.0368271, 0.0760203, 0.0567272, 0.109004, -0.0277662, 0.0990467, 0.0474916, 0.0838654, 0.070251, 0.00954808, -0.073155, -0.0374526, -0.0416083, -0.0540294, -0.0712729, -0.0485097, -0.0213903, -0.0330394, -0.0140377, -0.00676871, 0.0344935, 0.0666054, -0.0510546, 0.0144715, -0.019113, -0.0114311, -0.00568052, -0.0209351, 0.0539551, 0.186094, 0.0904754, 0.0514822, 0.0401112, 0.132992, 0.0721986, -0.000345255, -0.0602744, -0.0906388, -0.0471765, 0.083304, -0.0603065, 0.133971, -0.0145225, 0.0336801, -0.10995, 0.0370324, 0.022162, -0.0259794, -0.091995, -0.00468616, -0.0253743, 0.0467835, -0.0580349, -0.111598, -0.217946, -0.0509843, -0.092701, 0.0453517, -0.0234746, -0.0280569, 0.0718447, -0.0508891, -0.00649459, 0.0172951, -0.0268573, -0.0310351, -0.0240694, 0.052833, 0.0035912, -0.114553, -0.106844, 0.083057, -0.0123806, 0.0358342, -0.0191632, -0.0981093, 0.00652759, -0.0988894, -0.106796, -0.104171, 0.0129784, -0.104864, -0.226132, 0.0598799, 0.0461905, -0.0673247, -0.0215946, -0.0598239, -0.0247095, -0.117403, -0.127017, -0.00351116, -0.0179028, -0.0623193, -0.0138391, -0.105605, 0.00883384, -0.0177612, -0.0123234, -0.056555, -0.0403349, -0.0536358, -0.0485062, 0.0041946, 0.0212647, 0.0106783, -0.119238, -0.088551, -0.0366403, -0.0710208, -0.159911, -0.079384, -0.0499198, 0.0358625, -0.154475, -0.0981327, -0.0893701, -0.112979, -0.111552, -0.101966, -0.12858, -0.0444916, -0.00344537, -0.133464, 0.0411771, 0.0407498, 0.0666232, -0.03438, 0.0213786, 0.0789319, -0.0770125, -0.0417723, -0.0619243, -0.0589659, -0.00714623, -0.140528, -0.0346237, -0.115748, -0.0370193, -0.0269528, -0.0250011, -0.00974007, 0.00215075, -0.0408367, -0.048372, -0.0480617, -0.165494, -0.0350897, -0.0582822, -0.0157383, -0.0839663, -0.146826, 0.076802, -0.0134281, -0.0307319, 0.0233947, 0.0932685, -0.0355366, 0.0865777, -0.057324, -0.0695774, -0.0141228, 0.0948984, -0.0805195, -0.0538408, 0.0206951, -0.073179, 0.0490269, -0.0399649, 0.0203201, 0.0287611, 0.0647053, -0.00455439, -0.00723136, 0.061845, -0.0171123, -0.0523103, -0.0365911, 0.0513021, -0.0104366, -0.0582522, -0.0341, -0.0479274, 0.0457432, -0.0354463, 0.0137568, -0.027924, 0.0438399, -0.0046288, -0.105744, -0.0132886, -0.0650542, -0.194268, -0.0039628, -0.0289106, 0.0748807, 0.057945, 0.0356162, 0.0470855, 0.047784, 0.0437532, -0.00540435, 0.0319211, 0.0766774, -0.062775, 0.0430969, -0.0326413, -0.0144886, -0.0107358, -0.0307868, 0.00746678, 0.0404917, -0.0411498, 0.0959479, -0.013681, 0.0314971, 0.0252226, 0.0562264, -0.000384783, -0.0351382, 0.00964633, -0.174159, 0.0268202, 0.0352058, 0.0178964, 0.0583775, 0.0414303, -0.00576604, 0.065374, -0.0773038, -0.0545154, 0.0149114, -0.0258313, -0.0267193, -0.0699686, 0.0342568, 0.0276055, -0.0774139, -0.0633965, -0.0129512, -0.0116139, 0.0771997, -0.00474112, 0.00492031, 0.109501, 0.0776623, -0.0258103, -0.0792962, -0.0546692, -0.0639499, -0.08604, 0.14747, 0.0603593, 0.0368774, 0.0819272, 0.142981, 0.0119476, -0.0535924, -0.0597866, -0.0543306, -0.0437492, 0.0384785, -0.080777, 0.0434454, 0.0646285, 0.0553689, 0.05034, -0.00735329, 0.00842263, -0.0470882, 0.0506762, 0.00170843, -0.0607981, 0.0407068, -0.107153, -0.168108, -0.0429487, -0.0301077, -0.022366, -0.0366637, 0.0290346, -0.0187003, 0.0363018, -0.0968409, -0.00833837, -0.13988, -0.0477521, -0.00946575, -0.116104, 0.0225383, 0.00450266, -0.0279549, 0.0791328, 0.00124416, 0.00698094, -0.0182531, -0.00592871, -0.0253176, -0.00354814, 0.0448598, 0.0113078, 0.0183295, -0.0199518, -0.130165, -0.030516, -0.0816812, -0.101468, -0.0738067, -0.138577, -0.159535, -0.138352, -0.102311, -0.136381, -0.158206, 0.0610246, 0.016076, 0.00766365, 0.040684, 0.0335263, -0.0480724, -0.0349574, -0.0825473, -0.0214902, -0.0236945, 0.0257515, 0.0918086, -0.00829065, -0.00575022, 0.0231707, 0.00331275, 0.0585944, -0.0085802, -0.0219623, 0.0689675, 0.000650694, 0.067819, -0.0498939, -0.0345398, -0.00347335, 0.0561029, -0.0494516, 0.0232949, -0.0120678, 0.0264458, 0.000920247, 0.0286618, 0.0087447, 0.00543215, 0.050901, 0.0472462,
0.023245, 0.0678609, 0.104623, -0.0377171, 0.00371861, 0.0311967, 0.0062286, -0.0375782, 0.0111984, 0.0545178, -0.0184445, 0.130151, 0.00373495, -0.00709733, -0.00443511, 0.0539121, -0.0272103, 0.000193426, 0.0669842, 0.0401239, 0.0740941, 0.0540678, 0.023151, 0.0321358, 0.0340062, -0.0301561, -0.003896, -0.0901171, 0.0172775, -0.0313746, -0.023746, -0.0761078, -0.0036384, -0.000141416, -0.00680431, -0.0581983, 0.128663, -0.0584793, -0.00212374, 0.00893942, -0.0393814, 0.0299782, 0.0491932, -0.019059, 0.00516424, 0.0722474, -0.111957, -0.0265536, 0.0452744, 0.0735767, 0.0615288, -0.105478, 0.0458597, -0.0569654, 0.055101, -0.103365, 0.0410398, 0.00531628, 0.0758542, -0.00633351, 0.101907, -0.0588805, -0.00455371, 0.00141376, 0.0689153, 0.0442646, -0.00571604, 0.021613, -0.0200992, 0.0336006, -0.0198998, 0.023705, 0.0245233, 0.084706, 0.0609423, -0.0198423, 0.0128582, 0.0407549, 0.0415078, -0.00667313, -0.0236523, 0.0756589, -0.0348341, -0.0454015, -0.0374168, -0.0318596, -0.047449, -0.0643389, 0.0417916, -0.000346927, 0.0761355, -0.0465061, 0.0369863, -0.0270647, 0.0311551, 0.00990199, 0.0799957, 0.0280875, 0.0470535, 0.0420382, 0.0719523, 0.098773, 0.0450904, 0.0797367, 0.0827943, -0.060212, -0.00744373, 0.0230335, -0.0220033, 0.137261, 0.0173813, -0.0387717, -0.0439154, -0.100109, -0.0140448, 0.00420173, 0.0906486, 0.0119706, 0.0723645, -0.0186354, 0.0289755, -0.00577822, 0.00707742, 0.0415938, -0.00520358, 0.0848165, 0.0195056, 0.00807765, -0.0183005, 0.048406, 0.0116078, 0.0476701, -0.0804827, -0.0492928, -0.0790353, 0.0103641, -0.0414412, 0.00261853, -0.00638168, 0.0372667, 0.0523744, 0.0375037, 0.0428275, 0.028638, 0.0301714, 0.0223773, 0.0183815, -0.0211469, -0.00252233, -0.0944546, 0.107939, -0.0491844, -0.0981017, -0.140637, -0.150134, -0.0393225, -0.00850926, -0.0823903, 0.0552776, -0.0503907, 0.0122508, 0.00226845, -0.00659253, -0.0119303, 0.00288006, -0.0109556, -0.04376, 0.0487291, 0.0149539, -0.0174725, -0.00775886, 0.0464844, -0.0461722, -0.0154522, -0.0160382, 0.000477092, -0.0070357, -0.0299932, -0.0941491, -0.0253719, -0.0777633, 0.000868148, -0.0292702, 0.0562485, 0.125962, -0.0967341, -0.00834868, 0.025191, -0.0768156, -0.0868373, 0.0542076, -0.0200531, 0.0338529, -0.0858562, 0.143474, -0.0120835, -0.0820867, -0.0956108, -0.0219036, 0.0273054, -0.104481, 0.0624643, -0.000453933, 0.0178907, -0.00843726, -0.0422897, -0.0254671, 0.0786371, 0.0359005, -0.0595998, 0.116403, 0.0543662, -0.0369036, 0.0774864, 0.014374, 0.0799952, 0.00506332, -0.0174514, 0.0251734, -0.13501, -0.0339116, -0.0770946, -0.025676, -0.121847, 0.039746, 0.0259376, -0.0402569, -0.03892, 0.0453166, 0.118036, -0.0861329, -0.0449683, 0.0362129, -0.00939443, -0.00171219, 0.115679, -0.149161, -0.0521913, 0.0274985, 0.103944, 0.0491461, 0.0625256, 0.0220655, 0.127818, 0.0260017, -0.082097, 0.0104765, -0.0171261, -0.0456812, -0.0313901, -0.0064486, 0.0477738, 0.0106664, -0.0250963, 0.0233599, 0.0896282, -0.00928919, -0.00368257, -0.0391785, 0.0386107, 0.0168506, -0.0321035, 0.0943909, 0.0454985, 0.141162, -0.192867, -0.164342, -0.0672869, -0.00321143, -0.0288804, -0.0491013, -0.050171, 0.0585093, -0.0189274, 0.0205483, -0.0168478, -0.00167886, -0.0767603, -0.0957107, 0.0480422, -0.00724997, 0.0429913, 0.0262636, 0.0173502, -0.0737984, -0.0700721, 0.0958178, 0.0379474, 0.015079, -0.0820773, -0.0483202, -0.0513617, -0.202643, -0.272695, -0.107357, 0.0642466, 0.0740723, 0.0484294, -0.00449802, 0.0810966, -0.148653, -0.0850137, 0.0295325, -0.037776, -0.0266234, -0.00941096, -0.0172802, -0.0348873, -0.0219646, 0.0173559, -0.0270792, -0.0846047, -0.0863857, -0.019889, -0.05747, -0.0839858, -0.0803623, -0.228042, -0.231935, -0.405312, -0.501001, -0.190618, 0.0184777, 0.0256925, 0.00557151, -0.0739315, 0.000972718, 0.0433236, 0.0302642, -0.00296843, -0.0541732, -0.0681719, -0.0402175, -0.0485419, -0.0156736, -0.0202876, 0.0457111, -0.0304461, -0.0310176, -0.00715056, -0.139372, -0.122556, -0.1395, -0.332006, -0.235175, -0.369474, -0.299125, -0.329692, -0.0727434, -0.00297605, 0.120125, 0.0787808, 0.125507, -0.00176722, 0.0285452, -0.0280558, -0.10816, 0.0494293, 0.0283687, -0.00350039, -0.0620211, 0.0767097, -0.0402077, -0.0364599, -0.0563614, -0.0622302, 0.00793286, -0.0628254, -0.128287, -0.163237, -0.205744, -0.301759, -0.36524, -0.167249, -0.173225, -0.0703778, 0.107987, 0.205169, 0.0960227, 0.0127062, 0.0305072, -0.0609925, -0.0126421, -0.0380167, 0.00252485, -0.107477, -0.0503761, 0.00856803, -0.0318436, -0.100995, 0.0384678, 0.0153542, -0.0300261, -0.0624208, 0.0324495, -0.0698481, -0.118835, -0.181541, -0.204309, -0.190971, -0.05602, -0.0236089, 0.0720905, 0.184896, 0.23874, 0.109615, 0.0910197, -0.100275, 0.0703073, 0.0891403, -0.0191646, -0.0944113, -0.0108465, 0.0774682, 0.0226234, -0.0178181, 0.0727255, 0.0110254, 0.0543075, 0.0220777, 0.0195118, -0.0302381, -0.0326497, 0.00876504, -0.0325342, -0.0330368, -0.0325185, -0.038847, 0.073431, -0.0431512, 0.123648, 0.201999, 0.174225, 0.131269, 0.0502736, 0.00863247, -0.0288605, 0.0660132, 0.129261, 0.094427, 0.000824064, 0.0365538, -0.0588064, 0.0636057, 0.0818602, 0.0379671, -0.0661943, -0.0584958, 0.0841498, 0.0306549, 0.0114937, 0.0388954, -0.0161634, 0.105849, 0.137817, 0.0375019, 0.177839, 0.118395, 0.0236227, 0.126007, 0.0787296, 0.180055, 0.0136701, 0.0372173, 0.0352448, 0.128398, 0.0898778, 0.111166, 0.0149414, 0.111579, 0.00247864, 0.0846694, 0.0721987, -0.0488971, 0.0815209, -0.015226, 0.0503178, -0.0176063, 0.0355972, 0.0834862, 0.0415703, 0.169418, 0.129428, 0.10294, 0.175061, 0.0921531, 0.111338, 0.110024, 0.11918, 0.0773499, 0.00217604, 0.0330121, 0.0749243, -0.0050099, 0.0147841, 0.0194157, 0.0546436, -0.0184503, -0.0428051, 0.046051, 0.0816464, 0.135164, 0.0175931, -0.0189963, 0.0781593, -0.0612219, 0.0797567, 0.0741132, 0.0536809, 0.168313, 0.110225, 0.051619, 0.0968951, 0.0408442, -0.0455115, 0.021875, 0.0679127, 0.056659, 0.0826359, 0.0342522, 0.0749819, 0.0549444, 0.147633, 0.131808, -0.100388, -0.00901425, 0.0214636, 0.129324, 0.0297533, 0.00155811, 0.00549288, -0.0817274, 0.0315575, 0.0862184, -0.00837899, 0.121573, -0.0321231, 0.0278428, 0.105358, 0.0692233, 0.0250019, -0.00684625, -0.0726136, 0.000135978, 0.0261199, -0.0257847, 0.0575501, 0.0781499, 0.0886307, 0.0613837, 0.057558, 0.0470069, 0.0279012, -0.0654259, 0.100236, -0.112836, -0.0521523, 0.0350501, -0.00312865, 0.0262966, -0.00833751, 0.0480835, -0.0237153, 0.0661292, 0.0158802, 0.109501, 0.0868181, 0.0082995, 0.0968915, 0.0529449, -0.0621165, -0.0553341, -0.00816413, -0.0228704, 0.0424981, 0.0534787, 0.0952041, 0.0680314, 0.0960868, 0.0129696, 0.0520523, -0.124609, -0.0717482, 0.00325492, -0.00391137, -0.0188827, -0.0387102, -0.00719405, 0.0213576, 0.0496201, 0.11457, 0.0577938, -0.032434, 0.0656752, -0.00103743, -0.0771134, 0.0180402, -0.0854358, -0.0741869, -0.0839653, -0.00489883, -0.0119623, -0.0294463, -0.148782, 0.0920435, -0.0308432, 0.0186328, 0.0499574, 0.0797375, -0.0142554, -0.00662523, -0.100476, -0.0479118, -0.0209781, -0.0852219, -0.0304807, 0.102925, -0.00752774, 0.0191306, 0.0457434, -0.00875475, 0.0283122, -0.110232, -0.12972, -0.0187715, 0.00506217, -0.100026, -0.0614279, -0.174643, -0.0338531, 0.0779577, -0.060461, -0.0556567, -0.100504, 0.0707522, 0.115158, -0.0295665, 0.0807987, -0.146584, -0.0165183, -0.024403, -0.0273689, 0.0286581, -0.00107906, 0.0022149, -0.0174502, 0.00723773, -0.0245002, 0.0494561, 0.115238, 0.0510135, -0.0245279, -0.0238917, -0.0601028, 0.00402062, -0.0277035, -0.0652859, 0.00154222, -0.016335, 0.00630378, -0.0879669, -0.13558, -0.0666129, -0.178139, -0.0446822, 0.00745567, -0.0552684, -0.0254597, -0.013975, -0.0597409, 0.0573252, 0.0095076, -0.0231841, -0.00684049, 0.0375804, 0.0257889, 0.0458124, -0.0357815, 0.0456198, -0.00866445, -6.91097e-06, 0.00968893, 0.0927596, 0.06957, -0.0605242, -0.0461441, 0.0342595, -0.121831, -0.076908, -0.00740973, -0.131557, -0.144739, -0.0553777, -0.0502578, -0.00773389, -0.124506, -0.086623, 0.0327389, -0.0718918, 0.00357265, -0.0442523, -0.0364301, 0.00130428, 0.0241768, 0.018999, 0.0130816, 0.0121588, 0.0378612, 0.078445, -0.0956383, 0.0627152, -0.0101148, 0.0344126, -0.0581834, -0.0240617, -0.107496, -0.025151, 0.0170139, -0.196504, -0.152672, -0.110676, -0.0348276, -0.053472, -0.0181232, 0.0302373, -0.0333124, 0.0234973, 0.0701619, 0.0153377, 0.0425253, 0.0186385, 0.0323058, -0.0417352, -0.0611256, 0.0241874, -0.0860882, 0.0699606, 0.0326992, -0.100803, -0.00168185, -0.12012, 0.0144022, -0.0909105, -0.0215217, -0.0502037, -0.0427453, -0.0190361, 0.0117154, -0.00946473, -0.13652, 0.0709055, -0.0155231, -0.00851958, -0.0142906, 0.0749736, -0.0503966, -0.00247457, 0.0470223, 0.035799, 0.01511, -0.0182528, -0.0833575, 0.0668771, 0.0271313, 0.0888999, -0.110362, 0.0120704, 0.0468235, 0.0590364, -0.0199158, -0.00543036, -0.0584066, -0.00373715, 0.000471304, -0.0273389, -0.0471884, -0.0206852, -0.0327895, -0.0368677, 0.0299298, -0.0299984, -0.0104428, -0.0347693, 0.0413017,
-0.0704934, 0.063704, -0.0278412, -0.0265797, 0.075473, -0.0676314, 0.0310866, -0.0704042, 0.0970222, 0.0442283, -0.051184, -0.0380964, 0.00227695, 0.0674589, 0.0991778, 0.0162595, 0.0324107, -0.046977, -0.00187713, 0.0784113, 0.0995608, -0.00811102, 0.0335617, -0.00564766, -0.0168653, -0.0189654, -0.000708166, -0.0691983, -0.00297974, 0.00645343, 0.0414544, -0.0256764, 0.015453, -0.00796395, -0.0611407, -0.0120789, 0.053609, 0.0939505, 0.0677373, -0.00592785, -0.0027185, 0.0069504, 0.0970377, -0.0864423, 0.0295606, -0.0368611, -0.0912813, 0.0577754, 0.0174037, 0.0410616, -0.0100359, -0.0546395, 0.0533448, 0.00965327, -0.102824, 0.017431, -1.97816e-05, -0.102878, -0.0489615, 0.00433449, 0.0343024, -0.0110892, -0.0872454, -0.0455249, 0.029496, 0.0218929, 0.0237413, 0.128207, 0.032541, -0.0171853, 0.0341771, 0.0497287, 0.0162918, -0.0909731, 0.0752602, 0.0389237, -0.0351247, -0.0117296, 0.00104922, 0.00447453, -0.0234218, 0.06102, 0.0780514, 0.026198, -0.0241391, -0.0563183, -0.0263498, -0.0490988, 0.0168575, -0.0331198, 0.0146623, -0.0297849, -0.012381, -0.044372, -0.0440393, 0.00677717, -0.00806336, 0.0716903, 0.0508222, 0.0217253, 0.0372467, -0.00294467, 0.0690267, -0.0507094, -0.0391757, 0.0266937, 0.0232343, 0.0602919, -0.016059, -0.0103057, -0.0200706, 0.0454685, -0.0215602, 0.000837167, -0.0299973, 0.0235338, -0.00170046, -0.0277159, 0.0124315, 0.00747897, -0.00189255, 0.0593436, 0.0318559, 0.0177506, -0.0353599, 0.0151793, 0.065973, -0.118495, -0.0406644, -0.0599118, -0.0459903, -0.00363118, 0.0542886, 0.00408005, 0.0938804, -0.039605, 0.0282026, 0.0400652, -0.00818888, 0.0563935, 0.0144488, -0.0110472, -0.0281829, 0.0229917, 0.00128758, -0.0188029, 0.035378, 0.0551438, -0.0241892, -0.00376884, -0.00921539, 0.0635728, -0.0397798, 0.0377521, 0.00756443, -0.0512417, -0.154705, -0.03477, 0.0152484, 0.00539427, -0.00713572, -0.0418048, -0.0178683, -0.00252818, -0.0745447, -0.102104, -0.060731, 0.0151255, -0.00768421, -0.041543, -0.0980443, 0.0407111, 0.117116, -0.00554638, -0.00581906, 0.0415399, -0.0778264, -0.0839183, 0.0189487, -0.0129948, 0.000327273, -0.00384846, -0.0395706, -0.0865034, -0.0786725, -0.0697904, 0.0165779, 0.0607689, 0.0423076, 0.0231716, -0.0503443, -0.00265126, -0.0826514, -0.0323703, -0.0124019, 0.0605267, -0.0642035, 0.0684743, -0.0330117, 0.0426285, 0.0691754, -0.0374797, 0.0529455, -0.0615832, -0.0164381, 0.0232109, 0.0490615, 0.0591874, 0.0253048, 0.0414452, -0.0597421, 0.0421112, -0.0604005, -0.0397227, 0.069726, 0.089054, -0.0231904, 0.181037, 0.0527192, 0.0181367, -0.0536831, 0.0236083, 0.0082448, -0.0285907, 0.018014, -0.0116334, -0.0086771, 0.0250405, -0.0125524, 0.076305, -0.0339176, -0.119142, -0.0671766, -0.00126918, 0.0236605, -0.0455583, -0.0157632, -0.0729652, -0.08158, -0.105964, 0.0190479, 0.0261534, 0.018671, 0.0181887, 0.107175, 0.0562605, 0.0249826, 0.00320745, -0.00567471, 0.0111318, 0.0202814, 0.0147382, 0.0533476, -0.00649561, -0.132455, 0.0384389, 0.0126702, -0.0108054, -0.0386824, 0.0193361, -0.0982321, 0.0302038, -0.0155429, -0.152715, -0.0912355, 0.00368714, -0.201467, -0.216216, -0.0847054, 0.0201363, 0.014232, 0.172985, 0.141264, 0.159374, -0.0671868, 0.0566149, 0.0064476, -0.0643157, 0.0440763, 0.0511063, 0.00988828, -0.0388761, -0.0231326, 0.0418291, -0.0170437, 0.0229619, -0.0684116, -0.0109234, -0.0186667, 0.0260273, -0.0286014, -0.0147876, -0.104343, -0.136926, -0.216756, -0.116102, -0.104949, 0.0401385, -0.00750535, 0.0467463, 0.0684958, 0.124942, 0.1438, 0.0464119, 0.00831013, -0.145537, 0.0658843, -0.00639722, 0.0244166, -0.0660043, 0.0188595, 0.00944964, -0.0552427, 0.100758, 0.0865825, 0.0552174, 0.0673363, 0.0281105, 0.130564, -0.0795221, -0.104285, 0.00691621, -0.160866, -0.289374, -0.347366, -0.169265, 0.0136034, 0.00803866, 0.102891, 0.126134, 0.178761, 0.106542, 0.0143118, -0.0773399, -0.0451464, -0.0650507, 0.036773, 0.118323, 0.00636152, 0.0840699, -0.0867805, -0.0665583, 0.0699425, 0.0122752, 0.0962042, 0.148565, -0.0128527, 0.0497784, 0.00232882, 0.0388904, -0.239961, -0.329189, -0.216663, -0.0498811, 0.0304142, -0.0427905, -0.0220657, 0.116301, 0.174921, 0.138749, 0.0636918, 0.0518007, 0.00177065, -0.00223361, -0.118674, -0.000306887, 0.0348924, -0.00398384, 0.0305726, -0.0148744, 0.222106, 0.0596529, 0.182466, 0.13033, 0.160537, 0.00528218, -0.0765465, -0.0525241, -0.141786, -0.224184, -0.175064, -0.0306583, 0.00884939, 0.0498318, 0.0727648, -0.0293977, 0.135356, 0.0425099, 0.0624993, 0.0198498, 0.0301591, -0.0946803, 0.00611173, -0.0128996, 0.0593568, 0.0745314, 0.0756077, 0.0843551, 0.0933984, 0.0965719, 0.0974162, 0.206233, 0.120629, 0.179536, 0.00124475, -0.0828122, -0.188108, -0.13655, -0.112193, -0.00560797, 0.08965, 0.0878962, 0.118975, 0.0449346, 0.155421, 0.048702, 0.00219331, 0.0482399, 0.112445, 0.0803838, 0.069207, -0.0255529, 0.0340833, 0.0949, -0.102275, 0.0117917, 0.00973405, 0.0730886, 0.101654, 0.197635, 0.210788, 0.131692, -0.0183759, -0.0518241, -0.172869, -0.182219, -0.0230304, 0.0961144, 0.0838439, 0.156033, 0.0671827, 0.0582352, 0.089643, -0.0417185, 0.032093, -0.00885079, -0.0311113, -0.0652053, -0.0064518, 0.0339128, 0.0650535, -0.0608167, -0.0651766, -0.00789869, 0.0366875, 0.0649028, 0.199792, 0.10641, 0.136542, 0.00363711, -0.0870412, -0.0961747, -0.104977, -0.0453495, 0.0701501, 0.178345, 0.110058, 0.127768, 0.073223, 0.0197855, -0.020233, -0.109919, 0.13081, -0.0754556, -0.0728291, 0.0587795, 0.0106515, 0.0610766, 0.102794, -0.0652132, 0.0157916, -0.0159193, 0.105735, 0.0538215, 0.11202, 0.129204, -0.0212309, -0.145143, -0.239559, -0.13334, 0.0563542, 0.103875, 0.14101, 0.0812017, 0.138748, 0.0999542, 0.0295772, 0.139597, 0.0129228, -0.0172713, 0.00656391, 0.0879635, -0.0166098, -0.00538623, 0.020986, -0.00321479, -0.0710691, -0.109001, -0.0941059, -0.18125, -0.0269381, 0.06112, 0.0208647, 0.150703, -0.0357313, -0.28985, -0.199029, -0.010796, 0.0125617, 0.0142653, 0.18516, 0.0793888, 0.206052, -0.0196438, -0.0534073, 0.062237, 0.0728663, -0.0238212, 0.00451273, -0.0543431, -0.0177749, -0.0534165, 0.0208121, 0.106746, 0.0613015, -0.0652234, -0.0137682, 0.0168691, -0.0123959, 0.0631786, 0.100651, 0.205275, -0.0043288, -0.0990897, -0.141111, -0.0275515, 0.0150206, 0.217195, 0.067788, 0.0920876, -0.00560163, 0.123143, 0.00399093, -0.0611837, 0.0417096, 0.0189952, -0.0311547, -0.0535601, 0.0569223, -0.0343831, -0.00277072, 0.0074461, 0.022794, 0.0161028, -0.176794, -0.0749908, -0.0453901, 0.102674, 0.0293044, 0.0847828, 0.0156218, 0.025815, 0.0265977, -0.0186084, -0.00289084, 0.108918, -0.0524841, 0.0232232, 0.0650339, -0.00135393, -0.103528, 0.0386368, 0.0312379, -0.0352426, 0.00354564, -0.0799836, 0.0245467, 0.0229331, -0.0205811, -0.0300923, 0.00793215, -0.0377135, 0.0397383, 0.061845, 0.0305356, -0.00314586, 0.000936281, 0.0392823, 0.0976774, 0.0367583, -0.0996204, -0.0502905, 0.0817249, 0.104809, 0.0829781, -0.07791, -0.0619367, 0.0387323, 0.00324297, -0.0807788, 0.0915151, 0.0414494, -0.0122293, -0.10054, 0.0672759, 0.0190633, -4.8032e-05, 0.0778102, -0.0435756, -0.00492072, -0.00926186, 0.00620376, 0.0717117, -0.0250092, 0.120061, 0.0845684, 0.0803906, 0.173494, 0.0762895, 0.0394712, -0.0326247, 0.111274, -0.0318746, -0.0681683, -0.082617, 0.0164027, -0.0943126, 0.0856069, -0.0922089, -0.0322992, -0.0084358, -0.00247208, -0.0770207, 0.0414955, 0.0379649, 0.0713413, -0.0193691, -0.00870401, -0.137119, -0.0195149, 0.0607373, 0.0341674, 0.107042, 0.103447, 0.0153612, 0.000676124, 0.0759439, 0.0226864, 0.0622378, 0.0209776, 0.0557052, -0.094469, -0.0643806, -0.0624199, -0.114385, 0.0673148, -0.00263627, -0.0343532, 0.00226308, -0.0908846, -0.0412584, -0.0802285, -0.00926794, 0.00829128, -0.0334071, -0.0312952, 0.0231821, -0.0742602, 0.0655343, -0.115366, 0.00126816, 0.0164919, 0.0697099, 0.0244206, 0.0161938, 0.0388554, 0.00938441, -0.0109976, 0.00659899, -0.0444585, -0.0868408, -0.104449, 0.00685332, -0.0524561, 0.0212778, -0.00738582, -0.0308493, 0.0462984, 0.0194678, 0.0121099, 0.0588074, 0.00397176, 0.0311574, 0.0174888, 0.0422859, 0.0143378, -0.0316992, -0.136184, 0.0359389, -0.0446028, -0.00802112, -0.0879333, 0.00326748, -0.0410749, -0.0117469, 0.0463096, 0.00807776, 0.0677906, -0.111689, -0.0725879, -0.0836578, 0.0336422, -0.039726, -0.0204965, 0.0491171, 0.0767986, 0.0145131, -0.0438515, -0.0736983, 0.0345066, -0.0485024, 0.0256446, -0.0636065, 0.0245988, -0.0149544, 0.0176651, -0.0642959, -0.0303644, -0.10821, -0.0470374, -0.0651357, 0.0722001, -0.0951993, -0.0609254, 0.0217279, -0.066742, -0.0459854, -0.0119732, -0.00103037, -0.0797836, 0.0218773, 0.00798392, -0.0560796, -0.0507485, -0.0496128, -0.0806787, -0.000355257, -0.080049, -0.0194315, -0.0326421, -0.0978259, 0.0121984, 0.0322815, -0.0700765, -0.0638508, 0.0300799, -0.0451258, -0.0266513, -0.0276765, 0.0809905, -0.0889684, -0.034737, 0.000170335, 0.0978042, 0.000602636, 0.0455864, 0.0221171, -0.0211833, -0.116578, -0.0280447, 0.05357, 0.0120956, 0.0224617, 0.075568,
0.0274166, -0.131752, -0.000372098, 0.0476495, -0.00878494, -0.039954, -0.0276064, -0.00187086, 0.00666718, -0.0252451, -0.00726129, -0.0126522, 0.113085, 0.00172653, -0.0493266, 0.0696719, -0.106492, -0.0352972, -0.0234504, -0.00863116, -0.033368, -0.0602585, -0.0151302, -0.0154695, -0.00896793, 0.0391833, 0.030402, -0.0266683, 0.031595, 0.00996439, -0.0623059, 0.0522887, 0.0319466, -0.046315, 0.0447783, -0.0951107, -0.053491, 0.0171893, 0.0765448, 0.000804895, -0.0269735, 0.0184708, -0.017361, -0.0492276, -0.00139807, 0.0020565, -0.116268, 0.0715809, -0.0426024, 0.0686291, -0.0732201, -0.0156142, 0.0356242, -0.0332953, -0.0548328, 0.00646904, 0.053459, -0.0663403, -0.0485033, -0.00300525, -0.00247589, -0.00679193, -0.00487196, 0.00235763, -0.0315128, -0.027931, 0.0541821, -0.0369419, -0.0682717, -0.00676854, -0.0494364, 0.0241491, 0.0383611, -0.0360132, 0.0776003, -0.0215044, 0.00631759, 0.0378681, -0.0560452, -0.0132257, -0.0517789, -0.0483936, -0.0208648, -0.04973, 0.0146338, -0.0151062, 0.0410516, 0.0281005, -0.0288787, 0.036809, 0.0121961, 0.0156893, -0.0676516, 0.0888335, 0.0664436, -0.00310967, -0.0587792, -0.0223548, 0.0424135, 0.0702967, -0.0374291, -0.00634932, 0.0820843, -0.00112601, 0.126034, -0.0284049, 0.0438188, -0.0133748, -0.0290875, 0.0307214, -0.00373173, -0.0310844, 0.0919732, 0.00278225, 0.0104075, 0.0571791, 0.0406899, -0.00249469, -0.00178064, -0.0924804, -0.0259891, 0.0158385, -0.0647312, 0.107006, 0.0176167, -0.0679427, 0.035203, 0.0820203, 0.068063, -0.00554512, 0.0498408, 0.0899866, 0.159196, 0.239781, 0.0906875, 0.126157, 0.00822602, 0.119481, -0.0813552, 0.0794308, 0.0285252, -0.0707252, -0.047674, 0.0550847, 0.00157163, -0.0194872, 0.0387942, -0.133276, -0.007623, 0.109761, 0.091662, 0.0859612, -0.022276, -0.0405655, -0.0876116, 0.00420647, 0.0882698, 0.0571138, 0.185749, 0.0915117, 0.0931238, 0.0305255, 0.178365, 0.134073, 0.119938, 0.11642, 0.00964926, -0.0518824, -0.0374914, 0.0416253, -0.0491061, -0.0313672, -0.0883562, 0.0268186, -0.0491527, -0.0815495, 0.0294534, 0.011911, 0.00100414, 0.0260338, -0.0249251, -0.0482597, 0.0565182, 0.128039, 0.103043, 0.128372, 0.183176, 0.132536, 0.197723, 0.0387469, 0.0987743, 0.055583, 0.104913, 0.0193422, -0.0392547, 0.0183733, -0.047965, -0.0107853, 0.102957, 0.107422, -0.158265, -0.0283777, -0.0134963, 0.05657, 0.0865558, 0.0706126, 0.124585, 0.0465258, -0.0105903, -0.00298965, 0.0983927, 0.0486404, 0.25017, 0.1352, 0.171772, 0.114932, 0.11521, 0.0321638, 0.0041308, -0.00607296, 0.0727321, -0.0567723, 0.0285353, -0.0137672, -0.0282408, -0.0557716, -0.0161646, -0.0243132, -0.0241828, -0.0368, -0.0943452, -0.0023085, -0.0320135, 0.121583, 0.0380914, 0.00506972, -0.0245713, -0.0603604, -0.0451491, -0.11121, -0.123797, -0.122663, -0.0708003, -0.0948096, -0.0935074, -0.118725, -0.159918, -0.200801, -0.0122655, -0.122953, 0.055907, 0.0557262, -0.0475928, 0.0232044, -0.0096209, -0.0280113, 0.0372896, 0.0302768, -0.0111276, 0.00681859, 0.127895, -0.0595079, -0.0323296, 0.0237415, -0.00956852, -0.0555698, -0.105854, -0.187346, -0.379982, -0.413679, -0.396422, -0.400374, -0.270999, -0.328985, -0.242063, -0.210184, -0.0493188, 0.0314728, -0.0460238, 0.0158935, 0.017506, 0.019316, 0.0654988, -0.0187823, 0.0237346, -0.0374781, 0.0526858, -0.0152042, 0.10805, 0.0800594, -0.0367193, -0.0900591, -0.0224421, -0.0795402, -0.0815933, -0.233917, -0.257451, -0.235335, -0.308941, -0.364331, -0.288152, -0.290281, -0.294527, -0.203455, 0.0151708, -0.0667763, -0.0803941, 0.0481209, -0.0103989, -0.0164049, 0.0345687, -0.0408116, -0.0410751, 0.0517952, 0.0590406, -0.0122599, -0.0477895, 0.0386281, 0.00534069, 0.0408217, 0.0557563, 0.0418846, -0.0212915, 0.118152, 0.0744023, 0.0516979, 0.0461213, -0.155913, -0.227438, -0.204603, -0.224227, -0.11816, -0.187639, -0.0486376, 0.0226382, -0.0403271, 0.0527373, 0.00416658, -0.090325, -0.0268143, 0.0484116, -0.0239175, 0.0451714, -0.0019723, -0.0423186, 0.0247775, -0.0394, 0.0927891, -0.0534023, 0.0239205, 0.062002, 0.0884522, 0.0788144, 0.0520226, 0.205254, -0.00684182, 0.0618452, -0.130203, -0.0485227, -0.0592651, -0.0557224, 0.0222625, 0.0462731, 0.0262846, -0.000740672, -0.0422346, 0.0242845, 0.0307925, 0.0685865, 0.0854125, -0.0590162, 0.0593365, -0.129487, -0.0556824, -0.0289684, -0.0764985, 0.0479903, 0.0546502, 0.0202163, 0.14053, 0.122155, 0.115675, 0.0450062, 0.0965968, 0.109771, 0.00891727, -0.00769238, -0.0579659, -0.114376, -0.114927, 0.0597027, 0.0216956, -0.0196916, 0.0450386, -0.0647268, -0.0178151, -0.0435442, -0.00710851, 0.0568302, 0.0435635, 0.0606053, 0.100506, 0.0346098, 0.0170887, -0.006152, -0.0880914, -0.0140185, 0.0190499, 0.0826408, -0.0165879, 0.175357, 0.112795, 0.0400592, 0.0435288, -0.0221168, 0.00905463, 0.0354385, -0.02897, 0.0588375, 0.0625524, -0.0674243, -0.0186635, -0.030336, -0.120569, -0.0766569, -0.00874958, -0.0708709, -0.0324359, 0.020527, 0.0202493, 0.0242562, 0.0553899, -0.042055, 0.000434212, -0.0464197, -0.0534208, 0.099623, 0.0947837, 0.130577, 0.0832748, 0.100334, 0.108743, 0.0947562, 0.0620886, 0.0507345, 0.0509131, 0.00313202, 0.0147106, -0.105155, 0.0586294, 0.0427198, 0.0598255, -0.058612, -0.0553059, -0.0279464, -0.0635575, -0.0431528, -0.0797583, 0.0201368, -0.151839, 0.125238, 0.0148156, 0.113548, 0.0862989, 0.061366, 0.0334872, 0.0256878, 0.100816, 0.0779077, 0.0252859, -0.00542885, 0.12611, -0.00869536, 0.0337098, 0.0305555, -0.13279, 0.0135823, -0.0385116, 0.013402, 0.0561951, 0.015136, -0.135696, 0.0127158, -0.103097, 0.0519159, -0.0630158, -0.0399987, -0.0645522, 0.05931, -0.00677939, -0.0739926, 0.0522982, 0.0760181, 0.0206991, -0.0891449, 0.108172, 0.0480885, 0.121864, 0.0658336, 0.0541322, 0.0426709, 0.0394064, 0.00688933, -0.00641872, -0.0642594, 0.0719769, 0.0725288, 0.0188052, 0.0463417, -0.0538509, 0.107903, 0.0242095, 0.0947863, -0.0791948, 0.0520811, -0.00406318, -0.0713881, -0.0425361, 0.077376, 0.0303834, -0.0635695, -0.10131, 0.0166619, 0.0118907, 0.0797365, 0.0718716, 0.051373, -0.0426137, 0.0136999, -0.0289088, 0.0153171, -0.0199951, -0.0416546, -0.0452784, -0.0780556, -0.00528134, 0.00404491, 0.00408766, -0.0720498, -0.103085, 0.0155709, -0.0133137, -0.00524032, 0.0595473, 0.143799, -0.052171, 0.0169919, -0.0740362, -0.0709995, -0.00402081, -0.0186499, -0.0149041, 0.0122368, 0.0154322, -0.0161799, 0.0261735, 0.0833797, -0.00149286, -0.0735174, -0.0955238, -0.0515627, 0.0504901, -0.0470735, 0.0855734, 0.0951739, 0.103685, -0.0663198, -0.166362, 0.0337215, 0.0157882, 0.0158332, 0.0416307, -0.0320009, 0.0745649, -0.114285, 0.00911523, -0.0180411, -0.032071, -0.109133, -0.0305988, -0.0446227, 0.051465, 0.0871407, 0.059546, 0.00800912, 0.0675297, -0.0188518, -0.0822612, 0.0328094, -0.0445037, -0.0680788, -0.0237609, 0.0834623, 0.0830886, 0.017033, 0.0383773, -0.0778699, 0.0132111, -0.0654361, 0.001337, 0.00474895, -0.0474794, -0.0631071, 0.010896, -0.0136141, -0.0614802, -0.053519, 0.0693075, 0.0835077, 0.0288607, -0.0377937, -0.0405292, 0.0857319, 0.0262223, -0.0333753, -0.00223361, 0.0801468, -0.0869378, 0.0113991, -0.0237367, 0.0936489, 0.00305334, -0.198433, -0.0684208, -0.00708194, 0.0699385, 0.0389381, 0.0407205, 0.00860633, 0.036051, 0.0896519, 0.0134179, 0.0285458, 0.00258592, -0.0110985, 0.0418116, -0.0380338, 0.0468352, 0.0907909, 0.064938, -0.00741369, -0.0387039, -0.0224713, -0.069781, 0.0508517, -0.0711775, 0.0570622, -0.0197045, -0.00944086, 0.0091897, -0.0654538, -0.0940686, 0.030178, 0.0195207, 0.0604772, 0.0204398, 0.0010899, -0.0677526, -0.0463721, -0.0343978, -0.0150482, 0.00128683, -0.0772786, 0.0298033, 0.0498723, 0.111094, -0.0466884, 0.0642471, -0.0221105, 0.147512, -0.0251052, -0.0248833, -0.0353329, 0.0258227, 0.0565608, 0.0791145, -0.0323704, -0.0217064, -0.0373068, 0.0222503, -0.0416049, 0.110987, -0.0442792, -0.0308791, -0.0361498, 0.0601463, 0.024432, 0.0186482, -0.0307877, 0.0315656, 0.0300166, 0.00407898, -0.00596065, 0.0584723, 0.0134591, -0.0706559, 0.0505188, -0.0699801, -0.0840735, -0.0319479, 0.0491383, -0.0241375, -0.0973377, 0.0586767, -0.0215596, 0.0208712, 0.110372, 0.0496036, 0.0368885, 0.00746513, 0.0369065, 0.0940127, 0.12039, 0.107125, 0.052711, 0.0963988, 0.0657452, 0.026744, 0.00133868, 0.164263, 0.119419, 0.0405738, -0.0369319, 0.0353507, 0.0523393, -0.0363406, -0.0743853, 0.0140775, 0.0547125, -0.013106, 0.079972, 0.0635612, 0.0118018, 0.00562279, -0.00436371, 0.0621904, 0.0197579, -0.00532803, 0.00678761, 0.043109, -0.0165847, -0.0618769, 0.0256152, -0.0960357, -0.00614657, -0.0232329, -0.00154157, 0.0471088, 0.0225134, 0.0444506, -0.0635335, 0.0701096, 0.0297353, -0.126292, -0.0374433, -0.061247, 0.0933362, -0.0846573, -0.110241, -0.0228682, 0.0984109, 0.000305067, -0.069695, -0.00442138, -0.0173208, -0.0513768, -0.136256, 0.0136889, -0.0569643, -0.0231023, 0.0548648, 0.0736213, -0.00105314, -0.0106782, 0.039119, -0.044785, -0.0290624, -0.00480897, -0.0501488, -0.0737227, 0.11281, -0.0420705, -0.0570919, 0.0345706,
0.0103253, -0.0170348, 0.0149459, 0.0120042, 0.006557, 0.0161164, -0.0699664, -0.0457158, -0.0718048, -0.0635844, -0.0589131, -0.0199283, -0.00245488, -0.0140922, -0.0017897, 0.0752853, 0.088307, 0.00675936, -0.0224451, 0.0516339, -0.0183239, 0.0573645, -0.00843321, 0.0820803, 0.113198, -0.0722872, -0.0611161, 0.0996845, -0.0332708, -0.0420866, -0.00604027, -0.0218148, -0.0280556, 0.00714424, 0.0408977, 0.0141185, 0.0389624, -0.054307, 0.0306667, -0.0166022, -0.0470972, 0.0460797, -0.071581, -0.0770303, 0.0360493, 0.00105768, 0.0449368, -0.0215085, -0.0259429, 0.00488149, -0.0180066, 0.066253, 0.0184422, -0.070329, -0.168687, -0.0320092, -0.00277861, -0.0988041, -0.0604375, 0.0462548, -0.00309699, 0.149728, -0.0292928, -0.0359699, -0.0471023, -0.0360919, -0.000778576, -0.0371097, -0.0350412, -0.132429, 0.0513385, -0.0232977, 0.0485217, 0.0287084, -0.025831, -0.0684191, -0.0378772, -0.0755266, -0.0529778, -0.00965375, 0.0811495, 0.076532, 0.0341085, -0.0709771, 0.0564345, -0.0579784, 0.0122203, -0.00881981, 0.0195086, -0.108548, 0.09164, -0.0440721, -0.104909, -0.0172495, 0.0746453, 0.0756757, -0.0218222, 0.00878095, 0.0554971, 0.0292001, -0.143808, -0.0852267, -0.155528, -0.0644451, 0.0468232, -0.0953218, 0.0191429, 0.01021, 0.064824, 0.0932409, 0.122288, 0.0238985, 0.0246639, 0.0278514, -0.0701962, -0.118395, -0.0999982, -0.0082189, -0.0167374, 0.0293949, -0.0666228, -0.0300655, 0.0986993, -0.077511, 0.0694011, 0.0506671, -0.044296, -0.0695151, -0.064948, -0.0315979, -0.0417266, 0.0614947, 0.0200211, 0.0210517, 0.042867, -0.0743151, -0.135723, 0.0823285, -0.00307936, -0.00701373, 0.00114612, 0.0142181, -0.0705627, -0.0224604, 0.102331, -0.035566, -0.0662607, 0.0450785, -0.0900721, 0.0266996, 0.0533988, 0.125065, 0.071578, 0.0885193, 0.0568605, 0.0589167, 0.0527236, -0.0405734, 0.0556062, -0.0661961, 0.0285943, -0.0162348, -0.0123131, 0.0236768, 0.0484961, 0.0287545, 0.0270073, -0.0172325, 0.0307725, -0.0205347, -0.0552763, 0.0729332, 0.00415784, 0.0206664, 0.00439176, -0.051262, 0.0320491, 0.0158566, 0.0343036, -0.00540647, 0.0759672, -0.110865, 0.00702265, 0.035412, 0.0312705, -0.0157321, -0.0573295, 0.114487, 0.0474376, 0.0872209, -0.103287, 0.000660643, -0.00454186, -0.046082, -0.0405345, 0.0537885, -0.0190102, 0.0634242, 0.00134078, 0.0184001, 0.0116748, -0.0287043, -0.00953684, 0.0194587, 0.0756699, 0.0070621, 0.0627379, -0.144751, -0.0627567, -0.0746467, -0.17744, 0.0330818, -0.0538921, -0.0888163, 0.0936586, 0.0352302, 0.0584145, 0.105674, 0.0171095, 0.0613811, -0.0533731, -0.000770559, 0.093784, 0.0354509, -0.0200661, 0.0590752, -0.0515134, 0.0315094, 0.0715978, 0.0617194, 0.106515, 0.2152, 0.0463957, 0.153436, 0.108363, -0.103188, 0.0271156, 0.0749577, 0.0340081, -0.0459497, -0.0887023, -0.00479678, 0.0289759, 0.11833, 0.0346891, -0.0389101, 0.0410693, -0.0159062, 0.00172868, -0.0441963, 0.0497587, 0.0199995, -0.0417667, 0.00360278, 0.0434371, 0.0137257, -0.0407602, 0.0210599, 0.0737094, 0.107723, 0.0655236, 0.0683687, -0.0736677, -0.0961186, 0.0277721, 0.0707691, 0.0628679, 0.0588467, 0.0382998, 0.0663058, 0.0709314, 0.0354756, 0.0184056, 0.00718136, 0.00894612, 0.0397202, -0.0308545, 0.00337752, -0.0156178, 0.0249388, 0.0207037, -0.00635497, -0.0310905, 0.00505789, 0.0889112, 0.0428436, 0.00416871, 0.0446509, 0.0149241, 0.0182384, 0.217545, -0.00607302, -0.0326928, 0.0313839, -0.0507775, 0.0614367, 0.041507, 0.0745283, 0.032042, 0.00971022, -0.109571, 0.0221453, -0.0182592, -0.139358, -0.0290351, -0.00389133, -0.082341, 0.00675825, -0.0378196, 0.021465, 0.0833925, -0.110834, 0.040869, 0.101123, -0.0261104, 0.0160963, 0.0824291, 0.137201, -0.0475113, 0.0869356, -0.0818707, -0.132543, -0.0560603, 0.0234091, -0.00984021, 0.0459894, 0.00847638, 0.0247479, -0.0429261, 0.0132219, -0.0645187, -0.132983, -0.0567115, 0.0299169, -0.0956953, -0.0149105, 0.0600656, -0.0219886, -0.07398, 0.0291858, -0.0802284, -0.0543638, -0.095403, 0.0405386, 0.12345, 0.0181135, -0.143969, -0.174993, -0.092046, -0.184826, -0.0514123, -0.092512, -0.015908, -0.0718393, -0.0721086, -0.0905415, 0.000768522, -0.114474, -0.0650675, -0.130016, -0.024903, -0.0707374, -0.0595691, -0.0180529, -0.0302458, -0.0233894, 0.00130006, 0.0200682, -0.0642798, -0.125838, -0.0663802, -0.0780217, -0.0623722, -0.0307463, -0.135664, -0.10393, -0.158987, -0.134912, -0.0596188, -0.111139, -0.126019, 0.0260248, -0.008531, -0.119792, -0.0406278, 0.000869294, -0.0150879, -0.0245026, -0.0225556, 0.0112421, -0.0145826, 0.0586571, 0.0233916, 0.0459737, -0.0458083, -0.0734871, -0.0716844, -0.0189557, -0.198221, -0.108464, -0.0618405, -0.135804, -0.106686, -0.045089, -0.108205, 0.00426007, -0.0534534, -0.0727127, -0.0659914, 0.0420465, -0.0132229, -0.0918116, -0.0619003, -0.133316, -0.0130199, -0.0772095, 0.0289814, 0.0710932, 0.0925842, -0.0968896, 0.0715213, -0.0343131, 0.031124, -0.0206561, -0.174606, -0.202314, -0.0766633, -0.168166, -0.0807297, -0.0613014, -0.0909859, 0.0446052, -0.0532791, 0.120712, 0.0790784, 0.0956835, -0.115459, -0.00553007, -0.0859594, -0.0975168, -0.0990639, -0.0205943, -0.123395, 0.00393289, 0.0322677, 0.0728321, 0.0282656, 0.00891939, -0.0601105, 0.057085, -0.0325467, -0.0917478, -0.0507714, -0.101118, -0.073313, 0.00444271, -0.041461, 0.0200173, -0.0537836, 0.0548844, 0.101868, 0.11075, 0.269355, 0.100577, 0.21241, 0.0249061, -0.026177, 0.026455, 0.0150432, 0.150101, -0.0243266, 0.127232, -0.0151517, 0.0366274, 0.0363264, 0.00569257, -0.0363912, -0.0565885, -0.0507794, 0.025019, 0.0188967, 6.92959e-05, 0.0211987, 0.0128777, 0.0936697, 0.152027, 0.0939169, 0.111728, 0.20884, 0.208394, 0.223315, 0.106806, 0.0581592, -0.00225766, 0.0197946, -0.0416772, 0.0177852, 0.050757, 0.072598, 0.0313479, 0.130375, 0.0219115, -0.0199038, 0.0785472, -0.00909861, 0.0191457, 0.0498634, 0.0423568, 0.0162067, -0.0119712, -0.0556017, 0.0968387, 0.0910619, -0.0244562, 0.166706, 0.0553781, 0.186217, 0.226431, 0.19977, 0.133705, 0.0102673, -0.0299659, 0.0144901, 0.0249273, 0.0246109, 0.0342879, 0.0199028, 0.119699, 0.0877614, 0.0627532, 0.0181343, -0.000684804, -0.0315864, -0.0248953, 0.0648262, -0.0155499, 0.037255, -0.023439, -0.00890045, 0.0281712, 0.0201932, 0.115287, 0.126722, 0.0354207, 0.0214318, 0.297746, 0.1423, 0.0404824, 0.0766809, 0.115711, 0.0111825, 0.0210393, 0.0072514, -0.0880328, 0.0310505, 0.0471178, 0.113215, 0.0713511, 0.0421245, 0.0435886, -0.0871732, -0.0233179, -0.0263507, -0.0504847, 0.0659594, -0.0272759, -0.105354, 0.0514881, -0.00656574, -0.0421503, 0.012881, 0.0222322, 0.0288078, 0.0920665, -0.0216008, 0.140215, 0.0248158, 0.0854081, -0.019843, -0.0760091, -0.0152018, -0.0639315, 0.0307561, 0.0947213, 0.0118582, 0.00962203, 0.0239269, 0.00870388, 0.0699427, -0.0882669, 0.0243559, 0.0917462, 0.0188337, 0.0728129, -0.0781765, -0.0315179, -0.105293, -0.0347307, -0.0733998, -0.102816, 0.0030388, -0.00728796, 0.12037, 0.149338, 0.0801484, -0.00447315, -0.0329012, 0.0583784, 0.0403406, 0.0539718, -0.0268114, 0.0921573, 0.0790199, -0.0280997, -0.0297268, -0.0291969, 0.0760947, 0.0584618, 0.022855, 0.0481917, 0.0111608, 0.163928, 0.0265232, 0.0935671, 0.0394864, 0.0707299, -0.0706092, 0.0148327, 0.0114682, 0.098869, -0.0323502, 0.0917302, 0.0850666, -0.0469913, -0.0485724, -0.000411406, 0.0267199, 0.0242159, 0.084458, 0.0380717, 0.049125, -0.0185954, -0.0516803, -0.010408, -0.0714283, 0.000946944, 0.0318642, -0.0746144, -0.00235863, -0.0429515, -0.0276084, 0.0786225, 0.0307297, 0.0873711, 0.0645028, -0.0130223, 0.0918526, -0.00178955, 0.0623491, 0.0714389, 0.0982555, 0.0612409, 0.0804126, 0.0748421, 0.0972092, 0.103964, 0.025662, 0.0488779, 0.0583687, -0.104759, -0.0166771, -0.0385, 0.00686855, -0.109196, -0.00979066, -0.0748822, -0.00956184, -0.0580777, -0.033391, -0.0454714, -0.0806724, -0.00893932, -0.0917969, 0.0293379, -0.0519327, 0.0411488, -0.0190792, 0.117802, 0.00502387, 0.0429921, -0.0205394, 0.0462668, -0.0539237, 0.0356378, -0.0956526, 0.0179568, 0.0288421, -0.0230465, -0.0163699, 0.00716107, -0.0447705, -0.0291824, 0.0728455, 0.0423768, 0.0366664, -0.036245, 0.0110549, 0.0318326, -0.112354, -0.0224111, -0.0779042, -0.115722, -0.211869, 0.00172, 0.0180193, 0.0732191, -0.0300107, -0.0116816, -0.0629626, -0.0456333, -0.0489744, -0.093189, -0.0310487, 0.106335, -0.0033146, 0.0410615, 0.0630198, 0.00507666, 0.0360806, -0.086843, -0.0471724, 0.011278, 0.0253463, -0.0239313, -0.163007, -0.0252815, -0.0580171, -0.0316332, 0.0136189, -0.103855, -0.0807816, -0.0199507, 0.0129271, -0.0112563, -0.0420337, -0.0544611, 0.0558218, 0.0673618, 0.0258048, 0.00302725, -0.0997016, -0.0145912, -0.0299799, 0.0184739, 0.153065, 0.0270635, -0.0570722, -0.0289736, 0.00533757, 0.120218, 0.0449525, -0.00753431, 0.0713086, 0.00795953, -0.0811748, -0.0504986, 0.00277443, 0.101159, 0.0464056, -0.0288929, -0.0593917, -0.00229009, 0.049582, 0.0570373, -0.00613821, 0.0636349, 0.019768, 0.0121866, -0.00364872, -0.0649787, -0.00100415, 0.0925862, 0.0222449, -0.0267501,
-0.0363666, -0.0273418, -0.0333863, 0.056985, 0.0895482, -0.0544085, -0.0684158, 0.0212857, 0.012885, 0.0502806, 0.0391229, 0.0705099, 0.0185971, -0.0645797, -0.0762287, 0.0784859, 0.18385, -0.0531778, -0.146384, -0.0571994, -0.0503604, -0.0430524, -0.0835973, 0.0401797, 0.0154151, -0.00751776, 0.028543, -0.0487302, -0.0116359, -0.0222713, 0.0342215, 0.0160051, 0.0279985, 0.0636089, -0.0756584, -0.0155273, 0.0367188, 0.144074, -0.0282694, 0.068968, 0.0195288, -0.013842, -0.0171545, 0.0274986, -0.0840567, -0.0372904, 0.0536911, 0.027563, 0.0406548, -0.0236312, 0.0586199, 0.0292001, -0.0231545, 0.0844291, 0.0231365, -0.00759669, 0.0932801, -0.0197953, 0.0146792, 0.0215477, 0.0364741, -0.00615038, 0.0855516, -0.0109717, -0.0312128, -0.0109406, 0.0208896, 0.0745429, -0.0690038, 0.0438868, -0.038905, 0.0214846, -0.0673487, -0.0426812, 0.0372384, 0.0258472, 0.010163, -0.0288477, 0.0215661, 0.0356867, -0.00536132, 0.0559665, 0.00699488, -0.0104225, 0.0652414, -0.00792229, 0.0116409, -0.0286935, 0.0254271, -0.0730782, -0.0244422, 0.0369848, 0.0913093, -0.0508707, 0.0162921, -0.00921591, 0.0721655, -0.0483312, -0.0792541, -0.00698779, -0.0562309, -0.0926269, 0.0418129, 0.0223518, -0.0656846, -0.000805014, -0.0789228, -0.0245426, 0.0503572, -0.0338237, 0.0492447, -0.00996148, 0.0169031, 0.157609, 0.0102587, -0.0156334, -0.0272283, -0.0369226, -0.0346352, 0.0240517, 0.0196705, 0.0220864, -0.0701138, 0.0180367, 0.00540015, -0.041722, 0.0522167, -0.0153546, -0.0249244, -0.0344999, -0.0199139, 0.0402745, 0.101092, 0.0311735, 0.0648855, -0.0287212, -0.0440728, -0.0167732, -0.00188511, 0.0453054, -0.0457351, 0.0192568, 0.00293281, 0.0135762, -0.116323, 0.00182423, 0.018846, 0.0429345, 0.0633329, -0.018654, -0.0894714, -0.069004, 0.0512803, -0.0241382, -0.0392977, -0.00277814, 0.0729932, 0.0401455, 0.0127657, 0.0628207, -0.0941527, 0.0527193, 0.0971035, -0.0746747, -0.0113203, -0.0332708, -0.0638198, -0.000477628, 0.0595512, 0.00411085, 0.0147348, -0.0569336, -0.0447236, 0.040861, 0.0025419, 0.0721172, 0.11809, 0.024876, 0.125545, 0.0383805, 0.0330326, 0.0551662, -0.0962089, -0.0815137, -0.0201406, 0.00648826, 0.0528683, 0.156014, 0.0651708, 0.0765892, -0.015634, -0.0795728, -0.0166302, -0.0678155, -0.0498372, 0.0730077, 0.103057, -0.0355423, 0.022634, 0.125015, -0.0841331, -0.0328171, -0.0917972, -0.0223861, -0.0512894, 0.0405605, -0.000217777, 0.0147227, 0.0210595, 0.0559637, -0.00786969, -0.0533771, 0.00718827, 0.0626302, -0.006146, 0.0425779, 0.0445098, 0.00819968, 0.0828942, 0.0040797, 0.00669356, -0.0124775, 0.0775261, 0.116599, -0.0103241, -0.00139197, 0.0543416, 0.0198075, -0.0211951, 0.0698688, -0.0692323, -0.0420649, -0.0592551, 0.013829, 0.0132667, 0.0208713, 0.106684, -0.0479099, -0.0198151, -0.0811894, -0.0231944, 0.140584, 0.0560324, 0.152021, -0.0519159, 0.0700412, 0.0431645, 0.0395503, -0.0788782, 0.0404822, 0.0453671, 0.133447, -0.130188, 0.0435844, -0.0077439, -0.0120096, -0.0211589, 0.0257236, -0.0425918, -0.000717617, 0.0653617, -0.0810778, -0.135946, -0.0378961, -0.00206386, -0.0738218, -0.0581104, -0.0174716, 0.104253, 0.121616, 0.0560037, 0.0137313, -0.0285324, -0.0598075, -0.0877051, -0.207636, -0.0221312, 0.0463097, 0.0703951, 0.0753643, 0.0591615, -0.0228974, 0.0445052, 0.0266579, 0.0463036, 0.119323, -0.0545979, 0.0427782, 0.00511775, -0.0742984, -0.00521481, -0.128106, -0.224262, -0.114568, -0.0155079, 0.162411, 0.180449, 0.157368, 0.0533656, 0.014285, -0.0137312, -0.0427668, -0.0906629, -0.0660766, 0.021034, 0.0247739, 0.0509375, -0.0419671, -0.0820766, -0.0878619, 0.0342476, 0.0151421, -0.00176119, 0.0438566, -0.0547082, -0.0684207, 0.0277653, -0.0335674, -0.101685, -0.177238, -0.181783, 0.0100666, 0.160736, 0.0453207, 0.0733097, 0.0571659, 0.0111663, 0.00186977, -0.0198537, 0.0817654, -0.0858166, -0.146552, -0.0459181, 0.00415806, -0.0386548, -0.0727419, 0.000752084, 0.0119692, -0.10925, -0.0780832, -0.0382551, -0.101785, 0.00576278, 0.00800151, -0.114569, -0.0525206, -0.226234, -0.0613065, -0.0282817, 0.111657, 0.141573, 0.0805667, 0.0298529, -0.0512034, 0.000903326, -0.0720379, -0.0301011, -0.115587, 0.0508344, -0.0335201, 0.0693851, 0.00332241, 0.0398633, 0.0717305, 0.0110702, -6.14029e-05, -0.00184058, -0.0109332, 0.0253402, -0.0181907, -0.172632, -0.0347545, -0.110106, -0.136829, -0.157252, -0.109876, 0.0204984, 0.0324363, 0.107762, 0.0225871, 0.0847945, 0.0220248, -0.00892769, -0.102041, -0.000682364, 0.0415465, -0.156785, -0.0283382, 0.0467725, 0.0610599, -0.0273848, 0.177401, -0.0673267, 0.0325756, 0.0578805, -0.0700544, 0.0492931, -0.0536981, 0.0532127, -0.13646, -0.0683788, -0.0308383, -0.0643473, 0.0357011, 0.0778681, 0.192926, 0.0631673, 0.0567777, 0.0958866, -0.0223071, -0.00462409, 0.081436, -0.0258227, 0.113641, -0.0106016, 0.0278493, -0.0603738, 0.0263913, 0.0402319, -0.114507, -0.0285151, 0.0593272, -0.126057, -0.0945684, 0.106332, -0.015055, -0.059681, -0.0570218, -0.0530614, -0.0305189, 0.0436533, -0.00630103, 0.162444, 0.0655802, 0.0969407, 0.0555115, -0.0114791, 0.145264, 0.179613, -0.0264971, 0.117684, 0.0849684, 0.00488727, 0.0010603, -0.0192493, -0.0419377, -0.0683747, 0.0338915, -0.0396987, 0.00906298, 0.071108, -0.00681251, 0.00496653, -0.0361717, -0.0375569, -0.0490884, 0.0577548, 0.0224054, -0.00957607, 0.00700624, 0.23945, 0.219444, 0.12595, 0.0168233, -0.0519386, 0.109758, 0.0914982, 0.119881, -0.00855992, -0.0173896, -0.0571859, 0.0546022, 0.0918514, 0.00199211, -0.00236858, 0.0213268, -0.0522485, -0.0621512, 0.0215162, -0.0191245, -0.0539575, 0.00296839, -0.0275279, 0.070145, 0.0361676, -0.0361003, -0.0717708, 0.0964823, 0.208713, 0.127825, 0.0525817, -0.0759303, -0.0595685, -0.0135735, 0.0450483, 0.0629018, -0.0726468, 0.0292165, -0.0522741, 0.119385, 0.0769297, 0.100844, -0.0448426, -0.0235248, 0.0290636, -0.0318484, 0.0195748, -0.0883663, -0.0553699, -0.114963, 0.0248088, 0.077736, 0.0209987, 0.00349997, -0.0103545, 0.106898, 0.213424, -0.0458593, -0.0858059, -0.0926448, 0.00692408, 0.000985761, 0.00261215, -0.000220258, 0.0718419, -0.0270829, 0.0255522, 0.0593095, 0.052197, -0.00616091, -0.0249373, -0.00425497, -0.113734, 0.0222854, -0.0105449, 0.0788692, 0.050023, -0.0313653, -0.0887994, -0.00263531, 0.0849541, 0.042345, 0.0412464, 0.0337419, -0.0271927, -0.0141917, -0.14518, -0.104952, -0.156903, -0.171639, -0.198586, -0.129564, -0.0273739, -0.021726, -0.187575, -0.0305546, -0.0169673, -0.0762047, 0.0179263, -0.0754719, -0.0263711, -0.0664137, 0.0287218, -0.0265073, 0.0100781, -0.00589782, -0.113795, -0.0492375, -0.0923879, -0.0168909, 0.00642143, 0.0800278, 0.0439506, -0.0330768, -0.0260398, -0.171461, -0.109484, -0.117129, -0.139659, -0.0854363, -0.0237754, -0.0158415, -0.113919, -0.0494424, -0.0476192, 0.0643183, 0.00973219, -0.0445888, 0.022901, 0.0933588, -0.0338896, 0.0340735, -0.0432395, 0.016437, 0.0721089, -0.0243018, -0.155433, -0.0148893, 0.0414325, -0.00840073, 0.0809146, -0.00544509, -0.0580028, -0.0651729, -0.112877, -0.21338, -0.132951, -0.0880674, -0.00221202, -0.0754921, -0.00281811, -0.0401152, -0.0506047, -0.0855043, -0.00918263, 0.00290449, 0.0387682, 0.0229756, 0.0631836, 0.00810595, 0.0367962, 0.0144877, 0.0915113, 0.0736551, -0.0253258, -0.0379466, 0.0617091, 0.0473577, 0.0467376, -0.0185051, 0.0376658, -0.0257827, 0.122973, 0.0449337, -0.0738688, 0.00988344, -0.026714, -0.0394648, -0.0535165, -0.022128, 0.0510006, 0.0540323, 0.0573474, -0.00318212, -0.0221206, 0.0443781, -0.0061726, 0.0265542, 0.0197792, -0.00382042, 0.0794146, 0.0257629, -0.03924, 0.00254084, 0.0954224, 0.0940887, 0.08737, 0.160659, 0.101345, -0.0416036, -0.0909404, -0.0337815, -0.115442, -0.0408072, -0.081506, -0.0939143, -0.0645366, -0.053545, 0.0054599, -0.0796065, -0.0544966, -0.0671318, -0.049354, 0.0664248, -0.0130159, 0.00598266, -0.00256524, -0.0779297, 0.0952226, -0.0716137, -0.00836569, -0.016517, 0.0304915, 0.10144, 0.12027, 0.0930815, 0.0748238, -0.0641731, -0.088309, -0.161279, -0.205058, -0.0524991, -0.0800466, -0.0785959, 0.0241711, 0.0270576, 0.0198643, -0.102269, 0.00344521, 0.0639402, 0.0327724, -0.00768987, 0.0306165, 0.125991, 0.013697, 0.0724021, -0.0383281, -0.136436, -0.100075, 0.0475794, -0.0911837, -0.0346581, -0.058467, -0.0990538, -0.0676217, -0.0529596, -0.104821, -0.168525, -0.11858, -0.0595619, -0.0733998, -0.0609113, -0.0754917, 0.0152773, -0.104972, 0.0293059, 0.0231922, 0.106024, -0.00861442, -0.0282691, -0.0183994, 0.0546142, -0.125692, -0.0152188, 0.0689118, -0.0596087, 0.0548647, -0.00914875, -0.00855179, 0.00935683, -0.0688293, -0.0721237, 0.0747239, -0.0867181, -0.105264, 0.052452, 0.0271169, -0.0323299, -0.00663789, -0.0379387, -0.0273324, -0.07329, -0.0230691, 0.050531, -0.0241087, -0.0709907, 0.00312839, -0.0254021, 0.0317018, 0.0781543, 0.0558342, 0.0115499, 0.0412236, 0.0691568, 0.000777683, -0.0303669, 0.0433732, 0.0223995, 0.044294, 0.045801, -0.060665, 0.0580331, -0.0820858, -0.0175466, -0.012089, 0.0575898, -0.0130023, 0.0618584, -0.0400424, 0.0572963, -0.0366807, -0.0527569, -0.0133208, 0.123774,
0.0167832, -0.0347578, 0.0254965, 0.0265201, -0.000928791, 0.0437301, 0.0499096, 0.0969503, -0.0476742, 0.0371195, 0.0268122, -0.0634201, -0.0156836, 0.00425826, -0.0251935, -0.0119612, -0.0389856, 0.041564, 0.0385038, 0.00753081, 0.0214734, -0.0506932, 0.0931426, 0.016733, -0.0115809, -0.0178086, -0.0830814, 0.00163616, -0.0387047, 0.170744, 0.0415314, -0.0406751, 0.143492, -0.0494623, -0.0733878, 0.0214538, -0.03703, 0.0196136, -0.0095616, 0.0244545, -0.0273408, 0.0697115, -0.0386417, -0.0368822, 0.0142666, 0.0223932, 0.0540218, 0.0273436, 0.0418154, -0.00941087, 0.00451112, -0.135412, -0.0141934, 0.0117019, -0.0977364, 0.145453, 0.0511083, -0.017479, -0.0172457, -0.06631, -0.0723454, 0.0705772, 0.0574935, 0.0126019, 0.0655475, 0.0443681, -0.034818, 0.0175251, 0.0520531, -0.00708144, -0.0142337, -0.10441, 0.0225522, -0.0224831, -0.0285538, -0.00265484, -0.146496, -0.0553268, -0.0456344, -0.0505297, 0.0636161, -0.0561957, -0.0141988, -0.062629, 0.0655519, -0.0164521, -0.0195791, -0.0545885, 0.0295724, 0.0105985, -0.00530022, 0.000310592, -0.128919, 0.0172887, 0.0567092, 0.0437202, 0.038108, 0.126171, 0.0991111, 0.0645753, 0.00452569, -0.0571769, -0.045348, 0.0221137, 0.0211875, -0.0292471, -0.0726565, -0.0328889, -0.0076796, 0.0208735, 0.0607204, -0.0103887, 0.00684028, -0.0328003, -0.0347741, -0.00366681, -0.0152081, -0.00256317, 0.0598969, -0.0275677, 0.0188172, 0.0813146, 0.0517624, 0.187463, 0.146993, 0.159571, 0.0717194, 0.0897284, 0.0933672, 0.0657956, 0.133326, -0.00386947, -0.1513, -0.0111166, 0.00489524, -0.134483, -0.0616555, -0.0147819, -0.00116594, 0.00110561, -0.0392838, 0.137219, -0.0493148, -0.0878817, -0.00659142, 0.000147711, 0.0606693, 0.00998006, 0.081645, 0.137462, 0.0608502, 0.0779077, 0.173419, 0.186728, -0.0171064, 0.0805244, -0.0411057, 0.0120599, 0.102751, -0.0226858, 0.076087, 0.0224432, 0.0354394, -0.102932, -0.0973321, 0.0482341, 0.0437183, 0.0565773, 0.0240475, -0.00844222, 0.0178439, -0.0452626, 0.0101396, 0.0642619, 0.099921, 0.135477, -0.0575313, -0.0244329, 0.105706, 0.100619, 0.11712, 0.190833, 0.0861002, 6.46568e-05, -0.0498591, 0.0119856, 0.161256, 0.0296144, -0.0004756, -0.0277205, 0.0505263, -0.0432986, 0.0306762, -0.0540774, -0.0153366, 0.0471671, -0.0636163, 0.0501415, -0.0310737, -0.0118514, 0.0731492, 0.00593099, 0.121273, 0.0899206, 0.0483265, 0.034849, 0.0777584, 0.167315, 0.14439, 0.174146, 0.139017, -0.0171284, -0.026424, 0.00563884, 0.0031674, 0.0630239, 0.032048, 0.0464587, -0.0605734, -0.0424972, 0.0773603, -0.0688686, -0.021452, -0.0441961, 0.0785682, 0.0168017, 0.0131931, -0.0374745, 0.0702864, -0.044836, 0.0441156, 0.0139825, -0.0901389, 0.0343781, -0.0314241, -0.109745, -0.0773582, -0.11337, -0.141592, -0.00525094, 0.0250142, 0.022886, -0.00159816, 0.020282, -0.0642272, -0.0414009, 0.0129407, 0.0657271, -0.0199356, -0.00587543, 0.0621903, -0.0596939, -0.037107, 0.0183394, 0.0152965, 0.000707492, 0.0238652, -0.0407317, -0.165523, -0.184868, -0.0967821, -0.209464, -0.405087, -0.426636, -0.290377, -0.204494, -0.0438078, 0.0174593, 0.0374653, 0.0478639, 0.154752, -0.0613426, -0.0916601, -0.0204074, 0.0449593, 0.0353944, -0.0674788, 0.040039, 0.0428192, -0.00127617, -0.0145446, -0.00516148, -0.00631314, -0.0744165, -0.153989, -0.262085, -0.283946, -0.284732, -0.396546, -0.416282, -0.406939, -0.282174, -0.224002, -0.125014, -0.0266317, -0.0384513, 0.1045, 0.0405231, -0.014401, -0.0977689, -0.0314565, -0.0134135, -0.00891637, -0.0547446, -0.0264495, 0.0219322, -0.00704255, -0.0437942, -0.00199357, 0.0125319, 0.018365, 0.00465578, -0.1071, -0.154876, -0.210539, -0.285787, -0.246258, -0.28283, -0.0892243, 0.0527411, 0.222583, 0.135156, -0.0566992, 0.113423, -0.0754402, -0.00607151, -0.0403826, 0.0633649, 0.0294744, -0.0339237, 0.0186525, 0.00144317, 0.0232204, 0.0361657, 0.00669406, 0.0654716, 0.0285898, 0.00770726, -0.0480458, -0.0450936, -0.15704, -0.120263, -0.225447, -0.163504, -0.110866, 0.13547, 0.203947, 0.152086, 0.176771, 0.138694, 0.0475292, -0.0995219, -0.0236725, 0.0145299, -0.0192334, 0.0191427, 0.0101014, -0.00228204, -0.000235001, 0.0552808, 0.0481729, 0.114535, 0.0224924, -0.000418256, 0.00612901, 0.00272288, -0.0293237, -0.0459747, -0.0288702, -0.00374868, -0.0527242, 0.10817, 0.173848, 0.195138, 0.292539, 0.108104, 0.108784, 0.082151, -0.0641254, -0.0179281, -0.0204831, -0.103581, -0.029878, -0.0101592, 0.0751512, 0.037193, 0.139164, -0.11096, -0.0286594, 0.0225427, 0.0103366, 0.0429316, -0.0254969, -0.0373818, 0.0680939, 0.0652377, 0.132281, 0.0076824, 0.161753, 0.189494, 0.209962, 0.153124, -0.00329456, 0.0143132, -0.0383059, -0.0637619, 0.0533844, 0.0726905, 0.0285086, 0.00830423, -0.0492064, 0.00398557, 0.0554073, -0.043591, -0.038302, 0.139646, 0.0468683, 0.0809205, -0.0192741, 0.0960819, 0.0372681, 0.0437489, -0.0116618, 0.0477467, 0.128588, 0.0519743, 0.15012, 0.140816, -0.0602859, -0.0750563, -0.0233454, -0.0275081, -0.0575478, 0.0405064, -0.0620528, -0.0205259, -0.022949, -0.069332, -0.0338321, -0.0848097, -0.0422901, -0.0573147, -0.0141292, -0.0683137, 0.00749001, 0.0227259, -0.00101092, -0.0306706, -0.10824, -0.0745993, -0.0148058, 0.021769, 0.0939854, 0.157866, 0.0504358, 0.0289833, 0.018474, 0.0277677, -0.100512, 0.0480432, -0.0368007, -0.0611738, -0.0306233, -0.00630047, -0.0442848, -0.0610376, 0.00145622, 0.0846044, 0.0131799, -0.0529678, -0.0383711, -0.0172305, 0.082906, 0.015228, 0.0216357, -0.00297318, -0.0749335, 0.0312494, 0.0255774, 0.0254288, 0.0610787, 0.0779867, 0.0346652, -0.0037042, -0.0175103, -0.0801867, -0.0531452, -0.0323273, -0.0664405, 0.0422572, 0.0504251, 0.0351206, 0.0159733, -0.00596112, 0.0207376, -0.0120304, 0.0915501, 0.114271, 0.070705, -0.00721921, 0.0617253, -0.0350129, 0.0548322, 0.0194602, 0.0174106, 0.0344738, 0.115153, 0.167904, 0.0627139, 0.00242679, 0.0274423, -0.0070067, 0.0995764, 0.0796722, -0.00500212, 0.0182519, -0.0848151, 0.0304901, -0.0721535, 0.0234049, 0.126722, 0.0162944, 0.00390706, -0.00836588, -0.00825033, 0.0877396, 0.0171776, -0.034574, 0.0429542, -0.0188131, 0.00850731, -0.0205421, -0.0181717, -0.0549912, -0.0234976, 0.0451561, 0.0144245, 0.0071494, 0.0569952, 0.00825893, -0.0187808, -0.120976, 0.0149034, 0.0982063, 0.0109853, 0.0541339, 0.0188036, -0.0985922, -0.0117489, 0.0274658, 0.0537005, -0.0215088, -0.0442304, 0.0106266, 0.0248345, 0.0449606, 0.0119328, 0.0272148, 0.0692667, 0.0767403, 0.0623216, 0.000408717, 0.0379048, -0.0265446, 0.0259779, -0.00542424, -0.0248054, -0.00860898, -0.0285584, -0.015829, -0.129968, 0.00681084, 0.0319879, 0.0284911, 0.0113132, -0.0699832, -0.036977, -0.103506, -0.0160186, -0.0139377, -0.119349, 0.06985, -0.0481687, -0.00322538, 0.0619692, -0.069483, -0.0745509, 0.0401765, -0.07809, -0.0148451, -0.0444031, 0.0340025, 0.0770801, -0.0942158, -0.0421113, -0.047943, -0.0657317, -0.0515815, -0.0237415, -0.0217821, -0.0522016, -0.0851348, -0.0512862, -0.0701074, 0.0564361, 0.0691512, -0.0317415, 0.0264632, -0.0901268, 0.00403426, -0.0566429, 0.039524, 0.0481673, 0.0451467, -0.0265788, 0.00337031, -0.0139152, -0.00215384, 0.0762363, -0.045615, -0.0216982, 0.076895, -0.0856119, 0.035762, -0.0968617, -0.0939265, -0.000840868, -0.0252621, -0.00321725, 0.0221745, 0.0444218, 0.0339198, 0.00198269, -0.0220559, -0.0997761, -0.00564454, 0.0182727, -0.097856, 0.0263392, -0.0598292, -0.0266975, -0.0199541, 0.0620937, 0.0459636, 0.0823296, 0.115378, 0.0305237, -0.0066649, 0.0178652, 0.0619311, -0.0152332, 0.0554787, 0.0786698, 0.000139774, -0.00234849, 0.0635346, -0.0247715, 0.0141093, 0.057209, -0.0385655, 0.00045169, -0.0324645, -0.104818, -0.0415946, -0.0044221, 0.0521532, -0.017421, -0.0509188, 0.0152785, 0.0810855, -0.0130477, 0.032349, -0.0188132, -0.00591395, -0.0129663, 0.0611003, -0.0748176, -0.0100071, 0.00938269, 0.078325, 0.0387016, 0.0929132, -0.0171047, 0.0267577, -0.0521152, 0.0417262, 0.103063, -0.0547409, -0.0629667, 0.00141967, 0.0430744, -0.0255153, -0.0147494, 0.0150138, 0.0659272, -0.0439566, -0.00786009, 0.0293783, 0.0177377, -0.104309, -0.0821429, -0.0239991, 0.0222835, 0.0322166, 0.0318381, -0.0536362, 0.00260165, 0.0314843, 0.0411139, 0.0959067, 0.0453537, -0.0357554, 0.0969306, 0.00623498, 0.0252458, -0.00282333, 0.0543106, -0.0597579, 0.00252205, -0.00884278, -0.0502542, -0.0347866, -0.0541126, -0.0290534, -0.0110327, 0.0516211, 0.167146, 0.00354933, 0.0435928, 0.0567601, -0.0329072, -0.0198521, 0.0257164, -0.0750328, 0.0149743, 0.00823035, -0.0217955, 0.00961888, -0.0181, -0.00688817, 0.0131081, 0.0706868, 0.0237127, 0.0566147, -0.0281121, -0.0583537, 0.0550147, 0.000171483, 0.0176312, -0.0187403, -0.0731335, 0.0373106, 0.0588949, -0.0357379, -0.00969508, -0.118392, -0.0387084, -0.0713265, -0.0754891, -0.0454727, 0.0113985, 0.0213259, 0.0230974, 0.036421, -0.0407324, -0.0176376, 0.0562626, -0.0145581, -0.0269046, -0.0179992, 0.0561449, 0.0144249, -0.0489566, 0.020869, -0.014594, 0.0380198, 0.0558165, 0.0100691, -0.0279386, -0.0138892, 0.0589594, 0.0429028, 0.0611288, 0.0404967,
-0.0657799, 0.0217273, 0.0232027, 0.0977153, -0.0468156, -0.0152407, 0.0447715, -0.0175045, -0.0515297, 0.0598607, 0.019394, 0.036358, -0.00372071, 0.0180886, -0.00121455, 0.0159876, 0.0187201, -0.00225049, -0.0851807, 0.00235801, -0.0394343, 0.00200635, 0.0089998, 0.0459044, -0.0932619, -0.0429542, -0.0632472, -0.063323, -0.0694685, 0.0010984, 0.0506717, -0.0413985, -0.0349776, 0.0182377, -0.00059545, 0.0572026, -0.0154017, -0.00601634, 0.0234398, 0.00126729, -0.00186746, -0.040554, -0.105816, -0.0107553, -0.11133, 0.0239023, 0.0229233, -0.0295402, 0.00740722, -0.0415868, -0.0189507, 0.0975459, -0.00246191, 0.0732451, 0.0995547, 0.117582, -0.00110058, 0.0483097, -0.0240411, 0.0266358, -0.0537911, 0.00878954, -0.0455984, 0.0584489, 0.121947, -0.0432179, -0.0367258, 0.0216185, -0.00671121, 0.038119, 0.0601352, -0.0738657, 0.0432057, 0.0440698, 0.0356692, 0.0532377, -0.0380289, 0.0499404, -0.0299905, -0.00715623, -0.0363688, -0.0100805, 0.0175174, -0.0106357, -0.00668583, 0.0798624, -0.0165641, -0.051016, -0.0803228, 0.00406207, -0.0661578, -0.0159099, 0.151961, -0.0562872, -0.0567237, 0.0355189, 0.150644, 0.0104733, 0.124225, 0.0752893, -0.0204362, 0.0564305, 0.0492658, 0.0534065, -0.0141762, 0.0200318, -0.014205, 0.035425, -0.0264016, 0.0687555, -0.0107036, 0.0523291, 0.0397479, -0.133216, -0.025278, -0.064467, 0.0826338, -0.013669, 0.052354, 0.0383751, -0.00864204, 0.0352444, 0.117996, -0.0145498, 0.0867896, 0.0280095, 0.0471865, 0.00248284, 0.0329187, -0.0960634, -0.0149206, -0.000930768, -0.0138668, -0.0909854, 0.0395154, -0.00817241, -0.0635414, -0.10455, 0.0127197, 0.103124, -0.0789557, -0.069639, -0.0306321, 0.014411, 0.0392504, 0.0900974, 0.184221, 0.0323871, 0.0830611, -0.069271, -0.0177362, -0.0239403, 0.0105901, 0.0652372, -0.0429379, 0.00288674, -0.0240823, 0.0255181, 0.0336056, 0.0551491, -0.0408885, -0.129256, -0.126375, -0.0435539, -0.0885096, -0.00881128, -0.0233527, -0.0200665, 0.013872, 0.022307, 0.0232158, -0.051375, -0.017789, 0.0572929, 0.0223395, -0.00461691, -0.0918564, -0.0947597, -0.0168223, 0.11076, -0.00216422, 0.138966, 0.122325, 0.110464, 0.0816944, 0.0351205, 0.0247004, -0.0277985, -0.0137073, -0.117062, -0.0661977, -0.18679, -0.0222087, 0.0345544, -0.0591218, 0.000744931, -0.0185858, -0.0100283, 0.0078274, -0.0418319, 0.101993, -0.0154682, 0.0137377, 0.0332029, 0.0228987, -0.0166355, -0.0356006, 0.0725306, 0.0267335, 0.16216, 0.154051, 0.115225, 0.0520734, -0.0286349, -0.0423134, -0.0774913, 0.010598, -0.12954, -0.19688, -0.165149, -0.024462, -0.0248945, 0.00849736, -0.00767966, 0.0215484, -0.0728827, -0.0317155, 0.0588055, 0.141546, 0.0176318, -0.0100385, -0.0148962, 0.0885645, 0.0131313, 0.000137794, 0.0415582, 0.00470302, 0.0355771, 0.0282355, 0.0832023, -0.0474572, -0.0193317, 0.0596049, -0.072772, -0.0221033, 0.056202, -0.117194, -0.163256, -0.0230986, -0.0409275, -0.0477541, -0.00827904, -0.0816117, 0.085074, 0.0418004, -0.0185946, 0.0548349, -0.0201937, 0.0136802, 0.0242044, 0.106381, 0.0257201, 0.0761573, -0.0675357, -0.0275787, -0.0201786, 0.138029, 0.0474989, 0.0477779, 0.00836283, -0.0489788, 0.0398919, 0.0595772, 0.13122, 0.00837288, -0.104442, -0.0178656, 0.030736, -0.0746215, -0.0839704, -0.0411043, -0.00456202, -0.0530945, 0.0590607, -0.0145091, 0.00451387, -0.0225446, 0.0515647, 0.0693326, 0.0241547, -0.109125, -0.139415, -0.0729872, 0.0953004, 0.115737, -0.0419555, 0.0760647, 0.0625248, 0.0583652, 0.0234684, 0.086132, 0.0315337, 0.00317277, -0.0455199, -0.0657808, -0.042516, -0.0647637, -0.0440245, -0.0293652, -0.0174543, -0.0129878, -0.022411, 0.0947777, 0.0808773, -0.0713311, -0.0898682, -0.050012, -0.0821864, -0.0678597, -0.0284416, 0.0610771, 0.0320267, 0.0866355, -0.00087078, 0.149912, 0.134016, 0.0478527, 0.116256, 0.150971, 0.000133932, 0.0385958, 0.0492786, 0.0183299, -0.0661579, -0.00926313, -0.0645851, 0.00415997, 0.0317101, -0.0464127, -0.0421314, -0.0466036, 0.0156953, -0.0377335, 0.0274993, 0.0166108, 0.0234064, -0.0647238, -0.0543049, -0.165921, -0.0132414, 0.115931, 0.0751671, 0.00548885, 0.0158026, 0.014757, 0.0204745, 0.0775509, 0.0442451, 0.00911612, 0.0792838, 0.0108243, -0.0544621, -0.0118299, 0.00661934, 0.042659, -0.074104, 0.0489161, 0.031735, 0.126581, 0.084649, 0.00406778, 0.0433138, 0.00199044, -0.0632465, 0.0419253, -0.0495918, -0.185659, -0.0386638, 0.0228369, 0.0536673, 0.046871, 0.0314895, 0.0346366, 0.0230641, -0.0195006, 0.105111, -0.0426116, 0.0534114, 0.00505542, 0.0195829, -0.0643758, 0.0502169, 0.131366, 0.0204869, 0.0910408, 0.0729509, 0.0214263, 0.0580153, 0.0853323, 0.0383883, 0.201066, 0.183102, -0.0125685, -0.0833187, -0.1424, -0.0692143, -0.0412521, -0.000720617, 0.0938777, 0.0326791, 0.0480082, -0.0116995, 0.12008, 0.102251, 0.125555, -0.0321227, -0.026581, -0.0335111, -0.023684, 0.0165933, -0.0309111, 0.0808041, 0.0222289, 0.0665683, 0.0256987, -0.0259957, 0.00829477, -0.0372792, 0.0890182, 0.0412503, 0.0131555, -0.0142837, -0.117967, -0.110209, 0.0131625, 0.0700717, 0.0897459, 0.0567015, 0.145362, 0.102978, 0.140544, 0.103305, 0.14371, 0.135189, -0.0151909, -0.0200196, -0.0896814, -0.00574715, 0.0515805, 0.0163384, 0.0184822, -0.0151413, 0.0750625, 0.0192046, 0.0592991, 0.00370194, -0.00876207, -0.0441481, 0.0250393, 0.0330452, -0.0159657, -0.0753109, -0.112141, -0.0561074, 0.00771056, 0.0919428, 0.0359222, 0.0838478, 0.183082, 0.059342, 0.190417, 0.112443, -0.00399247, -0.090113, -0.088551, 0.0248061, -0.00186169, -0.0616695, 0.0606826, 0.0469952, -0.0634543, -0.049881, -0.0657202, 0.00263056, -0.0514588, -0.14745, -0.0266271, -0.0845759, -0.00141174, -0.15196, -0.0380816, -0.117927, 0.0468242, 0.0693225, 0.18688, 0.0847265, 0.0783573, -0.0432982, -0.0168437, 0.109153, -0.1061, -0.144703, -0.0485665, -0.017196, 0.00918961, -0.0203822, 0.0463615, 0.0751841, -0.00214818, 0.0289646, -0.0182535, 0.0190521, -0.0492188, -0.0586182, -0.0711491, -0.099421, -0.120839, -0.17995, 0.0275327, 0.00151531, 0.00293217, 0.201679, 0.0853428, 0.109264, 0.0622804, 0.0491836, -0.0221457, -0.185773, -0.0581107, -0.00773173, -0.0982509, 0.0224411, -0.0553121, -0.0658225, -0.0271844, 0.0348582, -0.0721258, -0.0085865, -0.115771, -0.118793, -0.0369077, 0.00811403, -0.0725674, -0.0126329, 0.0519989, -0.149148, 0.0116841, 0.00756716, 0.0359465, 0.0816092, 0.153591, 0.132633, 0.0780341, -0.0165564, -0.0976951, -0.0658868, -0.0770302, -0.0587646, -0.0123604, 0.107143, -0.0335843, 0.0317307, 0.014692, -0.00955946, -0.0756179, 0.0819929, -0.0974578, 0.0455783, -0.026049, 0.0305334, -0.0285174, -0.152231, -0.126983, -0.0509943, 0.0759781, 0.159072, 0.133276, 0.155057, 0.0156829, 0.0574143, 0.0212875, -0.0427591, -0.0714837, -0.0528518, -0.12669, 0.0452017, -0.0508826, -0.0498849, -0.0504384, -0.00932863, -0.0802897, -0.0203572, 0.0697677, -0.0695483, 0.0167933, -0.0250087, -0.1034, -0.116263, -0.0636455, -0.14193, 0.00860163, -0.01023, 0.158229, 0.191794, 0.125876, 0.0909051, 0.0768853, -0.0102246, -0.00798599, -0.111673, -0.03348, -0.0635938, -0.0721156, -0.0271953, -0.0440222, -0.032751, -0.0302027, 0.0095444, 0.0046498, -0.0264198, 0.0690252, -0.0510936, -0.00387193, -0.0629939, -0.0455272, -0.106216, -0.123949, -0.0491632, 0.0347153, 0.0505023, 0.193838, 0.17487, 0.098508, -0.0189367, 0.00778694, -0.094367, 0.0470614, -0.0885552, -0.0342033, -0.0633936, -0.0688646, -0.00853182, 0.0179155, 0.0369042, 0.030538, -0.0295499, -0.135663, 0.0281377, -0.046001, 0.0583261, 0.0340297, 0.0135272, -0.00312467, -0.0798643, -0.0516256, -0.0617983, -0.129083, -0.0323263, 0.0265716, 0.108346, -0.0371509, 0.0468388, 0.0448669, -0.0205762, 0.0438291, -0.0521159, -0.067969, -0.0428133, -0.0275318, 0.0247277, -0.0366066, 0.045446, -0.0288626, 0.0700632, 0.0454585, -0.0242142, 0.0561392, 0.0324434, 0.0534867, 0.102682, -0.0113819, 0.0137525, 0.0607278, -0.00117799, 0.0529331, 0.0449982, -0.112544, 0.0562871, 0.037187, 0.00593904, -0.0263187, -0.0454426, -0.0380724, 0.0740245, -0.00338959, 0.00743211, 0.0701592, -0.014172, -0.0636516, 0.0443071, -0.000956964, -0.0746793, 0.0527286, -0.00625892, 0.0139505, 0.170671, 0.0881431, 0.0186685, 0.157734, 0.0632499, 0.109486, 0.0281089, 0.0345535, 0.0259376, 0.111501, -0.0321199, 0.0298924, 0.0780708, -0.0316459, 0.149826, 0.112165, 0.0951403, 0.0541116, -0.0426234, -0.0239916, -0.0468915, 0.00519909, 0.0214608, -0.0205212, 0.0639181, -0.00123032, 0.015791, 0.00903078, -0.0292077, -0.0407156, -0.0433484, 0.0262342, -0.0810124, 0.120203, 0.00668978, 0.0437075, 0.0872998, 0.193918, -0.012424, 0.110896, 0.105811, 0.100336, 0.073346, -0.0289178, 0.115661, 0.0962341, 0.0107191, -0.0882968, 0.00405825, -0.0633222, 0.123507, -0.0362877, -0.0106971, -0.0161828, 0.000439166, 0.0324293, 0.104839, -0.0652149, 0.0704102, 0.0394511, 0.0449729, -0.0422659, 0.0370379, -0.00333417, -0.00520978, -0.0210495, 0.0645476, -0.0227979, -0.0635823, 0.00837493, 0.133374, -0.0376973, 0.0928405, 0.0243499, -0.0138456, -0.0461784, 0.0256403, 0.114889, 0.0482684,
0.0379385, 0.0804082, 0.0636453, -0.0344845, 0.0578076, -0.080736, -0.000875001, 0.133386, -0.0245857, -0.00935971, -0.0382642, 0.0246326, -0.0392133, 0.0445844, 0.0449155, -0.0722252, -0.00677665, 0.0151972, 0.0208513, 0.0069125, 0.0296028, -0.00233468, 0.0603282, 0.0867216, 0.00517537, 0.0676088, 0.0371377, 0.00404346, 0.051605, -0.0449284, -0.0165064, -0.0137582, -0.0368014, 0.0526532, -0.0211174, -0.0157699, -0.0280397, -0.0363281, -0.0641996, -0.0688898, -0.063173, -0.0938746, 0.00402369, -0.0527952, 0.0724487, -0.018724, -0.026416, 0.0541178, -0.00684018, 0.0422757, -0.0220215, -0.00687612, 0.0768149, -0.00668114, -0.11979, -0.000668655, -0.0375312, -0.0434268, 0.024722, 0.00941372, -0.0604005, 0.0770563, 0.0185298, 0.0167871, 0.0136815, -0.0299045, -0.0530817, -0.0382321, -0.0609651, -0.0754802, -0.0440601, -0.11291, 0.0105018, -0.0538633, -0.0327125, -0.0726419, 0.0661298, 0.02535, -0.0444842, -0.0178231, -0.0659949, 0.104045, -0.0204812, 0.0331284, -0.0148918, -0.0309847, -0.00563898, 0.0196512, -0.0315171, -0.000602279, 0.0225653, -0.0580267, 0.033234, -0.0325172, 0.0325078, -0.129145, 0.0381926, -0.0325483, -0.0817691, -0.0385309, -0.0526667, 0.0325858, -0.0466838, 0.0229381, -0.0778693, -0.0897417, 0.0741367, -0.0167173, -0.0284631, -0.0375821, 0.0286575, 0.0863599, 0.0290113, 0.022255, -0.0539053, 0.00211159, -0.0695943, 0.0746043, 0.0802466, -0.00946528, -0.0396154, -0.150243, -0.0584143, -0.0351837, -0.0800724, -0.125264, -0.134623, -0.0466426, -0.117903, -0.0834169, -0.0669278, -0.0397555, -0.075069, -0.0135261, 0.0321133, -0.00072563, 0.0951438, 0.0347338, 0.0416211, -0.0369947, 0.0437705, 0.00172564, 0.0248469, -0.0565776, 0.0261715, 0.049185, -0.00677522, -0.0553318, 0.0999509, 0.00229284, -0.0479955, 0.101811, -0.0144769, 0.0326307, 0.0212145, -0.0614803, -0.00207619, -0.0739926, -0.12459, -0.168234, -0.161661, -0.0145497, -0.0984146, -0.0018823, 0.0235717, -0.0333572, 0.0370294, -0.0596808, -0.0125372, -0.0450471, -0.034645, -0.0862624, 0.0575263, 0.0322747, 0.111086, -0.0411079, -0.0292217, -0.0118524, -0.0543208, 0.0773698, -0.00828199, 0.140311, 0.00538683, 0.0521497, 0.0296701, 0.062529, 0.081326, 0.0341754, -0.0123385, 0.0276787, -0.0775088, -0.0131723, -0.0470599, 0.00225447, -0.0809337, -0.085191, 0.0743482, 0.0317993, 0.0630622, -0.00726948, 0.0136171, -0.0373381, 0.01922, 0.0130841, -0.0476084, 0.0163357, -0.0648411, 0.00469656, 0.0126565, 0.0228124, 0.0476174, 0.027332, 0.0518625, 0.0344106, 0.180768, 0.123958, 0.0762709, 0.0260027, -0.0355014, 0.0893955, -0.0471038, -0.0219139, -0.0513673, 0.0114513, -0.00835114, 0.00670838, 0.00910489, 0.0169817, -0.0426309, 0.000319077, -0.0216799, 0.0473634, -0.0818478, 0.0657691, 0.0869144, 0.0211966, 0.0155905, -0.00903378, 0.0615982, 0.142203, 0.00384352, -0.0484378, 0.0304188, 0.0436617, -0.0476609, 0.0657363, -0.0334428, -0.0244904, 0.0399768, -0.00272107, -0.0399704, 0.00309094, 0.0240617, 0.0644237, -0.0210417, 0.057006, -0.0531426, -0.0683121, -0.00136187, -0.0739354, 0.0469499, -0.0631877, 0.0877115, 0.150861, 0.0350787, 0.068899, -0.0179592, 0.0547847, -0.0110076, 0.00464851, -0.0603116, 0.111701, 0.0336219, 0.104704, -0.0103642, -0.0705093, 0.0415129, 0.0613641, 0.0206866, 0.00397743, -0.0155856, -0.0672368, 0.1101, -0.0278216, -0.0970331, -0.0765073, 0.0139749, -0.160679, 0.0570354, 0.0766173, -0.0455995, 0.00457691, -0.00846584, 0.0462131, -0.0666644, -0.201593, -0.0495922, -0.0940525, 0.0213067, -0.0104906, 0.063823, -0.0280924, -0.0256599, 0.0340493, 0.048287, -0.0448299, -0.0130577, -0.0863357, -0.051416, 0.0468032, 0.0833225, 0.0380882, -0.0674771, -0.120573, -0.172585, -0.156021, -0.107307, 0.0614204, 0.196105, 0.0707867, -0.0117698, -0.154343, -0.226404, -0.16734, 0.0419304, 0.0996796, 0.149334, 0.0156577, 0.0219326, -0.0175924, 0.0226148, 0.0510535, 0.0176119, 0.0643549, -0.0628659, -0.0122327, 0.00482488, -0.0431879, 0.144966, 0.0330682, 0.00510985, -0.0437341, -0.0489875, -0.108974, 0.0706271, 0.0555264, 0.146342, -0.0343471, -0.0233919, -0.137362, -0.00232708, 0.0853558, 0.00334993, -0.0162506, 0.0832809, -0.0121001, 0.0314259, -0.0330925, 0.102881, 0.0578285, 0.0343712, 0.0491927, -0.0423657, -0.0305039, -0.0652349, 0.0395626, -0.0721872, -0.018693, -0.0337601, -0.197332, -0.162032, 0.0107116, 0.0107424, 0.220477, 0.0430095, 0.0505746, -0.00212759, -0.0857563, 0.100824, 0.144067, -0.0279098, -0.035709, -0.0427479, -0.0526609, 0.0551044, 0.0034467, -0.0364865, -0.012701, 0.0211499, 0.0172546, 0.0150558, 0.0310886, 0.034151, -0.0722004, 0.0319389, -0.0126054, -0.0866588, 0.0566574, -0.0710266, 0.0124161, 0.0751587, 0.085372, 0.078533, 0.0257947, -0.0228716, 0.0624062, 0.235379, 0.147378, 0.0433776, 0.0239427, -0.0259498, 0.047135, -0.0265738, -0.0131267, -0.0239693, -0.0840872, -0.061254, 0.0359172, -0.0334384, -0.0251431, -0.0232403, -0.0101357, 0.0223636, 0.0328283, -0.0667882, -0.170629, -0.04987, -0.0480709, -0.0284595, 0.00661718, 0.0478582, -0.0053837, -0.0356654, 0.0703877, 0.0387021, 0.119734, 0.0733077, 0.145923, 0.0988446, -0.10471, -0.0104331, -0.0018784, -0.047932, -0.0363475, -0.0595938, -0.0632633, -0.0632464, 0.0209213, 0.0643278, -0.0652064, 0.0464059, -0.00705183, -0.00350464, 0.00852901, -0.0507307, -0.0652225, -0.0551195, -0.0906607, 0.0408999, 0.0705953, 0.00745238, 0.00828813, 0.0287037, 0.127409, 0.0621224, 0.0243861, -0.0712794, -0.163452, -0.170298, -0.234831, -0.0868666, -0.115846, -0.0676034, -0.000361871, 0.0257999, 0.0197404, -0.0629464, 0.0547331, 0.0840865, -0.0145054, 0.0717188, -0.0548911, -0.0634241, -0.0490086, -0.1515, 0.0109725, -0.059499, -0.00814799, 0.0583524, -0.0544944, 0.0214359, 0.149434, 0.130337, -0.0650219, -0.0620371, -0.164616, -0.0756519, -0.0635315, -0.151839, 0.0351282, -0.00855635, 0.0107879, 0.0463784, 0.00634144, -0.0047821, 0.0735307, -0.0487125, 0.0368548, -0.0160518, -0.0336117, -0.0143853, -0.146223, -0.0638929, -0.177131, -0.0685779, -0.0133349, 0.0520365, 0.14263, 0.0909778, 0.0128003, 0.0961478, -0.012247, -0.0671359, -0.187891, -0.188009, -0.120745, -0.0630967, -0.063086, -0.0696893, -0.00698913, -0.0988658, 0.0262389, -0.0618311, -0.00541252, 0.047911, -0.0345213, -0.000784458, -0.137272, -0.0716163, -0.0381469, 0.0212441, -0.144527, 0.00489361, -0.0636625, -0.00546231, 0.119155, 0.132464, 0.0289781, -0.0338349, 0.00572219, -0.222264, -0.162789, 0.0361059, -0.102856, -0.0783324, -0.158919, 0.00638524, 0.00623738, 0.0475617, 0.00782339, 0.0645714, -0.0577473, 0.0149888, -0.116128, -0.0670231, 0.0176994, -0.0451141, -0.112468, 0.0167436, -0.122198, -0.0393054, -0.0690803, 0.0551297, 0.0682798, 0.0307485, 0.0565264, -0.0536024, -0.0865011, -0.136402, -0.226656, -0.0717148, -0.0746204, 0.00758969, -0.0427238, -0.0550104, 0.00753401, -0.0259665, 0.0115674, -0.0431282, -0.042215, 0.00416724, -0.0985327, -0.0416457, -0.113458, -0.048579, -0.0769345, -0.00246559, 0.000614102, 0.0476663, 0.011502, 0.0845301, -0.0559222, 0.0697231, -0.0793403, -0.0340067, -0.0423339, -0.177543, -0.244178, -0.155427, -0.0161195, -0.0715441, 0.0417394, 0.0705319, -0.0964749, 0.106315, -0.0188504, 0.0566364, 0.00135008, -0.0181107, -0.0118155, -0.0672518, -0.050993, 0.0969982, -0.0146856, 0.0112616, -0.000718467, 0.0617238, 0.0543287, 0.0255447, 0.0177655, 0.0404108, -0.150027, -0.120259, -0.127894, -0.231021, -0.195623, -0.0957766, 0.0510423, -0.0911157, -0.054877, -0.158046, 0.0679706, 0.0124255, 0.0254705, 0.0506257, -0.00559459, 0.0852235, -0.0158478, -0.0787711, 0.0484891, -0.034075, 0.0720781, -0.0492766, 0.0222222, 0.0818601, 0.0599117, -0.0721575, -0.04573, -0.0952129, -0.0477054, -0.131209, -0.0932279, -0.151662, -0.147502, -0.0986728, 0.025434, 0.0154745, -0.0733508, -0.00300279, -0.106905, 0.0280921, 0.017955, 0.0511513, 0.0508777, -0.0782239, 0.0734356, -0.0275754, 0.172251, 0.085971, 0.145357, 0.0336903, -0.0243684, -0.00999556, -0.0146918, -0.0744031, 0.0946919, -0.0728596, -0.094162, -0.133703, -0.130773, -0.12192, -0.152483, -0.0619901, -0.0959696, -0.00945382, 0.0479458, 0.0278093, -0.0017814, -0.0552704, -0.00583289, -0.0123799, -0.006029, -0.0550412, 0.0240116, 0.0614623, 0.0402591, -0.0291054, 0.030799, 0.0893617, 0.105938, 0.0156895, 0.133196, -0.0842309, -0.0527535, 0.0303654, -0.0339397, 0.0749491, -0.0415556, -0.0251113, -0.0929992, -0.0560975, 0.0232449, -0.145746, -0.115515, 0.000284404, -0.0300551, 0.00358934, -0.0486123, -0.00952586, 0.0922354, 0.0689253, -0.00260138, -0.037568, 0.0788635, 0.0110199, 0.00153421, 0.0427702, 0.0876947, 0.0526768, -0.0213704, 0.14182, 0.147639, 0.120061, 0.0319608, 0.0869956, 0.0463913, 0.0790625, 0.0279495, -0.0160096, -0.0363334, -0.0659437, -0.072124, 0.0830234, -0.0123208, -0.0498764, 0.0660935, 0.00459643, 0.0302339, -0.0393849, -0.0632374, 0.0358581, -0.00898806, -0.0429657, 0.0860004, 0.0533768, -0.0338581, -0.00361144, 0.0472582, -0.0192804, -0.0423882, 0.0180373, 0.0098144, -0.0405133, 0.0179643, 0.0235556, 0.080936, 0.0627165, -0.0218732, -0.0361633, -0.0154377, -0.0705507, 0.0158562, -0.0854937, -0.0971026,
0.00913135, -0.0780394, -0.0595778, 0.0381617, 0.0734751, -0.0228768, -0.00251173, -0.033541, -0.0498052, 0.060704, 0.0599179, 0.0109699, -0.0857952, -0.0290801, -0.021474, -0.0124421, -0.0817354, -0.0711146, 0.0560433, 0.00700129, 0.0779, -0.0550388, 0.0575075, 0.0203709, -0.096701, 0.0331526, 0.0164911, 0.0310982, 0.0479812, -0.0213208, -0.0302128, -0.0103974, 0.072774, 0.0365084, 0.00156697, 0.0192468, -0.0143761, 0.0265375, -0.0559435, 0.0813129, 0.0386518, 0.031751, 0.0225081, 0.0325598, -0.0300413, 0.0323522, -0.0106805, 0.0554338, -0.0591679, -0.024963, 0.0522731, 0.00991735, -0.0321113, 0.033484, -0.00737355, 0.00167743, -0.0312961, -0.0388847, 0.0706422, 0.00442696, -0.0113481, 0.0353506, -0.0898513, 0.0605696, -0.0401756, 0.10206, -0.0485514, 0.0371599, -0.0170535, 0.00230819, -0.0792999, -0.0312433, 0.069572, -0.00564415, 0.0327333, 0.0393261, 0.0218556, -0.00954285, 0.0575357, 0.0739528, -0.0320138, -0.024035, 0.0765525, 0.0808094, -0.0476123, -0.0204827, 0.0535706, -0.128293, 0.00813835, -0.0216924, -0.065495, 0.0922095, 0.0307907, -0.0543165, 0.0567881, -0.0912277, -0.102168, -0.0961258, -0.00540212, 0.0197176, 0.0239563, -0.0546393, 0.0261417, -0.0934216, -0.0991871, -0.0149541, 0.0068462, -0.0520543, -0.0629977, 0.0730125, 0.0104711, -0.0282971, -0.0729321, -0.00128157, -0.0632948, -0.0573516, -0.0145926, -0.000496147, -0.0342079, 0.0355929, 0.00693504, 0.027256, -0.0780918, 0.0320871, 0.0275424, 0.0543583, 0.114185, 0.0866983, 0.0194113, 0.0801792, -0.0468207, 0.0525555, 0.0100709, 0.0372943, 0.0307399, 0.0763189, 0.0973472, 0.102612, -0.0119387, 0.0340769, 0.0351601, 0.0136596, 0.00602037, 0.00118796, -0.0535099, -0.0599356, 0.0802388, 0.087689, -0.00105924, 0.00658855, 0.0677021, 0.0170899, 0.0184289, 0.0480104, 0.0814293, 0.0713236, 0.0140833, 0.100493, 0.0178341, 0.0735678, -0.0255716, -0.103075, -0.0258112, -0.0284808, -0.0617808, 0.053512, 0.00309666, -0.0162672, -0.0508305, -0.0776512, 0.0231055, 0.120466, 0.0613706, -0.0424717, -0.098088, -0.0552389, -0.0292495, -0.0371872, -0.0895517, -0.0134677, -0.0524279, 0.0897665, 0.0376928, 0.176609, 0.114126, 0.0891147, 0.0805773, 0.0683645, 0.0507569, 0.0468156, -0.072944, -0.05027, -0.0075543, -0.0242257, -0.0199734, -0.0289074, 0.0245694, 0.0180962, 0.034063, 0.00320609, -0.0213658, 0.015217, -0.0159017, -0.0219536, -0.128596, -0.0412768, -0.071855, -0.152786, -0.0142305, 0.0323741, 0.112597, 0.137301, -0.00761606, 0.17406, 0.063769, 0.0160116, 0.0136818, -0.0786086, -0.0242059, -0.00646936, 0.00677748, 0.0630196, -0.0213471, -0.0548915, 0.0293793, -0.0828312, 0.0477931, -0.00542713, 0.0363799, 0.0884377, -0.119414, -0.0961185, -0.0871498, -0.0321876, -0.163821, -0.11541, 0.00970832, 0.107338, -0.0181959, -0.00583129, -0.0246676, -0.004782, -0.000975595, 0.008334, -0.150957, 0.0526028, -0.0368956, -0.0354062, -0.0329459, -0.0342139, -0.057749, -0.0509134, 0.00514261, -0.0345624, -0.00259656, 0.0626497, -0.0144715, -0.0532944, 0.0197862, 0.0400456, 0.0415202, -0.0639092, -0.0607715, -0.103781, -0.0926267, 0.0228655, -0.123143, -0.127026, -0.151048, -0.123085, 0.0222625, 0.00637529, 0.0287401, -0.0700842, -0.0818956, -0.0288343, -0.107725, -0.164355, -0.026822, -0.0255319, 0.0464794, 0.0448091, 0.034093, 0.0422298, -0.0125317, 0.0210121, -0.0502138, -0.137163, -0.0141067, 0.0456451, 0.0162751, 0.0159863, -0.0361061, 0.00826464, 0.0733159, 0.0421573, -0.00220342, -0.0354402, 0.0280604, 0.0282612, -0.0846996, -0.0421428, -0.0708058, 0.015218, 0.0125382, -0.0240703, -0.031597, 0.00125193, 0.0432556, -0.0446969, -0.0654186, 0.0426853, -0.0403767, -0.0672615, -0.00482576, -0.0553235, 0.0819995, 0.00473315, 0.0514561, 0.0426953, 0.0253887, 0.179624, 0.116538, 0.177234, 0.0432391, -0.02195, -0.0239488, 0.0625787, 0.0138328, 0.0476796, 0.00944617, -0.0927216, -0.081417, -0.0336838, 0.0472826, -0.0858723, -0.0189618, -0.0810651, 0.0142457, -0.0225308, -0.00759919, -0.012281, 0.0133365, -0.0527131, 0.0313673, 0.0815159, 0.000210862, 0.0942539, 0.109356, 0.126447, 0.0996281, 0.0257711, 0.0195656, 0.0515654, 0.0317083, 0.104415, 0.0536689, 0.0734532, 0.0483751, 0.157767, -0.00381093, -0.106856, -0.0374171, 0.0239309, 7.42047e-05, 0.138322, -0.120154, 0.0279334, 0.0137686, -0.102552, 0.0803291, 0.0868883, 0.0293007, 0.0413289, 0.13652, 0.0697492, 0.0566546, 0.0229407, -0.0932923, 0.0848205, 0.0574622, 0.0362017, 0.0451334, 0.0536978, 0.0841161, 0.111356, 0.141304, -0.0439009, -0.0344224, -0.0485668, 0.0171004, -0.0558712, 0.0966592, -0.0119419, -0.0873413, -0.0765545, 0.021443, -0.0375982, 0.0121246, 0.0701898, 0.140629, 0.0777097, 0.0885339, 0.0863342, -0.0112851, 0.0220141, 0.0718939, 0.00610509, 0.0729824, 0.0519154, 0.0513715, 0.150298, 0.207411, 0.0356756, 0.0156098, -0.025983, -0.0797262, -0.0607177, -0.0651052, 0.0377779, -0.0198311, -0.0290608, 0.0680805, -0.0166622, 0.049251, -0.0795458, -0.0177145, 0.0710305, 0.182449, 0.0840119, 0.0166653, 0.108157, 0.162024, 0.0183443, -0.00648163, -0.00589546, 0.0502679, 0.0274085, 0.100029, 0.161818, 0.143662, 0.00768837, 0.0110397, -0.08099, -0.0589105, -0.0653918, 0.0351258, -0.052283, -0.0115658, -0.0349571, -0.0635855, 0.00480917, 0.0186756, -0.06869, 0.0818458, 0.0957404, 0.1475, 0.0982152, 0.130121, 0.202225, 0.155091, 0.0132163, -0.111801, -0.029004, 0.0535763, 0.065525, 0.108292, 0.0153139, -0.0158017, -0.00483478, 0.0503538, -0.0179579, -0.0859697, -0.0367486, -0.0893663, -0.0188098, 0.0287927, -0.0460046, -0.0550878, -0.0454502, 0.0581512, -0.0108384, -0.0227034, 0.112793, 0.149257, 0.0529712, 0.0623626, 0.0754008, 0.120472, 0.0959069, -0.0291019, -0.014686, 0.112938, 0.115054, -0.064716, -0.061078, 0.0772571, 0.0482861, 0.0452726, 0.0482216, 0.0320055, -0.0971609, 0.0312168, -0.0100953, -0.0778821, 0.0369915, -0.0996938, -0.0821566, -0.0647617, -0.0233373, -0.0765425, -0.0128938, 0.145369, 0.0621783, 0.0475109, 0.0486534, 0.116614, 0.108188, 0.0038763, -0.173757, 0.065857, 0.0670629, 0.0256077, -0.0478477, -0.0430443, -0.0182091, 0.042747, -0.0225032, -0.0910589, 0.0402472, -0.00329165, -0.00148775, 0.047286, -0.127777, -0.0171011, 0.00404907, -0.0599622, -0.0214789, 0.0397928, -0.0369544, -0.0742281, 0.0135994, -0.0146285, -0.0762219, -0.0541367, -0.0291426, -0.124795, -0.0310953, -0.0117962, -0.0591893, 0.0304175, -0.0376795, -0.0847787, -0.00534347, 0.0141582, 0.021527, -0.0918937, -0.0774302, -0.0175333, 0.0575167, -0.087451, 0.0335327, 0.0766366, -0.0305687, 0.143459, 0.00733712, -0.0396816, -0.0209586, -0.169279, -0.0906647, -0.114973, -0.0756839, -0.0982157, -0.002041, -0.0297443, 0.0513026, 0.1305, 0.00095816, 0.0834044, -0.0444149, 0.0781064, 0.00300725, 0.0641617, -1.54682e-05, -0.0316099, -0.0394177, -0.0472582, 0.00992396, 0.1004, -0.108458, -0.0505891, 0.0766529, -0.0276749, 0.0551395, 0.107942, -0.0235101, -0.0554822, -0.0452074, -0.116562, -0.141343, 0.0407797, -0.0247554, -0.0870557, -0.0318753, -0.0361135, -0.00195365, -0.0789275, -0.103179, -0.0144317, 0.0104144, -0.0202753, -0.00413881, -0.0597695, -0.0654092, 0.0771874, 0.000362279, 0.0392683, -0.0479799, 0.00603111, -0.0318035, -0.0271087, -0.0257705, 0.0191325, -0.0456309, -0.0697214, 0.00633413, -0.0391668, 0.0602308, 0.0330924, 0.0415647, -0.0456863, -0.0720714, 0.163201, -0.0484681, -0.0470687, 0.0440879, -0.0268085, -0.0231381, -0.0441651, -0.0344919, 0.0339277, -0.0870461, -0.011989, 0.132997, -0.0923638, -0.0332675, -0.0179915, 0.0211549, -0.0064852, 0.0100876, -0.0612062, -0.0675744, 0.0677798, 0.0548002, 0.0626474, 0.129654, 0.0350016, 0.0358185, -0.0908632, -0.0182331, -0.0814324, -0.082278, 0.00171908, -0.0589206, 0.0186169, -0.00903414, 0.0469065, -0.039132, -0.0331886, 0.0785808, -0.0289417, 0.0762012, -0.0244829, -0.0488343, -0.0770786, 0.0430588, 0.0229279, 0.0519624, -0.0618826, -0.0137134, 0.000638734, 0.0700721, 0.0878486, 0.0881703, 0.0342881, 0.0992353, -0.0163033, 0.00697318, -0.00352111, -0.119931, -0.0362845, -0.0702135, 0.0432862, 0.0246966, 0.0642201, -0.0289169, 0.00654978, -0.00339239, 0.0178954, 0.00487093, 0.0829941, 0.0120561, -0.0490304, 0.000291076, -0.0409143, 0.0437322, -0.0284907, -0.0375465, -0.164308, 0.0728209, 0.0689098, -0.00899422, -0.096417, 0.00377358, 0.018474, 0.0508532, -0.0244599, -0.0679334, 0.0440825, -0.00213144, 0.0368658, 0.0034874, -0.00778291, 0.00827668, -0.017427, 0.0103547, -0.0356589, -0.0865436, -0.0706437, 0.0500727, 0.0197982, 0.0642337, 0.0464079, 0.0487467, 0.0292268, 0.0114819, -0.0664267, 0.0737318, -0.0387334, -0.000874714, 0.0703302, 0.0463229, 0.0555936, 0.0711435, 0.0556547, 0.128537, -0.00675336, 0.0091994, 0.00140885, 0.0508238, 0.000345583, -0.0565554, 0.0117245, 0.01446, -0.0330393, -0.0281127, -0.068307, -0.0477942, -0.0806105, 0.0747252, -0.00711256, -0.0331716, 0.0582769, 0.0812416, 0.101936, -0.0271542, -0.0312026, -0.0168268, -0.0318197, 0.0729597, 0.0758373, 0.0295433, 0.0427299, -0.0126929, 0.00512707, -0.0047858, 0.072562, 0.0510471, -0.0210513, 0.101032, 0.0150619, -0.0732814, 0.0883003, -0.0414576,
-0.0311602, 0.05166, -0.0456765, 0.0768996, 0.00353853, -0.00501918, 0.0367759, -0.00471732, -0.0965819, -0.0173448, 0.0399105, 0.0833277, -0.106189, 0.0241989, -0.00162847, -0.0115245, -0.0779676, 0.0695769, -0.0214903, 0.0731403, -0.0191006, -0.0870543, -0.000625838, 0.102786, -0.0527028, 0.0137695, -0.0742015, -0.0134907, -0.0188583, 0.0357185, 0.122311, -0.0222181, -0.0197945, -0.178728, 0.0268118, -0.0242913, 0.0634119, 0.0322998, 0.0187821, -0.0462274, 0.00583635, 0.0149751, 0.0799727, 0.0123113, 0.0804478, -0.0116377, 0.079027, 0.00985686, -0.0837926, -0.0340749, 0.03804, -0.0584017, -0.0265603, -0.0499432, 0.00906865, -0.0267798, -0.0995079, 0.00318197, 0.0415, -0.00897438, 0.0786451, -0.0132296, 0.0200518, -0.036135, 0.0271431, 0.0602605, -0.0252318, 0.00702771, -0.0904449, -0.00774385, -0.0417757, 0.0144146, 0.0192066, 0.152947, 0.0582132, 0.0360658, 0.0332833, 0.0679287, -0.0295376, -0.0110569, -0.0124274, -0.024345, -0.0622788, 0.004805, -0.00392349, -0.0282211, -0.0184888, -0.0709416, 0.0679294, 0.0294934, 0.0429582, -0.00355631, 0.0800465, 0.0461526, -0.0527328, 0.0766119, -0.0802473, -0.0431889, 0.0549023, 0.0938829, 0.000128276, 0.0240664, 0.0111902, 0.0749024, -0.0155383, -0.0522685, 0.0370806, 0.00259088, 0.0212323, 0.0636672, 0.0235941, -0.0940782, 0.0159729, -0.045122, 0.0450619, -0.0412433, -0.086077, 0.0198561, 0.0533344, -0.0856431, 0.0771662, -0.018577, 0.0260091, -0.0239297, 0.0289107, 0.113207, 0.090061, -0.0337529, 0.0264754, 0.0738744, -0.00900486, 0.0567721, 0.0524467, 0.00715779, 0.0557796, 0.0343529, -0.0522598, 0.0457321, 0.0518844, 0.10054, 0.0677751, 0.0371384, 0.0392049, -0.0150077, 0.0539021, -0.0585063, 0.0646475, 0.0582912, -0.022503, 0.00914024, 0.0263768, -0.0336219, 0.0866669, 0.003252, 0.0566629, 0.059053, 0.0958203, 0.0336404, 0.0527299, 0.0425447, 0.0559307, 0.0765692, -0.0718805, 0.0628691, -0.00352865, 0.00653201, 0.0359057, 0.0451039, -0.0436351, -0.0592236, -0.092412, 0.108683, 0.0547165, -0.0295369, 0.0972815, 0.0464473, 0.00976824, 0.0964401, 0.0106739, -0.0905005, 0.0267742, 0.127971, 0.0418243, 0.0754059, 0.046523, -0.036644, -0.0590227, -0.139052, -0.110364, -0.116586, 0.0136091, 0.0828793, -0.016506, -0.0126127, 0.0505989, -0.0225264, 0.0485166, -0.0605072, 0.0835937, 0.00614194, 0.136443, 0.0551379, -0.0586435, 0.0989688, 0.166488, -0.0430397, -0.0818163, -0.0266241, -0.039455, -0.0101823, 0.00388189, -0.0773186, -0.165506, -0.0723426, -0.156939, -0.212585, -0.0748482, -0.0945613, -0.118597, -0.0877982, 0.0281754, -0.0108818, -0.0348907, 0.0175799, -0.0360174, -0.00248453, 0.0926128, -0.0213049, 0.067969, 0.0879489, 0.0580136, 0.103241, -0.0718959, -0.105565, -0.0530712, -0.0940596, -0.0867544, -0.013807, -0.131321, -0.13268, -0.111815, -0.0127074, -0.0567986, -0.0877154, -0.14895, -0.0937076, -0.19655, -0.0164332, -0.0544938, 0.0126345, -0.0929956, -0.0312932, 0.0412456, 0.100699, -0.000401059, -0.0185457, 0.110405, 0.0425563, 0.00347958, -0.0236294, 0.00390859, -0.111445, -0.116978, -0.122677, -0.0370596, -0.0979644, -0.102427, -0.0100005, 0.0378872, -0.0763384, -0.0449014, 0.0293264, -0.0353105, -0.0455119, -0.0365434, -0.025774, -0.00508828, -0.0372197, -0.0149881, -0.0655008, -0.0432354, -0.0424705, -0.0596123, -0.0448129, -0.0488414, 0.0468655, 0.0871329, 0.0204728, -0.0215945, -0.0825091, -0.0733923, -0.037648, -0.0938984, -0.0521811, 0.0259659, 0.051308, 0.101657, 0.143579, 0.197373, -0.0443774, -0.0310792, -0.0241346, -0.0572512, 0.0552293, -0.0977119, -0.0709956, -0.0122425, 0.0551123, -0.0818725, -0.0508552, -0.0269616, -0.0195742, 0.0762953, 0.111178, -0.0224662, -0.0419498, -0.0449198, -0.078633, 0.0700636, -0.0438442, 0.0910234, 0.141955, 0.0616866, 0.069099, 0.0564172, 0.0730978, 0.169902, 0.0940668, 0.0173749, 0.0594916, -0.0462753, -0.0514817, 0.00383514, -0.0507815, 0.0335674, 0.0135107, 0.0397034, 0.0291399, -0.0376367, 0.0101389, -0.037704, 0.014683, 0.0468238, -0.0471504, 0.00218171, -0.0535928, 0.108945, 0.0470596, 0.199248, 0.190608, 0.119698, -0.085847, 0.06153, 0.11903, 0.0196977, 0.0644975, 0.115204, 0.00649795, 0.106101, -0.0397109, -0.0409556, -0.00663794, 0.00579861, 0.0491496, -0.0527545, 0.0129704, 0.012062, -0.0825047, -0.101312, -0.013027, 0.106917, -0.0593398, -0.0815488, -0.0508839, 0.00654621, 0.030473, 0.123808, 0.0308903, 0.0216822, 0.0536423, 0.105584, -0.00597039, 0.132623, 0.145954, 0.0978143, 0.0351442, 0.0891106, 0.0278659, 0.0772661, -0.0418743, 0.0402777, 0.0926257, -0.0236188, -0.00342145, -0.00925959, 0.0292326, -0.0823425, 0.0395789, 0.047509, -0.0574007, 0.0305443, 0.055316, -0.00283861, -0.0437989, 0.153773, 0.0313008, 0.0733749, -0.0318407, 0.0780701, 0.0694277, 0.0324979, 0.0773114, -0.00837859, 0.0353947, -0.0186153, 0.0222643, -0.021564, 0.0247492, -0.0390955, -0.000750561, -0.0129422, -0.0439523, -0.0186071, 0.0253551, -0.0041598, -0.0609269, 0.0514942, -0.0434467, -0.0310785, -0.0713527, -0.0322448, 0.0196245, -0.0133819, 0.0471277, 0.138645, 0.137738, 0.127131, 0.120147, 0.107016, 0.159779, 0.0939325, 0.113521, 0.110332, 0.0777926, -0.0138705, 0.00770789, -0.0105921, 0.0405825, -0.119807, -0.0489598, -0.0695527, 0.0568062, 0.0330856, 0.00518357, 0.0278558, 0.0122315, -0.00204787, 0.0480894, 0.0541056, 0.0278006, 0.0752683, 0.0694734, 0.134104, 0.147833, 0.0154789, -0.0777443, 0.0183794, 0.101879, 0.0572392, 0.115811, 0.0491082, 0.076717, 0.0109579, 0.0648284, 0.0468509, 0.059838, 0.018875, 0.0680318, -0.0327457, 0.0476388, -0.00106015, 0.0516081, 0.0398556, -0.0238665, 0.0224355, -0.0593221, -0.0669868, 0.138085, 0.126707, 0.0557781, 0.0887597, -0.0232681, -0.0589927, 0.0479672, 0.112591, 0.0854358, 0.0719433, 0.078731, 0.0220945, 0.0259667, -0.0254753, 0.0539864, -0.0507314, 0.101488, -0.0493583, -0.0416284, -0.0227734, 0.136123, 0.0779509, 0.0681147, -0.00479634, -0.0125592, -0.0449869, -0.0417132, -0.0877824, 0.0146362, 0.133301, 0.0630548, -0.008184, -0.0792895, -0.013115, -0.054412, 0.0177387, 0.0604023, 0.122844, 0.130146, 0.0250624, 0.125145, 0.0338605, -0.0181851, 0.0958717, -0.0279268, -0.00594314, -0.0241466, -0.0897501, 0.024916, 0.0762993, 0.106771, -0.0517641, 0.0355678, 0.00589031, 0.0218201, 0.000320685, 0.0214373, -0.0112974, -0.088264, -0.0105809, -0.0335395, 0.0496167, 0.0915362, 0.0745403, 0.0537781, -0.0597876, 0.148741, 0.081647, 0.0195534, -0.00755822, -0.0376981, -0.0565465, -0.0534564, -0.121347, 0.0421471, -0.0227608, 0.00833092, 0.0510352, -0.0335947, 0.022203, 0.0787553, -0.079892, 0.00304998, 0.0842893, 0.0608992, -0.0588655, -0.0700116, -0.0125867, -0.0717636, 0.046024, 0.134284, 0.0242287, -0.0448656, 0.127181, 0.0271115, -0.00708249, -0.0952889, 0.000730629, -0.0194984, 0.0272373, -0.0620854, 0.0165567, 0.0516606, -0.0387428, 0.0955918, 0.0429939, -0.0275513, -0.0404021, -0.000907529, -0.008306, 0.0642027, 0.00997933, 0.0733498, -0.0293699, -0.0651632, -0.0774833, 0.010991, 0.11867, 0.125469, 0.0377216, 0.01625, 0.0387328, -0.0786443, 0.00371521, 0.117057, -0.0813134, -0.037131, 0.0232984, 0.0570645, -0.0529646, -0.0434593, 0.0365501, -0.0608293, -0.000924253, -0.118262, -0.0975715, -0.00746995, -0.0771126, -0.0309463, 0.117732, 0.157273, 0.00205383, 0.0306897, -0.0466736, 0.067668, 0.113315, -0.0552682, 0.0352499, 0.0872955, 0.0776776, -0.0310022, 0.0311222, -0.00479145, -0.0704648, 0.0198881, -0.0403016, 0.00989284, -0.0183747, 0.0148152, 0.0892338, 0.060092, -0.127378, -0.115126, -0.00324016, -0.0685649, -0.0804524, 0.031826, 0.056898, 0.0437837, 0.0345803, 0.0959293, 0.0214301, 0.00739734, 0.0219283, -0.0227405, 0.0575612, 0.0405628, 0.0852927, 0.139195, -0.0191518, 0.0359481, -0.0221276, 0.0240219, -0.0129523, 0.0203734, 0.0206073, 0.00118345, -0.0167329, -0.01616, -0.0155999, -0.0273326, -0.0673728, -0.0487136, 0.0302509, 0.0625732, -0.00289117, 0.0214211, 0.0152883, -0.0172788, 0.130584, -0.0272671, 0.140283, -0.0623495, 0.0792805, 0.0732033, -0.144495, 0.0743943, 0.0333671, 0.0861134, -0.00631345, 0.0364305, -0.0689367, 0.00920474, -0.026399, -0.0911763, -0.0389154, -0.0152828, 0.0075342, -0.0230075, -0.0132362, -0.0274506, -0.0942975, -0.0209479, 0.0969061, -0.0479095, 0.0542146, 0.091027, -0.00272672, -0.0143065, -0.0944354, -0.00382198, -0.0195732, 0.00310561, -0.0742676, -0.0871578, -0.0358922, -0.0219917, 0.073347, 0.0238105, 0.0379347, -0.00396678, 0.0277026, -0.0307481, 0.130801, 0.00596967, -0.0124124, -0.0670462, -0.0520531, -0.0527784, -0.0646627, -0.0524962, -0.0342144, -0.0359266, 0.0475917, -0.0203313, -0.109657, 0.00850886, -0.0281633, -0.0955325, -0.0128868, -0.0579454, -0.0606061, -0.041362, -0.00531589, 0.0109529, -0.00626844, -0.0779495, -0.032197, 0.0307661, -0.0408061, 0.0224746, -0.0579327, -0.0140537, -0.042324, -0.0614928, -0.0216033, 0.0070928, -0.013154, -0.000656392, 0.0164265, -0.0285133, -0.0324976, -0.110384, -0.00982411, -0.0160246, 0.0431851, 0.0870463, 0.107886, 0.0139374, -0.0131572, -0.02671, 0.053334, 0.0114406, 0.0837588, 0.0269773, 0.0146759, 0.0600085,
-0.0420323, 0.0142375, 0.0818459, -0.0314205, 0.0313942, -0.0419616, 0.0901317, -0.0676124, -0.081814, -0.0121108, 0.0578596, 0.0378917, -0.0121619, -0.0353166, 0.0806619, 0.0195192, -0.0500249, 0.00350525, -0.0866313, 0.0230385, 0.0675455, 0.0470098, -0.00764217, 0.000297559, 0.0195731, -0.00135637, 0.0460716, -0.0444327, 0.0242418, -0.0267388, -0.0827406, -0.000306343, 0.0402473, -0.000882162, 0.00644632, 0.0340012, -0.0327425, -0.0839967, 0.0775634, -0.0601496, -0.0630732, -0.019402, -0.0159914, 0.0118419, -0.0471793, 0.0525465, -0.0193107, 0.0624928, -0.052169, 0.0395592, 0.0360665, -0.104033, 0.0153898, 0.099329, -0.0189611, -0.0570453, -0.0337775, -0.0097926, -0.0131134, 0.0489857, 0.0134534, 1.60442e-05, 0.0242211, 0.0697239, -0.113539, -0.0286041, 0.0575605, 0.00412488, -0.0511679, -0.0413238, -0.0520265, -0.0127085, 0.0173839, 0.0244495, 0.0425329, 0.0167633, 0.029226, 0.0309213, -0.0182418, 0.00457195, 0.0303132, 0.0824275, 0.00583645, -0.0689217, -0.00474939, -0.0203218, 0.0264388, -0.0133062, 0.0298803, 0.0196403, 0.0322976, 0.0556877, 0.00585486, -0.0205089, 0.0112088, 0.00197808, -0.0523973, -0.0288276, 0.111164, 0.0235782, -0.0761616, 0.0431099, -0.0843855, -0.0420972, 0.0256504, 0.0194704, -0.046181, -0.0544718, -0.0516067, 0.0291205, -0.00623205, 0.0253113, -0.0211639, -0.0421567, -0.0313777, 0.0100421, -0.0647472, -0.0381689, -0.0900512, -0.00570624, 0.0331747, -0.0529181, -0.0811061, -0.0338843, 0.0639691, 0.119927, 0.0432736, -0.0338893, -0.0332402, -0.190118, -0.0785071, -0.0163537, -0.014245, 0.0300165, 0.0109486, -0.051288, -0.0403083, 0.0361255, -0.0265112, 0.0333079, 0.064506, 0.00873867, 0.0432225, 0.0213033, 0.0550779, 0.049124, -0.0840787, 0.0104986, -0.0253185, -0.00740467, -0.0537289, -0.0698408, 0.0536332, 0.0410442, 0.0232505, -0.0213745, -0.104953, -0.112637, -0.149342, -0.0788782, -0.00824468, -0.0523982, -0.0426726, -0.0680368, 0.0151951, -0.0638883, -0.0738778, 0.0257119, -0.0376725, -0.0126587, -0.142036, -0.0610428, -0.0461411, 0.082776, -0.00641161, -0.0624177, -0.0398771, -0.0107025, 0.0362442, 0.0896229, 0.191038, 0.111002, 0.0816116, -0.190521, -0.298793, -0.192917, -0.0989967, -0.0719407, -0.0828714, -0.0140744, 0.0719047, -0.000581364, 0.0955028, 0.00908819, -0.0477731, -0.08566, 0.0570701, 0.00330523, 0.0727393, 0.010113, 0.0610646, -0.0957746, -0.0929004, -0.0509233, -0.117259, -0.00283825, 0.144842, 0.129653, 0.143285, 0.0688643, -0.00320025, -0.235956, -0.309149, -0.154014, -0.0194369, 0.0104707, 0.0290542, -0.0218272, 0.0915357, 0.115858, 0.0322805, 0.0030948, 0.027536, -0.00442469, 0.0212978, 0.0669388, -0.0328571, 0.0870202, 0.105468, -0.053052, -0.0890683, -0.0632199, -0.000724409, -0.00191809, -0.0528109, 0.0798543, 0.169795, 0.158005, 0.0408531, -0.0936678, -0.171597, -0.0499767, 0.151032, 0.0162964, 0.0691936, 0.0615397, 0.139232, 0.0924994, 0.115816, 0.0623814, -0.0228131, 0.0697953, -0.0187582, 0.0481685, 0.0654368, 0.0580254, -0.0316224, 0.0860799, -0.0243569, -0.0849146, -0.0719298, 0.068807, -0.0728248, 0.051917, 0.109784, 0.0618126, 0.0998439, -0.0975253, -0.149363, 0.120944, 0.109787, 0.112646, 0.0115059, 0.0718778, 0.0881021, 0.117646, 0.0481211, -0.0540579, 0.0784397, 0.011299, 0.0608863, -0.110624, -0.00395049, -0.0610979, -0.00890738, -0.0445969, -0.0918061, -0.0419529, -0.0805776, 0.00598223, -0.0715202, 0.0785942, 0.0723304, 0.178802, 0.0119062, -0.161083, -0.08956, 0.109584, 0.126552, 0.100327, 0.117288, 0.237822, 0.0417668, 0.0999, 0.139311, 0.0851913, -0.0275965, -0.00620773, 0.0533686, -0.0117933, 0.00669205, -0.0191706, -0.0753608, -0.0297895, -0.104502, -0.0901941, -0.126746, -0.0906651, -0.00408745, 0.0229948, 0.168531, 0.0201787, -0.0604663, -0.221551, -0.128144, 0.1181, 0.0417803, 0.0917325, 0.0347939, 0.0412123, 0.132991, 0.0415033, 0.0183545, 0.0466541, -0.0751663, 0.0254608, -0.029477, -0.0157626, -0.0461117, 0.0927338, 0.0670727, 0.0351392, -0.0778713, 0.0106149, 0.00382592, -0.0749127, 0.0976927, 0.142059, 0.169665, 0.114329, -0.043103, -0.159357, -0.151528, 0.0787487, 0.014701, -0.117254, 0.00756423, 0.0928567, -0.0285734, -0.00849361, -0.057306, 0.10483, 0.0798615, -0.0484002, -0.0478411, 0.0756635, -0.0822401, 0.117012, -0.0131755, -0.020869, -0.077263, 0.12492, 0.0883895, 0.104537, 0.117135, 0.148923, 0.168689, -0.0778302, -0.0454697, -0.092532, 0.107065, 0.0195715, 0.0267419, -0.0553654, -0.122252, -0.140583, -0.0831455, -0.0176773, 0.124531, -0.0528059, 0.0648916, -0.0813338, -0.0730534, -0.0280476, -0.0574282, 0.0384836, -0.0844697, -0.0047698, -0.0752266, 0.0104086, 0.0597505, 0.0132945, 0.133862, 0.0533433, 0.0393031, -0.0859825, -0.0957793, 0.00263292, 0.0258183, 0.0988227, -0.0493387, -0.0709143, 0.0242902, -0.0779022, -0.0339418, 0.0244958, 0.0312974, 0.000123471, 0.0122005, 0.0284172, 0.0967529, 0.0216828, 0.00551957, -0.0378155, 0.0640768, -0.00946725, 0.0139112, 0.165628, 0.0624802, -0.012915, 0.0460394, -0.0600043, 0.00460823, -0.081948, -0.0459816, -0.121798, 0.0360297, 0.022474, 0.0704186, -0.12429, -0.0380246, 0.0487832, 0.00636625, -0.00292348, 0.0255617, -0.0429164, -0.0147049, -0.0266524, 0.0455525, 0.0325468, -0.0292527, -0.0635027, -0.000827253, -0.0374909, 0.0302106, -0.0742291, -0.0491781, 0.0190061, -0.0132628, 0.00860731, 0.0442903, -0.0901032, -0.0969956, 0.00682698, -0.0769787, 0.0447121, 0.0622685, -0.0553203, -0.0572467, 0.0586472, 0.0467586, -0.00748838, -0.0253603, 0.0733653, 0.00111149, 0.0348241, -0.0496923, 0.0291636, -0.047168, -0.0560459, -0.00417524, 0.0247712, 0.0286891, -0.0528648, -0.0732319, 0.0208622, 0.0423678, -0.0187888, 0.0269765, -0.0423777, -0.043343, -0.0625668, -0.00207864, -0.0422728, 0.0539879, -0.0689307, -0.037767, -0.029332, -0.0479418, 0.0575036, 0.0240666, 0.0374386, 0.0763813, 0.0320506, -0.0281219, -0.0380922, -0.0158276, -0.00483249, 0.00135973, -0.1201, -0.00162658, 0.0292733, -0.0169378, 0.0854115, 0.0778711, 0.0847223, -0.00379127, -0.0550044, -0.0391059, -0.0352645, 0.017368, 0.0261801, 0.0306729, 0.0141533, 0.0119581, -0.0805774, -0.104663, -0.0571198, 0.00860186, 0.00999719, -0.0149416, -0.0448132, -0.0845537, 0.00686786, 0.116445, -0.0365613, 0.0150056, -0.0308425, -0.0823779, -0.0446578, 0.0272236, 0.0687624, 0.0319316, -0.02951, 0.0509527, -0.0816571, -0.0369666, -0.0591228, 0.0272298, 0.00114932, -0.0303491, -0.0395685, -0.0122907, -0.0752009, 0.0316149, -0.0445627, -0.0402718, 0.0715928, 0.107665, 0.060246, 0.0603172, 0.0487109, -0.00804003, -0.040316, 0.0316617, -0.0914927, 0.057695, 0.0460737, 0.0770052, 0.137312, 0.0620736, 0.0342444, -0.00582761, 0.0531763, -0.064043, -0.00272605, 0.0359733, -0.0943824, -0.0663043, -0.116311, -0.0630527, -0.00850593, 0.0139389, 0.0125458, -0.0455421, 0.0625311, 0.0408278, -0.120554, -0.0175626, 0.0345709, -0.00848415, -0.00156303, -0.0599447, 0.0591111, 0.0590478, 0.0236226, -0.00713874, 0.0487764, -0.030335, 0.0603499, 0.0389953, -0.0598219, -0.109172, -0.0191817, -0.0605254, -0.0301796, -0.0771626, -0.0624254, -0.0274271, -0.0304107, -0.0725266, -0.0693469, -0.0444176, 0.0456132, -0.0442307, 0.0308245, 0.00138954, 0.0337941, -0.0183049, 0.0928581, 0.0291791, 0.0670188, 0.0792247, 0.0262987, 0.0276214, 0.0344058, 0.0728792, 0.0269336, -0.0511829, -0.0411017, -0.0734941, -0.0608545, -0.0242239, -0.0240761, -0.0520526, -0.00358297, -0.00667416, 0.0644425, -0.00665789, -0.116106, 0.042588, 0.0817761, -0.000245356, 0.0808048, -0.0544788, -0.0417668, -0.0402295, -0.0250645, 0.014736, -0.00661647, 0.0247789, 0.0776008, 0.0467834, 0.00947928, 0.0522194, 0.122585, 0.0561356, -0.0138338, -0.0405669, -0.0416608, 0.00625144, -0.134176, -0.0607137, 0.0117132, -0.126487, -0.00926098, -0.0703142, -0.00933812, -0.0258482, -0.0448793, -0.0425934, -0.0210228, -0.0125317, 0.0510966, -0.0817795, 0.0181594, 0.043919, -0.0106192, 0.0837734, -0.00185301, -0.0986218, -0.0421384, -0.00637955, 0.0806215, 0.0681311, 0.0166484, -0.0039372, -0.030559, -0.0669781, -0.120751, 0.00254954, -0.0346016, -0.0229932, 0.0375526, -0.0242385, 0.0238031, 0.00595025, -0.0279113, -0.0728951, 0.0512001, -0.0490607, 0.054547, 0.0180244, 0.0267073, 0.0859388, -0.0517781, -0.0937267, 0.0416109, -0.0109434, -0.0702562, 0.0916787, 0.0288434, 0.0730911, -0.010117, -0.0364157, -0.00309525, 0.0373123, -0.0645862, -0.0183825, -0.0604975, -0.0498873, 0.00825584, 0.0027496, 0.0548209, 0.0943976, 0.00343624, 0.00430273, 0.0716549, 0.0978351, 0.0369948, 0.00887829, -0.0566219, 0.0185558, 0.03884, -0.0424526, -0.0707603, -0.012558, -0.0833939, -0.0384634, -0.0151988, -0.0495064, 0.0289831, -0.0932522, -0.0373102, 0.0533384, -0.0699627, 0.0106683, -0.00153416, 0.00631434, 0.0348445, -0.00918436, 0.02172, -0.0218828, 0.00682006, 0.00220312, -0.126658, -0.000422898, -0.0620594, 0.00615887, -0.032023, -0.0282725, -0.0408268, -0.0136328, -0.0627002, -0.015161, -0.0038218, 0.00202584, -0.11514, -0.027945, 0.01982, 0.0821937, -0.0741984, -0.0630574, -0.0392434, -0.0307126, -0.0534855, 0.0473283, 0.0545977, -0.0230992, -0.000873928, -0.0519869, 0.00797868, 0.0223816, -0.052061,
-0.00153895, -0.0131185, -0.00879448, -0.0114247, 0.033995, 0.0108455, 0.0171713, 0.0202445, 0.164238, -0.0619181, -0.0284872, -0.0760189, -0.0568547, -0.0539037, 0.0136655, -0.0490449, 0.154329, -0.0147312, 0.0742269, -0.0135899, 0.0341775, -0.0336709, -0.0246354, -0.0403815, 0.0158944, -0.0547964, -0.10147, 0.0872627, 0.0785853, -0.0139733, 0.0483304, 0.0802459, 0.0310954, 0.00874244, 6.04059e-05, -0.110592, -0.00692745, -0.00844773, -0.000241975, 0.044787, 0.00395802, 0.0733321, 0.0288498, -0.0683013, 0.00718437, -0.0619744, -0.00945216, -0.0658072, 0.00126977, 0.0322742, -0.0166506, -0.00957029, -0.00193724, 0.00373732, 0.0313831, -0.00157246, 0.0713007, -0.0339436, -0.0284905, 0.0222447, -0.020837, 0.0603485, -0.02233, 0.058561, -0.00475753, 0.0171485, -0.052117, -0.0132779, 0.0336822, -0.102136, -0.101366, -0.0580251, 0.00338674, -0.0141112, -0.0370512, 0.0449149, -0.0490593, -0.0308238, 0.0291578, -0.0626618, 0.101365, 0.00585344, -0.0598348, 0.0452912, 0.0516817, -0.0211223, -0.0405473, 0.0309103, -0.0323342, -0.0602849, -0.0290694, -0.0786059, -0.00888946, -0.103776, -0.0744594, -0.0323633, -0.110949, -0.121574, -0.0173668, 0.0501399, 0.0941339, -0.00413572, -0.0576008, -0.0767193, 0.0153675, 0.082824, -0.0100424, -0.0729629, 0.0475341, -0.0130998, 0.0154538, 0.0368244, -0.0590261, -0.079004, -0.164636, 0.0576622, -0.0112814, -0.0373923, -0.00569612, -0.0581765, 0.00817552, -0.055569, -0.0917041, 0.00697376, -0.100614, 0.0399748, -0.0162149, 0.0961069, 0.0587544, 0.079023, -0.0150724, -0.114183, 0.0895928, 0.0640349, 0.0900528, 0.00999679, 0.082007, 0.0292754, 0.0738624, -0.0992871, 0.00250543, 0.0325953, 0.043695, -0.0227785, -0.0153235, -0.0708871, -0.059907, 0.0104549, 0.0527196, 0.0138313, 0.0858368, 0.0639649, 0.0673888, 0.0275441, 0.0139293, 0.0437102, 0.0765964, 0.0887353, -0.0495177, -0.0632998, -0.107879, 0.0845965, 0.0328894, -0.0100581, 0.0692029, 0.0315848, -0.0215953, 0.119119, -0.0690379, -0.0173984, 0.0802472, -0.0132918, 0.0802649, 0.0282196, 0.109847, -0.0396805, 0.0296267, -0.060686, -0.00432189, -0.0502432, -0.0488499, -0.0497108, -0.0390072, -0.0206144, 0.0633456, -0.00713895, -0.0463763, 0.0580063, 0.0389341, 0.0704284, 0.0658066, 0.0789805, 0.0801323, 0.00954987, 0.076343, -0.0615895, 0.0143421, -0.035659, 0.0156217, 0.0879986, 0.0758909, 0.0350359, 0.0581217, -0.0114775, 0.0868745, -0.100134, -0.100902, -0.0534665, -0.010451, -0.0304812, -0.137399, -0.0270539, 0.130634, 0.103252, -0.0386121, -0.0329436, 0.103259, 0.0683928, 0.11273, 0.0858677, 0.125851, 0.0354843, -0.0348866, -0.0147053, 0.0864463, 0.0694443, -0.035552, 0.0104528, -0.028204, 0.00442445, -0.0116022, 0.0489862, -0.0922154, 0.0132561, -0.165028, -0.0868506, -0.0948438, -0.0722636, 0.052744, 0.160303, 0.194003, -0.0309585, 0.0344135, -0.083214, -0.0144256, 0.0758935, -0.0104814, 0.110992, 0.216774, 0.112337, -0.0197532, -0.0267049, -0.00559723, -0.035345, -0.120926, 0.152838, -8.09542e-05, 0.0548238, 0.0190614, 0.0408915, -0.0479524, 0.0320885, -0.0414178, -0.0248558, 0.0680129, -0.0929057, 0.0578373, 0.21063, 0.152326, 0.112657, 0.0899329, -0.0217877, 0.0805337, -0.0321319, -0.0174852, 0.0546134, 0.121397, 0.120282, 0.0033974, -0.0170884, -0.00377941, -0.0100997, -0.0154525, -0.0552999, -0.00105396, 0.0847179, 0.0228952, 0.00957821, 0.0615821, -0.0463929, 0.0597183, 0.0280269, 0.0967748, 0.0894189, 0.0828501, 0.10733, 0.105617, 0.149103, 0.0687636, 0.161683, 0.0311385, -0.0702514, -0.0911333, -0.116786, 0.0475044, 0.0483278, -0.00775282, -0.0773636, 0.0136466, -0.00862113, 0.0894852, -0.0108811, 0.0852436, 0.0504421, 0.0667466, -0.0601126, 0.107125, 0.107162, 0.0878133, 0.126745, 0.159554, -0.117434, -0.0657112, 0.131288, -0.030023, 0.160112, 0.0396662, -0.0300962, -0.0756524, -0.203372, -0.256716, -0.0797393, -0.175528, -0.0151882, -0.12184, -0.0644671, 0.09388, -0.060615, -0.0208158, -0.00537738, -0.0261543, -0.070777, 0.0340908, 0.0505481, 0.159502, 0.00890749, 0.162837, 0.0564988, 0.0410928, -0.0107373, -0.162877, -0.0106352, -0.0395572, -0.0473488, -0.0259825, 0.133434, -0.0827166, -0.0954095, -0.113819, -0.159599, -0.115349, -0.0592041, -0.0255488, -0.0539869, 0.0144479, 0.0187941, 0.0159416, -0.137863, 0.0938616, 0.00210566, 0.0255296, 0.206365, 0.145299, 0.0439803, 0.0264147, 0.00874086, 0.0716889, -0.113867, -0.144008, 0.0376933, -0.0155376, 0.168149, 0.0172237, 0.0278525, 0.00586032, 0.0404778, -0.0158929, -0.103761, -0.0624508, -0.0409929, 0.0421629, -0.0519589, 0.0444895, 0.052808, -0.015864, -0.0247267, 0.0272702, 0.0534513, 0.143044, 0.0760721, 0.0733576, 0.0294627, -0.0720532, -0.0881949, 0.0282329, -0.193002, -0.116127, -0.0590622, -0.0205663, 0.150218, 0.0512113, 0.105172, 0.124482, -0.0887176, -0.0344247, -0.0989834, -0.0110221, -0.0244455, 0.00918631, 0.0170902, 0.00835725, 0.00565842, 0.0261196, -0.00117895, 0.0382478, 0.0433995, 0.0816614, 0.0666636, 0.0512617, 0.00891071, -0.0800655, -0.037624, -0.044864, -0.15674, -0.137877, -0.0350314, 0.0243935, 0.121614, 0.0152935, 0.00440294, -0.0709653, 0.126796, 0.0812504, 0.129792, 0.0253513, -0.0428146, 0.00974126, 0.0540689, 0.00544665, -0.0245126, -0.00470142, 0.013518, 0.05583, 0.0186465, 0.0583703, -0.0350649, -0.0375623, -0.119882, -0.0683749, -0.0342295, -0.0389955, -0.0677047, -0.191409, 0.0332663, 0.0531504, 0.101039, 0.00421407, -0.0331099, 0.0730861, 0.119889, 0.0221733, -0.0436452, -0.0858425, -0.0032703, 0.0306592, 0.0441901, 0.0320364, -0.0049802, 0.031242, -0.00924252, 0.0190734, 0.000541122, 0.133586, 0.130056, 0.0656889, -0.105014, -0.0116349, -0.0992167, -0.080163, -0.0963709, -0.0580018, 0.117836, 0.169172, 0.170506, 0.129966, 0.0280517, 0.067448, 0.0569075, 0.0675194, 0.0451692, -0.0116874, -0.0970115, -0.0188524, 0.0393461, -0.0617069, 0.0939374, -0.0241419, 0.100468, 0.0296881, 0.0980792, 0.0852265, 0.0543032, -0.106618, -0.154562, -0.13717, -0.0823071, -0.0984429, -0.103714, 0.166507, 0.101616, 0.12787, 0.0260953, 0.0238304, 0.0234575, 0.128513, -0.00445146, 0.0436384, -0.0178357, 0.0292789, -0.0317692, -0.0584576, 0.0509763, -0.0440184, 0.106887, 0.0379055, 0.041823, 0.0272189, 0.133296, 0.0511859, 0.042958, 0.137598, -0.0388506, -0.142302, -0.117188, 0.019946, -0.0244114, 0.12213, 0.00554104, 0.0851681, -0.0239564, -0.0254842, -0.0108341, -0.0176278, 0.0773574, 0.000276012, -0.00275244, -0.0337479, -0.0600133, 0.0152277, 0.0154484, -0.00532888, -0.0257777, -0.00761578, -0.0452713, -0.00876264, 0.0829181, 0.0331026, -0.0613312, -0.0261661, 0.0782285, -0.0673594, -0.0511682, -0.075111, 0.124763, -0.011052, -0.087428, 0.0246832, 0.16523, 0.0915434, 0.0162667, -0.1189, 0.0441457, 0.0177947, 0.0618648, 0.0566656, 0.0641558, -0.0156088, 0.0698743, -0.0409662, 0.0169816, 0.01153, -0.058563, -0.102784, -0.00911086, 0.0115858, -0.0357717, 0.0485867, 0.0420402, -0.0628957, -0.00359275, -0.0513177, 0.0014598, -0.0473331, 0.121035, 0.0657204, -0.0200932, -0.0378678, -0.0244247, 0.0642517, 0.107582, -0.0306599, 0.0200038, -0.0147912, 0.0266754, 0.0506883, 0.0888217, -0.12624, 0.0644007, -0.0150006, 0.0431164, -0.0994712, -0.0657138, -0.0162048, -0.104831, -0.0556189, -0.01867, 0.0754045, -0.0551111, -0.0712859, -0.0361856, 0.018785, 0.0468378, -0.00827897, -0.0808474, 0.00395067, 0.0717278, 0.0755525, 0.112377, 0.0605603, 0.0899545, 0.116422, 0.0687805, -0.0503508, 0.0475076, 0.00273911, -0.0475446, -0.0222481, 0.00587198, -0.0283213, -0.0180085, -0.00179722, 0.0110097, -0.00086961, -0.0794029, 0.0820246, -0.0412567, -0.00853693, -0.0255078, 0.0469365, -0.083835, -0.00107066, -0.0605731, -0.00733949, 0.0730011, -0.068097, -0.0077417, -0.0287321, -0.00128855, 0.0475398, 0.0111191, -0.0297708, -0.0495369, -0.0338696, 8.37927e-05, -0.00953445, 0.0230504, 0.0377938, 0.0182013, -0.00690422, -0.0310972, 0.0193705, 0.0565033, -0.0106893, 0.0671449, -0.00630479, -0.00791961, -0.00181201, -0.0189578, -0.0183545, -0.0249143, 0.0382432, 0.0010704, 0.0425183, 0.00473549, -0.0625186, 0.0299509, -0.0337354, 0.0820964, 0.0226663, -0.0400638, -0.0703571, 0.0346908, -0.0138801, 0.0321152, 0.0968726, 0.0786833, 0.0220949, 0.0437243, -0.0321907, 0.0103995, -0.0358964, -0.168745, -0.0425404, 0.0375196, 0.00885963, 0.0441449, -0.0170139, 0.117991, -0.0432547, -0.0488519, 0.0134154, -0.101548, 0.0315312, -0.00967866, -0.0327939, -0.00306408, -0.0110635, -0.0613254, 0.0963852, 0.0255965, 0.0854645, -0.0327614, -0.00562077, 0.0663891, 0.0191218, -0.00116556, 0.0538862, 0.0281362, 0.0503935, -0.0479247, -0.0059297, 0.0266377, -0.0941432, -0.000170718, -0.0219327, -0.0247894, -0.0628753, 0.0704464, 0.0470464, 0.053822, 0.133472, -0.0478016, -0.000407383, 0.00585451, -0.021477, 0.00643225, 0.0269887, 0.000880335, 0.0241475, -0.0145409, 0.00709226, 9.9304e-05, 0.00723138, 0.0104766, 0.00269531, -0.0624756, -0.00223612, 0.0217812, 0.00365617, 0.00519987, -0.0571107, 0.0773624, -0.0282175, 0.0631519, -0.0612354, 0.0165443, 0.0528519, -0.00623186, 0.0557762, -0.0377119, -0.0141992, 0.0668868, -0.0326957, 0.0120551,
0.0402256, -0.0316493, -0.0282783, 0.0366424, -0.0252968, -0.0344785, 0.0843227, 0.0488583, 0.0850023, 0.0632471, -0.069368, -0.0156307, -0.0342814, 0.059604, 0.0168335, -0.0196214, -0.0135208, 3.76649e-05, 0.0149428, 0.0655285, -0.00788789, 0.088131, -0.00727537, 0.0647283, 0.089693, 0.0375687, 0.039382, 0.026785, -0.0319446, -0.0285951, -0.074677, -0.0836035, 0.0109487, -0.0237971, 0.00831408, 0.00991345, -0.0309799, -0.0945946, 0.0305035, 0.0289226, -0.0205872, 0.00996631, 0.0269052, -0.0273345, -0.00415271, 0.0716528, -0.0209553, -0.0403797, -0.0214194, -0.0759618, 0.0391724, 0.0693584, -0.0646473, -0.00144573, -0.0204721, 0.00441686, 0.0374803, -0.0282058, -0.0800824, 0.00521299, -0.0835371, 0.0566433, -0.0453021, 0.0400938, -0.036556, 0.0492736, 0.00880149, 0.00651264, -0.0125082, -0.0839386, -0.0928309, -0.063114, -0.0455055, -0.0547815, -0.0290158, -0.0757304, -0.0651785, -0.0399718, 0.0405337, -0.0129748, -0.0295282, -0.022334, -0.0184137, -0.0504471, -0.00752906, 0.0886319, -0.0202481, 0.0330783, 0.0254425, -0.0364073, -0.106482, 0.00831134, 0.0746778, 0.0034523, 0.0398582, -0.14157, 0.0756819, -0.0989698, -0.110651, 0.00801585, -0.144997, -0.021319, 0.0195178, -0.162073, -0.103729, -0.0592499, -0.095417, 0.0438477, -0.0231407, -0.015903, -0.0290159, -0.0389983, 0.0291202, -0.0310817, -0.105223, -0.00597399, -0.0285147, -0.0605255, -0.0153254, 0.00768185, -0.0127699, 0.0373428, -0.0826172, 0.0614124, 0.0305248, 0.0528296, 0.00471384, -0.0503022, 0.0408078, 0.0585804, -0.0704446, -0.108775, -0.0651608, -0.156172, 0.0112451, -0.0527956, 0.0261416, 0.0251476, -0.0509484, 0.0845271, -0.0406421, -0.0517736, 0.0679862, -0.0145732, -0.00792317, 0.0230385, 0.0834092, -0.00295684, 0.089138, 0.106506, 0.0833126, 0.106544, 0.096321, 0.0981107, 0.0180779, 0.0528106, 0.0767586, -0.0281785, 0.00566365, -0.064369, -0.085516, -0.108069, -0.0858629, -0.213978, -0.0115254, 0.0246312, 0.00245032, -0.00060764, -7.62209e-05, 0.0149438, -0.0727028, -0.0412811, -0.0185865, 0.0220547, -0.0805289, -0.0178401, 0.0984439, 0.163288, 0.18909, 0.161759, 0.079787, 0.0670919, 0.225058, 0.00889292, 0.173282, 0.14994, 0.00445017, 0.0280207, -0.0773487, -0.113702, 0.0429817, -0.0943505, -0.103067, 0.00446594, -0.0525084, -0.0670393, -0.0986092, 0.0469576, 0.0140843, 0.0211991, -0.0701619, -0.0885494, -0.0805513, -0.0242981, 0.0533416, 0.0723565, 0.21002, -0.0170917, -0.00765141, 0.197337, 0.0627818, 0.153675, 0.0973795, -0.000650555, 0.082293, 0.00817042, -0.154346, 0.043539, -0.0747105, -0.0353798, -0.120329, 0.00595482, -0.0354523, 0.0475107, -0.0194727, 0.00877115, -0.12816, 0.0697206, -0.0574429, -0.0303075, 0.018689, 0.0124014, 0.0823924, 0.0625522, 0.0483354, 0.0359326, 0.0143017, 0.0267456, 0.108838, 0.129792, 0.176915, 0.100332, 0.00258371, 0.0791878, -0.0181937, -0.00969543, 0.0384239, -0.00231134, 0.0167954, -0.00492993, -0.00755048, -0.0238189, -0.00835291, -0.0671635, -0.0241895, 0.0454569, -0.0684922, 0.038196, 0.0427827, 0.0217861, -0.0598738, 0.0254509, 0.10963, -0.0741521, -6.49679e-05, -0.0692322, 0.051366, 0.182344, 0.189068, 0.116132, 0.0633727, 0.0799158, -0.00759853, -0.00827219, -0.0161315, -0.0456812, -0.0333286, -0.00231115, -0.0402217, -0.0101736, -0.0247289, 0.0182412, -0.000485722, 0.000867946, -0.0687788, 0.0468122, 0.0417552, -0.00881816, 0.0017574, -0.0207346, 0.00776758, -0.129516, -0.081503, -0.177046, 0.111791, 0.104841, 0.198963, 0.0861112, -0.0504357, 0.0186461, -0.101205, -0.158169, -0.0337698, 0.127304, -0.0887386, -0.00749636, -0.0350008, -0.0356474, 0.0586862, -0.0544169, -0.0221887, 0.0525104, 0.0424136, -0.0207712, 0.092322, 0.0972935, 0.0165391, -0.0411258, -0.0356048, -0.0610777, -0.0361183, -0.0289701, 0.064463, 0.0716113, 0.0123353, 0.076222, 0.0546261, 0.0111174, 0.0384191, -0.0372509, 0.121437, 0.0654526, 0.0759509, -0.0171154, 0.0440368, 0.0393613, 0.033941, -0.0047222, -0.0923809, 0.0566232, -0.0409861, 0.0357181, 0.0770151, -0.0182322, -0.0175945, -0.0618108, -0.0195143, 0.0203687, -0.10212, -0.0441631, -0.0468604, -0.0519575, 0.0116364, 0.00722788, -0.0472212, 0.213126, -0.00219311, 0.0339692, 0.162336, 0.111083, 0.0578561, -0.0235176, 0.0104008, -0.0447986, -0.03881, 0.131044, -0.0303801, 0.000292418, 0.129585, -0.0102883, -0.0704053, 0.017028, -0.000690535, -0.108475, 0.00209293, 0.128725, 0.0558745, -0.0115432, -0.0610275, -0.00826038, 0.057832, 0.034938, -0.020534, 0.0547322, 0.0828783, -0.0386391, 0.0295537, -0.000763407, -0.0554115, -0.000683114, -0.0753108, 0.0168927, 0.0450812, 0.0670816, -0.0347005, -0.0601224, 0.0355111, -0.00587009, -0.0966318, -0.0406972, -0.0960658, -0.13114, 0.0877961, 0.104867, 0.0183853, 0.0758226, 0.00615509, -0.0398266, -0.00915275, 0.0824148, 0.121651, -0.0483536, 0.0303457, 0.0427114, 0.0191441, -0.0527591, -0.0602082, -0.0102024, 0.0226445, 0.0242526, 0.00958995, 0.0170292, -0.0751806, 0.0243337, 0.0661509, 0.00892128, -0.0311384, -0.107483, -0.0279919, 0.0246609, -0.0021365, 0.0252922, -0.0261837, -0.0310861, -0.021368, 0.122317, 0.132151, 0.047134, 0.0535246, -0.0181702, -0.0377426, -0.0490647, -0.0208324, -0.0888391, 0.043772, 0.00444962, 0.0536669, -0.00314501, -0.0150091, -0.0388679, 0.0371444, -0.0985002, 0.00880243, -0.00252727, -0.00135532, -0.0336085, -0.0537782, -0.0645401, 0.0378344, 0.00684885, 0.0547874, 0.0141181, -0.0374558, -0.0314348, 0.0477144, 0.00698513, -0.137746, -0.089052, -0.093948, -0.219915, -0.0759896, 0.000422271, 0.00761074, 0.111781, -0.0453429, 0.0533227, -0.0333556, -0.061319, -0.0766213, -0.10353, -0.0664106, 0.107016, 0.100054, -0.0798193, 0.0134317, 0.151422, 0.0857313, 0.0895409, 0.159365, 0.0446359, -0.0492254, 0.0873997, -0.00608856, 0.00579192, -0.147062, -0.0924586, -0.0986146, -0.164516, -0.0935895, -0.159072, -0.0715811, -0.0162296, 0.00361296, 0.0154351, -0.0197512, -0.0236628, 0.0589629, -0.0672244, -0.109732, 0.0124436, 0.00945974, 0.0841262, 0.0422593, 0.168277, 0.166669, 0.261367, 0.139715, -0.0120721, -0.0478901, -0.0753613, -0.0683783, -0.07158, -0.0424936, 0.00959468, -0.0164535, -0.0592923, -0.127746, -0.00339062, -0.0214514, 0.0253104, -0.0402608, 0.0313362, 0.0397576, -0.0291112, -0.0274126, 0.0206295, 0.00833839, -0.0247226, -0.00686472, -0.00195322, 0.0530491, 0.102694, 0.160765, 0.115895, 0.16785, 0.0229055, -0.0576398, -0.0249534, -0.1359, -0.0415552, -0.0558796, 0.0413568, -0.00461864, -0.113248, 0.142346, 0.0431532, -0.0382877, 0.0394685, 0.0578925, -0.0419919, -0.00946669, 0.0268214, 0.0958003, -0.0508628, -0.0558077, -0.000547955, 0.00797609, 0.0544634, 0.0508433, -0.110571, 0.0210478, 0.0215889, -0.0664639, -0.0764591, -0.140858, -0.035186, 0.0115567, -0.033913, -0.0520422, -0.028062, -0.0929899, -0.0394835, 0.067518, 0.161673, -0.0423506, 0.0496008, 0.0270338, 0.00234995, 0.0446658, 0.0350734, 0.00264652, -0.0201494, 0.0701499, -0.00072766, 0.0660375, 0.0485873, -0.0318511, -0.0203125, -0.104256, -0.135147, -0.110104, -0.135705, -0.117765, -0.041127, 0.0036969, 0.0523906, -0.0598813, -0.11102, -0.0183581, 0.0473362, -0.0108664, 0.126271, 0.0599496, -0.0392658, -0.0108135, -0.0382501, 0.0271076, -0.0203447, 0.00906007, -0.0800271, 0.0100336, 0.0882045, 0.0785602, 0.085858, -0.00285622, -0.05385, -0.0477843, -0.113397, -0.112547, -0.0199556, 0.0863089, 0.00568396, 0.0488603, 0.0653138, -0.036667, -0.101288, 0.0196356, 0.0470333, 0.0130958, 0.0693854, 0.0270215, -0.000733849, -0.0198611, 0.0717966, 0.0376844, -0.0639641, -0.022659, -0.0847825, -0.0848072, -0.0836166, 0.012302, -0.0794646, -0.0157428, -0.0976331, -0.0228142, -0.0313524, 0.0770957, 0.124343, -0.0647483, -0.0455315, 0.0286461, 0.0630866, -0.140183, -0.0622357, -0.0137004, -0.00297809, 0.0225235, 0.0723425, 0.0467396, 0.0487515, 0.0221653, -0.079753, -0.0968751, -0.0166538, 0.00945113, -0.0108745, -0.0211771, -0.0775988, -0.04809, -0.0682705, -0.0636748, -0.062399, -0.0454541, -0.0128544, 0.0720809, 0.00687668, 0.0357255, 0.0720538, -0.038786, -0.0270659, 0.0210716, 0.111577, 0.115771, 0.0627647, 0.104459, 0.097549, 0.0913595, 0.0216053, 0.0361112, -0.0366868, -0.0679309, 0.0650985, -0.0277155, 0.0242166, -0.0206473, 0.00912762, -0.0251691, -0.0395706, -0.00564432, -0.0560459, -0.049582, 0.111253, 8.11905e-05, 0.11738, 0.0685955, 0.0463919, 0.050924, -0.0199866, 0.0194323, 0.0677908, 0.0632759, 0.0373984, -0.00269344, 0.0316674, -0.108443, 0.0184266, 0.142039, -0.00923359, 0.00667655, 0.00238569, -0.0185892, -0.00524799, -0.0765926, 0.0389533, 0.062364, -0.0491001, 0.0246007, 0.0205895, 0.0466783, 0.0907245, 0.0579754, 0.000665293, 0.175573, 0.0462835, 0.054174, 0.11296, 0.109396, 0.0435404, 0.0214079, 0.00241109, 0.0180328, 0.0790969, 0.0919034, 0.0296161, -0.0982976, 0.00643971, -0.00315257, -0.0973628, 0.105945, -0.0337499, -0.0290219, -0.0286716, -0.0242897, 0.0416959, 0.0403407, 0.0306677, -0.00921804, 0.0174628, -0.0485555, 0.0420145, 0.0449763, -0.0692094, 0.0737899, -0.0166596, -0.0556562, -0.0111286, 0.0319246, -0.0345015, 0.0371666, -0.0301815, -0.00137493, -0.031123, -0.0524068, 0.084967,
0.00199321, -0.0245662, 0.00401068, 0.0473842, -0.0707369, -0.0332794, 0.0979571, 0.0543923, 0.00926008, 0.0712219, 0.0101502, -0.00741037, -0.014878, 0.0548994, 0.0188389, -0.123996, -0.0052597, 0.0712146, -0.0159026, -0.0693991, -0.0640084, 0.00487819, 0.00458577, 0.0731876, 0.036865, 0.0268795, -0.0255457, 0.101685, -0.0261871, 0.0403842, -0.0170042, -0.0125055, -0.0375133, 0.0831857, 0.0560002, -0.0182045, 0.0251126, -0.0236462, 0.0396503, -0.0160264, 0.00518468, 0.042501, -0.0459622, 0.0653962, 0.096029, -0.0528584, 0.0692795, 0.0352124, -0.0349143, 0.00612971, -0.0248684, -0.00499229, 0.0149227, -0.0261324, -0.0174164, -0.0398472, -0.0208157, 0.0181025, -0.0463506, -0.00815742, -0.0733005, 0.0258287, 0.0810716, -0.0887749, -0.0449459, -0.00619995, -0.0155437, -0.112534, 0.0315879, -0.0615757, 0.0306625, -0.00752694, -0.0892322, -0.0568732, -0.0717649, -0.0831449, -0.0655248, -0.0504781, -0.0276738, -0.0343169, -0.0802919, 0.0257818, -0.0380615, 0.0193013, -0.107626, 0.0363309, 0.0140704, -0.0112951, 0.0579475, 0.0439618, -0.0505746, 0.0106579, 0.0675353, -0.0406737, -0.0155559, -0.041168, 0.0654437, 0.0047524, 0.0729641, 0.115142, 0.0157862, 0.104829, 0.0251272, -0.0378135, 0.0554502, -0.00129133, -0.0783504, -0.110004, 0.116269, 0.0293291, 0.049995, 0.0665008, 0.023518, 0.0420127, 0.0116008, 0.00840794, -0.0379896, -0.0251506, 0.020677, -0.05956, 0.0437204, 0.0498848, 0.0465254, 0.114547, 0.11568, 0.14998, 0.151706, 0.149497, 0.0196217, 0.0488205, 0.119146, -0.0127858, -0.00223304, 0.0258404, 0.00589126, 0.00824515, -0.0701791, -0.00221033, 0.0561537, -0.00390023, 0.00172348, -0.0212639, 0.00117535, -0.0306111, -0.00649462, -0.0215152, 0.0128216, -0.0359136, 0.0680357, -0.00393002, 0.13402, 0.020859, 0.0644634, 0.130702, 0.167012, 0.113102, 0.0862074, 0.0874683, 0.0979778, 0.0840818, 0.113404, -0.0267881, 0.15957, 0.0467467, 0.0267914, -0.0613357, 0.118889, -0.0576532, 0.073212, -0.0253931, -0.0450791, -0.0231842, -0.0511126, 0.00284848, -0.0292327, 0.0132529, 0.0352476, 0.0477474, 0.164818, 0.057494, 0.0564437, 0.112237, 0.0716582, 0.0933915, 0.1041, 0.182798, 0.112376, 0.0783436, 0.158451, 0.0744962, 0.106061, 0.00536779, 0.0734178, -0.0324339, 4.85997e-05, -0.0656421, -0.0834062, 0.0990797, 0.0390399, 0.0161615, -0.0107458, 0.0341996, -0.116109, 0.0163235, -0.0292677, -0.061289, -0.0566836, 0.0717602, 0.140793, 0.195735, 0.112184, 0.110701, 0.195839, 0.125246, 0.169788, 0.0952521, 0.0766444, 0.117559, 0.0290949, 0.0084791, -0.0433137, -0.0621491, 0.0594224, 0.113248, -0.0392634, -0.115468, 0.0539104, -0.0903149, 0.055781, -0.0152802, -0.0232895, -0.0113694, 0.10669, 0.0383724, 0.00615637, 0.00851909, 0.0738397, 0.124021, 0.119515, 0.139392, 0.255246, 0.177188, 0.10584, 0.0973808, 0.0856114, 0.0698168, 0.0719671, 0.00763486, -0.0897737, 0.00629502, -0.0279351, -0.050961, 0.0484653, 0.0212942, -0.0177716, 0.00321462, 0.126323, -0.109531, 0.0392238, 0.0910095, -0.120416, -0.0887629, -0.0280469, -0.0813437, -0.0628617, -0.053599, 0.0167093, -0.0305649, -0.0154406, 0.0638002, 0.032659, 0.00509533, 0.0124877, -0.0406827, 0.0404451, 0.0462668, -0.0586165, 0.0365809, -0.0122148, -0.0141881, -0.0145245, 0.0128811, -0.0413814, 0.0487738, -0.00232961, -0.129897, -0.0426194, -0.049521, 0.000399778, -0.0413979, -0.0754266, -0.137049, -0.0923456, -0.0249605, -0.25237, -0.146747, -0.215543, -0.0335971, -0.0263698, -0.0793446, -0.044379, -0.0433769, 0.0386939, -0.0540719, 0.0456695, 0.063863, 0.0181399, -0.073051, 0.0068876, -0.00195583, -0.0504428, 0.00394172, 0.0124201, -0.207685, -0.151814, -0.136175, -0.0238721, -0.110264, 0.071842, 0.0219266, -0.06193, -0.0334895, -0.151412, -0.207114, -0.261406, -0.101808, -0.0629352, -0.0615002, -0.140972, -0.0501877, -0.0462992, -0.0222704, 0.0375677, 0.0289448, -0.000604222, 0.0259668, -0.0329425, 0.0157665, -0.0516319, -0.0141729, -0.0709755, -0.094571, -0.0399702, -0.0236894, 0.00183098, 0.0721816, 0.107798, 0.103144, 0.0865504, -0.0443347, -0.0848085, -0.113287, -0.172132, -0.138963, -0.0858012, 0.0215075, -0.0458822, -0.0268197, -0.109751, -0.107914, -0.0475701, -0.0646508, -0.0162927, -0.0131924, 0.0246136, 0.0190162, 0.0559713, -0.0171267, -0.0250352, -0.155027, 0.0492118, 0.0143981, -0.0161583, 0.125984, 0.0452239, -0.0178531, 0.00256353, 0.00770755, -0.0181998, -0.116695, -0.213639, -0.0880507, -0.0648435, 0.0326661, 0.0231187, -0.0725393, -0.116446, -0.217909, 0.0127362, -0.0132743, -0.0935902, -0.0885834, -0.0168706, -0.00408227, -0.0138116, -0.0859848, 0.00431511, -0.0293697, -0.0794164, 0.0515463, 0.00557765, -0.0317592, -0.0573573, -0.142326, -0.0359083, -0.0221765, 0.105806, -0.00173342, -0.0995881, -0.142961, -0.107258, -0.0832624, 0.0501697, 0.107485, -0.068948, -0.0869382, 0.0286169, 0.0290672, 0.0552176, -0.0275001, 0.0151343, -0.0419896, -0.00487664, -0.00379101, -0.00741369, -0.015198, -0.0418102, -0.0323704, -0.0280598, -0.0339539, -0.0335367, -0.0566927, -0.0356704, -0.0749087, -0.0871304, -0.0584095, -0.0642191, -0.105921, -0.0162944, 0.0181198, -0.0328847, 0.0321215, -0.0233275, -0.0930797, -0.0157795, 0.00495951, -0.0231441, -0.0166154, -0.0173371, -0.0245038, -0.0987771, 0.0129065, -0.075115, -0.0721445, -0.0959448, -0.18019, -0.122375, -0.035632, -0.0213209, 0.02699, 0.027965, -0.0415516, -0.0593743, -0.0413128, -0.0339317, -0.12673, -0.0401767, -0.00996629, 0.100847, -0.00588378, 0.048241, -0.0199192, -0.0196821, -0.0322422, 0.10418, -0.0400043, 0.0931775, 0.00770991, -0.0116478, 0.0155148, -0.0688701, -0.110974, -0.198423, -0.0278648, -0.0683659, -0.0269978, -0.0112306, -0.0974957, -0.103305, -0.0618551, -0.209746, -0.168382, -0.221031, -0.0223503, 0.0968146, 0.0397093, 0.0169837, -0.0135495, 0.0452237, -0.0111737, -0.111526, 0.0785731, -0.0190884, -0.0334818, -0.0718208, 0.0478909, -0.0425679, 0.0873429, 0.00165057, -0.0555187, -0.083129, -0.0860248, -0.0566498, -0.210249, -0.0959087, -0.193735, -0.255119, -0.283518, -0.156817, -0.053768, 0.00652906, 0.0126991, -0.0121154, 0.063016, 0.0467466, 0.0809856, -0.0312702, -0.0850581, 0.00463454, 0.0217628, 0.0649209, 0.0318574, -0.0217906, 0.0749235, 0.0517521, -0.0321838, 0.0561873, 0.0825342, -0.118124, -0.0614471, -0.0878721, -0.142008, -0.170522, -0.171519, -0.106303, 0.018978, -0.117307, -0.0435052, -0.000142096, 0.109749, 0.0240531, -0.0294667, 0.0743832, 0.0590157, -0.000873656, -0.019218, 0.00144907, 0.0304239, -0.0182242, -0.0511809, -0.0724957, -0.0185504, -0.00931833, 0.0447164, 0.0406567, 0.0484522, 0.0983189, 0.077608, 0.0105157, 0.0676673, 0.0534095, -0.0238309, 0.0200034, 0.117102, -0.000368881, 0.127519, 0.146808, 0.123604, 0.0107573, 0.0487073, 0.0518307, 0.105115, 0.0123566, 0.0240393, 0.0435949, -0.036436, 0.0731991, 0.0603529, 0.0278576, -0.0517228, -0.0183217, 0.132473, 0.0555688, 0.100693, 0.0964297, 0.134557, 0.167554, 0.0912561, 0.0825883, 0.177375, 0.221254, 0.13828, 0.250791, 0.156153, 0.272528, 0.0236531, 0.131453, 0.0290273, 0.0920218, 0.11202, 0.0617759, 0.0951617, 0.00664971, 0.020436, -0.0285569, 0.0265008, 0.0846661, 0.00419736, -0.0479568, -0.0666845, -0.0312991, 0.0743038, 0.189259, 0.195397, 0.196994, 0.102266, 0.154785, 0.2262, 0.20391, 0.148562, 0.103991, 0.146469, 0.137395, 0.134576, 0.0760017, -0.00821577, -0.00520089, 0.0354701, 0.0714573, 0.0988053, -0.0175052, -0.0316831, 0.000989198, -0.0135697, 0.0573732, 0.0576273, 0.00226971, 0.0575152, 0.0305371, 0.095307, 0.0802575, 0.144674, 0.120582, 0.222179, 0.340091, 0.305018, 0.190625, 0.243001, 0.167414, 0.144065, 0.0893442, 0.0979154, 0.0585575, 0.109408, -0.0179716, 0.104894, 0.0939087, 0.071396, -0.00368661, -0.00501108, -0.0235309, 0.0363276, 0.0267794, -0.0689561, 0.0231593, 0.0331616, 0.0779303, 0.0113152, 0.131105, 0.115607, 0.112803, 0.245392, 0.0499777, 0.115005, 0.196252, 0.16175, 0.0866147, 0.141483, 0.0272729, 0.128804, 0.0975552, 0.153258, 0.0463325, -0.0430002, 0.0641552, -0.00275066, -0.0168023, -0.119743, 0.040477, -0.0568681, -0.0149278, -0.00302966, -0.02825, 0.0109007, -0.00983564, 0.024565, 0.122797, -0.0184452, 0.0915106, 0.162722, 0.000552794, 0.00741093, 0.0484797, 0.0857722, -0.0970464, -0.0418383, -0.0139172, 0.119418, 0.00202061, 0.0253308, -0.00947604, 0.0287988, 0.0189658, 0.0163461, 0.0668104, -0.00820422, -0.0430492, -0.132099, -0.0106339, 0.0364858, -0.0274809, 0.0145518, -0.0450109, 0.0105, -0.0490488, 0.0249909, -0.0246817, -0.080553, -0.117974, 0.0112827, 0.0108559, -0.068897, -0.080906, -0.0203578, -0.0868525, -0.0367882, -0.026864, 0.0651046, -0.0695718, 0.0758651, 0.0112597, 0.0304439, -0.091648, -0.0757684, 0.0370349, -0.0361384, 0.0335473, -0.0459278, -0.0426508, -0.0514939, -0.00264931, 0.0831647, 0.0420087, -0.0658544, -0.0210624, 0.0943478, -0.053012, -0.0526668, -0.0408708, -0.0421743, -0.0136826, 0.032608, 0.00166812, -0.0876097, -0.00863105, -0.00986376, 0.0453823, 0.0818551, 0.0085291, 0.0177426, 0.0103517, -0.0535082, 0.0148361, -0.0221342,
0.00512744, -0.00409462, 0.0196046, 0.0207385, -0.0810929, -0.0124221, -0.0117628, 0.000340856, -0.0257055, 0.0446449, -0.0268218, -0.0846367, 0.0133035, 0.0389695, 0.0185574, -0.0190731, 0.03996, 0.0313326, -0.0588891, -0.126558, 0.00562413, -0.13194, -0.0308159, 0.0202829, -0.025891, 0.039363, -0.0762309, 0.0493113, 0.0315177, -0.0370211, -0.0254773, 0.0564522, 0.0116793, 0.0210047, -0.128254, 0.00386333, -0.0177463, -0.0764986, -0.000412392, 0.0150311, 0.000511239, -0.0247778, 0.0112708, -0.0392454, -0.0620918, -0.0304324, -0.0314757, 0.0532593, 0.0793433, 0.0169998, 0.015667, -0.022958, -0.00457271, -0.0669339, 0.00148713, -0.100406, -0.0489505, 0.0969544, 0.00161547, 0.0325343, 0.0668415, -0.116638, -0.037674, -0.0116228, 0.0174023, 0.085957, -0.0219821, -0.0560885, 0.0191676, 0.0457388, 0.071076, 0.00342495, 0.0476374, -0.0114827, -0.0728495, 0.127268, -0.0700104, 0.0195984, 0.00370975, -0.028221, -0.0611956, 0.0570169, 0.0221596, -0.0109761, -0.0133967, -0.108058, 0.0227135, -0.0366742, -0.0456943, -0.0270372, -0.091842, 0.074854, -0.0218156, 0.00909335, -0.139599, -0.078387, 0.00624586, -0.20305, -0.157288, -0.0777368, -0.0630162, -0.0691259, 0.0268498, 0.0544765, 0.00589101, -0.0251751, 0.0768724, -0.00858142, -0.0725915, -0.0414202, -0.0887931, -0.0562962, -0.0363132, -0.0701685, -0.0170019, -0.00558979, 0.0133463, -0.0599641, 0.0997334, 0.0211007, 0.103295, -0.00348472, -0.0246987, 0.0159133, -0.0724144, -0.132791, -0.217576, -0.167866, -0.125368, -0.100445, -0.00973426, -0.0544916, 0.00939866, -0.091472, 0.0406649, 0.0828894, -0.0239025, -0.0391555, -0.0302866, 0.0578453, 0.0999596, 0.0692746, -0.00915879, 0.0262111, 0.021673, 0.0390929, 0.0289717, 0.0932549, 0.072095, 0.0622495, 0.0878231, 0.0492299, 0.0428765, -0.0593467, -0.146572, -0.0574501, -0.159012, -0.0907565, -0.0177044, 0.0520114, -0.173444, 0.00360942, -0.0340088, -0.0257617, -0.0215389, -0.00976694, -0.0151006, -0.0288165, 0.0439756, 0.0269839, -0.0342054, 0.0407449, 0.0291612, -0.0728102, 0.0814682, 0.0974103, -0.0219942, 0.0466144, -0.0149077, -0.0271694, 0.110117, 0.116525, -0.0452797, -0.0519902, -0.0827765, -0.0492942, -0.0373502, 0.0745523, -0.0282606, -0.014534, 0.0242457, 0.0860397, -0.0719185, -0.0631556, -0.0646527, 0.0361517, 0.00559652, 0.034712, -0.0048562, 0.11957, -0.0284437, 0.0439978, 0.0576459, -0.015204, -0.0332274, -0.0647304, -0.0866515, 0.0260892, 0.109497, 0.135201, 0.120215, -0.0393812, -0.200016, -0.000384142, -0.0253094, 0.021018, -0.0038933, -0.0241015, -0.177868, -0.0328018, 0.0782048, 0.0229653, 0.0478594, -0.0267222, 0.0385101, -0.00318368, 0.0167197, -0.0291649, 0.00355234, -0.0785339, -0.0483171, -0.0500892, -0.129582, -0.042526, 0.0572677, 0.216948, 0.178621, 0.184252, 0.106807, -0.120744, -0.100712, -0.166824, -0.0195776, 0.0208386, -0.110524, -0.0353875, -0.0368973, 0.0214912, -0.00995488, 0.0247125, -0.027705, -0.0616576, 0.00359864, 0.0183597, 0.0406171, 0.0806302, -0.0482394, 0.00786088, -0.00874556, -0.0734375, 0.0025089, 0.081482, 0.0922682, 0.156957, 0.260687, 0.18878, 0.11102, 0.030115, -0.00328744, -0.0842202, -0.0652316, -0.0623302, -0.0103224, -0.0287572, 0.0968604, 0.0979509, -0.0829389, 0.027814, -0.151733, 0.0221761, -0.00122303, -0.0235238, -0.058933, -0.00331789, -0.0434329, -0.0203661, -0.10534, 0.0694728, 0.016213, -0.0505278, 0.0760454, 0.129562, 0.120176, 0.0565303, 0.103802, -0.00151963, 0.0483979, -0.0442024, -0.0874334, -0.159151, -0.0585294, 0.0862911, 0.0506783, -0.0403646, -0.032375, -0.0499375, -0.0422497, -0.0338179, -0.0716767, 0.0532943, 0.0925091, -0.0149675, 0.0263265, 0.0319702, 0.101291, -0.0116669, -0.039684, 0.00645924, -0.0121465, -0.044205, 0.00958461, -0.0161582, -0.00322179, 0.00172296, 0.0492967, 0.0490002, -0.00778251, -0.139827, -0.113072, 6.78384e-05, 0.0275645, 0.0705823, -0.00383397, -0.00584501, -0.0262246, 0.0350846, 0.0276003, 0.0835162, 0.0727978, -0.0564636, 0.106394, -0.0259025, -0.0615644, -0.0443298, -0.00739504, 0.0624625, 0.0129341, -0.0184528, -0.018896, -0.0465093, 0.0014277, 0.0317552, 0.0582586, 0.04984, 0.00309718, -0.107018, 0.0831799, 0.0726324, 0.0848722, -0.0841755, -0.0700218, 0.0226104, 0.0319988, 0.0284897, 0.00084494, 0.0441246, 0.0722221, -0.00189978, -0.00895965, 0.0667384, -0.00581889, -0.095095, -0.0447368, -0.0501668, -0.151297, -0.0533075, 0.0647648, 0.0747577, -0.0403823, -0.0762557, -0.0276557, 0.0871451, 0.0692618, -0.116065, -0.118172, -0.049684, 0.112129, -0.0587827, 0.0663488, 0.127263, 0.0978521, 0.123223, 0.051983, -0.0108097, -0.0596342, -0.0776996, -0.102441, -0.0249375, -0.147255, -0.0675595, 0.0295967, -0.0369778, -0.115017, -0.036247, 0.0061597, -0.123547, -0.0666315, 0.00932634, -0.106142, -0.0418189, -0.05483, -0.0162302, -0.0706199, 0.0410423, -0.0730595, -0.05187, -0.0111677, -0.00253588, 0.111008, 0.0264533, 0.0238046, 0.119777, 0.000383656, -0.0616462, 0.00209742, -0.0762226, -0.019361, -0.0461322, -0.0713937, -0.0460581, -0.0984233, 0.0875826, 0.0188424, 0.085126, 0.0707489, 0.186509, -0.0309894, -0.00663743, -0.0848868, 0.0357167, 0.106062, -0.0365742, -0.0329696, 0.00170256, 0.024699, 0.0118992, 0.0354225, -0.0250975, -0.103807, 0.0381087, 0.000156699, 0.0119469, 0.0715638, -0.0917266, 0.0711649, 0.0184303, 0.0737487, -0.000336931, -0.0123369, 0.171859, 0.0513215, 0.196325, 0.1597, 0.038364, -0.0206351, -0.0797512, 0.0140967, 0.0780922, 0.0759561, -0.00384277, 0.0753453, 0.0570551, 0.00641261, -0.0342166, 0.0205262, 0.0488861, -0.123118, 0.00636583, -0.0199699, -0.0478479, 0.0705483, 0.121225, -0.00868224, 0.0785525, 0.0109175, 0.110833, -0.0830768, 0.0693604, 0.140401, 0.154068, 0.0884352, 0.0265828, -0.106458, -0.0376797, 0.00804836, 0.00271534, 0.12299, 0.0611135, 0.0461486, 0.054518, -0.0177351, 0.0317638, -0.0303382, 0.0293814, 0.0782976, 0.0279422, 0.0134018, -0.0817536, -0.0323102, 0.0326081, -0.0188033, 0.0668029, -0.106303, -0.020328, -0.0103257, 0.0995131, 0.0958151, 0.143117, -0.0408324, -0.0516277, -0.0142584, 0.0854036, 0.0478083, 0.111861, -0.0143237, 0.0863439, 0.043352, 0.0943654, -0.0213329, 0.000978749, -0.00268662, 0.000502934, -0.0151254, -0.00447193, -0.0200881, 0.0105676, 0.121163, 0.0126806, -0.0270082, -0.0551826, 0.0238607, -0.141219, 0.00199968, 0.0730982, -0.0453774, 0.0311631, -0.0545886, -0.0719301, 0.0345289, -0.0580277, -0.0512755, -0.00789767, 0.0561554, 0.0337022, 0.0479157, 0.175008, 0.0680642, 0.0681419, -0.0235562, -0.0689386, -0.0205073, -0.0154275, 0.00804471, 0.0162509, 0.0386083, 0.00683983, 0.0196295, 0.0315757, -0.0671109, -0.0636801, -0.226597, -0.0507526, -0.102006, -0.0962626, 0.0192569, 0.068344, -0.00443332, -0.104553, 0.00197364, -0.0285674, -0.00826244, 0.0719364, -0.00592305, -0.0689718, -0.103841, -0.0106656, 0.0217835, -0.0948858, 0.0118112, -0.0363606, 0.047021, -0.0245883, 0.0642312, 0.0340366, 0.00579904, 0.050454, 0.0641539, -0.0736265, -0.0724455, -0.0644035, -0.0887329, -0.0484668, 0.122293, -0.0572027, -0.0158679, -0.0924346, -0.069507, 0.0687448, 0.035095, -0.0129014, 0.0667983, 0.0461865, 0.00546385, -0.0428449, 0.0856817, 0.137722, 0.108076, 0.0187802, 0.0272697, 0.000424476, 0.0866771, -0.0386072, 0.109733, -0.0886204, 0.126672, -0.0744149, -0.0831674, -0.0978664, -0.0019562, -0.0123221, -0.0712385, -0.0552265, -0.0604321, -0.125654, 0.0245144, -0.0436515, -0.0161773, 0.071989, 0.0248676, -0.036126, 0.0291455, 0.00188732, -0.01267, -0.083816, -0.0107484, -0.086209, -0.0652552, -0.113714, 0.0191128, 0.0168711, -0.0187319, -0.0317192, -0.0445106, -0.0833174, -0.000569903, -0.0419484, -0.0477272, 0.010569, 0.0291528, 0.0342973, -0.0834221, -0.0402004, -0.0342115, -0.130424, 0.00892937, -0.0460168, -0.0431473, -0.0702941, 0.0550592, 0.00585431, -0.0587542, 0.0295397, 0.0347523, -0.0570894, -0.002894, 0.023886, 0.00620888, 0.0107282, 0.0845538, 0.0229337, 0.0993001, 0.0590431, -0.00848407, 0.033923, -0.0484221, -0.023851, 0.046757, 0.0567274, 0.0122766, 0.0216161, -0.0167005, -0.0168745, 0.0162031, 0.0688312, -0.00500904, -0.107325, 0.0753658, -0.0105404, 0.0577779, 0.00173622, -0.0020716, -0.00383195, -0.032333, -0.0330132, 0.0265884, -0.00582426, 0.0586397, -0.0201293, 0.0697112, -0.0305703, 0.0589734, 0.0238968, -0.00762815, -0.0308026, -0.0201433, 0.0453251, 0.0674222, -0.0289075, -0.00270434, -0.034986, -0.0468009, -0.0826885, 0.0514062, -0.00669682, -0.0536867, 0.0583826, 0.000105787, 0.0844559, -0.00718623, 0.0776692, 0.0746568, 0.0342794, -0.0321304, -0.0366474, -0.0470221, -0.0572591, 0.0752745, -0.0303784, 0.0409178, -0.00207011, -0.0101203, -0.0109461, 0.0756812, -0.0623122, -0.0506618, -0.0935434, -0.0262962, -0.00465907, 0.0365149, -0.0477456, 0.0368123, 0.0268561, 0.0116944, -0.0305018, 0.0516967, 0.0391069, 0.0438221, 0.0102882, 0.069683, -0.00456316, -0.0070453, 0.0632239, 0.0828554, 0.0304284, 0.0487871, -0.0355004, 0.00922736, -0.00430746, -0.0166347, -0.0459169, 0.0139293, -0.0862012, 0.0238754, -0.0802175, 0.0104982, 0.0385368, -0.102519, 0.0760647, 0.0621362, -0.0429124, 0.0337566, 0.0999116, -0.0128941, -0.0360251,
0.105216, -0.0701638, 0.0680157, 0.0342924, -0.109297, 0.0299573, -0.0611148, 0.0221742, 0.0144733, -0.0208308, 0.0321437, -0.0498457, -0.0715695, 0.113926, -0.0289209, -0.0199858, 0.00382392, -0.0179121, -0.065006, 0.0682424, 0.00237548, 0.00434312, -0.0334959, 0.0275484, -0.0586918, -0.0078939, 0.0188564, -0.0502179, -0.0738971, -0.0928224, 0.00704909, 0.0315203, -0.100331, -0.0687144, 0.0177112, -0.045981, -0.0977394, -0.10217, 0.0723559, -0.0109848, -0.0182941, -0.0384954, 0.0566195, 0.0630995, -0.0109288, 0.0125039, 0.0550648, -0.0408477, 0.0118074, -0.0190515, 0.0360112, -0.0765449, 0.0223695, 0.0446777, 0.00646574, -0.0245804, -0.00138339, -0.108868, -0.00249124, 0.00290818, 0.08967, 0.00789085, 0.0942404, -0.00387224, 0.023963, -0.025401, 0.0950101, 0.000713194, -0.163361, -0.0951169, -0.0351135, 0.0085957, -0.00859118, -0.0571642, -0.0579239, 0.0118644, -0.100484, 0.0453232, 0.00243634, 0.00467109, 0.111207, -0.0563503, -0.0149269, -0.0344154, -0.020689, -0.0488111, -0.0749046, -0.0540578, 0.00383735, -0.0695046, 0.142641, -0.009217, -0.00304733, -0.0400589, 0.0363156, 0.0358877, -0.0428547, -0.0394638, -0.0826182, -0.0534047, -0.0523987, -0.0402069, -0.0361779, -0.0404503, -0.00513793, -0.0857751, -0.0310728, -0.0347274, 0.00193132, 0.0042765, 0.0186119, -0.00451054, -0.0586652, -0.00645911, -0.0355822, 0.020055, 0.0524245, -0.0438321, 0.0442813, 0.00936319, -0.00631098, 0.0109629, -0.00459059, 0.095314, -0.0732341, -0.0336335, -0.101192, 0.0343376, 0.011933, 0.0372624, 0.0145032, 0.0682926, 0.111712, 0.0755932, 0.0293154, -0.0811686, -0.0403043, 0.0505386, -0.0599391, -0.0497448, 0.0352473, 0.0445703, 0.0839762, 0.0281866, 0.0295692, 0.0420535, -0.0303459, -0.00202511, -0.0518968, 0.054131, -0.0539912, -0.046507, 0.0795465, 0.10897, 0.0211005, 0.0784731, -0.0709171, 0.052326, -0.0189594, 0.118481, -0.0105192, 0.00437189, 0.0489255, 0.0374188, 0.0174111, -0.0239177, -0.0406844, 0.00296164, 0.0509764, 0.0355951, -0.0666514, -0.0326722, -0.0487302, -0.0982901, 0.0155489, 0.0560287, 0.0946238, -0.0307856, -0.0773519, -0.0171231, 0.0413344, 0.0876342, -0.0481276, 0.0381611, 0.0012947, -0.0297156, 0.0624914, 0.152679, 0.140699, 0.153719, -0.00186355, 0.150815, 0.0503756, 0.0614085, -0.0425147, -0.0435709, -0.036414, -0.0220684, -0.0371475, -0.03698, 0.0197689, 0.0395817, -0.0165879, -0.0419965, -0.0136365, 0.0246298, -0.0314279, -0.0617745, -0.0186015, 0.0637287, 0.0647508, 0.0449555, -0.00765283, -0.109673, 0.0649817, 0.0432394, 0.103247, 0.0790893, 0.0381708, 0.195077, 0.0404849, 0.113251, 0.0259231, 0.0453681, -0.0802964, -0.0538915, -0.0316251, -0.00107983, -0.0147826, 0.000745148, -0.0040455, 0.0523452, -0.0555724, 0.0302512, -0.078243, -0.0316169, -0.0527516, -0.0109802, -0.0757547, -0.141275, -0.026916, -0.0628708, 0.0772317, 0.0659833, 0.0358463, 0.0399183, 0.133999, 0.0858457, 0.0831267, -0.019936, -0.0334207, -0.0439389, -0.0427419, 0.0643039, -0.00740821, -0.0342067, 0.00663537, -0.0226255, 0.100402, 0.0268492, -0.010726, -0.10681, -0.0818138, 0.010727, 0.0203553, -0.029023, -0.075564, -0.0934588, -0.0434844, -0.0168763, 0.0736469, 0.000375796, 0.0698712, -0.00563201, -0.0519734, 0.160991, 0.0808646, 0.0651499, 0.115553, -0.0271605, 0.137266, 0.0835577, -0.0269573, -0.0383047, 0.0292883, 0.0602281, 0.15668, -0.0045141, 0.0513343, 0.0370741, 0.0455935, 0.0457715, 0.0169739, -0.00374662, 0.0799477, 0.0767012, -0.0399792, 0.105952, 0.0503396, 0.0817866, -0.0433003, 0.0150821, 0.0620549, 0.0352404, 0.0721046, -0.0674587, -0.032073, -0.035032, 0.0443739, 0.0350322, 0.060736, -0.0259192, 0.00540456, 0.0510958, 0.0649857, 0.0440539, -0.0168536, 0.0222727, 0.171784, 0.0637076, 0.227606, 0.151159, 0.0763658, 0.117928, 0.118996, 0.1155, 0.0750724, 0.0305783, -0.00527913, 0.00205448, -0.101989, 0.0343516, -0.0502574, 0.0699819, 0.00913519, -0.065575, -0.0545297, 0.0111666, -0.0204118, -0.0213561, 0.0763869, 0.0436613, 0.0210827, -0.0103477, 0.0361007, -0.0202206, -0.112346, 0.0446407, 0.14885, 0.156005, 0.0228848, 0.0639779, 0.0794413, 0.116714, 0.199582, 0.0500227, -0.00164017, -0.0434463, 0.0279841, 0.0216555, 0.0332164, -0.0119025, -0.0332278, -0.0424238, -0.0292739, 0.0527111, 0.0285231, -0.0082113, 0.126151, 0.0340837, 0.0314655, 0.0269455, 0.0569617, 0.0370889, 0.0368295, -0.0290532, 0.0164277, -0.00653527, -0.0114196, -0.0236376, 0.0668112, 0.0433126, 0.0523446, 0.052427, -0.0254441, -0.0958736, -0.12948, -0.0792584, -0.0307384, -0.0350463, -0.015033, -0.0156436, -0.06067, -0.0343206, -0.028979, 0.0190884, 0.0897479, 0.0873134, 0.0265982, -0.0142327, 0.0465347, 0.116067, -0.00289835, -0.0419117, -0.160834, -0.0905692, -0.0460427, -0.00280756, 0.0385611, 0.091568, 0.0120937, 0.0736418, 0.0141829, -0.0651545, 0.0075698, -0.0706307, -0.0396202, -0.058357, 0.0454858, -0.0347474, 0.0861066, -0.0169088, 0.0337059, 0.102938, -0.0168513, 0.0603223, 0.0730422, -0.0306876, 0.0627067, 0.0546138, -0.0310262, 0.0169564, -0.182066, -0.201676, -0.0145553, -0.0628384, -0.0141161, -0.0140549, 0.0183031, -0.0109387, 0.0294911, -0.100131, -0.0353907, -0.045765, -0.0227852, -0.192729, 0.021722, -0.0435895, 0.0244081, 0.0276299, -0.0583328, -0.0700627, 0.0479067, 0.016236, 0.0872322, 0.00412004, 0.0195817, 0.051305, 0.0594886, -0.000612881, -0.0251442, -0.0899836, 0.0129817, -0.0800294, 0.0200347, 0.0673327, 0.109884, 0.0602728, 0.0672864, -0.0157939, 0.00862097, 0.122332, -0.0390583, -0.0355651, -0.0324793, 0.0465737, 0.0734971, 0.0245214, 0.0793151, 0.0385334, -0.00305365, -0.0200619, -0.0449764, 0.0767347, 0.0546355, 0.0984534, 0.0793172, 0.0384551, 0.012649, -0.106994, -0.0374936, -0.118089, -0.0432008, -0.0187867, 0.0597033, 0.0855599, 0.0794638, -0.0625111, 0.043243, 0.0422554, -0.127516, -0.0310682, 0.0178054, -0.0185083, 0.0180354, -0.0651715, -0.0869526, 0.0179308, -0.0333057, -0.0193365, 0.0298174, -0.050663, 0.0468477, -0.0380398, 0.0483076, 0.0787919, 0.142256, 0.102611, -0.00305041, -0.0381138, -0.0532745, -0.0664034, 0.0217897, 0.00864059, -0.00655091, -0.0340696, -0.107618, 0.0318222, -0.022984, 0.0382165, -0.0232265, -0.0791537, -0.0374072, 0.00253345, -0.0405365, -0.0900619, -0.0606051, 0.0386588, 0.00687768, -0.0503847, 0.0829055, 0.0527444, 0.036809, 0.0648233, 0.131338, 0.0687097, 0.0107305, -0.150256, 0.0368392, -0.0317505, -0.00524922, 0.000408349, 0.0714558, -0.00656855, -0.0657626, -0.000110824, 0.0235372, 0.029915, -0.0578608, 0.0251021, 0.0110237, 0.0522945, -0.0663874, -0.0551746, -0.0668676, -0.04592, -0.00407286, -0.105263, 0.0885731, -0.0469496, 0.0263853, 0.037667, 0.0478024, 0.0699014, -0.0556091, -0.176949, -0.0131424, -0.0105069, -0.0240738, 0.00692323, -0.0192438, -0.0229267, -0.0622009, 0.0368866, -0.0114215, -0.0341544, 0.0469749, 0.054512, 0.038359, -0.00342069, 0.111176, -0.00530475, 0.0163901, -0.0589695, -0.0172892, -0.112146, -0.0436512, -0.0631406, -0.0764312, 0.0720807, -0.0548764, -0.00844199, -0.0852601, -0.0284836, -0.0322306, 0.0611154, -0.0317458, 0.144637, 0.13727, 0.0513, 0.0598529, -0.0109295, 0.031169, 0.042963, -0.00764184, 0.0279388, 0.00933229, 0.0215981, 0.0555562, 0.0132071, -0.0551294, -0.00346278, -0.0859556, -0.0130122, 0.0268855, -0.0862513, -0.0619776, 0.0642564, -0.0257367, 0.0637167, 0.0487024, 0.0909433, -0.0118504, 0.0312, 0.0386674, 0.0569985, 0.130509, 0.0370717, 0.0298498, 0.0260326, -0.0258827, -0.0678281, -0.0519568, -0.0220777, -0.04238, -0.0107237, 0.00351157, -0.00949419, 0.00773467, -0.0278464, -0.0280122, 0.020273, 0.125829, -0.011601, 0.048403, 0.0201029, -0.0571562, 0.0749994, 0.118237, 0.0969476, 0.0392069, 0.14158, 0.200127, 0.0308264, -0.00170597, 0.0550498, -0.0682341, 0.0459317, -0.0687579, 0.0658329, -0.0443528, -0.0100222, 0.0271128, -0.102801, -0.00359195, 0.107909, -0.0410313, -0.0451633, 0.0519691, -0.0233402, 0.0515281, -0.10053, 0.027472, 0.0905972, 0.0696371, 0.121605, 0.083896, 0.0557329, 0.117257, -0.00538489, 0.0651866, 0.0380347, 0.0782176, -0.0290603, -0.0236797, 0.0759454, 0.0278589, 0.0832535, 0.0419038, 0.00611182, -0.00192814, 0.0186203, 0.0392131, -0.0242854, -0.0229935, 0.0343338, -0.0829371, -0.0158627, 0.0683896, 0.0106516, -0.0042634, -0.0441621, -0.137816, 0.0965956, 0.073829, 0.0392638, -0.0293779, 0.0808785, 0.123953, -0.0294152, 0.0164602, 0.015979, 0.00911121, -0.0193185, -0.0424635, -0.0133448, 0.0241847, -0.0788181, -0.0108874, -0.0167243, -0.0499963, -0.0507764, 0.0159421, -0.0179362, 0.0898239, -0.0225486, -0.124975, 0.00772128, -0.0410024, -0.0289468, -0.0171015, -0.0540839, -0.00656159, -0.0253424, 0.00255798, 0.0827923, 0.00765663, -0.0605694, 0.0505375, 0.085492, -0.044229, 0.0646837, -0.0843628, 0.0317923, -0.0426344, -0.00293243, 0.0507665, 0.0401353, 0.00435434, 0.0403987, 0.0289897, 0.00178155, 0.0338331, -0.0109434, -0.0368304, 0.0227286, 0.111363, -0.0235516, -0.0112383, 0.000223126, -0.0225433, -0.0499079, 0.00369201, -0.0526974, -0.0184876, 0.0035301, 0.0295097, 0.02043, 0.0223415, -0.0229437, 0.0452872, 0.00692782, -0.0908396, -0.00252994,
0.0609949, -0.0384971, -0.028308, -0.0781702, -0.081552, 0.00836087, -0.0335396, -0.0378044, 0.0224507, -0.0448066, 0.00675563, 0.0763714, -0.000584122, 0.0743083, -0.0329081, 0.0129469, 0.0319438, -0.0273164, 0.0350169, -0.0729842, -0.0443372, -0.0828961, 0.00024914, 0.0501613, 0.0503743, -0.0660729, -0.0315708, 0.00128939, -0.0107503, 0.123236, -0.0500153, -0.0591208, -0.0172912, 0.00762876, 0.106937, -0.0584589, 0.0648411, 0.0473031, 0.00910739, 0.0118295, 0.0905709, 0.0763723, 0.0245553, 0.0235365, 0.0278021, -0.0192948, 0.0559368, -0.017465, -0.00279405, -0.00944799, 0.0320634, -0.00615594, -0.0926706, 0.0656737, -0.0188742, -0.0228349, -0.0450796, -0.0562146, 0.0782881, -0.0934951, -0.0883925, 0.00406097, 0.0606336, -0.0275635, -0.0021647, -0.0646585, -0.00128614, -0.129418, 0.0797594, -0.00401721, -0.062684, 0.0548173, -0.13376, 0.0056729, -0.0153509, -0.0600722, -0.00419922, -0.00188685, -0.0037774, 0.0556281, -0.042366, -0.0344031, -0.0217355, -0.00466339, -0.0182946, 0.115748, -0.0319826, -0.0170194, 0.0222362, 0.0169861, 0.0407858, 0.0458663, 0.10825, 0.0408957, -0.0143817, -0.00366422, 0.0204496, -0.0392434, -0.0654149, -0.0135205, -0.139095, -0.071058, 0.0317999, -0.0514501, -0.00880479, -0.0397274, 0.0714067, -0.0864822, -0.0187296, 0.0431616, -0.0472734, -0.0067959, -0.000787964, -0.0901455, 0.0361042, -0.103205, -0.0419625, 0.0416267, -0.0378758, -0.0419849, -0.0420634, -0.0531833, 0.0692392, 0.0381823, -0.0423819, -0.0426149, -0.00149548, -0.00229036, -0.017172, -0.104735, -0.074376, -0.0333677, -0.0020972, 0.042857, 0.123578, 0.0577607, 0.0963824, 0.0391271, -0.056839, -0.0215711, 0.0387999, -0.122227, -0.088204, 0.0126033, 0.0414694, -0.0684403, -0.0704777, 0.139928, -0.00846157, -0.0573127, 0.1705, -0.0216759, 0.103492, 0.0361611, -0.0172299, -0.0435593, -0.00609389, 0.0786974, 0.0747258, -0.0334056, 0.080836, 0.0364866, -0.0156682, 0.0331036, -0.00763939, -0.0199566, 0.0283832, 0.0690915, 0.0355073, -0.0132625, -0.100085, 0.008353, -0.00838586, 0.0575795, 0.0854638, -0.0370871, 0.127486, -0.0240366, -0.0329258, 0.0811158, -0.0443779, 0.0236358, -0.0125223, -0.0372091, 0.0770633, -0.0836605, -0.130209, -0.107218, 0.0190947, 0.00299795, 0.0443439, 0.0496143, 0.0427898, 0.0254294, 0.0232431, -0.0198448, 0.044255, 0.0370831, -0.0636156, -0.00714212, 0.0149475, 0.0391956, -0.0436175, -0.0350616, 3.63516e-05, 0.0112942, 0.0314892, -0.0823901, -0.011574, -0.0432318, -0.0435405, 0.10652, 0.0696386, -0.00956272, -0.145869, 0.0751868, -0.134719, 0.0166142, 0.0524777, 0.0314088, 0.00912499, -0.00371925, 0.00806874, -0.0754858, 0.135006, -0.0412964, 0.0240264, -0.0250391, 0.042192, -0.00626398, 0.0600936, 0.0334503, 0.0798697, 0.0852194, 0.0776839, 0.0114163, -0.069953, -0.117146, -0.0933809, 0.0963051, 0.0218994, -0.0355148, 0.0259905, 0.00400102, -0.0638036, 0.0300416, 0.037754, 0.0243881, -0.0184565, 0.0179274, 0.0298218, -0.0148737, -0.0593086, -0.0256059, -0.0584031, 0.0443662, -0.0813951, -0.0275367, -0.012173, 0.0236798, 0.0777256, 0.106455, -0.0103406, -0.031034, -0.014757, -0.150087, -0.163746, -0.0726497, -0.107596, 0.00630163, -0.0864753, 0.00593324, 0.11086, -0.0299261, 0.0747973, 0.0302533, 0.0191004, 0.0223792, 0.012069, -0.0661236, 0.0417695, -0.00608905, 0.0480039, -0.0128446, -0.0790531, -0.0758369, 0.0323615, 0.114557, 0.0321323, 0.0861142, 0.0936642, -0.0082113, -0.063105, -0.204539, -0.0990252, -0.171601, -0.112494, -0.0621506, 0.00283196, -0.0786795, 0.0879136, 0.0451174, 0.066127, 0.0518899, -0.0108849, -0.140893, -0.0129293, -0.0662724, 0.0532954, -0.057229, -0.129549, -0.120397, -0.0244971, -0.00589314, -0.0726354, 0.17384, 0.124913, 0.0988804, 0.0442968, 0.12018, 0.0980943, 0.00207948, 0.0941161, 0.0144881, 0.0403863, 0.100275, 0.0193229, 0.0202439, 0.0112134, 0.0321433, 0.097207, 0.0581469, 0.00841184, 0.0184998, -0.00611776, -0.0984226, -0.00714151, -0.0355576, -0.0983537, -0.0946537, -0.0213398, -0.00671322, 0.0510895, 0.0340872, 0.156051, 0.0762036, 0.178249, 0.249275, 0.147449, 0.225802, 0.20244, 0.0514103, 0.0652112, 0.049396, 0.182809, -0.00436146, 0.106393, 0.0562519, -0.0170085, 0.182522, 0.0239748, -0.0632825, 0.0685114, 0.0366843, 0.0738485, 0.0347673, -0.08155, -0.109092, -0.0636449, -0.0601316, -0.103092, 0.0498713, 0.052596, 0.0978652, 0.147617, 0.296975, 0.319908, 0.230115, 0.178838, 0.160752, 0.0388809, 0.0215188, 0.0447724, -0.0166529, -0.0740965, -0.0483751, 0.0846139, 0.056956, 0.0535831, -0.0332339, -0.0126686, -0.0580256, -0.0217831, -0.0339818, -0.0267193, 0.0544724, -0.038974, -0.0818145, -0.018705, -0.0471583, -0.0275846, -0.022897, 0.144124, 0.24164, 0.321303, 0.298078, 0.050733, 0.0454175, 0.139277, 0.0210487, -0.00942855, -0.178903, -0.115547, -0.0143864, -0.109295, -0.0240521, -0.0260908, -0.044968, 0.0561387, -0.0215237, 0.0298923, 0.00639208, -0.0199064, -0.0432836, -0.0397864, -0.234651, -0.2021, -0.171605, -0.214736, -0.130449, 0.0575089, 0.215285, 0.168971, 0.246965, 0.18425, 0.0710914, -0.0124932, 0.0426566, 0.0282596, -0.0167862, -0.0540989, -0.0710049, 0.0327286, 0.0751546, 0.0580928, 0.00053189, -0.00134136, 0.0237453, -0.00650526, -0.00723728, -0.0306862, -0.0840262, -0.197976, -0.111347, -0.275944, -0.270649, -0.302529, -0.331965, -0.319021, -0.142152, 0.165347, 0.112354, 0.0673227, 0.0214349, -0.0559364, -0.0612813, 0.0831922, -0.0164319, -0.0993561, 0.131964, 0.0514556, 0.0805402, 0.10763, 0.0155148, -0.0199247, -0.0178856, -0.0710748, -0.00273533, 0.00235072, -0.0227789, -0.048488, -0.163826, -0.191183, -0.150327, -0.281691, -0.40245, -0.498966, -0.342571, -0.250702, -0.0739776, -0.101943, -0.0092187, -0.0393036, 0.00904452, -0.0274103, -0.0100251, -0.0199275, 0.107599, -0.0628452, 0.0543802, 0.00516028, -0.154595, -0.100578, -0.0151354, -0.0265846, -0.00921262, -0.0736458, -0.0837892, 0.00491745, -0.142813, -0.0734944, -0.186943, -0.234999, -0.22477, -0.267603, -0.260839, -0.382376, -0.14461, -0.0827283, -0.126106, 0.063449, 0.0639554, -0.0387017, 0.0246232, 0.0106228, 0.0145312, 0.00851553, -0.02992, -0.0685926, -0.0195783, 0.00368089, 0.0334434, -0.027816, 0.0716716, 0.0869562, 0.0435897, 0.0408481, 0.0583625, -0.0671864, -0.155999, -0.158648, -0.0449453, -0.0411913, -0.15032, -0.141656, -0.208134, -0.127596, -0.136985, 0.022784, -0.0222148, 0.00884473, -0.0377332, 0.0346285, -0.0792535, -0.0302648, 0.0668241, -0.0813864, 0.0110996, -0.0263746, -0.00633494, 0.0740345, 0.00696342, -0.00801369, 0.0838484, 0.0353174, 0.0461689, 0.118494, -0.0326242, 0.0324012, 0.0595612, 0.0940959, 0.0547871, -0.0666514, -0.0845522, -0.0466911, -0.000449868, -0.0414792, 0.0219585, 0.0622497, -0.0534323, -0.00760672, 0.0268802, -0.0139627, 0.0116469, -0.0493913, -0.0600228, 0.0398916, -0.0448693, 0.052056, -0.0883602, -0.0736611, -0.0468796, -0.00630821, 0.0684823, 0.0827685, 0.0449422, 0.112373, 0.15025, 0.0426206, 0.0937957, -0.0380692, 0.0148442, -0.12393, 0.0188448, -0.0521378, 0.0487298, -0.0778619, -0.0172216, 0.015849, -0.0110284, -0.0632484, -0.0241635, -0.00108, -0.0750055, -0.0337275, 0.0868118, 0.0626337, 0.0614372, -0.0446113, 0.0426976, 0.108699, 0.046774, -0.0292798, 0.108885, 0.0261243, -0.13167, 0.121153, 0.0762018, -0.0508983, 0.014853, 0.00802341, 0.0203111, 0.0510276, -0.0152435, 0.00652001, -0.067422, -0.0301219, -0.0908949, -0.154245, -0.107675, -0.106931, 0.0112361, 0.0251905, 0.0207242, -0.0450887, 0.0266239, -0.0607903, -0.0297507, 0.072824, 0.0338387, 0.120738, 0.111163, 0.0703879, 0.0757999, 0.112673, 0.0532838, -0.0661002, 0.0381647, 0.0908768, -0.0479644, -0.00739812, -0.0139942, 0.0500809, 0.0116539, -0.0388911, -0.100804, -0.041743, -0.0484012, -0.141684, 0.00904565, 0.0212717, -0.00275554, -0.0254111, -0.100251, 0.0317017, -0.00455983, -0.00197156, 0.054446, -0.0185805, 0.0456848, 0.143045, 0.0793097, 0.146978, 0.0505798, 0.000686206, -0.0735996, -0.0338098, -0.0442159, -0.0420962, -0.0943723, -0.0197015, 0.0196436, -0.0496154, -0.0590248, -0.0792931, -0.0182666, -0.0152657, -0.0335204, -0.00181249, 0.0127831, 0.0209923, 0.0638361, 0.0147475, 0.00815641, -0.122217, -0.025249, 0.0706071, 0.0537955, -0.0103567, 0.0299305, 0.0334581, 0.0560216, -0.0561659, 0.00358803, 0.00817651, 0.0788011, -0.0132576, 0.0281328, 0.0124815, -0.0530152, 0.034588, 0.0816619, 0.0466085, -0.0882776, 0.0731043, -0.0510793, -0.00962351, 0.00673771, 0.00555335, -0.0234041, -0.0334046, -0.0491107, 0.0505737, 0.0318303, 0.0263377, 0.0943539, -0.0321696, 0.0556192, -0.0655963, -0.0127814, -0.014637, 0.0245113, -0.0630577, 0.0305794, 0.0213143, 0.0226606, 0.0665271, -0.0892335, 0.00411527, 0.0336893, 0.066339, -0.0511258, 0.0822774, 0.0251807, 0.0928567, -0.0356163, 0.00389819, -0.0339581, -0.0792343, -0.0731934, -0.0376655, -0.0353678, -0.0672407, -0.0746615, -0.0092214, -0.0195587, -0.0044354, 0.0217805, 0.0344779, -0.106236, -0.0198849, 0.0332917, 0.0674562, -0.0699605, 0.0299668, 0.0953107, -0.016227, -0.0482629, -0.0279164, -0.0129978, -0.0448399, -0.0765517, 0.103166, 0.00418017,
0.0251289, 0.0471857, 0.0510304, 0.0789817, -0.0486958, 0.0167186, 0.0177022, 0.0981073, -0.000103361, 0.0110727, -0.0152141, -0.0281273, 0.100766, -0.0265331, 0.0445312, 0.0376736, 0.0376284, 0.0617558, -0.0020367, -0.0165214, -0.0560923, 0.039489, -0.0272559, -0.0627116, -0.0692473, -0.0119646, 0.0770445, 0.0590604, 0.0200796, 0.0363173, 0.100078, -0.0495179, -0.00155061, 0.0783119, 0.013951, 0.0332197, -0.0400222, -0.0464575, -0.0205585, -0.0279692, -0.0695237, 0.0201075, 0.0388282, 0.0367451, 0.0414618, 0.0530779, -0.0360066, -0.0371317, -0.0654079, 0.0436203, 0.05213, -0.00558244, -0.124072, -0.000229735, -0.107054, -0.0330627, -0.0133914, 0.0955926, -0.0524709, -0.104277, -0.00814498, 0.0235561, -0.00833021, 0.0113373, -0.116364, -0.00222583, -0.0282181, -0.0580338, -0.00169034, 0.0261362, -0.00909087, -0.0289982, -0.0743254, -0.0392815, -0.0187265, 0.00422307, 0.0673718, -0.134948, 0.0506322, 0.0273975, 0.0906067, -5.99105e-06, 0.0129019, 0.0871299, 0.00846824, -0.00644161, 0.03266, -0.0225154, 0.0425298, 0.00157443, 0.0389749, -0.0652824, -0.00116737, 0.0780369, -0.0401586, -0.0244937, -0.0588383, 0.00539415, 0.0417295, 0.0141824, 0.0754697, 0.0558639, 0.129501, -0.0385074, 0.0105614, -0.0366919, 0.0595749, -0.0345905, 0.0400011, 0.0498578, 0.0126494, -0.0406677, 0.0769104, -0.0240456, -0.0519071, 0.0208849, -0.0426697, 0.0526361, -0.0620373, -0.00929442, -0.0188131, 0.0809416, 0.0767401, 0.032273, -0.0142834, 0.0733668, 0.052299, 0.0135288, 0.0457367, 0.118605, 0.0965219, 0.0847493, 0.0409929, 0.0772842, 0.105414, -0.0308926, 0.0473608, -0.122422, 0.0218143, 0.000529194, 0.0188389, -0.00136463, -0.0134414, -0.00207321, 0.0471755, 0.0530937, 0.0807993, -0.087797, -0.00976336, 0.0120501, -0.0973084, -0.0598731, -0.132001, -0.119442, 0.0767864, 0.121208, 0.0817147, -0.000371253, 0.105895, 0.154169, 0.0744912, 0.0651869, -0.058085, -0.0218371, -0.00549984, -0.0492606, -0.0234386, -0.0787465, 0.0297042, 0.0330417, -0.0508549, 0.105444, 0.0265145, 0.00292686, -0.13412, -0.0742188, 0.00537517, -0.012319, -0.0159561, -0.124951, -0.121396, -0.192984, -0.134065, 0.00261577, 0.0838756, -0.0199874, 0.0195221, -0.0998666, -0.00832035, 0.0995191, 0.0164682, 0.0641364, 0.081955, -0.0218153, -0.0214362, -0.0219672, -0.00512875, -0.0710057, 0.00129412, -0.00641741, -0.0518761, -0.0462147, -0.0453614, -0.0433134, -0.125431, -0.0342265, -0.128194, -0.0500735, -0.111131, -0.127627, -0.086177, -0.0298916, 0.0462044, 0.0107689, -0.0777462, -0.0218188, 0.00317794, 0.0740643, 0.0887755, 0.118533, 0.127128, 0.100557, 0.0289298, 0.0369135, -0.0272394, 0.030278, 0.00328531, 0.00879062, 0.0907338, -0.0371881, -0.0858142, -0.117907, 0.0749973, -0.0220648, -0.0927277, -0.210484, -0.14017, -0.168063, -0.104394, 0.124879, 0.0551418, 0.0414135, 0.0286628, -0.113575, -0.057006, -0.0184371, 0.0182861, 0.0843737, 0.10193, -0.0247652, -0.106077, 0.0147875, -0.0328405, -0.0159264, 0.0389364, 0.0563564, -0.0879077, -0.000935807, -0.0180959, -0.0573438, -0.149992, 0.101166, -0.121949, -0.024826, -0.203272, -0.0459303, -0.151074, 0.0361748, 0.0616644, 0.057155, 0.0448772, -0.139316, 0.0233382, 0.0215777, 0.0376345, 0.056509, 0.100557, -0.017068, -0.0343834, 0.0168416, -0.0311255, -0.048069, -0.0192027, -0.0712422, 0.00902276, -0.0764481, -0.0410259, -0.0542539, -0.0684303, -0.0730712, -0.0252809, -0.0694091, 0.094916, -0.261673, -0.119781, 0.139443, 0.105244, 0.180237, -0.0691653, 0.00831397, 0.0240653, -0.0579316, 0.103748, 0.0482921, -0.00907657, -0.00649808, 0.0542165, 0.0357612, -0.0183042, 0.0159697, -0.0364022, 0.0150253, -0.0344193, -0.0832774, -0.0721336, -0.0465986, -0.00199457, 0.00342267, -0.0348959, 0.0366345, 0.0133208, -0.104971, -0.158287, 0.0895224, 0.114445, 0.155895, 0.0829878, 0.0791711, 0.0344908, 0.070757, 0.0221006, 0.0841381, 0.0333068, -0.0155888, -0.0518994, 0.00426605, -0.0291218, 0.0453678, 0.0780096, -0.0514399, -0.0619453, -0.0158314, -0.0148549, 0.0387474, 0.000907514, 0.0293566, 0.124675, 0.10585, -0.0555753, -0.0321495, -0.027508, -0.0259706, 0.00968559, 0.0449467, 0.0439988, 0.0379112, 0.0312285, -0.0355007, -0.0383884, -0.00617406, -0.09684, 0.030926, -0.021664, 0.038404, -0.00585458, -0.0167934, -0.0326917, 0.00553645, -0.100334, -0.00785098, -0.0376076, 0.132833, 0.0055611, 0.0705067, 0.0240058, 0.100777, 0.0645963, -0.0230998, -0.0652466, -0.0217961, -0.0927531, -0.0607037, -0.00787461, 0.0188244, -0.105108, -0.0571179, -0.019889, 0.0415297, 0.0135191, 0.0386207, 0.0512678, -0.0507231, 0.0364945, 0.023572, -0.038073, -0.135546, 0.0176037, 0.0267364, 0.0410777, 0.0520243, 0.183961, 0.169521, 0.00309615, 0.0917681, -0.0147576, 0.0608741, -0.0617364, 0.00681635, 0.0335573, -0.0403749, -0.0860632, 0.0262929, 0.0687543, 0.0590626, 0.0730653, -0.032801, 0.120782, 0.0458488, 0.168932, -0.0253513, 0.0277657, -0.00482875, -0.0149613, -0.028118, -0.00741808, 0.0479331, 0.077563, 0.0455224, 0.13175, 0.194213, 0.0234512, -0.00284559, 0.0793879, -0.0250606, -0.0902548, -0.0316514, -0.0835527, 0.00241807, 0.0221158, -0.0269814, -0.0139215, -0.0404005, -0.0166218, -0.0292336, -0.145317, -0.0299163, 0.0449991, 0.104151, 0.0231679, 0.0866159, -0.016298, -0.0109866, 0.0084955, 0.0925709, 0.0612666, 0.0638925, 0.071392, 0.0700677, 0.140776, 0.0564722, 0.016803, 0.00353848, 0.041592, 0.0426188, -0.00670406, 0.00252597, -0.0270149, 0.0257748, -0.00172173, 0.0702317, 0.0421317, 0.03359, 0.0731602, 0.0689006, -0.0251296, 0.0124451, -0.189424, 0.0268713, 0.000298186, -0.127367, -0.0321456, 0.123438, 0.122653, -0.0237611, 0.0633245, 0.0733046, 0.145971, 0.063724, 0.144721, 0.0887391, 0.0244201, 0.00243577, 0.00420415, 0.0219841, -0.0735882, 0.0602037, 0.0182158, -0.0165981, -0.0232053, -0.0668595, 0.0471653, -0.0184491, -0.0970491, 0.0091739, 0.0681751, 0.0137548, 0.0332677, 0.0126548, 0.0920047, 0.0253194, 0.106616, 0.0146809, 0.114319, 0.134123, 0.108753, 0.0578615, 0.0883485, 0.0491065, 0.0866372, 0.0886031, -0.05638, -0.030624, -0.0594567, 0.0215333, -0.00615912, -0.070765, -0.0836339, -0.0114555, 0.112194, -0.00643823, -0.0409614, -0.0924628, -0.0453828, 0.0347782, 0.0407427, -0.0230286, 0.0264023, 0.0159884, 0.101088, 0.0364305, -0.00535338, 0.0276471, -0.0216075, 0.0644647, 0.122235, 0.0845951, 0.175469, 0.0418355, -0.0501949, 0.160933, 0.0423828, -0.0343822, -0.00166214, 0.0508956, 0.0457783, -0.0460474, 0.0427471, 0.0619093, 0.0285414, 0.0200059, 0.00205745, -0.0150221, -0.0188194, 0.076642, -0.0828841, -0.0264138, -0.0630554, 0.0264716, 0.00713426, -0.0156468, -0.0610864, 0.0874722, 0.12189, -0.0418054, 0.0229517, 0.141795, 0.127715, 0.0787786, -0.0184028, 0.0218413, -0.0237064, 0.0223322, -0.0360354, -0.030757, -0.0197956, -0.014384, -0.00861123, 0.0579873, -0.021755, 0.0474836, 0.0503081, 0.0959134, 0.0392674, -0.0968569, 0.0722724, -0.171845, -0.0527053, -0.110828, -0.0430722, 0.0023119, -0.10621, 0.00842469, -0.0166303, 0.0486925, 0.0892597, -0.0446483, 0.0229615, -0.0498823, 0.00411359, 0.0464908, 0.0226073, -0.0602443, 0.0369371, -0.0151704, 0.00817016, -0.0448669, -0.0208158, 0.0223007, -0.0915115, 0.0445156, 0.0156084, 0.0932082, 0.061064, 0.0446726, -0.104111, -0.120631, -0.0792578, -0.047569, -0.0083675, 0.0968617, 0.0627295, 0.0821365, 0.0906586, 0.168126, -0.0265146, 0.0277085, -0.0242358, 0.0483662, 0.0105425, -0.0361391, 0.0794813, 0.0096563, -0.00910529, -0.0645455, -0.00172348, -0.000510126, 0.0094473, 0.0730049, -0.0871202, -0.065976, -0.0759619, -0.0244645, 0.0139772, -0.0455172, 0.0713213, 0.102822, -0.0494998, -0.0343791, -0.0238679, -0.0364469, 0.00921712, -0.0126108, -0.0184717, -0.154639, 0.00374978, 0.0153953, -0.00532599, -0.042909, -0.00314076, -0.00620034, 0.074469, -0.071838, 0.0709814, -0.03429, 0.0427685, 0.0316441, 0.00891244, 0.00596619, 0.0295163, 0.0285322, -0.0475016, 0.0199062, -0.0590127, -0.0779295, -0.0707932, -0.0931223, -0.127873, 0.0109174, -0.0986944, -0.113607, -0.0391717, 0.0713795, -0.0834507, -0.0473455, -0.051752, -0.0343425, 0.0145752, 0.0509566, 0.0386677, 0.0351148, 0.00728069, -0.0404643, 0.0205566, -0.0768134, -0.0340183, -0.0598875, -0.0635117, 0.0724958, 0.0265071, 0.00331673, -0.131688, -0.018192, -0.101042, -0.10041, -0.0780429, -0.12738, -0.150734, -0.116211, -0.0467336, -0.00593755, -0.115946, 0.0896325, 0.0572988, -0.0491012, 0.0103251, -0.11709, 0.0889061, -0.0301627, 0.0183367, 0.00274952, -0.00634267, 0.0168662, 0.0826321, 0.0912117, -0.0937431, 0.0154299, -0.0499879, 0.00723236, -0.0802515, 0.0569706, -0.068956, -0.0319158, -0.0440696, -0.0199365, 0.00983597, -0.0149187, -0.0334899, 0.0499738, -0.0696418, -0.0636046, 0.0342467, 0.0839043, 0.032225, -0.0305003, -0.0396847, -0.01709, -0.0347566, -0.0375033, -0.00375699, 0.0733094, 0.0569302, 0.0646142, 0.044859, 0.0288931, 0.0695154, -0.00577381, 0.026314, 0.00517166, -0.00883072, 0.00644161, 0.0539905, -0.0235624, 0.0524767, 0.0215838, 0.00755033, -0.0637562, -0.0223989, -0.067389, 0.0515804, 0.0309784, 0.0615134, 0.0203661, -0.0378884, 0.0681186,
-0.0618116, 0.0346663, -0.0374544, -0.0629498, -0.0174288, -0.0487967, 0.0789505, 0.02192, -0.0370273, -0.0979281, -0.00874764, 0.131511, -0.0208892, -0.0324256, -0.0380784, -0.0696106, -0.0304266, 0.0349477, 0.0223317, 0.0229849, -0.0208763, -0.00770617, 0.0210469, 0.0392135, 0.0210753, 0.0256666, 0.0674367, -0.0184247, 0.00803999, 0.00227344, 0.0638914, 0.0291058, 0.00727466, -0.0312701, 0.0401045, 0.0345342, -0.0351654, -0.0115024, 0.0170862, 0.0533409, -0.0779542, 0.0111246, 0.0336856, 0.0438314, -0.132106, -0.065661, 0.0205377, 0.0672377, -0.0408122, -0.0305299, 0.0554958, -0.00428557, 0.0347271, -0.0532641, 0.0357983, -0.0288862, -0.140056, 0.0297974, -0.00510999, -0.0250037, 0.00821773, 0.00663454, 0.0198415, 0.0730283, 0.012294, -0.0024765, -0.0372338, -0.0710821, -0.0522611, -0.050673, -0.0423048, -0.0638526, 0.0451939, 0.00549643, 0.087576, 0.042319, 0.0199911, 0.0280662, 0.0698991, -0.0372471, -0.0576424, 0.00113149, -0.0325348, 0.0105166, 0.0640071, -0.000491576, -0.057335, 0.00982202, 0.00618771, -0.0236577, -0.00768995, -0.00849944, 0.0194121, -0.0904106, -0.122113, -0.0318657, -0.0868866, -0.0634438, -0.00535524, 0.0343885, 0.0543422, -0.0375982, -0.0490127, 0.0521316, -0.0343678, 0.00233807, 0.11106, -0.00668514, 0.00912622, -0.00946771, 0.0274526, -0.0121743, 0.0702016, 0.0389313, -0.0244717, 0.0707391, 0.0683854, 0.0238094, -0.0592696, -0.0762021, -0.140304, -0.0926053, -0.206742, -0.187099, -0.123341, -0.0357194, 0.0238595, -0.0465621, -0.116755, 0.00982762, 0.0816999, -0.0163116, 0.127315, 0.064536, 0.0415256, 0.084273, -0.0532893, -0.084621, 0.0429205, -0.02408, -0.179227, 0.0108365, -0.0124452, 0.00593228, -0.0101616, -0.00259758, -0.0389175, -0.0433472, -0.255361, -0.151129, -0.263854, -0.0370363, -0.0621759, 0.158225, 0.0836671, 0.037882, 0.00962225, -0.00643662, 0.00924909, -0.0867182, -0.0988427, 0.0371857, -0.0154039, 0.0590019, 0.11621, -0.0579804, -0.00717846, -0.0464527, 0.0353237, 0.0464869, -0.0389412, -0.055547, -0.0363228, -0.0488475, -0.0838902, -0.0712241, -0.14053, -0.173522, -0.165924, -0.104171, 0.0976514, 0.130962, 0.151162, 0.0792622, 0.0955739, 0.0128972, -0.121462, -0.0152381, -0.0208494, -0.015482, -0.0369065, 0.0333365, 0.0775833, -0.0260508, -0.0253975, 0.0480527, 0.0973636, -0.0530882, -0.00535074, 0.0140426, 0.0265864, 0.0143338, -0.0704473, -0.127889, -0.211274, -0.146887, -0.0245632, -0.0450604, 0.0113859, 0.149811, 0.229871, 0.0461909, 0.0397301, -0.0426475, -0.0821973, -0.134971, 0.00832091, -0.046023, -0.000164297, 0.0726322, 0.100641, 0.06769, 0.000190189, 0.0324515, 0.0474762, 0.077948, 0.0558077, 0.00853043, -0.0399525, 0.0196375, -0.082855, -0.174507, -0.142198, -0.0737093, -0.191029, -0.111437, 0.108538, 0.0621856, 0.0625721, 0.0776102, -0.023756, -0.0629953, -0.0606572, -0.0845574, -0.19428, -0.067816, -0.178332, -0.0594479, 0.143055, 0.00294914, -0.0378157, -0.0190319, 0.033162, 0.0210576, 0.041453, -0.0653523, 0.089147, -0.0285155, -0.0491134, -0.00448703, -0.0617363, -0.114449, -0.138596, -0.117985, 0.0724265, 0.0119644, 0.20359, 0.140263, -0.124966, -0.175834, -0.123737, -0.0909051, -0.157748, -0.0563918, -0.0964865, 0.00727544, 0.028361, -0.0290733, 0.0329905, 0.00939059, 0.105746, 0.00623598, -0.0524089, -0.0938528, -0.0175922, -0.136516, -0.0696088, -0.030876, -0.130426, -0.115183, -0.0976041, -0.0640897, 0.122029, 0.128852, 0.123121, 0.124636, -0.0787632, -0.144687, -0.0472528, -0.00298901, -0.110952, -0.027927, -0.048854, -0.132417, 0.000773157, 0.0189228, -0.0756357, 0.0280357, -0.00817398, -0.035599, 0.02906, 0.000549076, -0.0311053, -0.0125468, -0.00203233, -0.0471751, -0.0886508, -0.0261343, 0.0410764, 0.00393406, 0.183942, 0.108848, 0.0491617, -0.00323621, 0.0137679, -0.124404, -0.0585716, -0.0354001, -0.071739, -0.0992652, -0.0267846, -0.13588, -0.0795605, -0.0910335, 0.0919451, -0.0820862, -0.00574628, 0.0325044, -0.0392475, 0.0702727, 0.0368372, -0.0412474, -0.0914774, -0.0978196, -0.0678467, -0.00732765, 0.0895699, 0.213707, 0.197307, 0.195567, 0.0473869, -0.00429633, -0.0173167, -0.0768055, -0.0452507, 0.0187073, 0.0753136, 0.015877, -0.0929216, -0.0635198, -0.129821, -0.0392783, -0.0680465, 0.0698167, 0.13055, -0.0280648, 0.0760652, 0.0295692, -0.01566, 0.0293333, -0.148739, 0.0187684, 0.00399222, 0.0359347, 0.131658, 0.138954, 0.146998, 0.128589, 0.0495918, 0.0167247, 0.0620753, 0.0189631, -0.135492, -0.0809265, -0.0435713, 0.108747, -0.0468911, -0.0132586, -0.117007, -0.0758449, 0.0324801, -0.00324533, 0.00429602, 0.0770623, 0.061266, -0.00284439, -0.0955171, -0.0488833, -0.0115489, 0.0148483, 0.0701753, 0.127821, 0.0984024, 0.0646991, 0.0202113, 0.100009, 0.0365445, -0.0150901, 0.0475769, -0.130055, -0.00819988, -0.0258685, 0.0858993, 0.0102457, -0.0332437, -0.0559544, -0.102077, -0.135258, -0.0420525, -0.10962, -0.0511075, 0.0265463, 0.00167699, -0.0507896, 0.0374272, -0.0964238, -0.0496905, 0.104246, 0.0758248, 0.0228476, 0.11159, 0.0296348, 0.0500593, -0.0624168, -0.0207774, 0.0844291, 0.00298965, 0.0384821, -0.0835576, 0.0659888, 0.108579, 0.0291975, -0.0659916, -0.0531341, -0.0431981, 0.0130634, -0.0441776, 0.0641945, 0.0237543, 0.0274263, -0.0887807, 0.0494992, 0.00947891, -0.0899236, -0.011789, -0.0772377, 0.0699325, 0.0365095, -0.0889942, 0.134351, 0.00721096, -0.0185285, 0.00447129, 0.136581, 0.0264614, -0.0465963, 0.0442218, -0.00864653, 0.103328, 0.00256952, -0.046528, 0.0168711, 0.0501262, -0.0978941, -0.0125394, -0.0326251, -0.0476573, -0.0527009, 0.0718448, -0.0192303, 0.126339, 0.0901059, 0.0128337, -0.0364668, 0.00614724, 0.0346804, 0.134485, -0.0242075, -0.111813, 0.0044376, 0.121331, 0.0326373, 0.0102661, -0.0712751, 0.0969411, 0.00964781, 0.0497702, -0.0448914, -0.0889656, -0.0393623, 0.00607384, 0.0578635, 0.0303395, -0.0425201, 0.0507426, 0.0577299, -0.0698558, 0.0385417, -0.0198483, -0.0261022, 0.0199654, 0.0213711, 0.00843867, 0.0514873, -0.0973538, -0.0667277, 0.00374708, 0.147772, 0.0798358, 0.015238, 0.103542, 0.00622992, 0.0085229, 0.100557, 0.0387188, -0.0508275, -0.0878357, -0.0140861, 0.0120721, -0.0681055, -0.0211009, -0.100261, -0.0195182, -0.0261758, 0.00202578, 0.0703776, 0.0444802, 0.0698636, 0.0629599, -0.0076464, 0.119652, 0.066313, -0.104525, 0.0337725, 0.0398091, 0.0353519, 0.0515234, 0.108879, 0.0682516, -0.147549, -0.0359496, -0.0557083, -0.101187, -0.103111, -0.123911, -0.0986994, -0.0406274, 0.0341674, -0.0662541, 0.00853253, 0.0437364, -0.0187583, -0.047666, 0.0169396, -0.0191621, 0.0508069, 0.110477, 0.113384, 0.0234836, -0.0531582, -0.127364, -0.0838809, -0.046029, -0.0200105, 0.0898058, 0.0392396, 0.0589953, -0.0489284, 0.00777957, -0.00439238, -0.107795, -0.150639, -0.153444, -0.028738, 0.0624655, -0.0871074, -0.0118128, 0.0636009, -0.0639729, 0.0213019, 0.0456521, -0.0968402, 0.0085581, 0.0147694, 0.0822943, -0.023959, -0.0524937, -0.0517975, -0.105752, -0.0713538, 0.0012278, -0.0402679, 0.0703257, 0.0392094, -0.0181846, -0.0231859, 0.0490719, -0.0240587, -0.0112049, -0.0451521, -0.00170149, -0.0677768, -0.0723367, -0.0374461, 0.0537517, -0.0518318, 0.0954927, -0.0721255, 0.130994, -0.121747, -0.00477236, 0.00808736, -0.126291, -0.0606238, -0.103396, -0.0517166, -0.0379907, -0.0870434, 0.0192766, -0.0219955, 0.0610253, -0.0665596, 0.177045, 0.121947, 0.019813, 0.107895, 0.0185559, -0.0755271, -0.115503, 0.0015551, -0.100738, -0.0788669, 0.0301435, 0.00938851, -0.011386, -0.0300108, -0.0429144, 0.00752211, 0.0467053, -0.0348796, -0.0382645, -0.0449821, -0.0222859, 0.043561, -0.027825, 0.123046, 0.0326793, -0.031592, 0.0535437, 0.0246122, -0.00976821, 0.0436382, -0.024288, 0.118449, -0.0299556, -0.031286, -0.0472213, -0.015405, 0.0671636, -0.109227, 0.123361, 0.00648162, 0.00633876, -0.00413881, -0.0431072, 0.0147573, -0.0261692, -0.0148648, 0.0186227, 0.0386082, 0.10451, -0.103646, -0.0256531, -0.033954, -0.11402, -0.0592812, -0.000211712, 0.0521453, 0.0429705, 0.0268398, 0.0411384, -0.0223556, 0.00567919, 0.010055, 0.0894406, 0.0695081, -0.00204806, -0.0246377, 0.0292222, 0.0668627, -0.0410494, 0.00133094, 0.0403106, -0.0415077, 0.0371342, -0.101708, 0.00111931, -0.161984, -0.0851695, -0.0826266, -0.0123434, -0.0443743, 0.00497189, -0.0305607, -0.0622908, -0.0814299, -0.116438, -0.106192, -0.0909123, 0.0572035, -0.00226542, 0.0170169, 0.0326965, -0.0597512, 0.00055183, -0.0055364, 0.0663782, -0.102215, 0.0758782, -0.0641, 0.071285, 0.0239404, 0.10648, 0.0133015, -0.00259587, 0.0359437, 0.00575072, -0.14062, 0.0362662, 0.0208832, -0.0195757, -0.053119, -0.0507077, 0.00676352, -0.00848676, -0.0780533, -0.0272295, -0.0417229, 0.0151369, 0.00392067, -0.0689513, -0.0228769, 0.0115024, -0.0274378, -0.0394105, -0.0554353, -0.00598026, 0.0310025, 0.0793197, -0.00389834, -0.00137364, 0.0421461, -0.042238, -0.0861567, 0.0285231, 0.0686009, -0.0362131, -0.0210869, -0.0788883, 0.00647861, -0.00536875, 0.000326432, 0.025107, -0.00845517, -0.0557553, -0.00311517, -0.0547335, -0.00558925, -0.0559206, 0.00361306, -0.0914227, 0.0752612, 0.0584868, 0.0211692,
0.0156764, -0.0210134, 0.0646369, -0.100333, 0.028707, 0.0474233, -0.0208148, 0.103403, 0.00387386, -0.0173027, -0.00856517, 0.0263958, -0.00830758, -0.0542828, -0.108265, -0.0240753, 0.0349608, 0.0739031, -0.0519916, -0.0513389, 0.0116351, 0.0283109, -0.0449153, -0.0194223, -0.0472994, -0.0454427, 0.00711025, -0.0128857, -0.0220918, -0.106418, 0.0135816, 0.0514687, -0.0118791, 0.0711051, -0.0472547, -0.00994764, 0.00727271, 0.0280377, -0.0672161, -0.0182877, -0.00636312, -0.0381659, 0.0429378, -0.066252, -0.0357459, 0.00136933, -0.108101, 0.010529, -0.00969508, -0.034752, -0.0531392, 0.0137117, -0.0358283, 0.0130202, -0.0292179, -0.0452993, -0.0587057, -0.0709405, 0.000368145, -0.0101811, 0.0631365, -0.00466286, -0.0571702, 0.015831, -0.014019, 0.0486306, -0.00399469, -0.0705027, 0.0300581, 0.0340651, -0.0527511, 0.081294, -0.0902428, -0.071995, -0.165724, -0.0368367, 0.0041716, -0.00285718, -0.0659332, -0.0269076, -0.0383677, 0.109046, 0.00401785, -0.0255258, -0.0881793, -0.0999763, -0.00138147, -0.0327616, -0.00954218, -0.0488915, -0.119144, -0.019373, -0.0323293, -0.00488492, -0.0167319, -0.0456141, -0.115432, -0.135284, -0.015691, -0.0697046, -0.0111818, -0.0167334, -0.0325074, 0.00635097, -0.0350158, -0.0558745, -0.0966978, -0.0183607, -0.0143346, -0.00314276, 0.0467944, -0.084676, 0.00439859, 0.0451982, -0.0226497, -0.0387702, -0.0993088, -0.0240483, -0.12753, 0.0223321, -0.00138788, -0.0842276, -0.0804474, -0.0732621, -0.0890967, -0.071928, 0.00208255, -0.0102167, -0.0555816, -0.00592074, 0.0021567, -0.136799, -0.0141975, -0.105803, -0.0170164, 0.0782894, -0.0186994, -0.09089, 0.0656271, -0.0707154, 0.00147083, -0.0236216, 0.0689938, -0.0485626, -0.0822569, -0.00649586, -0.0513063, -0.0238255, 0.0923262, -0.0373789, -0.0115755, 0.0921391, 0.0885061, -0.00581698, -0.0312137, 0.0627429, -0.128613, -0.034544, 0.00889304, -0.0776398, -0.0663044, 0.02151, -0.00487086, 0.00721621, -0.040667, -0.0204419, -0.0369992, -0.0504057, 0.0399813, -0.0740761, -0.0189825, 0.087542, -0.0337304, -0.0370705, -0.0772487, 0.0204475, -0.0682202, -0.0170738, -0.0223074, 0.0282192, 0.0126719, -0.0166087, -0.0548087, -0.0359202, 0.0834745, 0.0943004, -0.0416886, -0.0917813, 0.0274842, -0.078991, -0.0659426, -0.0547573, -0.112183, -0.00613624, -0.013742, 0.0786953, 0.0498256, -0.0243481, 0.0769044, -0.0202335, 0.0274165, -0.0526113, -0.00916066, -0.018771, -0.0397956, -0.0179008, -0.109966, -0.0118703, -0.039341, -0.0731426, -0.117528, -0.103962, 0.0634472, 0.112937, 0.0178183, 0.00391411, -0.0106887, -0.0291883, 0.0294185, 0.0682682, 0.0899304, -0.0186788, 0.0831286, 0.0657476, -0.0357572, -0.0089544, 0.0853714, -0.0415656, -0.0415528, 0.0213288, 0.0659572, -0.0534877, -0.00243875, 0.0726021, -0.133877, -0.159647, -0.19131, -0.0719517, -0.0148894, 0.0781023, 0.0380515, -0.0123502, 0.0964753, -0.0878741, 0.0097541, -0.0252358, -0.0773972, -0.141933, 0.0349143, -0.0554269, 0.0841121, -0.0337968, 0.013877, -0.0161951, 0.0655902, -0.102762, -0.126992, -0.0132587, -0.103813, -0.0459706, -0.0659836, -0.0896381, -0.0519805, 0.0172692, -0.0920053, -0.0197982, 0.0250494, 0.164841, 0.151036, 0.0431291, 0.147243, -0.04148, -0.070595, -0.0337709, -0.0419854, -0.0177743, 0.0160078, -0.0197814, 0.0218782, 0.0338854, 0.041718, 0.0720932, -0.00810651, -0.0890393, -0.0249248, 0.0634299, -0.042141, 0.0135935, 0.0158305, 0.013469, 0.0394049, 0.0622745, -0.0399477, 0.0384469, -0.016898, 0.0770274, 0.115592, 0.0727023, 0.0241874, -0.000249254, -0.12491, -0.095968, 0.0193468, -0.0421376, -0.0485581, -0.0479099, 0.105896, -0.0298485, -0.0261305, -0.0628017, 0.0591555, -0.0216414, -0.0693863, -0.0458153, 0.0242895, -0.0140026, 0.0330616, 0.0897337, 0.129772, 0.0715046, 0.164709, 0.129202, 0.0248184, 0.142798, 0.133003, 0.0695597, 0.00614294, -0.024527, -0.0511753, 0.0810082, 0.0353915, -0.177455, -0.0184227, -0.00471328, -0.0482458, -0.160234, -0.0379478, 0.0201604, -0.0220472, 0.0591875, 0.0894999, -0.0625577, -0.000245446, -0.00325644, 0.0709451, 0.0873327, 0.0112468, 0.0533899, 0.0825893, 0.0959722, 0.0867954, 0.162567, 0.178117, 0.126604, 0.0608357, -0.0763228, 0.0541146, 0.0142642, -0.0749789, -0.035295, -0.00995545, 0.0567249, 0.0135814, 0.0227028, 0.00401916, -0.00240801, -0.0383259, 0.109965, 0.151028, -0.00520135, -0.0202563, 0.0225432, 0.0892213, -0.0265329, 0.0878003, -0.0164509, -0.0325478, 0.0807475, 0.0719846, 0.170058, 0.142221, 0.115491, 0.0775158, -0.10583, 0.0858119, 0.021271, -0.0686932, -0.095328, -0.109877, -0.0131036, -0.0297773, -0.00233873, -0.0867538, -0.00124739, 0.0198275, 0.00292188, 0.0331543, -0.0943323, 0.0560845, 0.0354282, 0.0218333, 0.062022, -0.0957295, -0.0840303, -0.0450883, -0.0355608, 0.0527876, -0.0351238, 0.152963, 0.0627805, 0.0851656, 0.00891825, -0.0462134, -0.0913209, -0.0516178, -0.0402224, 0.0383195, 0.0375977, 0.031381, -0.102997, 0.034879, -0.00589624, -0.0655845, 0.0372976, 0.0699069, -0.0456894, -0.0169987, 0.0593624, 0.098839, 0.0540297, -0.0236654, -0.0216776, -0.00805899, -0.0385137, 0.0607263, 0.0785164, 0.170996, 0.162599, -0.028466, 0.000352618, -0.0637616, -0.142301, 0.0916661, -0.0440492, -0.0617415, -0.0610309, 0.0513043, -0.0131289, 0.0176789, 0.0970789, -0.00640639, -0.0171409, -0.0027853, -0.0685667, 0.0517329, 0.0791331, 0.0287867, 0.115729, 0.0557471, -0.00527421, -0.0450433, 0.124055, 0.093119, 0.128394, 0.231175, 0.0567741, -0.131958, -0.0968677, -0.0686306, -0.166013, -0.0267622, -0.0219596, -0.0458971, 0.0365902, -0.00594096, 0.0594383, -0.00985001, 0.00257803, 0.00783855, -0.00565367, -0.0771843, -0.0134779, -0.0565279, 0.02712, 0.026221, 0.086481, 0.15289, -0.0161383, -0.0120165, -0.00525088, 0.175806, 0.129943, -0.0247412, -0.118278, -0.154252, -0.0697429, 0.00432007, -0.128981, -0.043865, -0.00967406, -0.00999052, -0.0210057, -0.0302996, 0.136119, -0.0651775, -0.100339, -0.0104523, 0.00611469, -0.0609841, -0.044352, -0.0755617, -0.0851788, 0.0182444, -0.047958, -0.00465623, 0.0259908, -0.0873551, 0.010264, 0.0224973, -0.0642546, -0.116847, -0.116016, -0.07501, -0.139557, 0.00997275, 0.0530403, -0.0448675, -0.0948669, -0.114823, 0.0172413, 0.0722786, -0.0464184, 0.0565464, 0.0142764, 0.0771941, -0.0336086, -0.0167677, -0.118592, -0.103514, -0.0849764, -0.0498789, -0.136883, -0.103195, -0.0746467, -0.142864, -0.0844237, -0.0830193, -0.0130784, -0.0269993, 0.0118851, -0.0418834, 0.0637971, -0.0344303, -0.0265957, 0.000501502, -0.093433, 0.0341655, 0.0150809, 0.0684015, 0.0155517, 0.0238224, -0.0179051, -0.0709121, -0.012798, -0.0805477, -0.140443, -0.226627, -0.0822705, -0.117892, -0.0855147, -0.0177718, -0.0589568, -0.0854592, -0.130203, -0.0388425, -0.0437867, 0.0633252, 0.0363896, -0.0633641, 0.031344, 0.0495984, -0.0643255, 0.00419847, -0.109766, -0.0595466, -0.0644737, 0.0236108, 0.0534797, -0.0734737, -0.0249744, 0.0584622, -0.00187927, 0.0363225, 0.101664, -0.0854809, -0.153483, 0.0265465, 0.0653334, 0.024607, -0.0521378, 0.0199344, 0.0100504, -0.0366803, -0.0588337, 0.0955653, 0.0722112, 0.0404236, 0.00278882, 0.0684353, 0.0248267, -0.0282083, -0.0724578, 0.0623077, -0.031974, 0.00509947, -0.0161352, 0.0661486, -0.0310044, -0.0432431, -0.0387034, -0.0494681, 0.0359745, 0.00799739, 0.0437304, 0.0288331, 0.0405263, 0.0329963, -0.045966, -0.0484184, 0.107072, 0.132631, -0.0500145, 0.016489, -0.122426, 0.0915087, 0.000683708, 0.083492, -0.0195195, 0.107534, -0.00126494, -0.00638995, -0.0386806, 0.0108064, -0.00351664, 0.0597384, -0.0201108, 0.0894221, -0.0122107, -0.0718542, -0.0653755, 0.0730491, -0.0795872, 0.00705927, 0.0185618, 0.000482284, -0.0528571, -0.00437777, -0.0462011, -0.0275794, -0.0521488, -0.0502996, 0.0103179, 0.0136001, -0.0109263, 0.0712558, 0.0511047, 0.0897101, -0.0601944, 0.0528631, 0.0208353, 0.0145972, 0.0357826, 0.0366621, -0.0242471, -0.0761821, -0.00729484, 0.00237832, 0.0130555, -0.0424714, 0.0426393, -0.100604, -0.0462019, -0.0442128, -0.0975214, -0.196261, -0.144329, -0.0939918, -0.0359152, -0.107937, -0.107587, 0.0630366, 0.031524, 0.176083, 0.0528849, 0.0287124, 0.0476982, -0.0344509, -0.00908275, -0.0982116, -0.0133119, -0.0476926, -0.0299796, -0.0124384, 0.00497155, 0.0919431, 0.0930663, 0.0691074, 0.0466687, -0.0141054, 0.0202639, -0.0470463, 0.0415805, -0.0201532, -0.137566, -0.005571, -0.117288, 0.0550164, -0.0763707, 0.0442833, -0.00672903, -0.0114899, 0.0118029, 0.0409915, 0.0598976, 0.0051696, -0.0697815, -0.0581748, -0.0281798, -0.084028, 0.00416117, -0.114466, -0.0597789, 0.067066, 0.0598586, 0.100309, -0.00831295, 0.03338, 0.00112055, 0.0156626, -0.0801786, -0.0411071, 0.0686098, 0.0814447, 0.0499387, 0.135187, 0.00136405, -0.035471, -0.00916318, 0.0309781, -0.0689363, 0.0212818, 0.0327335, 0.00294125, 0.00442438, 0.0721222, -0.00387745, 0.0310262, -0.0724042, 0.073598, 0.0142118, 0.00558798, -0.0785606, -0.0287389, 0.0516162, -0.0184031, 0.0571057, -0.0610043, 0.0277916, 0.081543, -0.0727171, -0.0514816, 0.0438272, -0.150181, 0.112081, 0.107689, 0.0412948, 0.0478499, -0.0426525, 0.0570772, 0.0104812, 0.0176856, 0.0214449, -0.0154471, -0.032527,
0.0466752, -0.0223742, -0.040789, 0.0735184, -0.0681021, 0.0661581, -0.067223, 0.0114152, -0.0376863, 0.0439019, -0.0101566, -0.0892716, -0.00271761, 0.00865585, 0.0571731, -0.0727146, -0.0256138, 0.0739784, -0.0718285, 0.0915369, 0.0557898, -0.0227205, -0.00918255, -0.0345558, -0.043451, 0.0043392, -0.0343852, 0.0240836, -0.077591, 0.00559884, 0.00505939, 0.0618628, -0.00557468, -0.0598548, -0.023205, -0.0488216, -0.120633, -0.000310582, -0.0754833, -0.0287226, -0.0251537, -0.0140608, -0.0647719, -0.00375572, -0.0380181, 0.00362111, 0.0359827, -0.0570008, -0.0617778, 0.00936076, 0.101249, 0.0112735, 0.0261165, -0.0590259, -0.0692373, 0.00641947, 0.0595852, -0.0357166, -0.0084708, 0.0104127, -0.0296753, 0.014418, 0.0251266, 0.0670103, -0.0137449, 0.0211839, -0.0125285, -0.0159258, -0.000360719, -0.0208325, -0.0418894, 0.0819711, 0.109428, -0.0414603, 0.0597846, 0.0316664, -0.0710154, 0.0617788, -0.000396535, -0.0542398, -0.0204907, 0.0273465, -0.0735771, -0.0324404, 0.0276214, -0.0571001, 0.0240181, 0.0237546, -0.0056437, 0.0251196, -0.0622694, -0.0942162, -0.099238, 0.0195656, -0.133246, 0.0272003, 0.0417896, -0.0944679, 0.00856717, -0.00257217, 0.0147152, 0.0994236, 0.0824406, 0.116595, 0.143987, -0.0200727, 0.0751256, -0.0634358, -0.036554, -0.0473629, -0.0877379, 0.0176195, 0.062259, -0.111909, -0.0550159, -0.105088, 0.0283104, 0.0238106, 0.0304644, 0.063514, -0.0231737, -0.0397946, -0.0708655, 0.0888001, -0.133919, -0.0900086, -0.150773, -0.0345903, -0.0584701, -0.0311341, -0.0525282, 0.0900676, 0.0128195, -0.0972085, -0.111024, 0.020164, -0.0271954, -0.0188817, -0.0271336, 0.0551449, 0.0341729, -0.0462449, 0.0352156, 0.0701382, -0.0152177, -0.071681, -0.101384, 0.00448522, -0.109867, 0.0658691, 0.00198077, 0.00780566, -0.0266291, 0.0880479, -0.0457254, 0.0323716, 0.0459551, -0.00662459, -0.0565821, -0.00851269, -0.0452697, -0.0923557, 0.0425657, 0.122027, 0.0263443, 0.0214177, 0.0906012, 0.0742355, -0.0949775, -0.0152663, 0.0370866, 0.0187286, -0.0570646, 0.0822888, -0.0241252, 0.0537262, -0.000911066, 0.0439195, 0.0746947, 0.0289799, -0.0145745, -0.0528104, -0.0436053, -0.0571828, -0.010178, -0.0276748, 0.0236442, 0.0731466, 0.0337917, -0.0508114, -0.023623, 0.035366, 0.0303182, -0.0737354, -0.0237244, -0.0557205, -0.0558591, -0.00407696, -0.0781387, 0.0979126, -0.0930716, -0.0387284, -0.0833887, 0.0446069, 0.00797505, 0.0174648, 0.0265732, -0.0136559, -0.0471013, -0.0788025, 0.150144, -0.0466065, 0.000383841, 0.0541545, 0.0569117, 0.0240929, -0.0012009, 0.0803772, 0.104094, 0.082856, 0.0698255, 0.0770197, 0.00490428, -0.0289343, 0.0132868, 0.0055285, -0.0491357, -0.0121845, -0.0269484, 0.0184144, -0.0373957, -0.048917, -0.0626768, -0.0827947, -0.0985495, -0.177115, -0.139807, -0.0936852, -0.0429827, 0.0144851, 0.0471541, 0.054369, -0.0135719, -0.09416, 0.0218865, 0.141448, -0.0195564, 0.0564229, -0.020593, -0.00848996, 0.0771181, 0.0268924, -0.0706972, -0.0541076, -0.111957, 0.0689458, -0.0100483, -0.0787982, -0.0635387, -0.023536, -0.157689, -0.139277, -0.0971166, -0.121672, -0.157181, -0.08913, -0.0611542, -0.117838, -0.0175889, -0.0213152, -0.0581051, -0.058641, 0.0181743, 0.00560004, 0.0131645, 0.0710331, 0.0167605, 0.0225093, 0.0332101, -0.0536812, 0.0111971, 0.0455378, 0.073119, -0.0349163, -0.0314662, -0.00996367, -0.0826498, -0.014656, -0.0848607, -0.161054, -0.22995, -0.245259, -0.185298, 0.00107943, 0.0420264, 0.0021239, -0.00644177, -0.097631, -0.0527259, -0.081183, 0.0205565, -0.211114, 0.0133738, -0.000147294, 0.0801587, -0.0108471, 0.0439718, 0.0295577, 0.0345484, -0.00533935, -0.0302356, -0.00179828, 0.00537178, -0.0740246, -0.0011197, -0.0684047, -0.0405791, -0.116403, -0.0416654, -0.0149021, 0.11167, 0.157965, 0.0899525, 0.0698649, -0.156915, -0.0585759, -0.0447543, -0.104878, -0.0765626, -0.13001, -0.131585, -0.100983, -0.0365643, 0.0397054, 0.0241034, -0.0281346, -0.0607644, 0.0347864, 0.00799603, 0.0148517, 0.0473427, 0.0441284, -0.0221154, -0.0453629, -0.0787518, -0.0515311, 0.0521681, 0.0567903, 0.208621, 0.176292, 0.0912267, 0.0066603, -0.0861256, -0.0260274, -0.0490405, -0.181695, -0.203133, -0.11783, -0.0787025, -0.0837506, -0.0559905, 0.00495487, 0.0328393, -0.119314, -0.0684373, 0.0503762, -0.0675004, -0.0268028, 0.020817, 0.00506193, -0.0684492, -0.0439928, -0.0842473, 0.0193328, 0.161141, 0.0554026, 0.120072, 0.0677289, 0.0503113, -0.0772287, -0.0101767, 0.00497614, 0.0156384, 0.0249481, -0.0618941, -0.15021, -0.209811, -0.0247678, -0.0481278, 0.00349559, 0.0478508, 0.0360939, 0.0325851, -0.00984519, -0.00924794, -0.011113, 0.0366644, -0.00238824, -0.0523987, 0.00504143, -0.0368741, 0.145894, 0.0437602, 0.142609, 0.119376, 0.02682, -0.0410296, -0.0520639, -0.135635, -0.0621335, -0.0400263, -0.0508662, -0.0424976, -0.0525358, -0.0284376, -0.0649203, -0.00122889, -0.0220628, -0.0575426, -0.00181621, -0.0946731, -0.0451545, 0.0500575, -0.0680573, -0.0849847, 0.0573025, -0.111068, -0.0946082, -0.165827, 0.14487, 0.0557037, 0.124696, 0.164433, 0.0714411, -0.0332117, -0.056322, 0.0740067, 0.0958986, 0.0025142, -0.00546899, 0.023861, -0.11888, -0.0482332, -0.0252045, -0.0746697, 0.0720504, 0.0596335, 0.0501745, 0.0195939, -0.0613504, -0.0367277, 0.000788391, -0.0271557, -0.127291, -0.194485, -0.13816, -0.118026, -0.0707322, -0.0432472, 0.0716021, 0.123103, 0.0365126, -0.0387787, -0.026507, 0.0101178, 0.00113673, 0.0594753, 0.042631, -0.0351313, -0.109759, 0.0357313, -0.0565109, -0.0857973, -0.0513746, 0.0169611, -0.0777718, -0.0195217, 0.0139604, 0.0315358, 0.0111217, -0.0280828, -0.0487762, -0.158643, -0.106513, -0.16354, -0.177387, -0.0370654, -0.0419069, 0.01619, -0.0290647, 0.0185058, 0.0740705, 0.0701354, 0.0391199, 0.0999325, -0.0138008, -0.0598636, 0.0358827, 0.0924594, -0.113506, 0.0114356, 0.021867, 0.0218678, 0.0281508, -0.0139007, 0.102745, -0.00975327, -0.0232657, 0.122348, -0.0766416, -0.0636405, -0.114261, -0.0332851, -0.115364, -0.146541, -0.0381838, -0.036819, -0.0714904, -0.00216017, -0.0352473, 0.0893328, 0.0533598, 0.0879264, -2.58814e-05, -0.104126, -0.0997158, -0.0603445, -0.0527214, -0.0848557, 0.0471277, 0.0185696, 0.106789, 0.0375585, -0.0712263, -0.0784388, 0.128741, 0.0602411, -0.0129211, 0.0825404, 0.0877152, -0.113567, -0.055631, -0.148351, -0.050773, 0.0745258, 0.086233, 0.0988143, 0.209686, 0.0960531, 0.0172442, 0.0723295, -0.0893858, 0.0178178, 0.0076812, -0.0535995, -0.0882964, -0.0925275, -0.000460282, 0.000697138, -0.0725782, -0.0580061, 0.0326293, -0.0155146, 0.0134791, 0.0505518, 0.0306171, 0.0744941, 0.0630315, 0.0125494, 0.0227356, -0.0429867, -0.104995, -0.0330007, 0.104852, 0.0530685, 0.0907214, 0.0741092, -0.0144921, 0.0516355, -0.0736662, -0.135978, -0.11626, 0.00787802, -0.0353657, -0.0180931, -0.00578703, 0.0164731, 0.0234218, 0.0721201, 0.0386243, -0.0250143, 0.00563048, 0.0605546, 0.0631985, 0.0387005, -0.000199768, 0.16047, -0.0159349, 0.065721, 0.0519026, 0.0133064, 0.035351, 0.0968103, 0.0293477, -0.0694413, 0.0692012, 0.00442902, -0.0174689, -0.0881611, 0.0549266, -0.0632198, -0.098243, -0.0208342, 0.0723342, -0.0803741, 0.0935946, -0.0424905, -0.00260846, -0.02271, 0.00506649, 0.00517763, 0.0430768, -0.0143643, -0.0264946, 0.0669665, 0.0627189, 0.0900192, 0.0386771, 0.114198, -0.0448337, 0.0802258, 0.07643, 0.00823628, 0.0190695, -0.15688, -0.0356385, -0.0969551, 0.0308535, -0.0401554, -0.0349615, -0.0718541, 0.0344808, 0.0395171, 0.0656862, -0.00303643, 0.0394791, 0.0388967, 0.0197415, 0.0415799, -0.0103244, 0.0143348, 0.0499675, -0.0933443, 0.0374561, 0.115145, 0.126481, -0.0152927, -0.00410926, 0.0318877, 0.0558135, 0.000377952, -0.0768941, -0.0267368, 0.0600392, -0.0900576, -0.0080345, 0.0572891, 0.0242006, -0.0116091, -0.0225376, 0.00433622, 0.0332271, 0.0554216, 0.0938338, -0.0370187, -0.0176814, -0.0171222, -0.0755321, 0.025106, -0.025395, 0.0909264, 0.0873994, 0.0169985, -0.0227269, 0.0618333, -0.0316271, 0.074019, -0.0510006, 0.0319162, 0.0434706, -0.0332194, 0.0367335, -0.01303, -0.0115112, 0.0211959, 0.00438631, -0.0288824, 0.0265503, -0.029585, -0.0575775, -0.0231923, -0.0943129, 0.0390665, 0.00401791, -0.000139521, -0.0427614, 0.0778312, -0.0127122, 0.142454, 0.092216, -0.0214531, 0.0660255, 0.0389472, 0.0381293, -0.0738657, 0.0466544, -0.0121633, 0.00777811, -0.0328147, -0.0138722, 0.0582387, -0.00344778, 0.0161994, -0.0159443, 0.030241, 0.0099094, -0.00523585, -0.0862648, 0.0924934, -0.0212726, -0.106372, 0.0284599, 0.0901695, -0.0594803, 0.00451972, 0.0306242, 0.0631143, 0.0203837, 0.0947718, 0.00292248, -0.00858169, 0.0471848, 0.0475609, -0.0731332, 0.00852456, 0.0306263, -0.0198197, -0.0194851, 0.00358011, -0.0477498, -0.0217542, 0.0223223, -0.0758677, 0.0225588, 0.0687233, 0.0225585, 0.0250737, -0.0349771, -0.0232947, -0.11182, 0.0245275, -0.00349165, 0.0232047, -0.0189893, -0.076756, -0.0109285, 0.118565, -0.0508918, -0.0865746, 0.0171779, -0.0841876, 0.030982, -0.0602976, 0.0221968, -0.0785859, -0.0120691, -0.102219, -0.0578189, -0.0518455, -0.0651285, 0.0424415, 0.0261556, -0.0465785, -0.0406496,
-0.0408371, -0.0482167, -0.048881, 0.0162423, 0.0231761, -0.00320998, 0.0153796, -0.000707051, 0.0802342, 0.0636003, 0.0688302, 0.0698434, 0.0231761, -0.0680458, -0.00333656, -0.0311255, 0.00300142, 0.00612828, -0.0136202, 0.0600759, 0.00403877, 0.0565351, 0.0445596, -0.0300074, -0.0407621, 0.0457101, 0.021132, -0.0513428, 0.07115, -0.0577524, -0.101706, -0.0208317, -0.0124582, -0.0513355, 0.034592, -0.0685327, -0.0504835, 0.00898034, 0.0498377, 0.121095, -0.101605, -0.00833449, 0.0343402, 0.0336099, -0.0264184, -0.018179, -0.0344977, 0.0358498, 0.0197839, 0.0386869, 0.0421993, 0.0193745, -0.0379795, -0.00425641, 0.0183039, -0.0339954, -0.0158192, -0.0640962, -0.0332115, 0.0773001, -0.0638811, -0.035025, 0.0240541, 0.00143751, 0.0421122, 0.0681727, -0.0788479, 0.0464362, 0.0224574, 0.0845112, 0.112933, 0.0911174, 0.0658, -0.0353591, 0.0670425, 0.0913762, 0.0720016, 0.0922291, 0.119904, 0.025824, 0.0517043, -0.00953172, 0.0722033, 0.0796362, 0.0237307, -0.0632065, -0.0161799, 0.0100165, 0.00760483, 0.0463551, -0.0549552, -0.0479051, 0.0101256, -0.00606261, -0.0124827, 0.0554688, -0.0514734, 0.00190541, -0.0541757, -0.0945106, 0.148359, 0.0492387, 0.131177, 0.0872804, 0.0526183, 0.0325617, 0.104346, 0.057273, -0.0114014, 0.0599289, -0.0426502, 0.0337413, -0.0910967, -0.0279554, -0.0539209, -0.0620705, -0.00742126, 0.0217868, 0.00172387, 0.0334535, 0.009416, 0.0137787, 0.0227941, 0.00748745, -0.0382592, -0.0684623, 0.00658356, 0.0309305, 0.0390548, 0.0472648, 0.0225334, 0.0433189, 0.0157574, 0.0218016, 0.175645, 0.0683856, 0.0503832, 0.00364507, 0.110168, 0.0199743, 0.0359474, 0.00124259, -0.000282663, -0.0674207, -0.0105382, -0.0466325, 0.0642423, 0.0186082, 0.00390106, 0.010156, 0.0470287, 0.0163535, -0.0487112, -0.159518, -0.0539299, -0.139569, -0.0178907, 0.0365008, -0.0924836, -0.0908652, 0.00759863, 0.0601291, 0.0404463, 0.118313, 0.177745, 0.00607243, 0.0279614, 0.0631409, 0.102285, -0.0150103, -0.053289, 0.0619906, 0.0204959, -0.104722, -0.0223241, -0.055444, 0.112881, 0.0708505, 0.00848898, -0.0424682, -0.0619214, -0.197186, -0.168426, -0.0908766, -0.11932, -0.0833163, -0.183363, -0.0634328, -0.123937, -0.0995651, -0.0777685, 0.115855, 0.037864, -0.0167561, -0.0332086, -0.0372857, -0.0214796, -0.0490296, 0.114269, -0.119831, -0.0166388, 0.0832943, 0.0225903, 0.0614951, 0.157558, -0.00546075, -0.0619907, -0.0710105, -0.0881283, -0.14919, -0.0506327, -0.0336842, -0.0237133, -0.104732, -0.199732, -0.106826, -0.345268, -0.184154, -0.0575977, 0.0436234, 0.0425185, -0.0704287, -0.0165934, 0.0686108, -0.00360417, 0.0113732, 0.0119567, -0.0155044, -0.0157425, 0.0405468, 0.0507165, 0.142269, -0.0234715, 0.0950094, -0.00386727, -0.0916564, -0.240576, -0.232699, -0.180596, -0.0570187, -0.127334, -0.171761, -0.125812, -0.246103, -0.125043, -0.0609857, -0.0265705, 0.0422576, -0.0457794, 0.0352102, 0.00882283, 0.0117196, -0.0432898, 0.0393597, 0.00354396, 0.0286432, 0.00435618, 0.096603, -0.0474988, -0.0244827, -0.0185291, -0.000449947, 0.00702266, -0.0360114, -0.118161, -0.26362, -0.236783, -0.19631, -0.0214784, -0.126835, -0.0487179, 0.0349435, -0.0249257, 0.0536078, 0.0380174, -0.000823681, -0.0335435, 0.118335, 0.0163794, -0.0846181, 0.0285199, 0.01048, -0.0724273, 0.0634743, -0.0501082, -0.0465574, -0.053841, 0.0361974, 0.0647786, -0.000233297, 0.0132677, 0.0711355, -0.0435799, -0.210624, -0.212918, 0.0681401, -0.0342727, -0.0122257, -0.0670461, 0.0587047, 0.0719629, 0.0337496, -0.00695575, 0.00649008, 0.0466148, 0.0374108, 0.0190948, 0.00528242, -0.00109072, 0.0172496, -0.0286437, -0.0667068, -0.0218878, 0.0176616, -0.0595073, -0.0135703, 0.0112541, 0.140124, 0.129743, 0.0928269, 0.0805765, -0.0735292, -0.130884, 0.0412295, 0.0040124, -0.0271213, 0.0738547, 0.0555792, -0.0252292, -0.0858808, 0.031749, 0.0992431, 0.0741784, -0.0395885, 0.052317, -0.0674587, -0.0494581, 0.0560525, 0.020147, -0.0105069, 0.0199687, -0.0219042, -0.0707459, 0.0509798, 0.0131413, 0.0530878, 0.110344, 0.127002, 0.12096, 0.028842, -0.0314863, 0.0739402, 0.073463, -0.0053945, -0.0417385, 0.0165199, 0.123317, 0.0781484, 0.00435576, 0.0910958, 0.0605153, -0.0099431, 0.00384899, 0.0497181, -0.00550731, 0.0553729, -0.0888777, 0.0464623, -0.130678, -0.0538592, -0.057481, -0.0084855, 0.0935943, 0.0708594, 0.0407659, 0.141805, 0.107332, 0.120443, -0.0491224, 0.0817051, -0.0347745, 0.00201228, 0.0701968, 0.0484408, 0.132713, 0.0546809, 0.0836706, 0.073992, 0.0467381, 0.00819612, -0.0254138, -0.180173, 0.0408409, -0.0178675, -0.0152026, 0.0028513, -0.0944422, 0.00481521, -0.00436791, 0.0196887, 0.116993, 0.0904224, -0.0310231, 0.118959, 0.0914549, 0.0531967, 0.0866756, 0.0842088, 0.00337747, 0.072274, -0.0306465, -0.00185147, -0.0299267, 0.0693643, 0.0693342, 0.0579093, 0.0215258, -0.00446013, -0.0675827, 0.0305938, 0.0386292, 0.0594248, -0.00909367, -0.0366871, -0.00820891, -0.025868, 0.0822684, 0.157727, 0.11951, 0.062926, -0.0513052, -0.00597722, -0.0198542, 0.11625, 0.163496, 0.0901287, 0.00397228, -0.103409, -0.103487, -0.0421215, -0.0480847, 0.0223289, 0.00706532, -0.0173712, -0.0107437, -0.0765178, 0.0493336, -0.0292491, 0.0355162, -0.0271994, -0.00839171, -0.0197412, -0.0274656, -0.0292854, 0.0236993, 0.0205734, 0.0877594, -0.0074031, -0.0546938, -0.143298, -0.0605463, 0.0480704, 0.100469, 0.0904107, 0.0812428, -0.0307752, -0.0812325, -0.11008, -0.00105071, -0.0306672, 0.125926, 0.00908909, 0.0384682, 0.0377916, 0.0425435, 0.0115507, 0.00539783, -0.0279452, -0.0399818, -0.074627, 0.117507, -0.00375306, 0.0467584, 0.0107175, 0.0296937, -0.112713, -0.119556, -0.0405645, -0.0223786, 0.0510834, 0.12467, 0.0958276, 0.250528, 0.144161, 0.141801, -0.0419105, -0.011422, -0.00997948, -0.0110263, -0.0249817, 0.112017, -0.00826266, -0.0222503, -0.0219177, 0.00950001, -0.0382656, -0.143802, -0.0104161, -0.0238406, 0.0462617, -0.0239716, -0.0603118, -0.0368962, 0.0323384, -0.056972, -0.145494, 0.125826, 0.222882, 0.0514846, -0.0476508, 0.113241, 0.170289, 0.0966683, -0.0282118, -0.0254526, -0.0398196, -0.0143463, 0.0546174, 0.0972696, 0.0187993, -0.000543508, -0.041324, -0.0934286, -0.0428377, -0.0442863, -0.0137747, 0.0830207, -0.0788139, 0.0509312, -0.022738, 0.0717568, -0.0721325, -0.00373654, 0.0536512, 0.0772803, 0.149225, 0.0750285, 0.0677491, -0.0661297, 0.137006, 0.152169, -0.122242, 0.0504702, -0.0647596, 0.0242783, -0.0141786, 0.0145043, 0.0480056, 0.000132984, 0.0501537, 0.00263182, -0.00167191, -0.0167309, -0.127273, -0.0219304, -0.0386008, -0.0950867, -0.152867, 0.00146004, 0.0192659, 0.00875227, 0.128406, 0.065569, 0.204939, 0.0870679, 0.0413716, -0.0468806, -0.0169306, -0.0292003, -0.0659627, -0.065263, -0.0445882, 0.0700375, -0.0134502, -0.0223237, 0.00755302, 0.0271654, 0.0444449, -0.00153269, 0.0099131, 0.0297132, -0.0365167, -0.141139, -0.12318, -0.0733339, -0.0191996, -0.0178339, 0.0280631, 0.097832, -0.00415488, 0.0739939, 0.143076, 0.186298, 0.123078, -0.0387197, 0.0696628, 0.0522462, -0.00871347, 0.0367042, 0.0193479, -0.0858714, 0.02567, -0.0216589, 0.0141866, 0.0456328, 0.0403502, 0.00722155, 0.0779938, -0.0692173, -0.0924655, 0.0404039, -0.0456959, -0.0940672, -0.0388245, -0.100455, 0.0172543, -0.0191336, 0.0733002, 0.0529442, 0.122856, 0.0798673, 0.00423832, 0.027296, 0.0250854, 0.010621, 0.0590632, -0.0855104, 0.00863639, -0.0425704, -0.0832599, -0.0767127, 0.0435279, 0.0332694, -0.0309587, -0.0322612, 0.0800866, -0.00257239, -0.0314935, -0.0619104, -0.0310457, -0.0763356, -0.0628278, 0.108446, -0.109454, -0.0971102, -0.104836, -0.0937234, -0.00918849, 0.0127081, -0.0405991, -0.00837347, -0.0140163, -0.00355613, -0.0716373, 0.0369286, 0.00669796, -0.0356338, -0.0344627, 0.0134133, -0.0633266, 0.0212011, -0.051428, -0.0247753, 0.0282321, 0.0534548, 0.0305314, -0.0442234, 0.0120827, -0.0803398, -0.111745, -0.0546864, -0.00608479, -0.0450044, -0.00634416, 0.0161356, -0.0344411, 0.0332559, 0.0673079, 0.0221056, -0.0677007, -0.0388015, -0.0108587, 0.0265054, 0.10444, 0.0489607, 0.0288505, -0.0606575, 0.0566852, -0.0194618, 0.0142595, -0.0280038, -0.0257402, 0.0225199, -0.033819, -0.051825, -0.0102583, -0.0605937, -0.0274505, -0.0682877, -0.00276002, -0.0116826, -0.0630672, -0.111454, -0.120619, -0.0811103, -0.000945422, 0.00443136, 0.0388965, 0.0168477, -0.0188353, -0.0711572, -0.072999, 0.0391302, -0.111572, -0.0408003, -0.0390117, -0.013262, 0.0411756, -0.0985194, -0.0748383, 0.0327877, -0.088945, -0.0738529, 0.0171189, -0.022419, 0.0491234, 0.00533974, -0.0658433, -0.0404664, -0.101817, -0.155357, -0.0116156, -0.102206, -0.146345, -0.136399, -0.0339067, -0.0770177, -0.0016353, -0.0425224, -0.0410073, -0.0896451, 0.0483238, -0.0260009, -0.0524942, -0.0418332, 0.0199591, -0.0461074, -0.0760779, 0.0159042, -0.030179, -0.0204788, 0.0223174, 0.0341272, 0.0105807, -0.0508388, -0.0144719, 0.0554873, 0.0296303, -0.00946254, 0.0420204, 0.0522575, -0.0673209, -0.0584794, -0.0320512, -0.0709642, 0.023221, -0.0747738, 0.0562834, 0.0795926, 0.040937, -0.0657351, 0.0312443, 0.0268851, 0.0107965,
-0.0216093, -0.00722722, -0.0463779, -0.0643019, -0.181478, 0.00270079, -0.0279889, 0.0708584, 0.0728202, -0.0263024, -0.0445699, -0.136725, -0.00824447, -0.019211, -0.0876247, 0.0407304, 0.00119485, 0.110438, -0.0239329, 0.0571647, -0.0308418, -0.0269694, 0.0886879, -0.00600112, -0.0504482, -0.0461116, -0.028132, 0.0557215, 0.0934655, -0.0285891, 0.000593621, -0.0272827, 0.0245495, -0.0204135, 0.0284567, 0.118112, 0.0527858, 0.0216157, 0.0132447, -0.0183039, -0.0158315, -0.00876848, 0.048256, -0.0408233, 0.0406718, -0.0282954, -0.00261707, 0.0361872, -0.0628823, 0.036085, 0.123119, -0.0143066, -0.00665886, 0.0546436, 0.00470493, 0.0330403, 0.0406156, 0.0065579, -0.0336383, 0.0172162, -0.0394826, -0.00958334, 0.0293638, 0.0243248, 0.0153355, -0.0156271, -0.106406, 0.11973, 0.0043819, 0.0421215, 0.0321877, 0.0270934, 0.119409, 0.04603, 0.0190381, 0.0025953, -0.061413, 0.0669152, 0.0700453, -0.0293744, 0.013032, 0.00951339, -0.0104574, 0.0770563, -0.0642691, -0.0226837, -0.0303799, 0.00835178, 0.0174304, 0.0722072, -0.0335619, -0.0012873, 0.0383465, -0.0508354, 0.0390537, -0.0113302, 0.0170206, -0.0507455, -0.161187, -0.0689195, 0.0405161, -0.0458617, -0.0491972, 0.0426936, 0.00297966, 0.115827, 0.0291184, -0.0164558, 0.0116071, -0.0604353, 0.107574, 0.0179799, -0.0384858, -0.011443, -0.0814271, -0.018787, 0.00418208, 0.0653498, 0.0121233, 0.10251, -0.10453, -0.00395654, -0.00874544, -0.144235, -0.0906143, -0.072017, -0.132066, 0.0324109, 0.0786759, -0.0884641, 0.0520227, 0.066468, 0.113426, -0.00195843, -0.0685257, -0.0155506, -0.0373984, 0.0554229, -0.0215062, -0.0573633, 0.0322149, 0.0170612, 0.0544864, -0.0776533, -0.0599298, -0.0154257, 0.0178076, -0.0397022, -0.0583868, -0.068748, 0.00195376, -0.167917, -0.117944, -0.113575, -0.0750811, -0.126115, -0.0340808, -0.0346459, 0.0381041, 0.0164702, -0.0371909, -0.00154668, 0.00393027, 0.0339762, -0.0449726, -0.0894201, -0.0278905, -0.0715942, -0.0248064, 0.0379591, 0.0370089, 0.0363261, -0.075119, 0.00169116, -0.074687, -0.0196745, -0.0398641, -0.0735211, -0.0489839, -0.0708177, -0.0274003, -0.0878687, 0.00422044, -0.0387251, -0.0156772, 0.0341883, 0.0695574, 0.0234183, -0.0502386, 0.0555103, 0.0570168, -0.00480966, 0.0451045, -0.0667034, -0.0468628, -0.102083, -0.0168203, -0.00555894, 0.028855, -0.00684084, 0.0228645, 0.0557675, -0.0566807, -0.0607078, -0.0124475, -0.103759, -0.0750111, -0.084323, -0.0458453, 0.0158509, 0.124128, 0.0475794, 0.0672171, 0.0956174, 0.0495266, 0.101131, 0.0359703, -0.0657085, 0.0237327, -0.0227611, -0.12071, -0.132025, 0.0737082, -0.0220859, -0.0852687, -0.018061, 0.0495324, 0.0378525, 0.028013, 0.0441713, -0.00659042, -0.0920302, 0.0260022, -0.00904898, -0.101271, -0.0873094, -0.12192, -0.0640771, -0.00297188, -0.00203528, 0.0833306, 0.134065, 0.0536002, -0.0157981, 0.0162986, -0.150747, -0.000805595, 0.00264289, -0.0489348, -0.000397109, -0.0379206, -0.0188333, 0.0323853, 0.018834, 0.0381784, -0.0378373, -0.0729565, -0.0264493, 0.0959068, 0.0070124, 0.0483234, -0.0256878, -0.0645175, 0.00772368, -0.110059, -0.11508, 0.0212394, 0.17413, 0.190082, 0.108448, 0.127538, 0.0225323, -0.00145604, -0.132501, 0.047404, -0.102627, -0.0491561, -0.0133508, -0.0427253, 0.0161803, 0.0334366, -0.0633513, -0.0139696, -0.0424925, 0.00395218, -0.0675218, -0.0114052, -0.0155721, 0.0316872, 0.143676, -0.004043, 0.00611604, -0.166443, -0.0146243, 0.176156, 0.18765, 0.111625, 0.037719, 0.0613108, 0.0444824, -0.0321173, 0.00846678, -0.0470136, -0.00477325, -0.0310488, 0.0356026, 0.0079194, -0.0074321, -0.0439803, 0.0392907, 0.00545458, 0.0299489, 0.0130626, -0.0445256, -0.0156594, -0.090475, 0.0202277, -0.135372, 0.00501836, -0.0227644, -0.154318, 0.114666, 0.0474037, 0.0706311, -0.039999, 0.0222279, 0.192288, 0.0619592, 0.0209621, -0.0184584, 0.0272056, 0.0181725, 0.0457631, 0.0490407, -0.0522692, 0.0553945, -0.00403826, 0.00287764, -0.0547416, -0.131949, 0.0181031, -0.0757464, -0.0969541, -0.0828226, -0.00816897, -0.0317031, -0.00464454, 0.0240436, 0.0198517, 0.0154168, 0.203823, 0.0491095, 0.0672822, -0.103802, 0.0636944, -0.0129093, -0.0262145, -0.00920797, -0.034158, -0.00456095, -0.0266967, -0.0105591, 0.0604078, 0.011732, -0.0322834, 0.0313128, 0.110604, -0.0626343, 0.00537647, 0.05857, -0.0548572, 0.0268233, 0.0131529, -0.0642296, 0.0837181, -0.0041364, 0.101242, 0.0996269, 0.189779, 0.0444023, -0.0629468, 0.0144016, -0.11132, 0.0068794, -0.0424015, 0.0130169, -0.0498229, -0.0248335, -0.0370371, 0.0461011, -0.0320911, -0.0400543, -0.0379349, 0.0452466, -0.0508896, 0.0584016, 0.0208732, -0.0335514, 0.0446986, 0.0699858, 0.120309, -0.0143816, -0.0554853, 0.0728537, 0.057971, 0.0637693, 0.103567, 0.0179607, 0.0950977, -0.0694835, 0.0649876, -0.050164, -0.0379271, -0.0278683, -0.0234157, -0.1037, 0.0741928, -0.00220673, 0.0345305, -0.0936078, -0.0364254, -0.00770242, -0.0535519, -0.059765, 0.0597347, 0.0296849, -0.190186, -0.136341, -0.0350674, 0.000889709, 0.0756093, 0.128099, 0.0320635, 0.0850509, 0.0557424, 0.0165299, -0.0237992, -0.104636, -0.051836, -0.0522331, -0.0622775, -0.0656063, -0.0849784, -0.11761, 0.026009, -0.031545, -0.0830147, -0.02909, -0.0331153, 0.0566083, -0.0639541, -0.00949911, -0.0134698, 0.0378345, -0.0587709, -0.104225, -0.0087627, -0.022703, 0.0499754, 0.090777, -0.0329335, 0.0741642, 0.137253, -0.0495466, -0.0202481, -0.0834106, -0.135453, -0.117341, -0.178208, -0.0168621, 0.0898163, 0.0714817, -0.0795146, 0.00362357, 0.0207534, 0.0755005, -0.0811893, -0.0142701, 0.0296119, 0.0106453, -0.0937615, 0.0762848, -0.0916952, -0.0921361, -0.0859549, -0.052417, 0.0422867, 0.0272902, 0.049171, 0.185406, -0.00783535, -0.114634, -0.0574561, -0.149914, -0.124556, -0.0936028, -0.0218986, 0.0644443, -0.048442, 0.0814664, 0.00980502, -0.0249458, 0.0433465, 0.0674832, -0.0732208, -0.0102269, -0.0939035, -0.0176891, -0.0119583, -0.00146258, -0.159007, -0.0179706, -0.119373, -0.109631, -0.0859422, 0.0292238, -0.00353021, 0.187797, -0.0429719, -0.225825, -0.221463, -0.130144, -0.111563, -0.086325, 0.0149608, -0.0536764, 0.00524143, -0.0220506, 0.0107008, -0.0706277, -0.161273, 0.0161434, 0.0287195, 0.062819, -0.0353785, 0.0116192, 0.0358526, -0.0360373, -0.113509, -0.131684, -0.0902685, 0.0429241, -0.0291133, -0.0643979, 0.122036, 0.216988, -0.0436363, -0.198961, -0.187606, -0.14805, -0.0894164, -0.0284653, -0.0588312, 0.0146719, -0.0419496, -0.0410315, -0.0101278, 0.0273984, -0.0457721, 0.0275025, 0.0197555, 0.0194749, -0.0263515, -0.0785565, 0.0309668, -0.0427325, -0.0399242, 0.0309148, -0.025322, -0.118696, 0.0312891, 0.108136, 0.199638, 0.0335996, -0.0450014, -0.226766, -0.0894925, -0.0631282, -0.0510795, 0.0895905, 0.0295559, 0.0428084, 0.054773, 0.0412745, -0.0226043, -0.0186216, 0.0540261, 0.00632365, 0.0542517, 0.0574047, -0.0996201, 0.0681098, 0.00596622, -0.0213877, 0.0439311, -0.0390778, 0.0466195, 0.0198303, 0.0557765, -0.0443841, -0.00102309, 0.046747, -0.118309, -0.190926, -0.0855654, -0.116303, -0.00471069, 0.0201168, -0.00410955, 0.0819798, -0.0629886, -0.0331796, -0.0443682, -0.0362788, -0.0380838, 0.0347207, -0.0109716, -0.00489303, 0.0365785, -0.111682, 0.00123564, 0.0261089, 0.105777, -0.0436093, 0.00863495, 0.00812731, -0.0437964, -0.0429975, -0.136178, 0.0609345, 0.0228342, -0.133527, -0.149334, -0.0701459, -0.0298326, -0.0302981, 0.0531585, -0.055077, 0.0767333, -0.0201175, -0.0770232, 0.0372836, -0.014296, 0.0290326, 0.0759171, 0.0517124, 0.063948, -0.0812841, 0.0264259, 0.0509126, 0.045545, 0.0377624, -0.0082926, 0.028689, 0.0190561, 0.0170962, 0.0333234, 0.0733544, -0.105221, -0.065303, -0.153902, -0.0896001, -0.0575335, -0.110662, 0.0400553, -0.108978, -0.0247904, -0.0484045, -0.00855169, 0.0975205, 0.0441898, 0.015907, 0.0725083, 0.000656339, -0.076341, 0.0452934, -0.0224725, -0.00913962, 0.0398207, 0.0653134, 0.0414309, 0.112618, -0.0229619, 0.00931754, -0.0681497, 0.0289683, -0.117008, -0.0377593, -0.162932, -0.070536, -0.0468646, -0.0676261, -0.0355269, -0.064389, -0.0202966, -0.0718191, -0.123483, -0.0716329, 0.012205, 0.0562006, 0.00956489, -0.0229767, -0.00609591, -0.0199153, -0.020339, 0.0124549, -0.0499032, 0.0917126, 0.0634175, 0.0808587, -0.0367236, -0.0396865, -0.0151449, -0.0470394, 0.020076, -0.0409953, -0.0837624, -0.0430852, -0.0494177, -0.0673167, -0.0240126, 0.047937, 0.00800935, -0.0349301, 0.0102218, -0.0226125, 0.0322135, -0.0236119, -0.0143587, 0.00621767, 0.0636821, -0.0726913, -0.0254641, 0.0219001, 0.0495246, 0.125679, 0.0363191, 0.0934879, 0.031187, 0.0166458, 0.0331634, 0.0417932, 0.00147645, -0.016083, -0.000311995, -0.0288689, 0.0575937, 0.0565489, 0.0643282, -0.0619948, -0.0312312, 0.0587505, 0.0333989, 0.0462952, -0.0125658, 0.11393, 0.00639484, 0.00130216, 0.0147709, -0.0733314, -0.0903704, -0.00822176, -0.0607676, -0.0696126, 0.00404755, -0.027863, -0.0698904, 0.0616832, 0.0322835, -0.0167665, 0.0432762, 0.0304151, -0.0234393, -0.0803932, 0.00672347, -0.0327777, -0.0670251, -0.0380009, 0.020477, -0.00917647, -0.0542727, 0.0144309, 0.119612, 0.0279883,
0.0627111, 0.0260082, 0.0331965, -0.0851183, -0.0131384, 0.0122278, 0.0387343, -0.0705152, -0.0478199, -0.0320074, 0.0473302, 0.00159819, 0.0580653, -0.0187611, 0.0625566, 0.0581487, -0.0449848, -0.0261452, -0.0136702, -0.0251824, -0.0970016, -0.00498814, 0.00246372, -0.0491622, -0.101118, 0.0260671, 0.0272134, 0.00358446, -0.0364297, -0.00503138, -0.0535544, -0.068562, 0.0184324, -0.0192444, 0.0015759, 0.0135977, 0.117005, 0.0282831, -0.0115281, 0.0187043, 0.0431962, -0.0125915, 0.0305125, 0.0461462, -0.0391705, -0.136707, -0.0322882, 0.0379177, -0.0052326, 0.0248668, 0.0890578, -0.0763294, -0.0111499, 0.0282964, -0.031456, 0.0192129, 0.0528736, -0.0284275, -0.0174679, 0.0690888, -0.0137391, -0.0299471, -0.0118497, 0.002566, 0.02309, 0.0480236, 0.0320355, 0.0627445, 0.021313, 0.0822219, 0.00270609, 0.0323546, 0.0407393, 0.0456258, 0.0116639, 0.0153191, -0.0486968, 0.0269522, 0.0140195, 0.0251966, -0.0727672, 0.0337189, 0.0487086, 0.0648613, -0.00195882, -0.000220496, -0.018675, 0.01936, -0.064857, 0.0538852, -0.0216447, -0.0245857, 0.0168231, -0.00696819, -0.0734286, 0.0170426, -0.0415107, 0.0578346, 0.0651665, -0.0997375, 0.0252979, 0.00965066, -0.0234261, 0.0460145, -0.0374772, -0.136849, -0.00348021, 0.0721241, 0.0362875, -0.0979247, 0.0399935, 0.0226768, 0.0634805, -0.000889505, -0.0708214, -0.0225171, -0.00959241, -0.000589084, -0.0164481, 0.00564437, -0.045885, -0.12084, 0.033067, -0.0628112, 0.053944, 0.00416659, -0.0661557, -0.0026951, -0.00346605, 0.0413248, 0.0419227, -0.0262469, 0.0115757, -0.0182873, 0.0493752, 0.00881169, 0.0642745, -0.0174704, 0.033824, -0.0119427, 0.0140167, 0.0351599, 0.0470813, 0.0320498, -0.0170556, -0.0474426, -0.118573, -0.115868, -0.0618728, -0.0420114, 0.0639275, 0.0836012, 0.14754, -0.0059131, 0.0290263, -0.0529051, -0.109542, -0.0576564, -0.0207243, 0.102234, 0.0871334, -0.0374621, 0.0598585, -0.0557854, 0.0606364, 0.0603133, 0.126933, 0.0447113, 0.0476231, 0.0452814, -0.0255766, 0.0591608, -0.0198307, -0.0813088, -0.0705531, -0.0172459, -0.0681616, 0.0397026, -0.0197586, 0.0201054, 0.143332, 0.140121, -0.00748733, -0.0803998, -0.0443445, -0.10012, -0.0396365, 0.033375, 0.0655578, -0.0590318, 0.176183, 0.0539644, 0.0814856, -0.0751962, -0.0453075, -0.0447407, -0.046126, -0.0566467, -0.0291605, -0.024194, -0.145294, -0.0678277, 0.0176265, -0.0856352, -0.0210697, 0.00588368, 0.0613617, 0.077311, 0.103521, 0.134843, -0.0363998, 0.103519, -0.0423001, -0.0440232, 0.036167, 0.0289561, 0.0357151, -0.00991668, 0.0343166, 0.0700237, 0.0809671, -0.0550756, -0.036421, -0.0950704, -0.063087, 0.0279857, -0.0088067, 0.0697513, -0.0644288, -0.0381203, 0.0314836, -0.0563037, 0.0463745, 0.0762775, 0.00153926, 0.134409, 0.0870099, 0.107034, -0.0408812, -0.0118114, -0.00468993, -0.0342648, -0.0642899, -0.0356227, -0.0495034, 0.0500189, -0.0314611, 0.0654214, -0.0490876, 0.0691597, 0.0500721, 0.0201647, 0.0603959, -0.0174595, -0.0656517, -0.0161238, 0.0508889, -0.0572502, -0.110314, -0.134952, -0.0587351, 0.0671211, 0.0818658, 0.0987984, 0.0119925, -0.0736532, -0.115064, -0.157509, -0.0174226, 0.0244454, 0.0243909, -0.0558482, -0.0388639, -0.0433651, -0.182245, -0.0181995, 0.0112453, 0.0273896, 0.0318908, 0.0408292, -0.0860877, 0.0561884, -0.0409536, 0.0752953, -0.0466065, -0.0284232, -0.0214356, 0.0530985, 0.0233563, 0.182231, 0.0897779, -0.0337098, -0.0232361, -0.176263, -0.178354, -0.151029, -0.207548, -0.0939375, -0.0364951, -0.0900207, -0.0788924, -0.118707, -0.0887571, -0.0490668, -0.0571091, -0.0914905, -0.0461807, -0.00388861, 0.0434137, -0.0166273, 0.101903, -0.136444, -0.0375049, 0.0604199, 0.0987556, 0.0828323, 0.125086, 0.091649, 0.0830314, 0.137055, 0.0866708, -0.087221, -0.223248, -0.181218, -0.262957, -0.138789, -0.0995247, -0.0507983, -0.0435186, 0.0510204, -0.0258773, -0.108238, 0.0130099, 0.00579894, -0.0325256, -0.0147316, 0.0353963, 0.00865543, -0.0628997, -0.0552959, 0.0642298, 0.0707804, 0.155602, 0.0427337, 0.0795806, 0.0360907, 0.0653807, 0.105151, 0.0365081, -0.155118, -0.219449, -0.265062, -0.0845876, 0.137609, -0.00370093, 0.0165114, 0.0452537, 0.00768013, 0.082889, -0.0223888, 0.0181419, 0.0250798, -0.116311, 0.00894129, -0.0245323, -0.0203361, 0.0391111, 0.0158021, 0.0414545, 0.0980783, -0.0209893, -0.0377766, 0.0370452, 0.095656, 0.111569, 0.0971047, 0.00780478, -0.0256715, -0.155104, 0.0785308, 0.0728663, 0.0316559, 0.117254, 0.1723, 0.00377276, 0.0511585, 0.0813022, 0.022759, -0.0522287, 0.0198859, 0.0763562, 0.113948, -0.0172, 0.0953432, 0.0161772, -0.0119296, 0.0788114, 0.0363154, -0.0204946, 0.129652, 0.0936038, -0.0780532, 0.093525, 0.043629, 0.00742991, -0.0569442, 0.0636494, 0.0308332, 0.0678706, 0.139482, 0.0690454, -0.00337446, -0.0991733, 0.0102508, 0.05886, -0.0821959, -0.056033, -0.01535, -0.0136346, -0.0082825, -0.0103042, 0.00655479, -0.0355655, -0.0131386, 0.0460655, 0.00702407, -0.0495116, -0.0274206, -0.0112956, -0.0392113, 0.0964353, 0.0725327, 0.0319894, 0.103288, 0.0594874, 0.0233755, 0.0109119, 0.0566777, -0.0210731, -0.10352, -0.0984981, -0.13121, -0.0455045, 0.0233948, -0.00392805, 0.0363073, -0.0210089, -0.0558943, 0.0360856, 0.00385028, -0.0177926, 0.0641813, 0.00540496, 0.00943908, -0.0580487, -0.0235446, 0.0294197, -0.0781466, 0.0610175, 0.179719, 0.179062, 0.116835, 0.190509, 0.123749, 0.112574, 0.0798837, -0.0424532, -0.0372281, -0.0690021, -0.0926927, -0.117903, -0.127395, -0.0512019, 0.0107959, -0.0536503, 0.00144104, 0.00280793, 0.112405, -0.0545776, -0.0386193, -0.0846823, -0.018548, -0.0701564, -0.0641108, -0.0553288, 0.0117473, 0.0951494, 0.12328, 0.323748, 0.208538, 0.00248789, 0.00422321, 0.00696442, -0.00412065, 0.0343847, -0.125325, -0.0450717, -0.136274, -0.0498653, -0.0394675, -0.0373773, -0.0691542, -0.0161076, -0.117944, -0.0121366, -0.0337368, -0.0598208, 0.0430447, 0.0133363, -0.0479004, -0.130786, 0.0285362, -0.099713, -0.00452159, 0.154892, 0.166967, 0.246366, 0.136765, 0.133206, 0.0235839, -0.101727, -0.0889273, -0.0746467, -0.0179653, -0.00819018, 0.00435268, -0.0207253, -0.0111094, -0.0510762, -0.0290329, 0.0142377, 0.0306174, -0.0139572, -0.0110629, 0.0568352, -0.0148227, -0.0543682, -0.164554, -0.0828485, -0.014534, -0.0260335, 0.0330916, 0.0250191, 0.0572993, 0.100023, 0.0526307, 0.0308422, -0.0520188, -0.0225439, -0.0460456, 0.0100311, 0.0387029, -0.00143437, -0.00644098, 0.0562524, 0.0645767, -0.00784811, 0.0385901, 0.0305732, 0.0885529, 0.00878391, -0.0172946, -0.0157582, -0.0161033, -0.00928771, -0.124971, -0.132131, -0.100648, -0.0468685, 0.0330292, 0.0991284, -0.0663082, 0.133818, -0.0229387, -0.0259455, 0.13617, 0.0267878, -0.0595275, 0.0152971, 0.0617773, 0.0653851, 0.0140657, -0.0310874, 0.0617324, -0.0288472, -0.050348, 0.0436206, -0.00817576, -0.0706593, -0.00706986, -0.0425073, 0.0389958, 0.000437338, -0.060998, -0.0442708, -0.00944775, 0.00665428, 0.187902, 0.0245969, 0.0919382, 0.0538737, 0.0506465, 0.0394061, 0.0655373, 0.0227138, 0.0841334, 0.0517354, 0.0380845, 0.0674238, 0.0841152, 0.0853774, 0.0677948, -0.036368, -0.0228133, -0.00624302, -0.0472048, -0.00578007, 0.012909, -0.0149473, 0.00176306, 0.0806187, 0.0654562, -0.00786292, 0.0326382, 0.0342793, 0.0753694, 0.0361832, 0.0897943, 0.049982, 0.072226, 0.0892166, 0.125586, 0.0216357, 0.00506384, -0.0480811, -0.0803963, 0.0933685, 0.0336068, 0.100438, 0.128388, -0.0838205, 0.0223908, 0.0109426, 0.00564408, -0.0220297, -0.0263347, 0.0595981, -0.0273894, 0.000106191, 0.0137443, -0.182618, -0.0956407, -0.0499466, -0.0113221, 0.011969, 0.0112709, 0.0843141, 0.140734, 0.0129791, 0.063276, 0.0228632, -0.0250594, -0.0156954, 0.140447, 0.125051, 0.023923, -0.0593992, -0.0084319, -0.0601929, 0.0291615, -0.0537038, -0.0520093, 0.0442942, 0.0278737, 0.025541, -0.060146, -0.084137, -0.0113355, -0.0449728, 0.0120428, -0.140529, -0.0283791, -0.0665076, 0.0116759, 0.123425, 0.134471, 0.076361, 0.0739618, 0.0196417, 0.011787, 0.125062, -0.0248406, 0.0396848, 0.00880134, 0.0515158, 0.0588356, -0.0347728, 0.048836, -0.0083482, 0.00680859, 0.0692604, 0.0494723, -0.0548103, 0.0539028, 0.0305251, -0.0249729, -0.054189, -0.00978565, -0.0233098, -0.0775386, -0.0709712, 0.0491173, -0.0707228, 0.0520837, 0.0232758, 0.0505007, -0.10008, -0.0496036, -0.0594186, 0.123789, 0.0351939, -0.00712503, -0.0334695, 0.052857, 0.0353122, 0.082481, 0.0687459, -0.017698, 0.0451526, -0.106928, 0.0805025, -0.0198567, -0.102852, 0.0722901, -0.00647225, -0.0679427, -0.0557777, -0.0456802, -0.150663, -0.00389492, -0.0450345, -0.0533762, -0.0391816, 0.0684045, -0.0463672, -0.00176505, -0.0152355, -0.0189291, 0.02068, 0.0583625, 0.0188917, 0.00382551, -0.0410435, 0.0155073, -0.018013, -0.0250723, 0.00396791, -0.02, 0.0118253, -0.0921907, 0.0605866, -0.0978004, 0.0466116, 0.0582585, -0.0269434, 0.0668013, 0.0735849, -0.057019, 0.0298963, 0.0383452, -0.00294231, -0.00783797, 0.0175066, -0.0248786, 0.0248629, 0.000881197, -0.0518563, -0.0541655, 0.00336248, 0.0215832, -0.0546306, 0.0783218, -0.112576, 0.008336,
0.0665986, 0.0405593, -0.0356339, -0.00275243, 0.0468254, -0.0183505, -0.0326735, -0.036478, -0.01526, 0.00999762, 0.0524026, 0.0402114, 0.0478157, -0.000700648, 0.0263458, 0.0337466, 0.0263037, -0.00392334, 0.0217776, 0.0426171, -0.0819058, 0.0188525, -0.0256535, 0.0143326, -0.00669383, -2.88352e-05, 0.0224613, 0.0473175, 0.0505864, -0.0623904, 0.0160139, 0.0851764, 0.0184063, 0.0333309, -0.00643874, 0.0835574, -0.0805365, 0.0259089, -0.046038, -0.109901, -0.0337325, -0.00215681, 0.0488628, -0.00633359, 0.0373029, -0.00258451, 0.0167145, -0.094629, 0.0574803, -0.0113822, 0.0121657, 0.00584529, 0.0553585, 0.0778084, 0.0582944, -0.00949732, -0.00471482, -0.111313, 0.0270588, -0.0337521, 0.0155471, 0.0249661, 0.047657, -0.0459851, -0.0180162, -0.07016, -0.00686916, -0.00206654, -0.0549065, 0.0155333, 0.0123838, 0.0673142, 0.0441507, 0.016542, 0.0259321, 0.0308198, -0.0279118, -0.0385634, 0.0120575, 0.0264531, 0.0359258, 0.0108581, -0.0579778, -0.00150733, 0.0320539, 0.0359953, -0.038561, -0.0409072, -0.0494664, -0.0288435, -0.0309725, -0.013186, 0.0325258, -0.0719861, -0.0723242, -0.0821056, 0.0493956, 0.0322068, 0.129887, 0.0245897, 0.0948136, 0.0825023, -0.0143617, 0.0475197, -0.0473438, 0.03288, 0.0163513, 0.0201675, 0.0255001, 0.057897, -0.0569296, -0.0186956, -0.0205031, 0.0736981, 0.0471775, -0.0168872, -0.0481189, 0.0975032, -0.080958, 0.0743793, 0.0635092, 0.125617, 0.0620869, 0.123868, 0.181601, 0.143542, 0.144207, 0.175846, 0.0688959, 0.070393, 0.166901, 0.064634, 0.135771, -0.048842, -0.0661663, -0.0458841, 0.048142, 0.0192023, -0.105719, 0.0505584, 0.102492, 0.0455495, -0.0347626, -0.0670128, -0.00121469, 0.0150401, 0.0313194, 0.102979, -0.0945748, -0.147603, -0.0505865, -0.0676818, -0.0612967, -0.0607847, -0.0809855, 0.0442544, 0.0383079, 0.0259677, 0.0565467, 0.219294, 0.155132, 0.0424959, -0.00845844, 0.0915921, 0.0751638, -0.0416742, 0.0328656, 0.0787874, 0.0352566, 0.00667866, -0.0602482, -0.0804726, -0.0421075, -0.115776, -0.0690045, -0.0729128, -0.0446994, -0.0185306, -0.0488797, -0.0835046, 0.0191353, -0.0487671, -0.13921, 0.0399543, 0.0164617, -0.0397829, 0.0418369, 0.0351422, -0.0191074, -0.053053, 0.00158559, 0.0123837, -0.0412009, -0.0113734, 0.053094, 0.0314352, -0.0134845, -0.0625964, -0.0488009, -0.0168463, 0.0659551, 0.00122601, -0.0374882, -0.114794, -0.0149075, 0.0357565, -0.0831908, -0.0565439, -0.0444205, -0.105396, -0.0845164, -0.0338345, 0.0760636, -0.0303694, 0.0298058, 0.0522216, -0.0636286, -0.0755947, -0.103385, -0.0774741, 0.0411317, -0.017708, -0.0362465, 0.0250231, -0.0712822, 0.0593367, -0.000518942, 0.0261578, 0.0315072, -0.00574191, -0.11713, 0.0135668, 0.0419503, -0.0233366, 0.00456156, -0.0883698, -0.143036, -0.0813676, -0.0604037, -0.0558713, 0.0762748, 0.131548, 0.0757392, 0.0619605, 0.0844367, -0.104309, -0.0189728, 0.0193674, -0.0524082, -0.0645667, -0.0272245, 0.0803852, -0.0680811, -0.0535186, -0.0180449, 0.0205676, 0.00411996, -0.01972, 0.0444288, -0.0514376, 0.10121, 0.0958304, -0.0115845, -0.0700147, -0.0298019, -0.0177387, -0.186977, -0.0950439, -0.10684, 0.0515109, -0.00884105, 0.198223, 0.167371, 0.0274264, 0.0260657, 0.00839385, -0.156148, 0.00655929, 0.012436, 0.00994721, -0.00915122, 0.0160056, -0.070204, -0.0494339, 0.0597714, -0.0106336, -0.0853154, 0.0517224, 0.0648411, -0.00451192, 0.0387861, 0.0704117, -0.0738289, -0.151672, 0.0092259, -0.100845, -0.00354472, 0.0600991, 0.0440219, 0.219071, 0.09254, 0.0259695, -0.0090133, -0.0601569, -0.141809, -0.127135, 0.00510007, 0.00735377, 0.0185592, -0.103604, 0.0215714, -0.00854775, 0.0627947, -0.0940289, -0.0348513, -0.0659424, -0.0850125, 0.0419816, 0.0677678, -0.0528435, -0.0108147, -0.0973817, -0.0606013, 0.0845078, 0.247039, 0.182897, 0.198292, 0.148568, 0.140153, -0.108775, -0.123418, -0.0438354, -0.00567285, -0.0621223, 0.00455721, 0.0501673, -0.00249859, -0.067674, -0.0320626, -0.034454, 0.0256295, 0.0798347, 0.00106162, 0.0149277, -0.135244, -0.0588485, 0.00505048, 0.0612041, 0.0765096, 0.0306782, 0.162725, 0.113885, 0.147285, 0.0811748, 0.0638643, 0.0120671, -0.0798099, -0.237389, -0.175709, -0.0719901, -0.0375758, 0.063299, 0.0617027, -0.0218468, 0.0291909, 0.0205081, 0.0177711, -0.0802658, -0.0429518, -0.00865479, 0.00638172, -0.018522, -0.0176377, 0.0120004, -0.162778, -0.0941083, -0.0658345, 0.100638, 0.173051, 0.235584, 0.0189057, 0.145158, -0.0531177, -0.143169, -0.109382, -0.100085, -0.146683, 0.000533479, -0.0247105, -0.0882019, -0.0586791, 0.0160297, -0.00252357, -0.0294782, 0.0715474, -0.0425981, -0.137112, -0.0740729, -0.0312924, -0.0175948, -0.00128059, -0.0117771, -0.127558, 0.0409572, 0.058492, 0.0394129, 0.315972, 0.158607, 0.037297, -0.005988, -0.103017, -0.103668, -0.060033, -0.0837809, -0.0746534, 0.0151057, -0.0254525, -0.00135406, 0.0929955, -0.101753, 0.104867, 0.0515371, 0.0377302, 0.046182, 0.0705882, -0.0296538, -0.0175427, -0.0070069, 0.0539771, 0.0122153, 0.0538656, 0.0491678, -0.0106223, 0.0498983, 0.0553839, 0.00555289, 0.0384278, -0.20956, -0.187677, -0.0485097, -0.110975, -0.03124, 0.0218303, -0.120563, -0.0605923, 0.0815768, 0.0834018, 0.0479423, 0.00314543, -0.0587567, -0.0430388, 0.0518054, -0.0497816, -0.000664823, -0.0372341, -0.0296032, -0.0318331, 0.10936, 0.0950482, -0.000544599, -0.0433036, -0.0182937, -0.042766, -0.0499826, -0.0637337, -0.077274, -0.0850483, -0.0854435, -0.106639, 0.0154883, 0.104135, 0.0119593, 0.0789289, -0.0185296, 0.0150877, 0.0768398, 0.0325035, -0.0959346, -0.0132549, -0.0168369, 0.0343964, 0.061954, 0.0661735, 0.0482853, 0.02093, -0.0111495, -0.0255702, 0.00269077, -0.163224, -0.0696507, -0.166532, -0.10356, 0.0102405, 0.0410401, -0.0424904, 0.0293617, -0.0808589, -0.0574153, 0.0783905, 0.00878905, 0.0250922, -0.0455759, 0.00536499, 0.0163781, 0.0132801, -0.0268331, 0.00528934, 0.0567654, 0.0451263, 0.00954904, 0.0760235, 0.148827, -0.0760184, 0.0681576, -0.0764709, -0.137317, -0.131947, 0.00144233, -0.0500645, -0.00762563, 0.105802, 0.03894, -0.0386783, 0.0102451, -0.0869667, -0.0840476, -0.0431681, -0.0287094, 0.083221, 0.00529362, 0.0621696, -0.0242554, -0.0623648, -0.0588384, 0.0645944, 0.0041892, 0.0242339, 0.0176894, -0.00276873, -0.023443, -0.0270267, 0.00322109, -0.0930219, -0.170855, -0.147662, -0.0684423, 0.0756155, 0.149931, 0.183415, 0.0292276, -0.056242, 0.0908123, -0.000616047, 0.00557119, 0.0257546, 0.0897322, 0.0530315, 0.023253, -0.0501492, 0.0062459, 0.0197559, 0.0623966, 0.0419145, -0.0050417, 0.0589123, 0.0532986, 0.0425462, 0.107947, 0.0524846, -0.082573, -0.11256, -0.176971, -0.0559865, 0.0382021, 0.064261, 0.0738226, 0.109077, 0.0354115, 0.0853157, 0.0658316, 0.0448775, 0.132411, 0.00627735, 0.0763366, 0.0640626, 0.0148252, -0.0436516, -0.00789475, 0.00068628, 0.00778788, -0.042619, -0.0168988, 0.0247634, 0.0135892, 0.102014, 0.164498, 0.0244479, -0.0340376, -0.0256126, -0.142578, -0.0195604, 0.0453084, -0.0634793, 0.0225201, -0.0485339, -0.0898153, 0.0186764, 0.0238346, 0.0556098, 0.057772, 0.0141523, -0.0041543, -0.0361157, -0.0104304, -0.0295651, -0.0226106, -0.111211, 0.0768115, 0.078336, -0.10147, -0.0454711, 0.0529531, -0.0403908, 0.143171, -0.0595794, 0.0470061, -0.0501406, 0.0175196, 0.00480613, 0.0476662, 0.0271099, 0.00628638, -0.0179272, -0.0375234, -0.0920648, 0.0403333, 0.0743774, -0.00406592, 0.026807, -0.0368016, -0.0760248, -0.0102945, -0.0321284, 0.0135995, 0.0405207, -0.0783132, -0.0155915, 0.0730013, 0.0131671, -0.149984, 0.0659261, 0.0231039, -0.00184495, 0.092573, 0.118692, 0.0945826, -0.0464135, 0.119591, 0.0622822, 0.0194642, -0.0563898, 0.0660384, 0.142661, 0.0802243, 0.00545966, -0.0409104, -0.0281296, -0.148677, 0.0352542, -0.163135, -0.0475868, 0.04303, 0.0106484, -0.0509442, 0.0120325, 0.0854656, -0.00736992, -0.0844045, -0.0170081, 0.0586881, 0.0308351, 0.102295, -0.00155436, 0.0302545, -0.0617702, -0.00235235, -0.0310523, 0.0304497, -0.0214945, 0.0909158, 0.00874152, 0.0848763, -0.0597666, -0.0362582, -0.148129, 0.0132232, 0.0404815, 0.0060801, -0.0568313, 0.00265416, -0.0313663, 0.00530966, -0.0384594, -0.0424965, -0.074545, -0.0292053, -0.0271432, 0.0336824, 0.0464011, -0.0614019, -0.0107616, -0.0694383, -0.00890883, -0.0411561, -0.062769, -0.0405195, -0.0433136, -0.0949015, -0.0101997, 0.00751172, 0.0641082, 0.043226, 0.0360293, 0.0238293, -0.0345489, -0.00253008, -0.0526934, 0.0893332, -0.0547363, 0.00771243, 0.0564114, -0.0318806, 0.0443225, -0.0376537, -0.067537, 0.128908, 0.101368, -0.0101317, -0.00815623, 0.0467041, -0.0683789, -0.0563798, -0.00195811, 0.0183781, -0.0016882, -0.0134909, -0.0201599, -0.0654659, -0.0216032, -0.035764, -0.0357345, 0.0780001, 0.0641732, -0.00223156, -0.0600625, 0.0940719, -0.0816502, 0.0404256, 0.0543625, 0.0254128, -0.0817588, 0.0569366, -0.0723778, -0.0250931, 0.0104566, 0.0254713, -0.0582036, -0.0820564, 0.0202343, -0.0647928, 0.036996, 0.0270061, 0.00582013, 0.0338485, -0.00782605, 0.0332015, -0.126989, 0.0723212, 0.049313, -0.0128064, 0.0404224, -0.00781421, -0.0848703, 0.0197084,
-0.0604591, -0.0377405, -0.00253991, -0.027004, -0.0514481, 0.119749, 0.020878, -0.0214647, 0.000632422, -0.129135, -0.0219097, -0.022492, 0.0365281, -0.0371747, -0.046944, -0.0070408, -0.0363317, 0.0627627, -0.00504162, -0.0172316, -0.0246667, -0.0299894, -0.0729943, 0.0360137, -0.0417304, 0.0539263, -0.0124542, 0.0668665, 0.0514371, 0.0897266, -0.0342041, 0.0219312, 0.0154102, 0.0372662, 0.081277, -0.0809516, -0.124604, 0.0322092, 0.0594912, 0.038545, 0.0530955, -0.0377309, 0.0516313, -0.00466466, -0.0302425, 0.0854374, -0.0329698, 0.037428, -0.0576066, 0.0034259, -0.0574195, 0.0500325, 0.0617359, -0.0208912, -0.00775085, -0.0357633, 0.000131711, -0.0695916, -0.0107281, -0.0175972, -0.0520471, -0.0258626, -0.056926, 0.0456881, 0.140749, 0.11432, 0.0482225, 0.0456819, 0.0035298, 0.00602224, -0.0383712, 0.00474174, -0.0705925, 0.0148726, 0.0556173, -0.0541267, -0.0133022, 0.00295974, 0.0203383, -0.00530981, 0.064558, -0.018212, 0.024857, -0.0733254, -0.0322675, -0.0191893, -0.0465335, 0.0270706, 0.0720621, 0.0277768, -0.0427688, 0.0293635, -0.0510156, -0.0058624, -0.0133158, 0.101882, 0.0266713, 0.036632, -0.03195, 0.0342827, -0.0821711, -0.0432172, -0.116116, 0.00462199, -0.046905, -0.0490799, 4.06504e-05, -0.0194485, 0.0220114, -0.00462611, 0.0128146, 0.0631992, 0.0630282, -0.0413089, -0.0332087, 0.0123017, -0.0488333, -0.0411115, 0.105028, 0.088975, 0.121301, 0.00941759, 0.0211702, 0.0602189, 0.0759208, -0.0168994, 0.00131739, -0.0165269, -0.075254, -0.179216, -0.0904965, -0.176022, -0.168617, -0.0735966, -0.00420627, 0.0109854, -0.0786804, 0.0520724, -0.0691338, -0.0131786, -0.0746291, 0.00661298, 0.0504421, -0.0249068, 0.0522825, 0.104835, 0.214754, 0.0329857, 0.0309211, -0.0152834, 0.0329478, 0.0357273, 0.0329639, 0.111219, 0.0822253, 0.188875, 0.0300413, -0.0554901, -0.171722, -0.141812, -0.206279, -0.0647094, -0.0705522, -0.0843482, 0.0780933, -0.0690604, 0.020421, -0.0582781, -0.0469834, 0.0182174, 0.0300985, 0.0357605, 0.011133, 0.0563541, 0.0411325, -0.0459087, -0.0797981, -0.0441999, 0.0125824, -0.0752026, 0.0363624, -0.0652996, 0.0654463, 0.173655, 0.0852119, -0.03174, -0.107549, -0.254423, -0.194067, -0.0983526, -0.0342516, -0.0465634, -0.00669162, -0.0879553, -0.0218175, 0.0397933, 0.0613845, 0.0107538, 0.0222088, 0.00628979, 0.0732096, -0.014991, 0.0683957, -0.100556, -0.0681591, 0.0537712, -0.173277, -0.0460867, -0.111684, 0.0420723, 0.0917811, 0.139229, 0.231637, -0.126084, -0.178093, -0.221222, -0.276078, -0.265437, -0.0366389, -0.120053, -0.0591188, -0.0141477, 0.0112039, 0.0365245, 0.0655869, 0.0548366, 0.00516417, -0.00915793, 0.0253252, -0.0573723, -0.124235, -0.0117261, -0.0787444, -0.103969, 0.101703, -0.103297, -0.0890223, 0.0151174, 0.242718, 0.292368, 0.0139912, -0.0342069, -0.212863, -0.219456, -0.273905, -0.2462, -0.117534, -0.082395, -0.0579987, -0.0668312, 0.0114146, -0.00999664, 0.013951, -0.188866, 0.00311512, 0.0369669, 0.0395661, 0.0679158, 0.0681085, 0.0375651, -0.0419471, -0.118737, -0.0882532, -0.032788, 0.0541927, 0.156035, 0.293814, 0.352577, 0.0625356, -0.101012, -0.271443, -0.182094, -0.20795, -0.131261, -0.0712316, -0.00585877, -0.0890392, -0.0420544, -0.00495733, 0.0536068, -0.000356679, 0.0114586, 0.0750241, -0.0247808, 0.0446957, -0.0160386, 0.0763889, -0.0159026, -0.128195, -0.0527255, -0.0398803, 0.138038, 0.0907775, 0.180289, 0.26607, 0.241075, 0.0940556, -0.146732, -0.268884, -0.269706, -0.0869742, -0.13653, 0.0188878, -0.0372249, 0.0296155, 0.0347766, 0.0713684, 0.0935824, -0.0697069, -0.0112572, 0.0447216, -0.05558, -0.0241455, 0.0262819, -0.0653151, -0.0372653, -0.104775, -0.0475547, 0.181574, 0.142387, 0.093661, 0.20682, 0.188743, 0.136167, -0.135262, -0.182346, -0.264991, -0.121338, -0.0808087, -0.0557957, 0.0277972, 0.0630479, -0.0578657, -0.100945, -0.0434808, 0.0356077, -0.0298426, -0.0163676, -0.0174955, -0.00531089, -0.0538203, -0.0876021, -0.0186849, -0.00608418, -0.0328842, 0.0674339, 0.086722, 0.0889426, 0.074146, 0.153385, 0.139002, 0.0558632, -0.111606, -0.191123, -0.0109408, 0.00566847, -0.125761, -0.0880267, -0.00665362, 0.0303229, 0.06998, 0.028882, -0.0616489, 0.0152658, 0.0130064, -0.0963471, -0.067772, 0.0179612, -0.0330371, -0.0933701, 0.00649832, -0.102317, -0.0446468, -0.0100724, 0.0866606, 0.0382193, 0.047388, 0.103841, 0.130127, 0.0236141, 0.131597, 0.0348239, 0.0415937, -0.0318979, -0.101697, -0.0537681, 0.0811216, 0.0551599, 0.128926, 0.073004, -0.0425975, -0.0781572, 0.0337248, -0.043348, 0.0119014, 0.0776823, -0.0876177, -0.0828011, -0.0607847, -0.132279, 0.0460124, 0.0653584, 0.0873345, -0.0645458, 0.0509344, 0.00592508, 0.0153881, 0.0709975, 0.0193644, 0.0534014, 0.0102325, 0.000113638, -0.0586682, 0.0545872, 0.216588, 0.00932348, -0.00129785, -0.0291853, 0.0629399, -0.0111036, 0.0505323, 0.0118458, -0.0755729, 0.0130303, -0.0345478, -0.0415935, -0.0641968, -0.0922851, -0.0355871, 0.0936619, -0.0304306, 0.0572844, 0.0582582, 0.137499, 0.0751601, 0.0446614, -0.0310772, 0.0388106, -0.141544, -0.103368, 0.0777486, 0.076228, 0.149705, 0.0749717, 0.0476353, -0.0528683, 0.0145019, -0.0330993, -0.0108394, -0.0801574, 0.02248, -0.0688158, 0.0289604, -0.0316878, -0.0498786, 0.0510966, 0.0539508, 0.00351242, 0.105012, -0.0149836, 0.120308, 0.00660718, 0.0709649, 0.00179256, 0.086578, -0.12152, -0.0456762, 0.0487046, 0.0948222, -0.0562149, 0.0805379, -0.00346868, 0.000815223, 0.0388998, 0.00978576, 0.0799115, 0.00205776, -0.0219641, 0.0221292, -0.0229572, -0.00831061, -0.044354, -0.0473726, 0.0263009, 0.0513624, 0.0907575, 0.0119401, -0.0998214, 0.021847, 0.0954603, 0.0142915, -0.0639505, 0.0415762, -0.0101263, -0.0352889, 0.0208505, -0.0902987, 0.0647625, 0.0442168, 0.0736323, 0.00855689, -0.00559727, 0.0659833, -0.0625821, 0.038176, -0.00515602, -0.0399884, -0.0796156, -0.0405144, 0.0307689, -0.0602312, -0.101334, -0.167249, -0.0592456, 0.0403422, -0.103118, -0.109395, 0.0156909, -0.0956052, -0.103965, -0.0479618, 0.0227674, 0.0484583, 0.0211181, 0.0736533, -0.0162907, 0.121399, 0.0167315, 0.00442628, 0.0240492, -0.00404092, 0.0273811, 0.0605784, 0.0164682, 0.0915271, -0.0160239, -0.0348516, -0.0294536, -0.0533292, -0.073281, -0.0206717, 0.130911, 0.0959554, -0.126199, -0.066266, -0.0476761, 0.0220632, -0.0286943, -0.0511925, 0.0446525, 0.0427131, 0.0206602, -0.0285452, -0.0388097, 0.0137548, -0.00379804, 0.0042014, 0.0202718, 0.0102916, 0.0124115, -0.00223571, -0.0402614, 0.0182284, 0.0192225, -0.0253651, -0.092257, -0.0525812, -0.0478738, -0.0133617, 0.011958, 0.0303469, 0.052716, 0.0503976, -0.0159502, 0.0328457, -0.0385989, 0.0831217, 0.0261499, 0.0506116, 0.0845449, -0.00171674, 0.0838771, 0.00251196, -0.0182333, -0.0105433, -0.0405678, 0.0272948, -0.0931923, -0.0625665, 0.0117236, 0.0442658, -0.0554949, -0.077681, -0.00600068, -0.101392, -0.0813419, -0.086286, -0.0686092, -0.0368225, -0.0359169, 0.0146053, -0.121943, -0.098238, -0.038291, -0.0738671, -0.135948, 0.0157501, -0.0489514, 0.0268355, -0.017906, -0.0716852, -0.0127087, -0.0167745, 0.00104512, 0.059826, -0.0500437, 0.0136532, -0.0326675, 0.0270564, -0.0965294, 0.000434684, 0.00663714, -0.0466327, -0.0345157, 0.00784325, 0.0940291, 0.0452094, 0.010508, -0.0319612, -0.0191794, -0.0036664, -0.0137626, -0.123598, -0.0905065, 0.0109786, 0.0207881, -0.0220225, -0.0407465, -0.0295826, 0.0196007, -0.0489304, 0.0354619, 0.031664, -0.0776625, 0.0561519, 0.0419662, 0.0244731, -0.0228591, 0.104432, -0.0729152, -0.0234064, -0.0604362, 0.0126061, 0.0500877, 0.124697, 0.026709, 0.0408648, 0.029161, -0.0783216, -0.0685248, -0.0389463, -0.0426456, -0.0667242, 0.0457622, 0.0482733, -0.000231765, -0.0212763, -0.0900037, -0.113326, -0.0379007, -0.0457995, -0.0665734, -0.0444857, -0.0185831, -0.00231558, -0.0318961, 0.0749288, 0.100397, -0.0112455, -0.117304, -0.0171986, 0.0235198, 0.00123651, 0.0182733, 0.0377138, -0.00316344, 0.0222807, -0.0604765, -0.126107, 0.0213815, -0.115722, 0.0311407, 0.0507886, 0.0627341, -0.00450627, -0.0390421, -0.00532375, -0.0619619, -0.00182941, 0.0174682, -0.0794633, 0.0149637, -0.0346371, 0.00749656, 0.0572626, 0.0290027, 0.010938, -0.00948347, 0.0203605, 0.0204387, 0.0949611, -0.0787373, -0.0394524, -0.0331727, 0.0677283, -0.0657777, -0.0139371, 0.00388662, -0.00050367, -0.0342833, 0.0806778, -0.0361341, -0.0502014, -0.0295364, 0.0580961, -0.0109398, -0.0378872, -0.0804683, -0.0607508, -0.0639869, 0.0570602, 0.0183207, 0.0406601, 0.0222355, 0.0518705, -0.0442474, 0.0308796, 0.0347342, 0.0602499, -0.0349601, -0.0636986, 0.0830899, -0.00205399, 0.0142932, -0.0737324, -0.0396838, 0.0575115, -0.0350806, 0.0256623, 0.0624065, 0.0255401, 0.120697, 0.0122387, 0.0638203, 0.0445568, -0.0144233, 0.0743878, -0.0519726, 0.00725236, -0.0777547, -0.0151374, -0.0737277, -0.00546918, -0.06022, -0.0101027, -0.063055, 0.00292984, -0.0808615, -0.0123012, 0.0481166, -0.01036, -0.0218974, -0.0594943, -0.0175291, -0.0109294, 0.00362181, -0.0181441, -0.0960076, -0.0120641, -0.144767, -0.0413334, 0.00369077, -0.0166895, -0.127387,
0.0141437, -0.0292874, 0.000400311, 0.0671244, 0.0363069, -0.0129714, -0.00806528, -0.00929206, -0.044589, 0.0328406, -0.0300597, 0.041234, 0.0237966, -0.0462032, 0.0342572, 0.0705124, 0.0307984, 0.0653225, 0.0280069, 0.0676061, -0.0113573, -0.0955476, 0.0519347, -0.0131978, -0.0684169, 0.12017, 0.022942, 0.141142, 0.0425727, 0.022975, -0.0553248, -0.0326177, -0.0371644, 0.0319805, 0.0803995, 0.0112805, -0.016929, -0.0409328, -0.0138942, -0.029982, 0.0411748, -0.0340132, -0.0658815, -0.0391179, -0.0524395, 0.012515, -0.020352, 0.0612535, -0.0304031, 0.00977315, -0.0403618, -0.00161055, -0.0168916, -0.00640139, 0.0960369, 0.097856, -0.00863029, 0.0559487, 0.00872137, -0.0572083, -0.140712, -0.0580394, -0.0653693, -0.0134976, 0.129418, -0.0131432, -0.0444424, 0.00674039, -0.0182739, -0.0481982, -0.0183969, -0.0317227, 0.0400028, 0.0134359, -0.00584454, -0.0090873, 0.0706708, -0.0732039, -0.0132204, 0.0720901, 0.0802514, -0.0879899, 0.0586004, -0.0704323, 0.0616755, 0.0364777, 0.00793364, 0.0537893, -0.034421, 0.000994829, 0.0555569, 0.0303695, 0.0517228, 0.189247, 0.0814735, 0.134452, 0.0872831, 0.0907155, -2.10782e-05, -0.00580855, 0.104454, 0.00941103, -0.0654471, -0.122865, -0.0564429, -0.0401642, -0.0516342, 0.0775966, -0.0032016, 0.0483062, -0.00445606, 0.0523812, -0.055976, 0.0511687, -0.0111192, 0.0677522, 0.00262852, -0.0994733, 0.0333975, 0.0249572, 0.0446603, 0.0909496, 0.0874821, 0.0971497, 0.0429494, 0.0255222, 0.121544, 0.167833, -0.0153324, 0.0547464, 0.0327526, -0.0204442, -0.0314815, 0.00165748, -0.0659307, -0.0584814, -0.0210145, -0.00417917, -0.0935725, 0.0311203, -0.0224041, -0.0787004, -0.00450536, 0.0323791, -0.0231876, 0.00552306, 0.0576883, -0.0283209, 0.049224, 0.0200152, 0.0770424, 0.130353, 0.0589995, 0.176425, 0.102839, 0.0820807, 0.0927648, 0.0221532, 0.061014, 0.00997297, 0.0206425, -0.012608, 0.00630777, -0.0510121, 0.0587781, 0.0477774, 0.0131512, 0.0583967, -0.0116042, 0.0983264, 0.0122863, 0.0164487, 0.0371609, 0.0233869, 0.00485245, -0.0537945, -0.0128596, 0.057, 0.0718152, 0.115575, 0.0990212, 0.243074, 0.162277, 0.194967, 0.0561678, 0.120246, 0.0709517, -0.0106472, 0.000272696, 0.0489973, 0.00823648, -0.0125491, -0.090808, 0.0335021, -0.0303364, 0.00977167, 0.0251299, -0.114175, 0.00608336, 0.135668, 0.0917951, 0.0408769, -0.00088731, 0.024772, 0.0569958, 0.133179, -0.0154848, 0.11982, 0.0772909, 0.18846, 0.0993056, 0.164666, 0.176114, -0.0619342, 0.00225243, -0.0301495, 0.0223424, -0.0915427, -0.0260667, -0.0349503, -0.037599, 0.0283085, -0.0111669, -0.002669, -0.0293783, 0.0597634, 0.00524369, -0.0156851, 0.00418365, 0.0485345, 0.00821687, -0.0798009, 0.142609, 0.0830264, 0.0805766, 0.0592983, 0.0572403, 0.0316823, 0.163743, 0.0427643, 0.0440188, 0.0194869, 0.00256415, 0.0854209, -0.0767004, -0.000186255, 0.0169045, -0.0750752, -0.224593, -0.0360034, 0.05207, 0.00702528, -0.00993389, 0.0320835, 0.00569383, 0.00990592, 0.0520566, 0.193796, 0.0160592, -0.0302605, 0.146049, 0.036472, 0.0663087, -0.0960233, 0.0481042, -0.0244105, 0.073152, -0.0659206, -0.0204043, 0.00407565, -0.0705335, -0.0300784, -0.00413839, -0.0384999, 0.0530248, -0.11804, -0.128248, 0.0123512, 0.132729, 0.0622129, -0.06576, -0.062426, 0.0536771, 0.00362343, 0.0144687, 0.0636291, 0.0681127, 0.102477, 0.0128334, -0.0624241, -0.084145, -0.00647798, -0.157931, -0.0435134, 0.0169366, 0.0500914, 0.184664, 0.167709, 0.196387, 0.0423866, 0.103205, -0.0290416, 0.124172, -0.0806244, -0.0852768, -0.05877, 0.034994, 0.0692487, -0.0909461, -0.0100814, -0.00690264, 0.0539937, 0.00846878, 0.038969, -0.00636332, -0.0294705, -0.107429, -0.0588001, -0.0817218, -0.0314374, -0.114384, -0.0418536, -0.0542663, 0.0822792, 0.0765551, 0.156258, 0.12609, 0.113731, 0.15253, 0.0504877, 0.0482697, -0.0413332, -0.0410319, -0.0155364, 0.000795978, 0.0818315, 0.034637, 0.0216936, 0.0660355, 0.0479277, -0.0141776, -0.170248, -0.114356, 0.00913837, -0.1561, -0.249542, -0.149715, -0.180446, -0.217305, -0.18908, -0.0349476, 0.0526979, 0.0627675, 0.107201, 0.168827, 0.0458063, 0.0120901, 0.0398361, -0.0859614, -0.065367, -0.0280548, -0.0447024, -0.0460313, -0.0303425, 0.0350535, 0.0389249, -0.0394804, -0.0396092, -0.0594004, -0.164158, -0.10025, -0.120896, -0.168064, -0.0832529, -0.19346, -0.123988, -0.100851, -0.0772804, -0.0964432, -0.00119693, 0.0635987, -0.00155616, 0.0531857, 0.0176273, -0.0160882, 0.0708412, 0.0570802, 0.0392523, -0.103582, 0.0114528, -0.0375555, -0.0441216, 0.0419669, -0.101161, -0.0014, 0.0809549, -0.103759, -0.161923, -0.0596298, 0.0256559, -0.0368667, -0.164057, -0.0878028, -0.100311, -0.0494543, -0.112319, 0.129642, -0.0540409, -0.0191972, -0.0861082, 0.0918858, 0.0054994, 0.00282923, 0.00298763, -0.0225063, -0.0452295, -0.0219344, -0.110074, -0.0373264, 0.117919, -0.0108734, 0.0491061, -0.0248656, -0.00738215, 0.0260547, 0.0146937, -0.0463498, 0.0510252, -0.0771619, -0.0315565, -0.0646433, -0.0337155, -0.0549854, -0.0829034, 0.0132698, 0.00875646, -0.0169745, -0.017219, -0.0609094, -0.0363679, 0.0771718, 0.0584804, 0.126918, 0.086294, 0.0365798, 0.0789887, 0.0156559, 0.0172079, -0.0151476, 0.0192091, 0.0836988, 0.0360437, 0.00178327, 0.0124818, -0.113889, 0.0712052, -0.13044, -0.0960455, -0.122435, -0.152136, -0.105854, -0.0232204, -0.0498843, -0.0224887, -0.00580637, 0.0236364, 0.02868, -0.12653, 0.0238602, -0.0297388, 0.044803, -0.013386, 0.00420125, -0.0104432, 0.0243068, -0.0292795, 0.00134308, 0.0116477, 0.0700001, 0.0139347, 0.108327, 0.0209706, 0.0203322, -0.0592558, -0.129106, -0.12884, -0.111198, -0.117255, -0.0742251, -0.0541331, 0.00862699, -0.0886359, -0.0459136, 0.0240568, -0.139541, -0.102556, -0.156179, -0.102606, 0.0255099, 0.040703, -0.00732661, 0.0608626, -0.0124302, 0.0340179, 0.00819936, -0.0351775, -0.0241686, 0.0539016, 0.0762308, 0.0890784, -0.0610412, 0.0404087, -0.0243231, -0.126939, -0.0925973, -0.0554758, -0.0990392, -0.0175442, -0.0682239, -0.0432312, 0.0246785, 0.00791172, -0.131588, -0.025229, -0.112372, 0.00866847, 0.00895024, -0.0205575, -0.0175926, 0.0576072, -0.102426, -0.0696141, -0.00519156, -0.103214, -0.00663406, -0.0354372, 0.0186125, 0.175204, -0.0552305, 0.0509445, -0.00686746, 0.0262272, 0.0494098, 0.0323064, 0.0704331, 0.143771, 0.0299271, -0.0994973, -0.0130907, 0.0366028, 0.042156, 0.0409538, -0.013749, 0.0347123, -0.0319162, 0.0123469, -0.00600065, 0.0834898, 0.000766084, -0.0478157, -0.014113, 0.00703274, 0.0412777, 0.114933, -0.0173343, 0.0592952, 0.0389063, 0.0342775, 0.127915, 0.0627362, 0.103581, 0.139968, 0.10992, 0.19609, 0.0434244, 0.0726881, 0.0597297, -0.0503701, 0.152764, -0.00527734, 0.0310519, 0.0330775, -0.00182794, 0.0611001, 0.0182299, -0.0156197, 0.0516634, 0.0350323, 0.00906686, -0.0641954, -0.012845, 0.103004, 0.0339229, 0.0605683, 0.018383, 0.121352, 0.0989437, 0.15298, 0.212182, 0.175236, 0.103179, 0.111835, 0.0643191, 0.0510767, 0.101352, 0.00933875, 0.0843467, 0.14801, 0.0651498, -0.0142263, 0.0421994, 0.0545946, 0.0279414, -0.122575, -0.00815453, 0.0284878, -0.0838051, -0.0274482, 0.0873433, 0.0558405, 0.068778, 0.0533889, 0.0458266, 0.0361635, 0.086046, 0.0381826, 0.133836, 0.050329, 0.0672743, 0.0820148, 0.153446, 0.0181767, 0.0568177, 0.0785861, 0.0147211, 0.0936525, -0.0363188, -0.0430953, 0.0134117, 0.0675875, 0.0839204, 0.0187756, 0.00137228, 0.056826, 0.00783812, 0.0482298, -0.0236669, 0.0138514, 0.108887, 0.103005, 0.057928, 0.0636814, -0.0158052, -0.000722168, -0.0752182, 0.112453, -0.0884481, 0.0556775, -0.0285589, 0.143922, 0.045496, 0.0450603, 0.0190195, -0.0305794, -0.00392791, 0.0475375, 0.0722976, 0.0633011, 0.00577012, -0.122696, 0.0360266, -0.0409132, -0.150046, -0.0750386, -0.0219979, -0.0117153, 0.0602331, 0.0223019, 0.0756887, 0.0779942, 0.0377599, -0.147201, -0.00436982, -0.12136, -0.134261, 0.0506047, 0.0761226, 0.0172777, 0.12526, 0.0618731, 0.0266449, -0.0951615, 0.00249644, -0.0192305, 0.062093, 0.0349835, 0.129804, -0.000807353, 0.0355575, -0.0198333, -0.0294695, -0.00439711, -0.00146425, 0.0290617, -0.0500664, 0.0184249, 0.0482057, 0.082771, 0.0247186, 0.0149529, 0.00698126, 0.109604, 0.0884911, -0.10362, -0.0491287, 0.119973, 0.00962281, 0.0637183, -0.0381013, -0.0189037, 0.0524075, -0.0887707, -0.0446583, 0.0110383, -0.00121099, 0.0207259, -0.033769, 0.0212001, 0.0168561, -0.0378135, 0.00527648, 0.0585541, 0.00710827, -0.0727477, -0.0406084, 0.0380126, 0.0752536, -0.0225605, -0.0150922, 0.0832223, 0.0131559, 0.0141069, 0.0538249, 0.0482487, 0.047512, 0.00377013, 0.061981, -0.00250603, -0.00267733, -0.112478, 0.0478971, 0.0267925, -0.0752902, -0.000750934, 0.0260843, -0.0799251, 0.0457439, 0.0133797, -0.0221656, -0.0491286, -0.00193411, 0.0525553, 0.0837616, -0.084209, 0.0526457, -0.00118094, -0.0318165, 0.0507952, -0.0252346, -0.0443124, -0.0466794, 0.0690585, -0.00547656, 0.00560296, 0.0100384, 0.0805572, 0.0424529, -0.0872565, 0.0493685, -0.0063135, 0.0357393, 0.0353425, 0.0179468, 0.0266312,
-0.067766, 0.041777, -0.0584885, -0.00530234, 0.0121075, -0.00446198, -0.0514146, 0.000589173, 0.0404765, -0.012847, -0.0610976, -0.0520648, -0.0451073, -0.0957031, -0.0160987, -0.012461, 0.0121009, -0.00745292, 0.0372423, -0.0210267, 0.042453, -0.0593461, 0.0209012, -0.0584736, -0.0390288, -0.0792517, -0.0939835, 0.0251195, 0.015214, 0.0869506, 0.0159169, 0.0151137, -0.0143193, -0.046195, -0.0013968, 0.0211838, -0.0592119, 0.0135584, -0.0779584, -0.0690197, -0.0388219, -0.0509863, 0.091487, -0.0117092, -0.0112757, 0.0784798, 0.0226765, 0.00127183, -0.114961, -0.0989123, -0.026451, -0.00475662, 0.0076886, 0.0492429, 0.0558047, -0.00797447, 0.0598896, 0.043942, -0.0273819, -0.0158947, 0.0184205, 0.0819852, -0.0448599, -0.00501056, -0.130085, -0.0354973, -0.023832, -0.0996873, -0.0901628, 0.00126725, -0.0475035, -0.133013, 0.0650339, -0.053822, 0.00402897, -0.00811361, -0.0616737, -0.0391891, -0.0552419, -0.0393134, 0.0510261, 0.0145906, 0.0885949, -0.0300267, -0.0136787, -0.0249556, -0.022662, 0.0648544, 0.0290817, 0.0705453, -0.0602735, 0.0203951, -0.124019, -0.0318689, -0.0420118, -0.0800362, -0.0897735, -0.0196467, -0.0706242, 0.00978618, -0.0285329, 0.00311063, -0.0415556, -0.0161848, 0.0206596, 0.000368669, 0.0021864, -0.0432052, -0.109379, 0.0112917, 0.0510879, -0.076669, -0.00225944, -0.0174385, -0.0766289, -0.0345643, -0.0306512, 0.0587868, -0.145123, -0.100531, 0.00394937, 0.000555142, -0.104199, -0.0143317, 0.0273344, -0.0731921, -0.00978714, -0.0137478, 0.0457241, 0.0365002, 0.0850338, 0.0729378, 0.0589135, 0.0803396, 0.0455151, 0.0657146, 0.0505973, 0.0311637, 0.00127377, 0.0383977, -0.0421241, 0.0663404, 0.0589243, 0.00174811, 0.0212717, 0.0178297, -0.00791729, 0.02952, -0.093292, -0.00721504, -0.0652292, -0.103973, -0.102432, -0.00447538, -0.0760576, 0.118511, 0.0749371, 0.0833668, 0.0375958, 0.0515536, 0.0799544, 0.073171, 0.0531235, 0.106382, 0.0443288, 0.0939515, -0.0118636, -0.0664743, -0.0965508, -0.0127214, 0.0411775, -0.00247794, -0.0434229, -0.0557372, 0.0199861, 0.0287493, -0.0731743, -0.0727304, -0.0526614, -0.0543503, -0.0227948, -0.0336658, -0.0836271, 0.00845519, 0.00418646, -0.0427687, -0.00594232, 0.0156464, 0.0532718, 0.0530298, 0.0505946, 0.022351, -0.0174052, -0.0468455, 0.0100962, 0.021425, 0.0118321, -0.0986904, 0.0671702, 0.0432312, -0.0150851, -0.0231175, 0.0652892, -0.0562369, 0.0328298, 0.0609141, -0.0159002, 0.0432295, 0.023079, -0.00188151, -0.107865, -0.0751458, -0.0874252, -0.0345327, -0.088006, 0.0692848, -0.0558685, 0.0317928, 0.044168, 0.0791312, 0.0260301, 0.0125926, 0.0657124, 0.0499221, -0.0591516, -0.0369704, 0.0709153, -0.0690973, -0.0293206, -0.0507728, 0.0268233, 0.0568291, 0.0618073, 0.00716125, 0.0247947, 0.0387032, -0.107653, 0.0363188, -0.0938889, -0.121139, -0.0677169, -0.080948, 0.0127451, -0.110689, -0.0183803, -0.0364939, 0.107516, 0.0166949, 0.100843, 0.0443905, 0.0860003, -0.0623405, -0.0285582, -0.00357169, 0.00715242, -0.0384184, 0.0126755, -0.0301296, -0.00570918, 0.0332413, -0.0418618, 0.069317, 0.00476746, -0.0516132, 0.0198693, 0.00697538, -0.0746801, -0.0712049, -0.0801893, 0.0540431, 0.112138, -0.0821819, 0.0586702, 0.164371, 0.0296923, -0.0184768, 0.0780491, -0.0261755, -0.0204454, -0.0727219, -0.0355829, -0.136677, 0.0514884, -0.0252567, 0.086914, -0.0631572, -0.0195026, -0.0157708, -0.0236669, -0.0244651, 0.0385771, -0.0307476, 0.103797, -0.100327, -0.140389, -0.0564532, -0.146851, 0.00493441, -0.0688922, -0.00133775, 0.0844068, 0.00493658, 0.0689515, 0.0608961, 0.0156371, 0.0416563, 0.0954304, 0.0371422, 0.0306882, 0.0463825, 0.0099436, -0.037054, 0.0528322, 0.068551, 0.0567666, -0.0951989, 0.0769232, 0.0260972, 0.0268372, 0.0675545, -0.0306139, -0.133806, -0.0473522, -0.0494443, -0.0824893, -0.0575965, -0.0562947, 0.0373803, 0.114749, 0.00483078, 0.0454099, 0.00619965, -0.0104533, -0.0525383, -0.0878921, -0.0841686, 0.0469984, 0.027888, -0.0703257, 0.0139788, 0.0745667, 0.0526766, 0.0586151, 0.0304412, 0.0198146, -0.0671806, 0.00158087, 0.0344284, 0.0220211, -0.0212368, -0.112645, 0.0383436, 0.0266321, 0.0233114, 0.111362, 0.146381, 0.0981182, 0.0684983, 0.0892342, -0.00419868, 0.0424389, 0.0109249, -0.00869259, 0.0195862, -0.039343, 0.141855, -0.00085031, -0.0566369, 0.0199965, 0.0223118, -0.0574934, 0.07203, 0.054249, 0.0015397, 0.0135143, 0.112278, 0.129499, 0.11877, 0.052886, 0.197229, 0.0131038, 0.104626, 0.132288, 0.188542, 0.186319, 0.120294, 0.0441857, -0.142153, -0.0265691, -0.000348551, 0.0642424, -0.0119284, 0.0697134, 0.079605, 0.013606, 0.0448415, -0.0588066, 0.0458884, 0.00162293, 0.109909, 0.149816, 0.0293814, 0.0674574, -0.00500605, 0.147617, 0.255134, 0.167827, 0.00903121, -0.0133394, 0.0491793, 0.100809, 0.0728213, 0.128359, 0.0381014, -0.0275614, -0.113832, -0.0225086, -0.00471586, 0.035753, -0.015927, 0.110026, -0.0216838, -0.0155269, 0.0446119, 0.0716918, 0.0381258, -0.00251857, 0.0452268, 0.0176647, 0.0708315, 0.153404, 0.0743788, 0.106605, 0.220609, 0.232964, 0.169678, 0.139339, -0.044693, 0.00864721, -0.0605081, -0.0359673, -0.0740851, 0.00131128, -0.0905562, -0.0480912, 0.088183, 0.00344492, -0.0284296, -0.0276161, 0.0655103, 0.0918617, 0.00949298, -0.0304291, -0.0314339, 0.0399639, -0.0521936, -0.00704231, 0.0632862, 0.10578, 0.0209272, 0.126596, 0.232057, 0.188283, 0.153403, 0.0366146, -0.0173679, -0.0132484, -0.199016, -0.108621, 0.0163956, -0.000477272, 0.0409149, -0.0622317, -0.0412103, -0.0847973, 0.0193571, -0.0607322, 0.0879172, -0.00725524, -0.0633734, 0.0364433, -0.00375955, -0.0291742, -0.00202829, -0.105388, -0.047854, -0.174156, -0.0594274, 0.113401, 0.118402, 0.0791999, 0.0451515, -0.0350297, -0.05222, -0.0877837, -0.0973337, -0.0324375, -0.147646, 0.035227, -0.0078449, -0.112077, -0.110603, -0.0263043, -0.0600248, 0.0162035, 0.02035, -0.0466704, 0.10722, 0.0493258, 0.0236088, 0.0477785, -0.00656158, -0.111227, -0.153733, -0.0801712, -0.0896184, -0.124361, -0.138911, -0.0157194, 0.0563, -0.0191748, -0.0668342, -0.135272, -0.0821296, -0.135764, -0.0693426, 0.031231, 0.0432887, -0.0206324, -0.0366549, 0.0324202, -0.0269143, 0.0871896, -0.0811284, -0.0333975, 0.0715765, 0.00966022, 0.0989336, 0.084255, -0.101145, -0.0113757, -0.0815013, -0.211354, -0.2247, -0.269561, -0.165108, -0.0730628, -0.0363178, 0.0554509, 0.0804279, 0.0099517, 0.0469754, -0.120596, 0.00641505, 0.0237023, 0.0689866, -0.0415901, 0.0124234, 0.0269161, -0.00485993, 0.0329498, -0.0373916, -0.052096, -0.0610833, -0.0504, 0.0406048, 0.036011, 0.111407, 0.0270496, -0.0687143, -0.155976, -0.0785159, -0.10311, -0.154241, -0.0977202, 0.0377398, 0.00261345, -0.0305118, 0.00804355, -0.0413138, 0.0303925, 0.0139973, -0.039578, -0.0152077, -0.127977, 0.0445305, -0.0878252, -0.0518713, 0.0726566, -0.0242965, 0.0764647, -0.045404, 0.0576052, 0.0454438, 0.104278, 0.0916671, 0.0594683, -0.00816243, -0.0450175, -0.0684997, -0.010804, -0.0230727, 0.0366961, -0.0127495, -0.0470102, -0.047863, -0.00888888, 0.0689154, -0.0111452, 0.0478532, 0.0288565, 0.0259416, -0.00502191, 0.0446579, 0.0363085, -0.0693739, -0.0696557, 0.0635442, -0.0209665, 0.0469788, -0.0132154, -0.00796467, 0.0582394, 0.0634486, 0.0434981, -0.04907, 0.0767252, 0.0685001, -0.0154195, 0.0405095, -0.0880667, -0.0649773, -0.201638, -0.0770439, 0.054549, -0.0565275, 0.0494914, 0.0392664, 0.000910944, -0.112823, 0.165088, 0.024493, -0.0905137, -0.0506117, -0.0471655, 0.0684637, -0.010507, 0.0120431, 0.0656538, -0.00250305, -0.00777013, 0.065096, 0.0340795, 0.0416013, 0.00562451, 0.0493564, -0.0837563, 0.0864199, 0.0443453, -0.0122585, -0.0188722, 0.00759973, -0.0296208, -0.00923095, 0.0138459, 0.116774, 0.0260464, 0.0134193, -0.0209828, -0.00783787, 0.0329568, -0.0209909, -0.0713803, 0.00106587, 0.00495731, -0.0294695, -0.0306511, -0.0588395, 0.0400287, 0.0237032, 0.111741, 0.0524148, -0.0935895, 0.0331262, -0.0266819, -0.0651096, -0.093829, -0.161755, 0.0310147, -0.103271, -0.0280401, -0.00966224, 0.149385, 0.0400787, -0.0129063, 0.104029, -0.0465192, -0.0323029, 0.0376777, 0.0371896, -0.00704488, -0.0133291, -0.0599556, 0.000442235, 0.0516703, 0.0411293, -0.0266389, 0.0401714, -0.0582405, 0.0106162, -0.0111465, -0.000526134, -0.0795804, 0.0111025, -0.0387889, -0.0240156, 0.0391631, 0.0441881, -0.0393109, -0.0360494, -0.124786, -0.010312, -0.095007, 0.11607, -0.000956311, 0.0658138, -0.108613, -0.0549454, 0.0136263, 0.00614543, -0.0214182, 0.0552676, 0.0536053, 0.10558, 0.0682175, -0.0952138, 0.00463901, 0.0156702, 0.0938037, -0.0638601, 0.00581261, 0.0403191, 0.00799714, -0.13046, -0.0123597, 0.0291817, -0.0929873, 0.0732444, -0.0328488, 0.0410379, -0.0467163, -0.0288137, -0.0269926, 0.0222091, -0.00476787, -0.103374, 0.102932, -0.0189171, -0.0448565, 0.0136426, 0.0099329, 0.0245538, -0.0103591, 0.0703502, -0.0373651, -0.0284099, -0.0373665, -0.0270289, -0.0436493, 0.0377363, 0.0385678, 0.0222884, 4.6797e-05, -0.0103326, -0.0441201, 0.0538209, 0.115595, 0.0761808, -0.0890045, -0.0431728, 0.0293993, 0.0429617, 0.00585904, -0.0124143,
-0.00881714, 0.149709, 0.0149994, -0.0228567, 0.0422533, 0.0246874, -0.00995538, -0.00484078, -0.0891852, 0.0377011, -0.0175816, 0.00965487, -0.0712724, -0.0140692, -0.0526051, 0.0327859, -0.0166811, 0.0412952, -0.0654291, -0.00766954, -0.0396334, 0.128359, 0.0308519, -0.0364828, -0.0541309, -0.0159891, 0.101543, -0.00106522, -0.0509785, 0.021463, -0.00910627, 0.00784545, 0.0484696, 0.0122079, 0.000317697, -0.0171812, 0.0196706, -0.00607391, 0.0639519, -0.0195545, -0.0553713, -0.0197738, -0.0439567, 0.0365816, -0.0667498, -0.0412618, -0.0525379, 0.0341516, 0.032597, 0.0638392, -0.00981166, 0.0120409, 0.00329113, -0.0151438, -0.0397124, -0.0579868, 0.00539063, 0.00313694, -0.0327776, -0.0523156, 0.0113736, -0.0427236, 0.0569563, 0.0351452, 0.0948579, -0.00111941, -0.0079248, 0.0111862, -0.062156, -0.0720671, -0.000975456, -0.0612085, 0.029677, 0.0183908, -0.00436694, 0.0884887, 0.039747, -0.0361029, 0.0736371, -0.0726548, 0.014485, -0.0233835, -0.0168255, 0.0422935, 0.0340215, -0.0670364, -0.00424899, -0.00441527, -0.0258889, 0.101569, -0.00478372, -0.033139, -0.0648297, -0.0331942, -0.034658, -0.100652, -0.0507289, -0.126632, -0.0493601, 0.00579159, -0.0990449, -0.0246512, 0.0397606, 0.164737, 0.0261472, 0.0792194, -0.0071911, -0.0426094, -0.0429799, 0.0501611, 0.00314714, 0.0686686, -0.0569145, -0.102847, 0.0422377, -0.00761264, 0.0843223, 0.0729089, -0.0134227, -0.0658803, 0.00710151, -0.0414846, -0.0049554, -0.240995, -0.168187, -0.0837192, -0.0497074, 0.0255935, -0.0438822, -0.0250619, 0.0328278, -0.127816, -0.0237948, 0.0349752, 0.126812, 0.0445159, 0.0442785, 0.0456947, 0.0408182, -0.0220127, 0.00755883, 0.0663476, 0.0692197, 0.0551607, 0.054395, -0.0141649, -0.00252297, -0.110336, -0.110317, -0.115305, -0.183472, -0.0950513, -0.0729314, -0.135543, -0.233605, -0.0120767, -0.0751472, 0.000558727, -0.0392252, -0.00207897, 0.0516259, -0.0473309, -0.133881, 0.0461188, 0.0231464, -0.0708409, 0.0673563, -0.0254832, -0.126434, -0.0271983, 0.124093, -0.0211933, -0.0036194, 0.0292238, -0.0688037, -0.0459049, -0.0920133, -0.203405, -0.196384, -0.0797376, -0.0480162, -0.0663514, 0.0244602, 0.00557129, -0.0564316, -0.0611781, 0.0599978, 0.0193791, 0.0358687, -0.00227393, 0.017825, 0.0906496, 0.0837018, 0.0864331, 0.0323718, 0.0610243, -0.0816656, -0.00372441, 0.0845183, -0.023085, -0.0130363, -0.0169763, -0.0614445, -0.0597408, -0.0569687, -0.308498, -0.127243, -0.0872782, 0.0651981, 0.0988877, 0.13317, 0.179337, 0.0415302, 0.0647877, 0.0240221, 0.041985, 0.0259653, -0.0298581, 0.0289192, 0.056259, 0.0694629, 0.0886758, 0.0481638, 0.00575812, 0.0973439, 0.00272932, 0.0240313, -0.00835882, -0.036154, -0.0616005, -0.122368, -0.19139, -0.195744, -0.127777, -0.167139, 0.0365995, 0.00595236, 0.127544, 0.0487438, 0.0177347, 0.0920609, -0.00573743, -0.0123889, -0.0115813, 0.00549568, 0.0198374, -0.00979039, 0.134132, 0.174045, 0.0204436, -0.0635158, 0.0161205, 0.076997, -0.00912745, -0.0122386, 0.0332152, -0.0186858, 0.0225242, -0.0592356, -0.0706163, -0.268869, -0.218682, -0.112009, 0.0760271, 0.0565019, 0.0221465, -0.000451032, -0.106994, -0.000100812, -0.132171, -0.165338, -0.00624403, -0.0288014, -0.0107339, 0.0655194, 0.0748052, 0.0796323, 0.0644698, -0.000574643, 0.00622573, 0.0444887, 0.0763244, 0.0371118, -0.0547822, 0.0157234, 0.0499396, -0.0499202, -0.103598, -0.193232, -0.130295, -0.0462752, 0.0364562, 0.0167708, 0.0295682, 0.11646, 0.0799333, 0.0712754, -0.086976, -0.0260921, -0.202649, -0.0765268, -0.00409433, 0.00493366, -0.011618, 0.131339, 0.0862142, 0.00834041, -0.0101584, -0.0160726, 0.0336702, -0.0717709, 0.0297496, -0.0130764, 0.0381877, -0.0907825, -0.12809, -0.19439, -0.161217, -0.0137765, 0.0182191, 0.000294671, 0.0825462, 0.147384, 0.190156, 0.0796018, -0.111423, -0.0633968, -0.109162, -0.128876, -0.0511992, -0.00167599, 0.0924621, 0.0655301, 0.082829, -0.0224689, 0.00973082, -0.00250005, 0.0160714, 0.0369368, 0.0483238, 0.0470159, -0.038929, -0.101392, -0.172686, -0.255434, -0.0204587, 0.0240978, -0.0559246, 0.016398, 0.155209, 0.19029, 0.141064, -0.0415637, 0.00162188, -0.0322924, -0.0629967, 0.0152952, -0.127898, -0.040953, -0.0627855, -0.0733144, -0.0637448, 0.0150211, 0.0150616, 0.0897508, 0.0514961, 0.0348543, 0.0935971, -0.0588278, -0.036013, -0.0346019, -0.0457854, -0.0509447, -0.068115, -0.0416233, -0.0677258, 0.071857, 0.127453, 0.0740681, 0.0447602, 0.0179305, 0.0590064, -0.156626, -0.0138773, -0.0994011, -0.159514, -0.160936, -0.140233, -0.0559606, 0.04003, 0.00660746, 0.0531294, 0.160946, 0.0284366, -0.0165995, 0.00693957, 0.00116801, -0.0248003, 0.0522953, 0.0166573, -0.0780657, -0.00340115, -0.146098, -0.0274252, -0.0618803, 0.108502, 0.0795275, 0.17735, 0.0417965, -0.0345426, -0.183531, -0.132428, 0.00564245, -0.0908964, -0.103993, -0.10944, -0.0615651, -0.00181814, 0.114017, 0.00857855, 0.0029338, -0.0459324, 0.0160987, -0.0230882, 0.00984343, -0.0387364, 0.0658229, 0.0102863, 0.0173982, -0.0432337, -0.11366, -0.147272, 0.0193719, 0.0417789, 0.0538795, -0.0605849, 0.0119447, 0.0612381, 0.0257252, -0.0828967, -0.126201, -0.0946162, -0.0793537, -0.126293, 0.032778, -0.022108, 0.0273257, 0.0067858, 0.0144272, -0.0798197, 0.0287452, 0.0932417, 0.048875, 0.043792, 0.0764198, -0.0454422, -0.0622868, -0.0356104, 0.00445994, 0.0368833, 0.0742406, -0.00325566, 0.0489602, 0.0440059, 0.0320493, -0.0122662, -0.0940795, -0.0853762, -0.0990402, -0.116651, -0.153763, -0.00710055, 0.00210761, 0.00704752, -0.000366371, 0.0323589, 0.0722834, -0.0897238, 0.0816281, -0.047906, 0.0990956, -0.0119356, 0.146015, 0.12092, -0.0211053, 0.0109916, 0.0461172, -0.00925084, -0.154357, -0.165732, -0.057662, 0.0399207, 0.0161458, 0.069735, 0.114654, -0.194453, -0.00193634, -0.149354, -0.0990406, -0.0499433, -0.0614286, 0.0940215, -0.0172076, 0.0658185, -0.0409676, -0.101994, -0.0629537, -0.0107155, -0.0746236, 0.117883, 0.0100252, 0.13985, 0.112964, 0.138732, 0.0266469, -0.000639534, -0.0748157, -0.13493, 0.0106012, -0.0449514, 0.176939, 0.19446, -0.00500832, -0.11396, -0.111793, -0.232297, -0.0654474, -0.0565185, 0.039157, -0.0117579, 0.0222024, -0.000610318, -0.00226622, 0.0670228, 0.0309922, -0.0529004, -0.0883139, -0.0388642, -0.0410279, 0.0702997, 0.0727177, -0.0731181, 0.157505, 0.0628934, -0.104379, -0.15245, -0.0636649, 0.0262225, 0.0422245, 0.120691, -0.022796, 0.00370759, -0.0438698, -0.150607, -0.164068, -0.0319363, 0.000998292, 0.0242478, -0.0501931, 0.0414484, -0.00564198, -0.00975034, -0.00867275, -0.0338014, -0.0438943, 0.0632187, 0.0473901, 0.016802, 0.0117769, 0.051179, 0.0568501, -0.0129493, -0.0798219, -0.00132484, 0.0297715, -0.0619066, -0.0224974, 0.0599156, -0.0208053, -0.0267672, -0.104959, -0.142122, -0.0725111, -0.0330984, 0.0869746, -0.011854, -0.0262582, -0.00504208, 0.0156462, -0.0762991, -0.127279, -0.0910312, -0.101023, -0.0702225, 0.0137875, -0.035887, -0.100281, 0.0717987, 0.0624668, 0.0343912, -0.00269781, -0.0352239, -0.0911842, -0.166944, -0.0281569, -0.030062, -0.146071, -0.126974, 0.028897, -0.154473, -0.0763377, 0.0261395, 0.0382833, 0.0375842, 0.0663376, -0.0316104, -0.0184279, -0.143977, -0.0462489, -0.0366727, -0.0311753, 0.0189568, -0.0204191, -0.0100007, 0.0589774, 0.131054, 0.0593371, 0.0963048, 0.128969, 0.0986901, 0.00262005, 0.0341564, -0.165282, -0.013887, -0.106382, -0.0307177, -0.0697151, -0.0490957, -0.0653253, -0.0162431, 0.0619393, 0.00657974, -0.0569797, 0.00536562, -0.0304716, 0.0408352, -0.000190882, 0.0698702, -0.010498, 0.0268679, 0.00771987, 0.0405498, 0.0348509, 0.11448, 0.100328, 0.0120369, 0.0525245, -0.00780243, -0.00665203, -0.0661908, -0.115172, -0.164574, -0.142722, 0.0682072, 0.0610434, -0.0737867, -0.0198067, -0.0626581, -0.0785288, -0.156472, -0.0427329, 0.0310641, 0.0201437, -0.0629347, -0.0702749, 0.0740404, -0.00760207, -0.0231346, -0.0791618, -0.00762536, 0.0338751, 0.0301882, 0.0520786, -0.0148693, 0.0166634, 0.00247218, 0.0340187, 0.016772, 0.0382775, -0.0459496, -0.0241448, 0.0304598, -0.0137205, 0.0474143, 0.000119191, 0.0479956, 0.00513323, 0.0607098, 0.00385192, 0.0323673, 0.0142281, -0.0866657, -0.0427221, -0.057991, -0.0338459, -0.112246, -0.00929103, -0.00460138, -0.0757283, -0.0523742, 0.0106911, -0.0609091, -0.00837192, 0.0808079, -0.00628109, 0.00563615, 0.12194, 0.0451131, 0.0169736, -0.0183163, 0.0493679, -0.0341334, -0.0223868, -0.0339673, -0.0292581, -0.0231538, 0.0076955, 0.0535616, -0.000684656, 0.0251344, -0.0486707, 0.0114268, -0.00385437, 0.0454258, -0.0490262, -0.0265519, -0.105751, 0.0578245, 0.00098684, 0.0421337, 0.0345834, 0.0256929, 0.0431626, -0.0159421, 0.0499703, 0.0414088, 0.0646874, 0.0806485, -0.108382, -0.107337, -0.0105995, 0.00967171, 0.013131, -0.034521, 0.0440449, 0.0107396, -0.070145, -0.00872842, 0.0146806, -0.0303435, 0.0416552, 0.0151178, -0.0318743, -0.033322, -0.023838, 0.0900216, -0.0609309, -0.0443601, 0.030493, 0.0221833, -0.00733009, 0.00411927, 0.0201054, -0.0189049, -0.00210089, -0.0694043, 0.00957616, -0.0365169, -0.019958, -0.0335951, -0.00990004, 0.0506233, 0.063667, 0.0395507,
0.0852279, -0.0824831, -0.0709573, -0.0128387, -0.015845, 0.0537569, 0.0425806, 0.034625, 0.0114198, -0.00474009, -0.0185767, 0.0648078, 0.0182071, 0.116881, -0.00357923, -0.0760662, -0.0103249, -0.0882325, -0.0191836, 0.0539652, 0.00101576, -0.0150124, 0.012622, -0.0264266, -0.0150655, 0.0211951, -0.105274, 0.0272749, 0.0624857, 0.00409825, 0.0421584, 0.0432848, 0.0360949, -0.0725631, -0.0180116, -0.0383929, -0.0572787, 0.0602613, -0.0226716, 0.0215484, 0.0921426, -0.0902637, -0.0101278, 0.0284477, 0.0716436, -0.0452209, -0.0286016, 0.025552, -0.0718772, 0.026652, 0.0451224, 0.0081009, -0.0934658, -0.0642241, 0.0603231, -0.0197828, -0.00615835, -0.0282413, 0.0164061, 0.0150419, -0.0561705, -0.00352814, 0.00587956, 0.0636375, -0.0907584, -0.0457935, -0.0645913, -0.0366145, 0.0291865, -7.26723e-05, 0.0120683, 0.109248, 0.0736426, -0.0474192, -0.00718368, -0.00757124, 0.0484049, 0.0306944, 0.000789376, -0.0117947, -0.00255425, -0.0813954, -0.000443061, 0.0678316, -0.048602, 0.0126824, 0.040862, -0.00396421, -0.0117058, 0.0205474, -0.0287563, -0.0128175, -0.0358906, -0.0331445, -0.0730239, -0.0306661, 0.0202782, 0.0375557, -0.0629787, 0.0439978, 0.0246111, 0.0227929, -0.00200908, 0.0559708, 0.100838, 0.0208112, 0.0620001, 0.0269309, -0.0369036, -0.0414332, -0.0197076, -0.0594473, 0.0114523, 0.00219798, 0.082599, 0.0290662, -0.00907691, -0.0845702, 0.0120509, -0.0111688, 0.0686745, -0.0422436, 0.0594554, 0.0199723, 0.0683913, 0.0972796, -0.0104623, 0.184974, 0.118237, -0.0461394, 0.0417812, -0.0211844, -0.0463057, 0.0261966, -0.0278016, 0.0489777, -0.0789157, 0.0202568, -0.05225, 0.0134846, 0.014953, -0.0356893, 0.0105648, -0.0289149, 0.08856, 0.0853248, 0.0424115, 0.0438758, -0.0256407, 0.0214122, 0.0303308, 0.00703447, -0.000654677, 0.106876, 0.0690433, 0.142859, 0.0602858, 0.0206752, 0.00396625, -0.0537101, -0.0250027, -0.00325722, 0.0475728, 0.0846145, 0.00247794, 0.12768, 0.0472958, -0.00744658, 0.0517286, 0.0326528, -0.00184538, -0.0836591, 0.00567339, 0.110544, 0.104283, 0.0284275, -0.00533367, -0.0230347, 0.0835358, 0.00869648, 0.1154, 0.0811986, 0.0720139, 0.041026, 0.0863386, 0.0737502, 0.125728, 0.00650219, 0.0194807, -0.0119918, -0.00149285, 0.0194311, -0.0104967, 0.139738, 0.0215355, 0.0530285, 0.051323, -0.00482055, -0.0285436, -0.0928835, 0.0582074, -0.0068509, -0.0114908, 0.0738932, 0.0159695, 0.0864986, 0.0268101, 0.0146135, 0.0982826, -0.06897, 0.160937, -0.0616476, -0.00865189, 0.0790702, 0.0119608, 0.120054, 0.136806, 0.0669788, 0.101611, 0.149825, -0.0340083, 0.0145854, 0.110271, 0.0088729, 0.0200947, 0.0595279, -0.00334032, 0.0215294, -0.0329456, 0.0138341, 0.126527, -0.0276869, -0.0733457, -0.000620106, 0.0443303, 0.0823141, -0.0163224, 0.0862674, 0.0420859, 0.0601843, -0.178942, 0.0745159, 0.0899813, 0.0607192, -0.00160252, -0.0155661, -0.101873, -0.0521029, 0.0324762, -0.0100946, -0.0155937, -0.0113666, -0.0122536, -0.0945095, 0.067284, -0.0290734, 0.0767991, -0.0258964, 0.028723, 0.0189248, 0.0194126, -0.0261488, 0.0944511, 0.0497002, 0.0145785, -0.0189374, 0.0162311, -0.033245, -0.0402497, -0.0564555, -0.0239135, -0.0714288, -0.0149937, -0.0749133, -0.172083, -0.137511, -0.00634313, 0.0011085, 0.0203562, -0.0141168, 0.0433212, 0.0959163, 0.0212724, 0.122482, 0.0821115, -0.00345245, -0.0707057, -0.0361882, -0.00491376, -0.0446038, 0.0195409, 0.0241719, -0.0188068, 0.0795634, -0.0281876, -0.0831679, -0.0709422, 0.0411604, -0.0392815, -0.0794272, -0.0751401, -0.139721, -0.0991405, -0.0633893, -0.0290967, 0.00746626, -0.0134488, -0.125762, -0.0175848, -0.0427394, 0.0228217, 0.0408511, 0.11273, 0.0951065, -0.0587415, -0.0626112, -0.0640071, -0.0639745, -0.0425592, 0.0311257, 0.0239878, 0.0574528, 0.105977, -0.102615, -0.089185, 0.051471, 0.0242891, -0.0420266, -0.0336949, -0.153906, -0.0694319, -0.173408, -0.0684049, -0.0288006, -0.00373593, 0.0177894, -0.0282171, 0.0233488, 0.000300538, 0.0170554, 0.110658, 0.0626938, -0.0800757, -0.0204881, -0.0690807, -0.150314, -0.0398983, -0.0120835, -0.00361435, -0.0289808, -0.062631, -0.0830421, -0.00689761, -0.0339753, 0.0480355, -0.094721, 0.00486687, -0.136174, -0.0410606, -0.133034, -0.0928381, -0.058094, 0.0720397, -0.0216235, 0.0931715, -0.0285069, -0.029793, 0.034925, -0.0218402, -0.0175346, -0.0434915, -0.0378243, 0.0587958, 0.0351887, -0.0425993, 0.0334569, -0.0719565, -0.0784422, -0.0493444, -0.0506131, -0.0253677, 0.0353486, 0.0673839, -0.00554614, 0.0249502, -0.0812375, -0.0141964, -0.0964261, -0.160634, 0.0073333, 0.0123481, 0.0622161, -0.00766693, 0.0807624, -0.0553962, -0.00585326, -0.0685394, -0.132591, -0.0960648, -0.0768603, -0.0595155, -0.0486758, -0.0733124, -0.0992663, -0.110111, -0.0101382, 0.0754365, 0.0672519, -0.00086723, 0.0212277, 0.0592197, 0.029841, -0.0434615, 0.0145401, -0.01889, 0.0297822, -0.052487, -0.0195581, 0.0121891, 0.0604964, -0.0028838, -0.0504469, 0.0172515, 0.00449264, -0.0700306, -0.0981095, -0.0751902, -0.226597, -0.150592, -0.12061, -0.128698, -0.182514, -0.0741452, -0.057451, 0.153766, -0.0212778, 0.0365499, -0.0120147, 0.134079, 0.0442442, 0.0752397, -0.101127, -0.0237286, 0.0711518, -0.0250635, 0.0902731, -0.0428325, 0.00154778, -0.00737985, 0.0348968, 0.0160763, -0.0262377, -0.00497843, -0.100105, -0.134859, -0.098868, -0.155319, -0.219412, -0.190462, -0.233876, -0.232431, -0.0869617, 0.212709, 0.178074, 0.121233, 0.147833, 0.0788395, 0.0783005, 0.0616469, 0.0533915, 0.16422, 0.0460476, 0.0411703, -0.024435, 0.00393473, 0.121713, -0.00111158, 0.0416147, 0.0239298, -0.0429276, 0.0605211, -0.118778, -0.0463131, -0.119849, -0.0636371, -0.190366, -0.201533, -0.229687, -0.31715, 0.0124894, 0.161301, 0.253566, 0.107605, 0.11591, 0.114077, 0.0447019, 0.0319339, -0.02722, -0.0926398, -0.0139763, 0.0193533, 0.020538, -0.0128015, 0.0241887, -0.00320262, -0.0387937, -0.0473771, 0.0133213, -0.0172441, -0.0400023, 0.0655319, -0.0410502, -0.0486744, -0.0937087, -0.21439, -0.22291, -0.317319, -0.0484034, 0.181078, 0.201941, 0.16881, 0.160185, 0.0395968, 0.0466163, 0.0701637, 0.0676851, 0.0204148, -0.0223871, 0.0850024, 0.0548083, -0.00872369, -0.0161115, 0.021601, 0.0142242, -0.0161071, 0.0243868, 0.0302222, 0.066442, -0.0194699, -0.00762667, 0.0715465, 0.0223838, -0.054784, -0.168741, -0.168653, 0.121387, 0.192812, 0.297886, 0.156233, 0.124854, -0.0106035, 0.101296, 0.0178055, -0.0964381, 0.0489011, 0.0646488, 0.0815506, 0.0123162, -0.0429109, 0.0500958, -0.0261713, -0.0289524, -0.0629429, 0.0705093, -0.0290382, 0.0178444, 0.0226706, -0.00845432, 0.0571233, 0.0194157, 0.0709882, -0.108242, 0.0654157, 0.074367, 0.145698, 0.113903, 0.119423, 0.25592, 0.0651426, 0.0563336, 0.0083854, 0.117268, 0.110634, 0.145239, -0.0596158, 0.0440256, -0.105534, 0.0409806, 0.116903, 0.00515394, -0.00154344, -0.0275244, 0.00661481, 0.0324559, 0.0970751, 0.117039, 0.0870265, -0.0239417, 0.00573283, -0.028316, 0.090545, 0.107127, 0.171345, 0.178858, 0.131119, 0.154401, 0.0989174, 0.0650409, -0.00222594, 0.069755, -0.0620558, -0.0673511, 0.0777513, -0.0372096, -0.0223641, -0.0321586, 0.0154406, -0.0337602, 0.0417019, 0.0770076, 0.0973228, 0.0813841, 0.0396377, 0.0848838, -0.0720077, -0.0405585, -0.0181206, 0.0475237, -0.0715, -0.0589939, 0.0259125, 0.144087, 0.054284, 0.126245, 0.0376201, 0.0180677, 0.0473384, -0.0796619, -0.170063, -0.0167456, -0.0129272, -0.0681622, 0.026433, -0.0229486, -0.0105315, 0.0396583, -0.0796271, 0.0627488, 0.027621, 0.0854038, 0.0466625, -0.0597409, -0.120194, -0.0395421, -0.0638068, -0.00961415, 0.0242885, 0.08031, 0.0437949, 0.122518, 0.0484505, 0.0375164, -0.0580508, -0.0408526, -0.00414105, -0.0547815, -0.076999, -0.0203421, -0.0185176, -0.135252, -0.0254544, -0.0656963, 0.014651, -0.05589, 0.0326858, -0.0644117, -0.0672345, -0.0685818, 0.0372991, -0.0344218, -0.000196552, -0.0755801, 0.0555175, 0.0186476, 0.0749651, 0.00646568, -0.0264973, -0.00168643, -0.0832195, -0.0413274, 0.00821031, 0.0166876, -0.0724892, -0.0432312, -0.102609, -0.082717, 0.0830086, 0.0176147, -0.0572412, 0.0266565, -0.00363251, 0.0210647, 0.0442536, 0.00496386, 0.0411409, -0.0399442, 0.0578034, 0.033487, -0.0199134, 0.0294565, 0.0109653, 0.137327, 0.114496, 0.0852963, 0.0528852, 0.0333265, -0.0286487, 0.0138958, -0.0262105, -0.0747585, 0.00221894, -0.0253979, -0.0737644, 0.00760495, 0.0279975, 0.00562903, 0.0516985, 0.0471967, -0.0320119, 0.049187, -0.00275386, 0.0348446, -0.0417729, -0.00206001, -0.0181928, -0.0161565, 0.0565104, -0.0268478, 0.00364871, -0.00531229, -0.0817436, -0.0380604, -0.000923758, -0.00205338, 0.0122772, 0.0771402, 0.0458933, 0.116964, 0.00429481, 0.0363963, 0.0284422, -0.00354711, -0.0229521, 0.0426389, 0.0341664, 0.0299315, 0.00504488, 0.0911977, 0.0528494, 0.0314674, 0.0913617, -0.0811576, -0.0603814, 0.00546527, 0.0746927, -0.00377842, -0.000941302, 0.0213282, -0.0523205, -0.0428748, 0.0205713, -0.00673666, -0.00645952, 0.0402318, -0.0344315, 0.030866, 0.035359, 0.0183341, 0.0289669, 0.0329628, -0.0109447, 0.0290781, -0.0640889, -0.0722184};
const float b1_784_50_100_10[50] = {0.218538,
-0.0818302,
0.215443,
-0.0451745,
0.267523,
0.0632012,
0.165671,
0.116356,
-0.0611378,
0.161707,
0.355983,
0.0586314,
-0.0391396,
0.12118,
-0.0446131,
-0.0759978,
0.0338482,
-0.0987334,
0.112936,
0.0112239,
-0.0207698,
0.0426785,
-0.0259693,
0.000267272,
-0.252033,
0.0840249,
-0.0420427,
0.163982,
-0.0130942,
0.0902743,
0.0706134,
0.271607,
0.243815,
0.0864178,
0.0227696,
0.0128946,
-0.0902722,
-0.0432404,
-0.189795,
0.24727,
-0.09475,
0.186543,
0.0640969,
0.257735,
-0.138166,
-0.0988472,
0.0605396,
0.156591,
0.322156,
-0.172205};
const float A2_784_50_100_10[50*100] = {-0.175455, -0.293976, 0.318016, -0.151656, -0.0393713, -0.00461134, -0.178859, -0.310203, 0.0276088, 0.176881, 0.362546, 0.0497319, 0.233007, 0.224833, 0.0636611, 0.117962, -0.226283, 0.165094, -0.0431845, -0.729818, -0.377964, -0.0443502, -0.411905, 0.0325772, -0.302445, -0.208551, 0.101613, -0.114106, -0.0337661, 0.219904, -0.204991, -0.18365, 0.392892, 0.195892, -0.0100539, 0.155415, 0.207933, 0.12304, -0.175826, 0.261208, -0.406308, -0.0588545, -0.230627, 0.165918, 0.531086, -0.106141, 0.0274009, -0.110777, 0.044949, -0.415746,
0.321764, 0.0813942, -0.324444, -0.18093, 0.504546, 0.134388, -0.0184928, 0.270823, -0.437684, 0.188031, -0.0320427, 0.0548543, -0.411347, 0.220789, 0.210474, -0.35451, -0.0195304, -0.00321599, -0.150471, 0.058332, -0.314767, 0.0633076, -0.308902, 0.171077, -0.00676379, -0.0833939, -0.0189303, 0.113986, 0.390935, -0.116629, 0.296469, 0.16684, 0.111237, -0.105764, -0.00267267, -0.00515436, -0.262353, 0.122034, -0.0350691, 0.272863, 0.236366, -0.27311, 0.120122, 0.504058, -0.0532424, 0.09174, -0.790805, 0.408823, -0.0882364, -0.268561,
-0.165924, 0.41282, 0.116427, 0.0820804, -0.42419, 0.264557, 0.380027, 0.000770265, -0.199316, -0.221412, -0.184594, 0.106288, -0.00845559, 0.144428, -0.14906, -0.0720941, -0.0863194, -0.0434563, 0.488798, 0.0511027, 0.130317, 0.402152, -0.333239, 0.0922881, 0.0311324, -0.223509, 0.46525, 0.247841, 0.102637, -0.0646452, -0.424869, 0.00189807, 0.467155, 0.163316, -0.401208, -0.285412, -0.666564, -0.225724, 0.112632, -0.0160642, 0.323526, 0.110321, 0.421231, -0.018553, -0.219406, 0.108747, 0.024755, -0.618127, -0.152682, 0.18137,
-0.057004, 0.173692, 0.0759402, 0.0270256, 0.0662568, -0.127186, -0.0981965, -0.344979, -0.0227594, 0.338413, 0.177308, -0.0874403, -0.171648, -0.0800489, -0.00149776, -0.0675763, -0.197668, 0.402545, -0.418598, 0.0820693, 0.0638131, -0.568982, 0.202219, -0.130629, -0.116539, 0.104726, 0.511102, 0.087824, -0.0129515, -0.13937, 0.0427029, 0.151785, -0.490674, -0.0665571, 0.0290507, 0.499213, 0.00326566, 0.0813642, -0.286126, 0.0271925, -0.319132, 0.257663, 0.0488243, -0.281936, 0.306269, 0.244912, 0.158518, 0.073043, 0.127721, 0.320356,
-0.254382, -0.127439, -0.0470376, -0.0556333, 0.0820268, 0.0349734, -0.025835, 0.00798457, -0.0191344, 0.0301384, -0.137484, 0.00937974, -0.147462, 0.106531, -0.0427783, -0.153433, 0.159731, -0.357582, 0.168887, 0.207317, -0.0326907, 0.123924, -0.307938, -0.0515973, -0.335799, -0.120002, 0.0483707, -0.29799, 0.0991215, 0.167997, -0.149437, -0.440274, -0.193784, 0.136268, -0.326725, 0.24425, 0.271941, 0.00821122, 0.229358, -0.290758, -0.30295, 0.0298275, 0.0528532, -0.0900721, -0.140744, -0.280422, -0.0210441, -0.310766, 0.0161952, -0.0753114,
-0.327518, 0.0502309, -0.137334, 0.123903, -0.0270926, 0.160678, 0.446311, -0.173122, -0.031574, 0.172379, 0.169791, -0.114406, -0.0663642, -0.144086, -0.0754176, -0.153218, 0.0467757, -0.0121971, 0.0387359, -0.0690412, 0.214273, -0.00774688, 0.301584, -0.233271, 0.0885566, 0.238181, 0.53805, 0.0742268, 0.375143, 0.250842, 0.380343, -0.037327, -0.549007, 0.109893, -0.1662, 0.542354, 0.117883, -0.00699593, 0.282784, -0.10394, 0.532937, -0.180045, 0.158484, 0.107107, -0.451409, 0.0641241, -0.340973, -0.0958565, -0.0764323, -0.0700275,
-0.0189349, -0.0984679, 0.374396, -0.13921, 0.0830714, -0.375435, 0.238955, -0.0468309, 0.0178922, 0.0258033, -0.215603, -0.382507, -0.0788635, -0.0183968, -0.44169, 0.153476, 0.219754, 0.0593345, -0.00734973, 0.164104, -0.0262202, -0.00460349, -0.0760884, -0.058723, 0.512842, -0.223908, -0.0188527, 0.140737, 0.00677586, 0.286047, 0.446562, -0.342424, 0.105343, -0.285309, -0.175302, 0.311583, 0.0555769, -0.840213, -0.074361, -0.241739, -0.00155905, -0.276971, -0.474862, 0.354622, -0.206202, 0.0368528, -0.0760068, 0.298312, 0.471707, -0.212123,
0.378897, -0.331794, -0.204538, -0.37117, 0.246982, 0.188151, -0.069268, -0.186075, 0.0378463, 0.13146, 0.213684, -0.243273, -0.0338334, 0.277643, 0.225859, -0.115811, -0.25452, -0.234904, 0.188002, -0.239741, -0.123389, -0.40247, 0.383842, -0.214197, -0.245187, -0.244019, 0.457998, 0.340602, -0.112374, -0.0397995, -0.400536, 0.108189, 0.0917477, 0.223992, 0.409197, -0.517564, 0.354997, -0.598782, -0.160365, 0.191419, -0.362431, -0.449914, 0.263042, -0.0663768, -0.113612, 0.362129, 0.214244, -0.0219739, 0.486704, 0.0128403,
-0.147662, -0.188396, 0.0369704, 0.220876, 0.248988, 0.45581, -0.222321, -0.0599798, -0.264577, -0.0745797, 0.0877624, 0.00809799, -0.0199525, -0.175329, 0.0150597, 0.143835, 0.0120718, -0.339219, -0.21584, 0.0149736, -0.441559, 0.448343, 0.243962, 0.0386536, -0.00536034, -0.368334, 0.163434, 0.372769, -0.0786742, -0.0411454, 0.272669, 0.157305, 0.344083, -0.103516, -0.2293, 0.224937, 0.219264, 0.0377261, -0.00607486, 0.333034, -0.0305675, 0.111438, 0.0586302, 0.14912, 0.0852008, -0.180227, -0.404872, 0.32886, 0.229494, -0.139593,
0.26675, 0.115469, -0.0742625, 0.281904, -0.0902726, 0.27744, -0.381145, 0.0482087, 0.130053, -0.511826, -0.18396, 0.0606011, 0.121019, 0.104978, 0.292472, -0.120892, -0.104601, -0.102614, 0.379405, 0.348482, -0.268129, 0.0778453, -0.273832, -0.108785, 0.0729631, 0.0355629, -0.336964, 0.0172041, -0.375309, -0.0517382, 0.250062, 0.0693797, 0.174832, 0.0842201, 0.312287, -0.182019, 0.0731344, 0.0447809, -0.186919, -0.10347, 0.123206, 0.185342, -0.0951055, 0.460272, -0.088481, -0.503838, -0.163464, 0.345768, -0.546018, -0.0479961,
-0.129226, -0.323228, 0.161667, 0.276053, -0.365691, -0.0166075, -0.0453996, -0.161973, -0.242416, -0.114668, -0.297723, 0.0536009, 0.494102, 0.0651916, -0.0924748, 0.0735499, 0.534851, 0.166616, 0.179833, 0.0886982, 0.232867, -0.128394, 0.240811, -0.4345, 0.290838, 0.309182, -0.0439566, -0.0724348, -0.0679416, -0.255058, -0.144203, 0.269881, -0.0426544, -0.161227, 0.138333, -0.388103, -0.128957, 0.0243149, 0.0135125, -0.200742, 0.232974, 0.0556599, -0.106365, -0.16635, -0.147422, 0.0208051, -0.0259793, 0.0318953, -0.277147, -0.0782972,
0.171452, -0.205781, 0.125519, 0.131866, -0.206927, -0.628686, 0.238323, -0.0152174, -0.173185, 0.187188, 0.0787388, 0.0270604, -0.179083, 0.0449506, 0.245307, 0.214992, -0.0279475, 0.332125, 0.0526591, -0.129362, -0.237913, -0.0966614, 0.111774, 0.0772089, 0.153195, -0.170722, 0.0269739, -0.147744, 0.217757, 0.178321, 0.195677, -0.00587545, -0.171298, -0.0448089, -0.213224, -0.22093, -0.296356, 0.242314, -0.351755, -0.289071, -0.0473391, -0.117936, -0.193005, 0.250974, 0.084903, 0.174126, 0.0923216, 0.294864, -0.085049, 0.0881481,
-0.119941, 0.0843724, 0.135508, -0.137177, -0.153192, -0.0610851, 0.125023, -0.306418, 0.267181, -0.0519834, 0.0547241, 0.027026, 0.0171217, -0.0116779, 0.0645303, 0.255193, -0.137795, -0.121506, -0.0567107, 0.0431322, -0.151795, 0.239847, -0.269779, -0.274997, -0.137737, -0.529916, 0.170022, 0.0710102, 0.283984, 0.0152416, -0.166271, 0.181215, 0.150461, 0.337413, -0.184489, -0.258345, -0.107035, 0.0997892, 0.126535, 0.253053, -0.284243, -0.366584, 0.422668, 0.0417226, -0.327597, 0.0386271, 0.0995099, -0.137192, 0.11505, -0.266087,
-0.109356, -0.254796, -0.173744, 0.152466, -0.0596349, 0.114118, 0.00674579, -0.200342, 0.0285164, -0.289599, 0.010259, -0.119598, -0.277496, -0.108537, 0.0488485, -0.243682, 0.165035, 0.0590268, 0.725135, -0.20878, -0.350817, 0.0456734, 0.144596, 0.342353, -0.116876, -0.0951692, 0.27497, 0.0662013, -0.177527, -0.128623, -0.124785, 0.0854508, -0.086349, -0.0721797, -0.0567853, 0.154932, -0.0748646, 0.000833615, 0.0122391, 0.106681, 0.169603, -0.18255, 0.348804, -0.0911679, -0.551204, -0.141034, 0.146957, -0.511464, -0.225216, 0.50407,
0.415417, 0.245724, -0.395238, 0.163974, -0.229094, -0.0683595, 0.102903, 0.423006, 0.0758649, -0.0266835, -0.195783, 0.407822, 0.243551, -0.0271841, 0.272174, -0.153187, 0.475857, -0.202269, -0.13527, 0.264828, 0.129769, -0.0109811, 0.205445, -0.0830238, 0.162349, -0.0350706, -0.103736, 0.483799, -0.406584, -0.323172, 0.208962, 0.013732, -0.0588936, 0.132342, -0.178606, 0.3695, -0.0224082, -0.0600355, -0.163483, 0.189966, -0.15401, 0.168446, -0.0587165, 0.0336813, 0.108811, -0.160625, 0.127315, -0.133213, -0.153567, -0.0602731,
0.10055, 0.398001, -0.153802, -0.175655, 0.00923353, -0.300369, -0.19555, 0.147248, 0.596748, -0.595238, -0.0855671, -0.167929, 0.0874382, 0.18451, 0.0756152, -0.00256737, -0.254922, -0.221572, 0.250365, 0.137504, -0.288059, -0.43115, -0.124077, -0.071445, -0.192656, 0.15721, 0.0208882, 0.162407, -0.364758, -0.155373, -0.221699, 0.00451548, -0.0436929, -0.171985, 0.122533, 0.333847, 0.157325, -0.284461, -0.0059809, 0.510959, 0.258199, 0.0464213, 0.355367, 0.590122, -0.171992, -0.116308, 0.336396, -0.205845, -0.0523912, 0.266596,
-0.143627, -0.0258083, -0.295361, 0.126508, -0.183944, 0.332124, 0.0407189, -0.0361466, 0.394124, 0.101627, 0.334176, 0.0816544, 0.197393, 0.222674, -0.22404, -0.0861106, 0.220839, -0.169824, -0.270399, -0.0110407, 0.0390839, -0.270558, 0.259294, -0.248362, 0.228898, 0.07405, -0.0906725, -0.0555465, 0.138233, 0.127975, -0.00579942, 0.0163442, -0.204957, -0.233734, 0.0969084, 0.158472, -0.0890319, -0.158388, 0.157692, 0.0479397, 0.238401, -0.0929189, -0.113311, 0.0146433, -0.348656, 0.159679, -0.122727, -0.100403, -0.0820656, 0.0190995,
0.076583, -0.230287, -0.286261, 0.16209, 0.238668, 0.217741, 0.25019, -0.15291, -0.345958, -0.0197897, 0.145991, -0.0218791, -0.0059577, 0.092265, -0.206561, -0.311325, 0.0666386, 0.0827525, -0.07679, 0.236921, -0.197751, -0.435563, 0.512768, -0.215065, -0.391946, -0.086325, -0.139361, -0.116309, 0.200154, -0.136367, -0.0494032, -0.164954, -0.265427, 0.58064, 0.0923568, -0.280111, 0.120705, -0.144803, 0.219717, 0.0265194, -0.0491357, 0.158084, 0.285824, 0.403338, 0.187328, -0.472212, 0.0339336, 0.0143827, -0.231761, 0.538419,
-0.250523, 0.302019, -0.0601075, 0.0134715, -0.23645, -0.130205, 0.0772984, 0.459065, -0.0549053, -0.199869, 0.10562, -0.244679, -0.320649, 0.30364, -0.126392, 0.0171487, 0.0117921, -0.280472, 0.039035, -0.321585, -0.0953917, -0.0310434, -0.0745732, 0.0579614, 0.197131, 0.172693, 0.0577415, 0.325408, 0.0435626, 0.266957, -0.217319, 0.372271, -0.0185126, -0.0596138, -0.244562, 0.222915, -0.240414, -0.252173, -0.405322, 0.00275676, 0.131231, 0.283707, -0.137077, -0.0539151, -0.192689, 0.131046, -0.182578, -0.170376, -0.206722, 0.311301,
-0.109433, -0.427433, -0.211512, -0.265665, -0.0782093, 0.0659582, 0.0315437, 0.018235, 0.374439, -0.0476798, -0.00342235, 0.2763, -0.159506, 0.168664, 0.238925, 0.274767, -0.170239, -0.329238, 0.45304, -0.152711, 0.252258, 0.117111, -0.23384, -0.03306, 0.0012576, 0.248704, 0.189198, 0.326908, -0.00935669, 0.0291428, -0.117068, -0.358859, -0.135603, -0.303607, 0.115228, -0.140274, 0.472804, -0.502632, -0.184323, 0.200463, 0.216406, -0.829954, 0.286335, 0.151763, 0.059349, -0.350928, 0.0223677, 0.273573, -0.00647629, 0.185601,
-0.568157, 0.10008, -0.160385, 0.140358, 0.175999, -0.344909, 0.135903, 0.111892, -0.159471, 0.126388, 0.373881, 0.258223, 0.0192429, -0.0105744, -0.179448, -0.20905, -0.0612837, -0.0851528, -0.0854678, 0.635013, 0.510248, 0.00756924, -0.36293, 0.110069, 0.00770984, 0.29458, -0.099722, -0.166942, 0.339315, 0.584225, -0.22682, -0.0734596, -0.0116891, -0.134356, -0.0772986, -0.291153, 0.172844, -0.129722, -0.138075, 0.190765, -0.347024, 0.422225, -0.30828, -0.162967, 0.312651, 0.188337, -0.183834, 0.243607, 0.0776634, 0.0108387,
-0.16405, 0.189874, 0.171467, -0.424165, 0.180441, -0.485437, 0.0189761, -0.236499, 0.191672, 0.0950615, 0.409872, 0.371432, -0.0311821, -0.105323, 0.412925, 0.058252, -0.115487, -0.250469, -0.143305, -0.482528, 0.289187, -0.157648, -0.176796, 0.57583, 0.0206894, -0.275016, -0.302763, 0.0560096, -0.523559, -0.414867, 0.129555, -0.236389, 0.439851, 0.469646, -0.0978625, -0.152325, -0.146398, -0.108798, -0.275108, -0.180169, -0.140619, -0.170183, -0.0277524, 0.277814, -0.0291247, -0.225271, -0.0844556, -0.324774, 0.0923593, 0.253487,
-0.150436, -0.096532, 0.0519241, 0.16799, -0.00774718, 0.186175, 0.0185231, 0.230295, -0.0365894, -0.0869466, 0.11062, -0.229845, -0.200942, 0.173431, -0.316721, -0.22026, -0.151805, -0.108839, -0.252061, -0.455273, 0.234353, 0.108628, 0.361883, -0.381773, 0.353228, -0.25761, -0.296492, -0.0794362, 0.0321284, 0.052476, 0.405224, -0.204497, 0.120422, -0.146181, -0.067199, -0.0764315, 0.0836748, 0.33933, -0.179866, 0.0643576, -0.0894672, 0.378483, 0.0719407, -0.185123, 0.0757793, 0.23436, 0.0192892, -0.29183, -0.153791, 0.439257,
0.373945, 0.640568, -0.108671, -0.255804, 0.170516, -0.00981288, 0.396277, 0.496158, -0.210282, -0.197288, -0.185936, -0.203707, 0.295416, 0.26841, 0.548198, 0.419998, -0.147059, -0.186909, 0.209688, 0.118965, 0.386387, -0.195116, 0.43143, -0.0689543, 0.135718, 0.157494, -0.0595895, 0.0421177, -0.327349, -0.133381, 0.488936, -0.0599776, -0.0682263, 0.527324, -0.13106, -0.422339, 0.321356, -0.53865, 0.0363451, -0.161673, 0.356362, 0.206777, 0.175077, -0.0179786, -0.466048, 0.0285819, 0.122571, 0.302163, -0.102306, -0.11086,
0.49448, 0.0988076, 0.307554, 0.0641247, 0.0898133, 0.00664153, 0.0603635, -0.618272, -0.137148, 0.0243815, 0.00891731, 0.107426, 0.225592, 0.307261, -0.125693, 0.51968, -0.0123677, -0.223665, -0.389123, -0.0798469, 0.38663, -0.501307, 0.109474, -0.361667, 0.24476, -0.176835, 0.227196, -0.0304134, -0.0963365, -0.0582072, -0.0847177, 0.276099, -0.60283, 0.0355123, -0.273915, 0.113056, 0.240576, 0.224812, 0.052939, 0.00917401, 0.345261, 0.149262, 0.0649975, -0.57981, 0.109989, -0.293971, 0.40385, 0.0294828, 0.398981, 0.372256,
-0.0864665, 0.598987, 0.293228, -0.220945, 0.240176, -0.0328342, 0.378422, -0.204633, 0.289624, 0.137368, -0.197858, 0.0288993, 0.00684938, -0.0486223, -0.231712, -0.432702, -0.00945046, 0.0227411, -0.13266, -0.219225, 0.439919, -0.229437, -0.0627793, 0.759653, -0.237989, -0.00759216, -0.156697, -0.320087, 0.0556368, 0.0529567, -0.0631109, 0.149057, 0.0608313, -0.202495, 0.0577282, 0.0294822, -0.321025, 0.185595, 0.11858, 0.0382324, 0.0901447, -0.0680036, -0.277582, -0.00337272, -0.204964, 0.0515508, 0.143141, -0.30633, -0.021467, 0.124679,
0.205498, 0.0329192, -0.163119, 0.277509, -0.0162845, 0.191546, 0.0554228, 0.199008, 0.332809, -0.158635, 0.0504906, 0.257878, 0.428637, 0.0924075, 0.117883, 0.184878, -0.0854706, 0.0884817, -0.0977081, 0.22588, -0.00737334, -0.443852, -0.00204972, -0.0728863, 0.178986, 0.0195861, -0.176882, 0.187427, 0.118774, 0.00646191, 0.282523, 0.0778291, 0.213488, -0.0551256, 0.440373, 0.106484, 0.0155259, -0.208073, -0.251288, 0.223268, 0.548728, 0.00585455, -0.0349648, 0.0690253, -0.144603, 0.0429293, -0.0249821, -0.128973, -0.380398, -0.0673888,
-0.039836, -0.330735, -0.249935, 0.110067, -0.133809, -0.346166, -0.108446, 0.0280185, -0.454854, 0.165316, -0.122773, -0.45823, 0.126053, 0.0812272, -0.338986, 0.106247, -0.117399, -0.0813911, 0.221088, -0.167814, 0.118847, 0.187331, 0.0441809, -0.127152, 0.357244, 0.0481986, -0.262112, -0.289869, -0.512659, -0.378817, -0.170664, 0.131322, -0.0572839, 0.0750256, 0.315727, -0.0576899, 0.074832, -0.0836412, 0.0672202, 0.00846748, 0.157569, 0.178205, -0.14086, -0.203903, -0.141404, 0.440483, 0.152246, 0.473928, -0.31532, -0.0483699,
-0.388536, 0.134336, 0.10591, -0.166293, -0.0290079, 0.199163, 0.142507, -0.0818868, 0.0929752, 0.24087, 0.0349401, 0.17417, 0.13184, -0.206532, -0.108102, -0.186813, -0.252469, 0.176358, -0.117521, -0.108663, 0.100435, 0.0482037, -0.175328, 0.0631119, 0.101875, 0.0509075, 0.111034, -0.119711, -0.00738296, -0.13636, -0.0195862, 0.0482351, -0.0913055, 0.435252, 0.124389, -0.116478, -0.451627, 0.141245, 0.0289437, -0.0465174, -0.0271956, -0.181373, 0.0860888, -0.187427, 0.477266, 0.177019, 0.384473, 0.0775786, 0.0572065, -0.176723,
0.125959, -0.197126, 0.235661, -0.151168, 0.184438, 0.0515587, -0.153591, 0.566592, -0.178498, -0.161249, -0.531292, 0.377367, 0.373341, 0.0834017, -0.000340603, 0.250275, 0.179221, -0.168936, 0.295865, -0.354705, -0.0525413, 0.106386, -0.366458, -0.114946, 0.122167, 0.23024, -0.234351, -0.0109004, -0.130789, -0.0501187, 0.255344, 0.40876, -0.313626, 0.00568022, -0.0977948, 0.0812266, -0.199234, 0.10035, 0.185207, 0.239514, 0.0141669, -0.0285976, 0.135016, -0.567086, -0.380215, 0.00307179, -0.283622, 0.139925, -0.0511121, -0.13807,
0.264409, 0.40174, 0.395282, -0.0616823, -0.0282753, -0.178911, -0.387688, -0.294821, -0.350047, 0.0206791, 0.218261, -0.0601489, 0.109353, 0.36678, 0.204401, -0.255308, 0.148769, -0.276104, -0.322105, -0.0251356, 0.192711, -0.0131955, -0.0423673, 0.138706, 0.0240261, 0.0184556, -0.163533, -0.169625, 0.12769, 0.239501, -0.146062, 0.202634, 0.169407, 0.0786315, -0.15876, 0.0691563, 0.11619, 0.232967, -0.528634, -0.0965453, 0.120851, -0.00110742, 0.0177496, -0.0484216, 0.168437, -0.178118, 0.020312, -0.0839082, 0.220325, 0.190364,
0.0235449, -0.115662, -0.00910864, -0.350301, 0.104302, -0.0476059, -0.198099, -0.0399368, -0.2315, -0.143724, -0.0420141, -0.182272, 0.00520617, -0.173982, 0.124394, 0.162481, 0.160974, -0.282748, -0.289948, -0.255933, -0.0970862, -0.194407, -0.143363, -0.159926, 0.0317464, 0.143642, 0.0121452, 0.179998, -0.094527, -0.317138, 0.268603, 0.128936, -0.154342, 0.17403, 0.17095, 0.0514713, 0.211197, -0.032666, -0.0834581, 0.142454, 0.0283483, 0.124168, -0.039579, -0.227613, 0.267225, -0.252167, 0.0528094, -0.132231, 0.275461, 0.101965,
-0.121212, 0.136578, 0.256583, 0.117289, -0.239628, 0.018542, 0.398082, -0.544119, -0.396724, 0.207233, -0.227763, -0.192136, 0.396861, -0.19079, 0.057405, 0.379759, -0.466089, 0.316501, 0.0220504, -0.030327, -0.258748, 0.160564, -0.125071, -0.293756, -0.0106934, -0.18295, 0.108666, 0.291269, 0.155166, -0.0219548, -0.382877, -0.176323, 0.581762, -0.12268, -0.141776, 0.0742197, 0.262427, 0.124456, -0.00724341, 0.0277901, -0.122106, -0.100531, -0.068482, 0.363303, 0.214981, -0.0850307, 0.0949917, -0.0259905, 0.0933907, -0.303866,
0.135455, -0.0282519, 0.198924, 0.0951881, -0.0300382, -0.281076, -0.197389, -0.138113, -0.522443, -0.185719, 0.0854313, 0.126407, 0.297192, -0.343954, -0.194024, 0.0667244, 0.0254111, 0.0449998, -0.261651, -0.180718, 0.141287, -0.57683, 0.351484, -0.116808, -0.131185, 0.056921, 0.252824, -0.178365, 0.173347, 0.291532, -0.017195, 0.0809352, -0.0383276, 0.335757, -0.0807451, 0.155905, 0.184443, 0.0243837, 0.453697, -0.703513, 0.226774, -0.236116, -0.104966, 0.380042, 0.0546915, 0.106429, -0.180413, -0.286662, 0.0147047, 0.220571,
-0.122234, -0.0359672, -0.0362668, -0.0468298, -0.317119, 0.275484, 0.544095, -0.335821, -0.32861, 0.222667, -0.173665, -0.0840749, -0.0431338, -0.0380063, -0.0575686, 0.480836, -0.0110642, 0.124015, 0.646741, 0.396856, 0.437217, 0.0559182, -0.393014, 0.555786, 0.32359, -0.185109, 0.323325, -0.073596, 0.208928, 0.3866, 0.058035, 0.140396, -0.270198, -0.650438, -0.311244, -0.197246, -0.00954091, 0.202344, 0.403496, -0.448572, 0.135441, -0.0117374, -0.00909461, 0.131776, 0.150303, 0.0146197, -0.613949, -0.349653, -0.259008, -0.179737,
-0.122351, 0.21427, 0.0873343, 0.10414, 0.309394, 0.122383, 0.066731, -0.247932, -0.0460332, -0.0182686, 0.0744769, 0.168277, 0.262858, -0.185866, -0.0653523, -0.297187, 0.0679598, 0.0851757, 0.209756, -0.229114, 0.00797532, 0.370032, -0.0586404, 0.12437, -0.293476, -0.157609, -0.0638744, 0.109729, 0.243179, 0.0720119, -0.382265, -0.211773, -0.0254895, -0.00326863, 0.188023, 0.146613, -0.435303, 0.284858, -0.0483054, -0.0396322, 0.343162, -0.100247, 0.0146398, 0.37966, -0.041564, -0.0784716, -0.0860112, 0.327845, 0.110165, -0.237587,
0.132692, -0.1065, 0.0440823, -0.0688676, -0.104091, 0.495013, 0.112974, 0.354181, -0.0574531, 0.323939, -0.0753081, 0.171751, -0.16234, -0.167006, -0.150904, 0.251506, -0.17374, -0.226406, 0.171084, 0.627962, -0.419803, 0.226523, 0.0235582, -0.0106793, -0.00425304, -0.239929, 0.0484037, 0.443478, 0.192679, 0.012751, 0.11696, 0.501095, 0.170405, -0.239315, 0.0181457, 0.0576528, 0.791633, 0.0977559, 0.370042, 0.126374, -0.176997, -0.125141, 0.162525, -0.164746, -0.433076, -0.304305, 0.211846, 0.1159, 0.413372, 0.101175,
-0.0132055, 0.310464, -0.0983307, 0.495712, 0.355395, -0.131823, -0.314798, 0.0570127, 0.0510248, 0.0364423, 0.459788, -0.150255, 0.298578, 0.268132, 0.0320439, -0.252644, 0.0991225, 0.300502, 0.247952, 0.0131012, -0.187172, -0.101085, 0.370615, 0.234388, -0.34926, 0.0103612, -0.0942326, 0.107097, 0.0724259, 0.15009, -0.152573, -0.28635, -0.0793188, 0.126494, -0.0581325, -0.328409, 0.200628, -0.00615556, -0.0868104, -0.0602124, 0.0226288, 0.24554, -0.367087, 0.0709632, 0.0670751, -0.0600993, -0.0124319, 0.214643, 0.376049, -0.317115,
0.096311, -0.211175, -0.223955, -0.144095, -0.311743, 0.112233, 0.0704024, -0.583454, 0.265729, 0.113082, 0.0808319, 0.206358, -0.365397, -0.0901427, -0.21006, -0.150812, 0.00749015, 0.0892947, 0.218596, -0.218702, -0.154611, -0.103981, 0.184652, 0.088746, 0.0972751, -0.320167, -0.21259, 0.473599, 0.0961833, -0.0996279, -0.266939, 0.136033, 0.232689, -0.279483, 0.188445, 0.1483, 0.268315, 0.199701, -0.109926, -0.289917, -0.29168, -0.36194, -0.116717, 0.283556, 0.0855679, 0.110107, 0.0688532, 0.0626617, 0.262346, -0.0927299,
-0.257576, -0.0439589, -0.651882, -0.0675658, 0.112019, -0.23077, -0.0875371, 0.156113, -0.209352, 0.0278337, 0.346174, 0.0961949, -0.152275, 0.225565, 0.117575, -0.126503, 0.118203, 0.00329636, -0.280206, 0.11925, 0.0259273, -0.177521, 0.0077593, 0.0672278, -0.0198364, 0.0818389, 0.143595, -0.148082, -0.0531971, 0.13339, -0.107034, -0.0680362, 0.151056, 0.108474, 0.100488, -0.285746, 0.200712, 0.0325334, -0.0291342, -0.100781, 0.492367, 0.108677, 0.271545, -0.0830261, 0.0282605, 0.311613, 0.104887, -0.0787139, -0.046707, -0.0798262,
0.0213459, 0.182015, -0.388427, -0.181924, 0.193578, 0.272407, -0.0688687, 0.298808, -0.0223218, 0.196165, 0.0472421, -0.0374587, -0.193415, -0.0714372, -0.130497, -0.14213, 0.300529, -0.102087, 0.444317, -0.191896, -0.168679, 0.02485, -0.169161, 0.0165031, 0.357161, -0.0635884, -0.0890691, 0.356853, 0.163718, -0.1064, -0.0699472, -0.184842, 0.127073, -0.51973, 0.0392263, 0.0359413, 0.14457, 0.0219226, 0.309797, 0.319145, -0.0173205, -0.0541471, 0.242755, -0.149603, -0.154846, 0.309167, -0.280732, 0.253598, -0.0353921, -0.115099,
0.169674, 0.171157, -0.0170186, 0.0687082, 0.183871, 0.206141, 0.0287564, -0.214926, -0.0167571, 0.140653, 0.0691806, 0.113209, 0.205525, 0.089226, -0.162849, 0.0918783, 0.159888, -0.150754, 0.00303744, 0.175448, -0.507641, -0.068544, 0.178191, 0.153115, -0.159772, 0.200235, -0.353666, 0.0752684, -0.152818, -0.355971, -0.346662, -0.146152, 0.0242941, 0.103753, -0.00744994, 0.307805, -0.244511, 0.0451421, -0.408293, -0.202263, 0.321815, 0.223119, -0.332025, -0.158553, -0.106676, 0.119983, -0.264489, -0.0741159, -0.0424388, 0.0657039,
0.426955, -0.22484, -0.22023, 0.181288, 0.0468605, 0.384749, 0.712039, -0.0435361, -0.0661311, 0.0224817, 0.115187, -0.31145, -0.114272, -0.012773, -0.108699, -0.222373, 0.233683, -0.0642169, 0.17849, 0.121115, -0.364191, -0.25453, -0.159348, 0.282743, 0.148977, 0.0504889, 0.135591, 0.104432, -0.32787, -0.250166, 0.0550138, 0.105062, -0.145158, -0.0457588, 0.481518, -0.252799, -0.448835, 0.229486, -0.0315589, 0.0179455, -0.0482254, 0.267593, 0.168687, 0.114685, -0.0356986, 0.135345, 0.371963, 0.344858, -0.269625, 0.303022,
-0.044811, -0.0467869, 0.17509, -0.0635632, -0.090318, 0.158793, 0.22285, -0.21246, 0.193296, -0.68862, -0.162003, 0.0436935, -0.0177823, 0.0670962, -0.110041, 0.306051, 0.0977553, -0.231703, 0.153648, -0.263682, 0.256867, 0.241225, 0.385441, -0.426818, -0.0612665, 0.040941, 0.081901, -0.0320711, 0.461256, 0.269698, 0.311053, -0.233515, -0.124224, -0.364158, -0.0543253, 0.0167994, -0.160326, -0.22497, 0.300147, 0.134021, -0.00844574, 0.104559, 0.480305, -0.0701205, -0.0464406, 0.273069, 0.083661, 0.0521291, -0.0617833, -0.0606013,
-0.232887, 0.200822, 0.0586298, 0.190014, -0.199407, -0.20937, 0.20147, 0.343544, 0.194722, 0.0172154, 0.0793238, 0.262282, 0.458582, -0.202605, 0.0675346, -0.104106, 0.063639, 0.272073, -0.271918, 0.12675, -0.244128, 0.0292521, 0.257257, -0.378682, -0.144187, -0.125967, -0.0421157, 0.121555, 0.0249044, -0.306071, 0.0963183, 0.213253, -0.509621, 0.149417, 0.226139, 0.0537705, -0.156099, 0.0160953, -0.328728, -0.03587, -0.33405, 0.19288, -0.00780148, -0.324296, -0.338463, -0.417879, 0.0799854, 0.165644, -0.187552, -0.455932,
0.260461, 0.0962912, 0.776426, 0.0427997, 0.13339, 0.159359, 0.383781, -0.052869, -0.50347, 0.187509, -0.0331335, 0.366177, -0.118419, -0.0930838, -0.0578887, -0.31408, 0.140883, -0.0760186, 0.0438593, -0.210361, -0.213113, -0.123824, 0.145159, -0.137946, -0.159526, -0.545906, 0.0115416, -0.0560225, -0.027967, -0.015057, 0.214713, -0.187638, -0.0471646, 0.0908318, -0.556118, -0.153543, 0.114658, -0.139266, 0.564669, -0.0971874, 0.229257, 0.335778, 0.0558783, 0.457071, -0.714325, -0.515217, -0.0508845, -0.0187786, 0.528432, -0.300053,
0.0741631, -0.476445, -0.317071, -0.325235, 0.100488, -0.21501, 0.104193, -0.039998, -0.0828901, -0.207759, 0.341794, -0.11295, -0.115855, 0.180222, 0.451885, -0.209658, -0.134484, -0.587722, -0.401368, 0.0244996, -0.245095, 0.230607, -0.0873321, 0.11737, 0.142293, -0.0854159, -0.241385, -0.0838352, 0.092051, 0.0962687, 0.4699, 0.000355053, 0.151593, -0.080595, -0.0674818, 0.074821, 0.088976, 0.452037, 0.0683122, -0.127283, -0.0913017, 0.217501, -0.314492, 0.332221, 0.241906, -0.259224, -0.146379, 0.352483, 0.380571, 0.238438,
-0.113251, 0.102549, 0.0646054, 0.165005, -0.178693, -0.0238323, -0.149253, 0.024453, -0.0541056, -0.200937, 0.521203, -0.0511851, 0.234137, 0.196944, -0.0502169, 0.145122, -0.119663, 0.0747254, 0.0373, -0.40372, 0.0215219, -0.242227, -0.0209667, 0.285988, 0.178451, -0.0340464, -0.0855403, -0.181231, -0.0781211, 0.0968497, -0.554983, 0.0252513, -0.249758, -0.0429948, 0.00822043, -0.333312, -0.123746, -0.231049, -0.0251037, 0.243443, -0.111168, -0.304028, 0.096741, 0.106802, 0.0402658, -0.0674244, -0.151003, 0.0753101, -0.0435916, 0.185664,
0.398633, 0.0612745, -0.365297, 0.0719209, 0.0715575, 0.122868, 0.245273, -0.120042, -0.104274, 0.000881665, -0.173271, -0.0997349, 0.115613, 0.200992, 0.0132677, -0.206498, 0.209946, -0.0326566, 0.128945, 0.313928, -0.281574, -0.0268157, -0.32557, -0.531187, -0.125953, -0.210934, 0.446671, -0.314724, 0.145356, -0.0846686, 0.0521023, 0.225028, -0.327266, 0.0481809, 0.0897437, -0.294348, -0.286359, 0.0615985, 0.481527, -0.0888406, -0.244246, 0.376141, -0.0494533, 0.129242, -0.194373, 0.200541, -0.0915433, 0.0883187, -0.0872994, 0.233778,
0.273172, -0.404731, -0.117931, -0.31868, 0.0836509, 0.0209254, -0.126088, -0.0737087, 0.287037, -0.0444331, 0.221342, 0.0186178, 0.453588, 0.0421993, 0.0797395, -0.00863196, 0.109163, -0.118472, 0.202006, -0.0503929, 0.231606, 0.24329, 0.11326, -0.427039, -0.284754, 0.0534385, 0.132363, 0.0572611, -0.408886, 0.106371, -0.0635601, 0.0244427, -0.112113, -0.151362, 0.0537116, -0.136117, 0.535331, -0.276963, -0.168707, -0.0557313, 0.0174031, 0.133032, -0.105513, -0.271093, -0.137195, 0.221332, 0.569504, 0.0613002, 0.287601, -0.0502074,
0.43896, 0.337268, 0.114708, -0.265131, 0.103637, 0.243211, -0.385149, 0.223239, 0.070134, 0.301919, 0.050779, -0.0674394, -0.110205, 0.27336, 0.421576, -0.269549, 0.288645, 0.0789262, -0.0234645, -0.251917, 0.0383399, -0.183943, 0.334121, -0.244952, -0.039595, 0.365336, 0.145591, 0.353241, -0.34838, -0.057078, 0.0754137, 0.0412508, 0.125804, 0.0643065, -0.339002, 0.0377021, 0.0229244, 0.24408, -0.465002, 0.0543794, 0.136358, -0.127306, 0.118881, -0.058584, 0.0499604, -0.191468, 0.150416, 0.18357, -0.188534, 0.487895,
-0.414309, 0.212409, -0.140231, 0.254634, 0.0175767, -0.00368651, -0.0361705, -0.260305, -0.452095, -0.224468, -0.235316, -0.496307, 0.294111, 0.0647559, 0.108897, -0.0935088, 0.0902764, -0.215197, -0.0728316, 0.0997335, 0.0625289, -0.277986, -0.138092, 0.122039, 0.0142308, -0.458403, -0.0862323, -0.27057, -0.0822427, -0.216167, 0.203662, -0.0583846, 0.263065, 0.0234095, 0.349793, 0.0592339, -0.0949826, -0.238167, 0.0175067, -0.242685, -0.0269424, 0.0941241, 0.351725, -0.261964, 0.300457, 0.164925, 0.213021, 0.198327, 0.0481907, -0.31006,
-0.050587, -0.550842, -0.289666, 0.391132, -0.044861, -0.313536, 0.212718, 0.0490731, 0.0307476, -0.41789, -0.104682, 0.158971, -0.310187, -0.0235844, 0.0212309, 0.109956, 0.084809, 0.230745, 0.0366428, 0.0408745, -0.00276295, -0.0179664, -0.105245, -0.0704551, 0.105856, -0.194177, -0.0587313, 0.217488, 0.181016, -0.141568, 0.12158, -0.149718, -0.135352, 0.0924077, -0.345316, 0.0101195, -0.0770423, -0.150093, -0.0113121, 0.234944, 0.129659, 0.254103, -0.135825, 0.207292, 0.033672, 0.25829, -0.0932282, 0.198221, -0.211941, -0.643094,
-0.0957048, -0.11369, 0.175802, 0.0909165, 0.271389, 0.0505918, -0.136943, -0.0690783, 0.178633, -0.063606, 0.0923332, 0.0335, -0.0344651, 0.139054, -0.191626, -0.16679, -0.0524293, -0.228936, -0.157011, 0.0189244, -0.0997487, -0.0616641, -0.182358, 0.183753, -0.366495, 0.063002, 0.211315, 0.0740896, 0.047778, -0.239268, -0.34423, -0.087983, -0.245606, -0.349658, 0.0623435, 0.233082, 0.215557, -0.0769524, -0.221101, 0.138692, 0.0126747, -0.120293, -0.0467883, -0.285074, 0.0123969, 0.0648296, -0.0982086, 0.0836576, 0.379907, 0.178717,
0.194038, 0.0351474, -0.269893, -0.246821, -0.14455, -0.265338, -0.180832, 0.226059, 0.171261, -0.253732, 0.171769, 0.0346105, 0.485948, 0.31442, 0.0935427, -0.100665, 0.205222, -0.0399046, 0.349409, -0.0716418, 0.125081, -0.373532, -0.177498, 0.0330842, 0.184717, 0.212061, -0.139577, 0.392011, 0.0241824, 0.171489, -0.239158, 0.146507, 0.128248, 0.307167, -0.217049, -0.24485, -0.0515992, -0.245669, -0.0561256, 0.291539, 0.0645169, 0.0128797, -0.112004, -0.0982853, -0.0602578, 0.131705, 0.274981, 0.196905, -0.438911, 0.0746239,
0.0365943, -0.167571, -0.157549, 0.159325, -0.33036, 0.239859, -0.0137265, -0.170143, -0.188467, 0.114305, 0.110699, 0.0719949, -0.344134, 0.0863356, 0.023482, -0.00857105, -0.107694, 0.244181, -0.00793614, -0.336814, 0.0738618, -0.108999, 0.0710515, -0.27933, 0.146005, -0.217931, -0.443198, -0.2486, 0.106888, 0.283488, 0.0740587, -0.136361, 0.180706, -0.575406, -0.0981283, 0.217126, -0.546568, -0.167376, 0.0783139, 0.485894, 0.321125, 0.428544, 0.310773, -0.231431, -0.415854, -0.023903, 0.13951, -0.0501428, 0.0880531, -0.207483,
0.375249, -0.262294, -0.05484, 0.041087, 0.44978, -0.275622, 0.0159287, -0.000838555, 0.153073, -0.030169, -0.0515117, -0.0405311, -0.318285, 0.0126105, -0.0817927, -0.168275, 0.0459981, -0.105249, -0.160146, 0.246035, 0.0871815, 0.375134, -0.132661, 0.0678319, -0.120072, 0.19459, 0.101208, 0.10996, -0.173551, 0.118997, 0.0503732, -0.0440491, 0.183214, 0.0295964, -0.100866, -0.285993, 0.094975, 0.419451, -0.103709, 0.0855461, 0.0840684, 0.0772267, 0.236753, -0.0932338, -0.0492666, 0.118675, -0.380623, 0.00344758, 0.15155, -0.279145,
-0.123601, 0.293582, -0.0456169, 0.0411083, -0.252016, 0.199989, 0.0592035, 0.202148, 0.373827, -0.00487453, 0.165065, -0.291021, -0.102479, -0.0476551, -0.163757, 0.021175, 0.0621616, -0.214002, -0.31107, 0.415825, -0.0104331, -0.0410483, -0.0865106, -0.239311, 0.252763, -0.0325731, -0.0246038, -0.0765138, 0.0241341, 0.195066, 0.240987, -0.0295819, 0.303427, 0.168514, 0.223721, -0.198504, -0.194497, -0.167257, 0.0323027, -0.170173, -0.172463, -0.233428, 0.294651, -0.149286, 0.190508, -0.00652951, 0.127235, 0.0455816, 0.241254, -0.113307,
-0.386814, -0.150825, -0.150893, 0.167558, -0.220123, -0.236312, -0.286345, 0.201171, -0.141561, 0.495895, -0.333993, -0.557361, -0.263146, 0.0737165, 0.260777, 0.330285, 0.200146, 0.0478525, -0.167897, 0.0133434, 0.135736, -0.0689596, -0.348999, 0.530008, 0.0839773, 0.0961618, 0.10956, 0.274509, 0.338184, -0.00209478, 0.0584504, 0.296487, 0.00671212, -0.232641, 0.448253, -0.111939, 0.146702, 0.0138877, -0.209819, 0.147507, -0.0133943, -0.14039, -0.0178435, 0.277872, 0.158416, 0.261016, 0.294671, 0.325048, -0.476676, 0.0545569,
0.105717, 0.101458, 0.0236403, -0.209853, 0.054899, 0.118425, 0.101481, -0.109596, -0.0992762, 0.0474929, -0.120523, -0.473867, 0.0490912, -0.194503, -0.115115, 0.0110791, 0.139496, 0.105062, 0.0621517, 0.224805, -0.342399, 0.0048537, -0.504085, -0.151759, 0.0404129, 0.0246989, -0.339, -0.0471754, -0.151918, -0.0718265, -0.0855963, -0.147769, -0.029829, -0.0885547, -0.0966042, 0.188978, -0.00571977, -0.0119005, 0.0639201, -0.00797127, 0.095367, 0.26763, -0.217085, -0.243551, -0.141408, 0.0325731, 0.0892951, 0.0950656, -0.148535, 0.423776,
0.311662, -0.218224, 0.363759, -0.218192, 0.0306516, 0.0156067, -0.131639, 0.0317046, -0.370994, 0.323465, 0.0598075, -0.290794, -0.183548, -0.108526, -0.0916746, -0.31852, -0.0211145, 0.27992, -0.251455, 0.49002, -0.0699964, 0.262702, -0.270354, -0.0608238, -0.23503, 0.00304167, 0.24679, -0.42544, 0.312028, 0.236237, 0.150816, 0.362042, -0.431739, 0.0758762, 0.161503, -0.453639, -0.790896, 0.0110069, 0.208509, 0.0301661, -0.112489, 0.0479979, 0.0640168, 0.0514118, 0.0711896, 0.137993, 0.014422, -0.400917, -0.0107789, 0.486284,
-0.138445, 0.121063, 0.197152, -0.163321, -0.314135, -0.0968481, 0.115348, -0.285512, 0.132751, -0.326142, 0.163597, 0.118559, -0.413751, -0.278844, 0.205245, -0.351577, -0.0438392, -0.0752028, -0.143762, -0.0278968, -0.0921022, -0.0244762, -0.145232, 0.271596, -0.05007, -0.0200573, -0.076511, 0.105683, -0.135658, 0.0198466, 0.0992441, -0.443969, -0.0955835, -0.388004, 0.081992, 0.117555, 0.321961, 0.189725, 0.241966, 0.307136, 0.0636694, -0.212107, 0.163518, 0.197724, -0.0469939, -0.0880918, -0.286608, 0.343716, 0.147056, 0.0645723,
0.00871754, -0.161896, -0.140168, 0.032392, -0.201541, 0.0375403, 0.0575594, 0.369079, -0.099426, -0.241357, -0.41194, -0.332213, 0.40879, 0.103371, -0.00766186, 0.0490589, 0.201821, -0.0822094, -0.00332649, 0.304603, 0.296236, 0.106603, -0.0609589, -0.105231, -0.0811251, -0.102433, -0.0908026, -0.348944, 0.160991, -0.00820576, 0.126881, -0.320861, 0.050929, -0.0521742, 0.123265, -0.0344321, -0.1862, 0.333072, 0.121905, 0.0265638, 0.0763744, -0.467048, -0.201695, -0.321951, 0.09744, -0.111448, 0.114231, -0.227464, 0.0410597, -0.0819089,
-0.18392, -0.314139, 0.0318228, -0.00842264, 0.110402, -0.41877, -0.488871, 0.0521601, -0.32315, -0.0172686, 0.330066, 0.267558, 0.290095, 0.206328, 0.00349827, 0.0387332, -0.48996, 0.0346622, -0.289429, -0.151868, 0.498738, -0.533294, 0.417211, 0.129081, -0.206377, 0.224213, 0.366842, 0.114465, -0.0985949, 0.404182, 0.115315, 0.138238, 0.152405, 0.341211, 0.145046, -0.245499, 0.402863, -0.167262, 0.160798, -0.401084, -0.141346, 0.182985, -0.56531, 0.133168, 0.167837, -0.302915, 0.338747, 0.0527251, -0.0579438, -0.410467,
-0.251963, 0.00630682, -0.0502171, -0.0188524, -0.29943, 0.10544, -0.14335, -0.224609, 0.00907665, 0.0354447, 0.0845239, 0.193827, 0.303547, 0.22979, -0.317881, -0.0754515, -0.579967, -0.0973385, 0.0585166, -0.0463995, -0.0375784, -0.128394, 0.515414, 0.0658186, -0.0614789, 0.0336551, -0.0788161, 0.122456, -0.140759, -0.22465, -0.276964, -0.271632, -0.13571, 0.0201326, -0.271139, -0.289506, 0.015917, -0.207986, -0.120677, -0.0767924, -0.150986, -0.00834616, -0.319305, 0.238703, 0.234122, -0.114556, 0.00367101, 0.0729764, -0.141128, -0.191171,
-0.286584, 0.122279, 0.104473, 0.207742, 0.0585593, -0.400527, -0.172395, -0.115252, 0.327319, 0.0700964, -0.10518, -0.163662, 0.253766, 0.0611342, -0.290512, -0.0661348, 0.168774, 0.103517, -0.0788394, 0.179342, 0.204946, 0.118355, 0.355362, -0.0936123, 0.13352, -0.312704, -0.0404482, 0.034507, 0.0235202, 0.337316, 0.037531, -0.0165959, -0.0921866, -0.0672958, -0.0830849, -0.211936, -0.066373, -0.162196, 0.0583042, -0.187634, 0.559711, 0.226924, 0.0275829, -0.236376, -0.177748, 0.00821331, 0.118756, 0.0461101, 0.101927, -0.0469041,
0.0685081, 0.245263, -0.0129707, -0.172741, -0.493373, -0.0334357, -0.400327, -0.330542, 0.193516, 0.206977, -0.295208, 0.0892304, -0.0644203, -0.0986342, 0.174681, 0.0119401, 0.184268, -0.25041, 0.0247704, 0.307143, 0.347551, 0.278993, -0.236186, 0.144934, 0.336881, 0.32846, -0.152002, -0.0384155, 0.269233, -0.141665, 0.0726256, 0.414743, 0.0358827, -0.0242075, -0.0980509, -0.0153042, -0.0593699, 0.150437, 0.157631, 0.298291, -0.145779, -0.263872, 0.170587, 9.04225e-05, 0.0213605, 0.295511, 0.112653, -0.52569, -0.231241, -0.166411,
0.205477, 0.347264, -0.207598, 0.228069, -0.124651, 0.00666893, -0.17789, 0.0191192, 0.322119, -0.0260543, -0.162019, 0.0497045, 0.0670039, 0.352774, 0.130369, -0.089802, 0.199577, 0.674373, 0.374321, 0.0225381, 0.00804065, -0.185063, 0.00511592, -0.128645, -0.125715, 0.281715, 0.0450747, 0.0120457, 0.150135, 0.307075, -0.0710548, -0.107724, 0.132047, 0.0789901, 0.313415, -0.114133, 0.0382178, 0.0184142, 0.0496693, 0.158071, 0.425283, -0.0661041, 0.207569, -0.154128, 0.0804997, 0.125272, -0.524368, -0.0478214, 0.0892811, 0.0251624,
0.40355, 0.208822, -0.144505, 0.123256, -0.360113, 0.214291, -0.00528693, 0.189578, 0.0999249, 0.295668, 0.0478715, -0.267127, -0.0612966, -0.0539172, -0.303694, -0.190734, -0.0276665, -0.0467158, 0.423456, -0.328699, 0.068216, -0.101153, 0.0271499, 0.00743013, -0.167937, -0.0983748, -0.196926, -0.188453, -0.539926, 0.1363, 0.198387, 0.0138093, 0.237977, 0.166203, 0.449929, -0.0601854, -0.195875, -0.0979294, 0.0830868, 0.198917, -0.0514048, 0.217855, -0.0108142, 0.036144, 0.195575, 0.230325, -0.220657, 0.175136, 0.0560964, -0.365914,
-0.136938, 0.0311226, 0.300449, -0.802793, -0.279304, 0.0748444, 0.104672, -0.187297, 0.316607, -0.371558, 0.309782, -0.206761, -0.0107151, -0.220951, -0.228547, 0.0791255, 0.0460777, -0.304001, 0.100746, -0.0368392, 0.0582884, -0.468651, -0.0500977, 0.0286683, 0.290056, 0.345795, -0.0735726, 0.299178, -0.186677, -0.125047, 0.298732, 0.161383, 0.0141757, -0.0737627, 0.0377018, -0.0613116, 0.201096, -0.207593, 0.409793, 0.0593893, -0.202097, -0.227417, 0.268647, -0.189613, 0.502912, 0.400853, 0.00919123, 0.476498, -0.109101, 0.667372,
0.533702, 0.416783, 0.132938, 0.252923, 0.081319, 0.199025, -0.341925, -0.124118, 0.035455, 0.178133, 0.123989, 0.0967305, -0.186811, 0.0119444, 0.0532298, 0.315593, 0.169901, 0.322584, -0.101331, -0.0664895, -0.118427, -0.0318455, 0.509203, 0.0747813, -0.0910469, -0.00200212, 0.346988, 0.34141, -0.0192011, -0.028863, -0.213161, 0.20267, 0.106721, -0.235515, -0.245466, 0.223677, 0.0261525, -0.130311, 0.294546, -0.340471, -0.131982, 0.539958, -0.14049, -0.0948347, -0.368601, -0.599678, 0.179372, -0.0339425, 0.106787, -0.158737,
-0.486164, -0.0504902, -0.134712, 0.095151, 0.0535549, 0.115451, -0.100441, 0.00719109, 0.151056, 0.0921906, -0.199985, 0.237281, -0.128562, 0.311989, -0.234491, -0.134711, -0.0812553, -0.175239, 0.173296, -0.0705748, 0.13967, 0.167285, -0.416073, 0.221392, 0.274718, 0.497211, -0.13701, -0.448036, 0.199932, -0.0473109, 0.078019, -0.0383235, 0.378308, -0.350212, 0.0680406, 0.48576, 0.114059, -0.187945, 0.0180284, -0.220909, -0.147192, -0.0088732, 0.231208, -0.0535699, -0.112235, 0.142756, 0.0710343, -0.0110367, 0.4683, -0.153948,
-0.0611163, 0.211195, 0.390049, -0.374629, 0.0269157, -0.0732004, 0.00650261, 0.489614, -0.386718, 0.124905, -0.0637891, 0.189858, 0.180819, -0.0100395, 0.648707, 0.26052, 0.157353, -0.297685, -0.330244, -0.23195, -0.170898, -0.19207, -0.162437, 0.139362, -0.171672, 0.446817, -0.221801, -0.33945, 0.0813814, -0.170456, -0.15291, 0.270247, 0.420972, 0.00293485, 0.285179, 0.0390872, -0.188916, 0.00731803, -0.248458, -0.254925, -0.000506213, -0.434179, -0.001665, 0.0785058, -0.343489, 0.0972646, 0.243511, 0.140582, -0.195719, 0.440282,
-0.139107, 0.390987, 0.101464, 0.161133, -0.0908286, 0.0519439, -0.233462, -0.147055, -0.102446, -0.162679, -0.0485844, 0.480333, 0.41417, 0.219009, -0.266816, -0.124679, 0.412412, -0.462715, 0.218699, 0.162338, 0.171326, -0.224433, 0.154616, -0.543157, -0.00614968, 0.108064, -0.211867, 0.0382027, 0.385709, 0.0308386, 0.360944, -0.0231651, -0.339439, -0.0946499, -0.387949, 0.112715, -0.157137, 0.0964979, -0.407727, 0.0626488, -0.104726, -0.115873, -0.254843, -0.633965, -0.146277, -0.0231654, -0.0972465, 0.269125, -0.188242, -0.0529321,
0.0471984, -0.175597, -0.227128, 0.00883032, -0.382149, -0.0853935, -0.168624, -0.108373, -0.0931991, 0.115769, 0.157991, 0.191708, 0.05875, 0.0380456, -0.304959, -0.0644525, 0.188301, 0.231374, -0.183786, 0.0716897, 0.208808, 0.238871, -0.156637, 0.0234726, 0.100992, -0.00870342, 0.169526, 0.0782617, -0.0306082, 0.118858, -0.177352, 0.281849, 0.00668963, -0.0696594, -0.0613527, -0.395022, -0.0985872, -0.0412695, 0.189158, 0.281763, 0.0464353, 0.122054, 0.34654, -0.187593, -0.00328575, 0.142185, -0.239763, -0.00592443, -0.0846511, 0.113011,
-0.265213, -0.0176369, 0.493451, 0.110276, 0.179216, -0.199226, -0.276317, -0.121566, 0.126038, -0.0106149, 0.687555, -0.166351, 0.00295482, -0.314632, -0.356983, -0.18865, -0.246083, -0.134762, 0.0578366, -0.0448304, 0.173422, -0.0831052, 0.337168, 0.114545, -0.130218, 0.126456, -0.135021, -0.540402, -0.0154492, -0.290398, -0.109366, -0.124742, -0.192226, -0.053275, 0.229043, -0.217644, -0.271027, -0.0353973, 0.0968761, 0.121873, -0.0238832, 0.00936398, 0.330536, 0.330993, 0.141586, -0.183899, 0.0677441, -0.00558138, -0.0466818, -0.315702,
-0.214797, -0.172568, -0.490718, -0.196459, 0.308414, 0.0305444, 0.156915, 0.323472, -0.300511, -0.164612, -0.0148892, -0.00397174, 0.116399, 0.0376731, 0.0390794, -0.259472, 0.0672516, 0.223944, 0.0658315, 0.393102, -0.0713983, -0.0425546, -0.0160994, -0.298241, 0.13964, 0.331611, -0.121948, 0.077516, 0.337135, -0.227233, -0.145432, -0.059776, 0.00168434, -0.241329, -0.323645, 0.215853, -0.203864, 0.0483379, 0.100946, -0.251965, 0.0458341, 0.0390062, -0.0754278, -0.11801, -0.369206, 0.154792, -0.111648, -0.546387, -0.234239, -0.121913,
-0.0541559, 0.0666688, -0.239769, -0.10201, -0.101601, -0.125663, -0.0744122, -0.303641, -0.240128, 0.182555, 0.314637, 0.321181, -0.0646134, 0.465909, 0.0197048, -0.142106, 0.109931, -0.165693, 0.216592, -0.113123, 0.21659, 0.0773567, -0.154648, -0.104611, -0.0352764, -0.0724476, -0.130336, -0.204927, 0.0489084, 0.0684557, -0.143369, -0.21071, 0.148819, -0.199596, 0.0678953, 0.102838, -0.248776, -0.198766, 0.254231, -0.280126, 0.0271796, -0.184156, -0.295057, -0.107192, 0.0291035, 0.17397, -0.261921, 0.30239, -0.0328865, -0.140486,
0.247789, -0.50637, 0.226475, 0.386635, -0.0786835, -0.00159666, -0.186226, 0.0887531, -0.154526, 0.0507661, 0.149906, -0.198302, -0.225229, -0.203113, 0.17548, 0.0524459, 0.212057, -0.0805479, -0.104856, -0.450277, -0.3466, 0.0521028, 0.105363, 0.0804818, -0.619189, -0.712319, 0.129186, -0.196353, 0.687164, 0.031221, -0.213502, -0.24808, 0.0116276, 0.304231, -0.288147, -0.0869166, -0.145005, -0.573574, -0.304848, -0.017781, -0.00531871, 0.219277, 0.0594608, 0.505677, 0.340684, -0.196733, 0.215509, -0.263453, 0.374447, 0.245209,
0.0366817, 0.150856, 0.112454, 0.0281868, 0.453835, 0.447598, -0.0166519, 0.190364, 0.183846, -0.225389, 0.060748, 0.107803, -0.179625, 0.189647, -0.449137, -0.074721, -0.0586015, 0.611317, 0.346088, 0.130605, 0.180499, -0.19665, -0.143802, 0.132765, 0.0363433, 0.31541, -0.148313, -0.389244, -0.460452, 0.155273, -0.00330537, 0.113514, 0.248631, 0.354176, 0.319724, 0.0845045, -0.624402, 0.335052, -0.133546, -0.311914, -0.0281113, 0.112491, 0.0484367, 0.269669, -0.178438, -0.154689, 0.443645, -0.2447, -0.124525, -0.0648921,
0.250866, 0.195033, -0.230802, 0.120595, -0.0793215, -0.247993, 0.11718, -0.089098, -0.237363, 0.107395, -0.220867, 0.000978349, 0.0113847, -0.0813412, -0.123689, 0.166153, -0.0459702, -0.0272464, 0.0547398, -0.252399, -0.147371, -0.0187557, 0.0162565, -0.0838047, -0.0633512, 0.108472, -0.229163, 0.0493734, -0.118304, 0.245377, -0.312586, 0.0551511, -0.270749, -0.0503313, 0.200319, -0.119238, -0.0637246, -0.0489867, 0.210738, 0.282924, 0.116036, -0.322812, -0.289783, -0.105625, 0.0584351, -0.00599599, 0.168565, -0.344267, -0.298542, 0.104354,
0.154754, 0.0228622, 0.174181, -0.0168848, -0.356716, -0.0199643, 0.0945743, -0.201986, 0.256371, 0.33919, -0.0448608, 0.0243003, 0.0221943, 0.220376, 0.24912, -0.134619, -0.376433, 0.246908, 0.156979, -0.454542, 0.130806, -0.144735, -0.217015, 0.205654, -0.0468316, 0.583347, 0.278455, 0.457843, 0.0235834, -0.248227, -0.251413, -0.323092, 0.523015, 0.243428, 0.258522, -0.34875, -0.17167, -0.102455, -0.411443, -0.190137, 0.0118035, -0.490445, -0.00547963, -0.118555, -0.0935438, -0.0395103, 0.377046, 0.406727, -0.123368, 0.0312248,
0.068232, -0.306341, 0.0433184, 0.0288068, 0.231251, -0.227015, -0.114438, 0.00770156, -0.0733435, -0.300637, -0.086887, -0.193827, 0.0793984, -0.00946303, -0.0966085, -0.316458, 0.0174952, -0.433502, -0.164646, -0.0376441, 0.407113, -0.10148, -0.0825468, 0.27007, -0.104638, 0.112227, -0.418813, 0.189346, 0.0316201, 0.122223, 0.34925, 0.0610299, 0.311375, -0.180719, -0.0583581, -0.025317, -0.248504, 0.148819, 0.00445883, 0.127733, -0.155754, -0.249238, 0.289655, -0.0433701, -0.122812, 0.215981, 0.681983, 0.000585339, -0.0192209, -0.251342,
0.0140512, -0.0788253, -0.26468, 0.138936, 0.123934, -0.128642, -0.0876182, 0.0701308, 0.190273, 0.166266, -0.011884, -0.00565948, -0.512493, 0.0162883, -0.136434, 0.266746, 0.129127, 0.133474, -0.243076, -0.146659, 0.316937, -0.0148324, 0.0186356, -0.249782, -0.0458432, -0.226652, -0.379105, -0.40203, 0.280262, 0.0190998, -0.22448, -0.347209, -0.142119, -0.101575, 0.080199, 0.0910073, 0.365363, 0.107745, -0.102314, 0.00177827, 0.345016, 0.126802, -0.166938, 0.280158, -0.161751, 0.104224, -0.234337, 0.414743, 0.219198, -0.00643325,
0.0186162, -0.0189021, -0.224947, 0.225759, -0.308634, 0.203412, -0.00692741, -0.086422, 0.184784, 0.0597181, -0.115347, -0.0672973, -0.13813, 0.19531, -0.28871, -0.218373, -0.165674, 0.312778, -0.262611, -0.26817, 0.127967, 0.0679519, -0.334147, -0.0452536, -0.0312948, -0.192823, -0.231002, 0.215595, 0.473791, 0.103646, 0.274246, 0.287721, 0.0607424, 0.00598383, -0.172026, -0.564975, -0.350218, 0.0219503, 0.227942, -0.117208, 0.380437, -0.0571861, 0.122618, -0.241561, 0.0134155, 0.21183, -0.0837681, -0.16946, -0.0500499, 0.0691852,
-0.230926, -0.0119482, -0.163754, 0.143279, -0.322152, 0.0456595, 0.0834552, -0.180699, 0.0608925, 0.162532, -0.195282, -0.0102081, 0.0731367, 0.0283397, -0.196613, -0.0353621, 0.127971, 0.0209633, -0.0102889, -0.21977, -0.185304, -0.0916524, -0.313014, -0.170682, 0.0115457, 0.195182, -0.262111, 0.166141, -0.0298181, -0.144718, 0.228728, 0.189888, 0.0640837, -0.0690635, -0.214257, -0.0162324, 0.0248364, 0.561686, -0.344991, -0.148226, 0.229313, -0.00662682, 0.0960746, -0.156869, 0.0791423, 0.17062, 0.265027, 0.0461503, 0.378828, 0.319585,
-0.00704383, -0.140268, -0.201048, -0.0150832, -0.22622, -0.208539, -0.157824, -0.147926, 0.174585, 0.0198509, -0.3242, -0.0824226, -0.181126, -0.0700965, 0.0437856, 0.182795, -0.00910094, -0.0652349, 0.297517, 0.117815, 0.177062, -0.0281778, -0.0306489, 0.119877, 0.30477, 0.1208, -0.104442, 0.0438712, -0.245629, 0.144381, -0.0552775, -0.16062, 0.311989, -0.117331, 0.0979442, -0.00722262, 0.325406, 0.034229, -0.0220573, -0.217179, 0.369693, 0.193067, 0.47656, -0.0317501, 0.194665, -0.244353, 0.00925772, -0.0632776, 0.224423, -0.298505,
-0.150912, -0.251406, 0.320868, -0.0481818, 0.170463, -0.117504, -0.181541, -0.670934, -0.135426, -0.143929, 0.209178, -0.209378, -0.119504, -0.374056, 0.0848028, -0.29485, -0.253781, -0.351648, -0.133996, 0.429471, -0.372432, 0.32194, -0.257282, 0.3603, 0.176723, -0.159277, 0.183081, 0.445273, 0.138451, 0.221642, 0.0654258, 0.076615, 0.0973245, -0.52455, -0.206324, 0.267514, 0.257643, -0.122702, 0.120094, 0.132197, -0.173771, 0.633784, 0.0544414, 0.343341, -0.0496007, 0.244538, -0.243461, 0.285645, -0.200353, 0.261674,
-0.0884441, 0.00252851, -0.00578709, -0.289686, 0.0804775, -0.322706, -0.0212985, 0.105326, 0.30774, 0.0864653, 0.0793489, 0.0268753, 0.0115955, -0.156514, 0.0437193, -0.337598, -0.327529, 0.45995, 0.202849, 0.300252, 0.362456, 0.190341, -0.179516, -0.113731, 0.114777, 0.616026, -0.27989, -0.330975, 0.0134993, 0.222797, 0.175168, -0.154081, -0.258271, 0.381348, 0.363321, 0.0418011, 0.182575, 0.107065, -0.13097, -0.191486, 0.1187, 0.00483179, -0.422261, 0.109037, 0.201361, 0.798295, 0.199746, -0.126423, -0.442338, -0.111398,
-0.243111, 0.131261, -0.306227, 0.0458877, 0.0636512, 0.0388696, 0.051178, 0.296881, -0.138833, 0.285534, -0.121713, 0.0971898, -0.334303, -0.23124, -0.0796344, -0.112102, 0.101952, 0.258487, 0.498055, 0.316409, 0.205318, 0.72955, -0.167406, -0.00441071, -0.0653576, 0.163087, 0.121558, 0.205551, -0.19194, 0.00112248, 0.119485, 0.238497, -0.033528, 0.0269291, -0.0573328, -0.143301, -0.322589, -0.246427, 0.018742, 0.04759, -0.384608, 0.284049, -0.279751, -0.196426, -0.0583255, 0.296862, -0.197714, 0.134547, -0.38917, 0.410128,
-0.341941, -0.0507345, 0.141608, -0.349757, -0.16937, -0.0448806, 0.136329, 0.0350823, -0.251026, 0.376587, -0.114916, -0.0533992, 0.262685, 0.128323, 0.237777, 0.296633, 0.194885, 0.272272, 0.0108618, 0.288871, -0.344741, 0.108868, -0.344109, 0.0784352, 0.142427, 0.0649757, 0.191543, -0.110006, 0.338047, 0.109291, 0.201423, -0.402331, -0.0451263, 0.198527, 0.019081, 0.0159894, -0.151074, -0.184205, -0.425177, -0.00405891, 0.168666, 0.120484, -0.171483, -0.259674, 0.0553583, 0.00650684, 0.34386, 0.312774, 0.0350854, -0.266628,
-0.204885, 0.107983, 0.541716, 0.0261457, -0.00434357, 0.250585, 0.0407534, 0.138215, -0.243164, 0.0890953, 0.265647, -0.0360517, -0.036056, -0.0198551, -0.145602, -0.116339, 0.040815, -0.23754, 0.0160291, 0.227704, 0.379952, 0.277856, -0.136002, -0.132898, -0.212762, 0.180782, -0.127127, -0.10291, -0.149744, -0.481835, -0.0450487, -0.23411, -0.12799, 0.433556, -0.187643, -0.183911, -0.0232259, 0.225835, 0.161274, -0.357976, -0.364807, -0.122226, -0.0397304, -0.36427, -0.0013413, 0.043672, -0.152077, -0.222311, 0.127567, -0.306211,
-0.183264, -0.121081, 0.271887, 0.358267, -0.256014, -0.605023, -0.312929, 0.255106, -0.107512, 0.415928, 0.404609, -0.380337, 0.0508715, 0.250641, -0.320998, 0.281617, -0.0858803, -0.234763, 0.0179243, -0.104151, 0.0684242, 0.207735, 0.0623548, -0.0897978, 0.195481, 0.348177, -0.149952, 0.0170524, 0.0858576, 0.277637, 0.240964, -0.375295, 0.320295, -0.286971, 0.00274236, -0.137198, -0.272492, 0.00118182, 0.00903271, -0.128999, -0.110073, 0.125383, -0.100551, 0.128432, 0.00561021, 0.0792144, 0.141004, -0.106242, -0.16273, -0.169034,
-0.287541, -0.0169757, 0.318992, -0.182358, -0.262055, 0.0748617, -0.111362, 6.42901e-05, 0.0295337, -0.0364526, 0.34268, -0.195073, 0.0928639, 0.26444, -0.179261, 0.127657, -0.0472367, 0.0442137, 0.1452, -0.0353573, 0.133063, -0.0819551, -0.0877272, 0.262725, 0.0160187, -0.423252, -0.352373, 0.0351428, -0.106642, -0.183013, -0.187153, 0.256007, -0.264335, -0.0610265, 0.148443, 0.0939439, -0.124136, -0.16813, -0.0217767, 0.0314988, -0.0518497, -0.449924, -0.163656, 0.0991832, -0.135318, 0.397007, 0.280241, -0.261743, -0.0335663, -0.20819,
0.256977, -0.0744221, 0.0489051, -0.0656905, -0.200068, 0.138825, 0.200218, -0.168564, 0.111073, 0.180772, 0.00848376, 0.067118, -0.135645, -0.331939, 0.201715, 0.198522, -0.285974, -0.0419745, -0.0230426, -0.206636, -0.126246, 0.233835, -0.254892, -0.58945, 0.515599, -0.0781576, -0.322769, 0.204351, -0.141664, -0.387755, 0.589099, 0.345529, 0.279575, 0.338306, 0.0740869, -0.0997301, -0.129031, 0.242254, 0.187776, 0.0643898, -0.179435, -0.303717, 0.120288, -0.00706433, -0.164426, -0.205336, 0.15564, 0.00593048, -0.0462586, -0.362508,
-0.276089, -0.232983, 0.0425048, 0.148243, 0.267629, 0.282748, -0.532036, -0.477845, 0.43499, 0.0402603, 0.0238691, 0.149137, -0.0770271, 0.415283, 0.0945517, 0.0819209, -0.218897, 0.138103, -0.0891979, 0.323729, -0.0650888, 0.0466718, -0.488965, -0.251841, -0.308957, 0.0914797, -0.0499364, 0.106729, 0.298226, 0.0175082, -0.28482, 0.527989, 0.0204008, -0.408955, 0.111855, -0.141353, 0.522685, 0.145459, 0.148604, -0.0340382, -0.262953, -0.369135, -0.187397, 0.0908855, 0.0620149, -0.331041, -0.273616, 0.254057, -0.396317, 0.00940274,
0.0558457, -0.127279, 0.17409, 0.0780287, 0.245713, -0.132427, 0.215719, 0.335036, -0.0989322, -0.217334, -0.0707002, -0.00183965, 0.1127, -0.0466141, -0.0111688, -0.158666, -0.0346559, -0.0348362, -0.193735, 0.0602845, -0.0411441, 0.289346, -0.389267, -0.153097, 0.130845, 0.155609, 0.102367, -0.379177, 0.22442, 0.277293, 0.0142306, -0.352261, -0.0771464, 0.368811, 0.441222, 0.00507368, -0.0218818, -0.0191494, -0.189525, -0.0248088, 0.157814, -0.0826859, -0.219446, -0.0948512, 0.0638714, 0.102589, -0.101636, 0.230225, -0.268992, 0.16385,
-0.0683471, -0.310428, -0.125706, 0.0439035, 0.192402, 0.366194, 0.428801, -0.47782, -0.0842806, -0.102062, -0.127652, 0.354646, 0.0114887, 0.147011, -0.0416828, -0.16571, -0.00208619, 0.0832194, -0.0442304, 0.313176, -0.28998, -0.367015, 0.0548344, 0.239605, -0.0142818, -0.090592, -0.175623, -0.000893967, 0.0874187, -0.193206, 0.235459, -0.389699, -0.187389, -0.0816016, 0.143597, 8.5181e-05, -0.0110671, -0.274705, 0.0268506, 0.011879, 0.131212, 0.124686, -0.23443, 0.344588, 0.0353117, 0.129618, -0.094811, 0.217493, 0.142379, -0.113744,
0.0904281, -0.179824, 0.218368, 0.0194503, -0.45706, 0.225195, 0.35112, -0.0320301, -0.224756, 0.0275414, -0.479483, 0.145097, -0.048566, 0.169399, -0.0352027, 0.0874918, -0.0154655, 0.119223, -0.0384954, -0.0263642, 0.0700483, 0.494655, -0.231997, -0.198181, 0.102929, -0.121476, -0.228599, -0.101971, 0.176042, 0.0637137, 0.151932, 0.204918, -0.624547, -0.0778217, 0.443292, 0.40014, -0.116933, -0.0432446, -0.157737, 0.0104551, -0.125288, -0.0332703, -0.0915768, 0.257133, 0.233855, -0.341734, -0.332236, 0.567486, 0.50996, 0.054608,
-0.00727693, 0.209044, -0.165201, -0.257146, -0.0322869, 0.0961324, 0.24064, 0.0146038, -0.211831, 0.00150649, 0.25003, 0.284749, 0.330604, -0.247226, -0.616634, 0.289077, -0.0574922, 0.472383, 0.203892, -0.0686977, -0.0188779, 0.184899, 0.215103, -0.0393983, 0.0843418, -0.0136342, 0.442031, -0.0425478, -0.0891068, -0.0905298, -0.0317705, -0.117739, -0.369747, 0.331672, 0.0198675, 0.013453, 0.128762, -0.215133, -0.264936, -0.334785, 0.625272, -0.0380616, 0.195898, -0.12068, 0.302653, -0.210497, 0.155681, -0.150919, 0.248864, 0.163627};
const float b2_784_50_100_10[100] = {-0.0850189,
0.017693,
0.283263,
-0.178919,
0.0963787,
-0.0671087,
-0.0547958,
0.10268,
-0.0465013,
0.310958,
0.154403,
0.28635,
-0.0791992,
-0.113963,
0.115406,
-0.0531061,
-0.120879,
0.107139,
0.112129,
0.38364,
0.182194,
0.23661,
0.0755317,
0.362166,
0.314732,
-0.28476,
0.162348,
0.284255,
0.0575022,
0.364906,
0.215969,
-0.206669,
0.109975,
-0.153147,
-0.029679,
-0.218228,
-0.174253,
-0.0148926,
-0.370133,
-0.502582,
0.348089,
0.028739,
0.0385617,
-0.258505,
-0.0266246,
0.412832,
0.39394,
-0.256676,
-0.00818287,
0.197286,
0.0323316,
0.0997329,
-0.321339,
-0.193662,
-0.145447,
0.083774,
0.0845832,
0.223896,
-0.08487,
-0.142081,
0.199783,
0.139729,
0.0928638,
0.149629,
-0.163409,
0.119838,
-0.0746265,
0.0225914,
-0.0170834,
0.295437,
0.0092527,
-0.0561737,
0.193435,
-0.158036,
0.333676,
0.0327572,
-0.222037,
0.0470245,
-0.0426035,
-0.311533,
-0.291315,
-0.148773,
-0.142426,
0.192461,
-0.0838357,
0.335467,
-0.633052,
-0.167688,
0.305845,
0.180986,
-0.0796626,
-0.0769335,
0.200963,
-0.361675,
0.0407668,
0.142496,
-0.0476392,
-0.284055,
-0.150179,
0.138695};
const float A3_784_50_100_10[100*10] = {0.184976, -0.585731, 0.417105, -0.327594, 0.00356979, 0.120079, -0.0440579, -0.133921, -0.105924, 0.00317975, 0.297499, 0.180033, 0.418311, 0.0725334, -0.107273, -0.0103783, -0.135592, 0.0908181, -0.316772, -0.585719, -0.390073, -0.337635, -0.292816, -0.0999463, -0.408299, -0.204585, 0.010097, -0.0898688, 0.236101, 0.225022, -0.123644, -0.104969, 0.695777, -0.02265, 0.0738928, 0.316754, 0.120883, -0.117598, -0.11869, 0.161659, -0.236302, 0.0400901, 0.140367, -0.101091, 0.441724, 0.205232, -0.359084, -0.0695489, -0.203833, -0.646658, -0.396271, 0.0941943, 0.150147, -0.191133, -0.279806, 0.25601, 0.0750904, 0.0275814, 0.14512, -0.258258, 0.085429, -0.144615, 0.0678846, -0.440129, 0.160363, 0.0583133, -0.100981, 0.191564, 0.262754, -0.332636, -0.0569962, -0.257726, 0.0339858, -0.160189, 0.0122445, -0.0530973, 0.0283243, -0.0277274, 0.144598, 0.433772, -0.0235605, 0.374089, 0.104278, 0.122525, 0.0584858, 0.0279945, 0.0532646, -0.384521, 0.111968, -0.108228, 0.393821, 0.441405, -0.15459, -0.0260232, 0.569985, -0.253131, 0.0627703, -0.391849, 0.18769, 0.123464,
-0.246958, -0.182909, 0.527738, 0.1504, 0.0687011, -0.279125, 0.191843, -0.164797, -0.0831015, -0.112669, -0.138052, -0.110337, 0.107848, -0.0436904, 0.205137, 0.109827, -0.046759, 0.312965, -0.0119506, 0.133098, 0.198398, 0.2586, 0.360225, -0.342152, 0.157872, 0.0261686, -0.288448, 0.236688, 0.250394, 0.0198464, 0.0499259, -0.251051, -0.0497691, 0.45181, 0.239395, -0.393809, -0.479827, -0.54569, -0.140069, -0.0487089, 0.186174, 0.157309, 0.380458, 0.524541, -0.0880339, -0.216219, 0.184076, 0.178687, -0.217259, -0.13475, 0.182777, 0.257827, -0.0567095, 0.244278, 0.172476, 0.243122, 0.0876109, 0.116956, -0.159376, -0.185289, 0.311056, 0.455632, 0.121871, -0.368355, -0.202866, -0.15223, 0.231842, -0.135693, -0.148218, 0.899852, -0.785004, 0.177757, -0.0166593, -0.25528, 0.309392, 0.126939, -0.0698834, 0.0626725, 0.123276, -0.0664936, -0.0303524, -0.220697, 0.148848, 0.00349724, -0.235221, 0.0763311, 0.238327, 0.290163, 0.0708207, 0.0391988, -0.402915, -0.0312297, -0.206206, 0.202252, 0.0705253, -0.0328261, 0.138497, 0.114345, 0.189717, -0.0497698,
0.230313, -0.193476, -0.00277191, -0.0591088, -0.0445394, -0.375766, 0.00720511, -0.282635, -0.414878, -0.0305877, -0.0197718, -0.0208881, -0.119146, 0.309527, -0.0640331, 0.486156, -0.0718044, -0.295669, 0.328274, -0.529137, 0.459893, 0.250586, 0.0574515, 0.251881, -0.683589, 0.0218602, -0.184816, -0.026158, 0.144972, -0.387174, 0.0813455, 0.103689, -0.250558, -0.680777, -0.431883, -0.00219365, -0.371505, 0.320075, 0.095608, 0.00323521, 0.117192, -0.217874, -0.322921, -0.0685609, -0.0499264, -0.516561, -0.266053, -0.115499, -0.0124742, -0.216015, 0.0533455, -0.0501997, 0.0696681, -0.268998, 0.154836, -0.0925829, 0.255286, -0.0132267, 0.139812, 0.193447, 0.196688, -0.27031, 0.167278, 0.203074, -0.0912141, -0.257233, -0.121886, 0.109797, 0.00260551, -0.187397, -0.170358, -0.00919195, 0.255404, 0.0924356, 0.169456, 0.438639, -0.173312, 0.0426288, 0.0594746, 0.729299, 0.0841103, 0.393725, 0.246932, 0.379036, -0.0596827, -0.54699, 0.0211091, -0.469579, 0.800717, 0.646272, 0.0447193, 0.431545, 0.286655, 0.570417, -0.205782, 0.37453, 0.297946, -0.265928, -0.0833855, -0.121726,
0.180259, 0.0449983, -0.215263, 0.339246, -0.100986, 0.195064, -0.353477, 0.609699, -0.638678, 0.226986, -0.0184577, 0.0605049, -0.268498, -0.400797, -0.264616, -0.390773, 0.0378824, -0.460596, 0.0327072, 0.499945, -0.0509882, 0.0651335, -0.11328, 0.134366, 0.47454, -0.0669926, 0.225879, 0.242526, -0.152383, 0.0927033, 0.360001, 0.105942, -0.0106925, 0.477267, -0.251641, 0.0750285, -0.453176, -0.0248893, 0.291342, -0.00455433, -0.699852, -0.0149028, -0.264459, -0.0655344, -0.122655, -0.780987, 0.253024, -0.211499, 0.178579, 0.271894, 0.289271, 0.253301, -0.0399868, -0.121717, 0.44866, -0.115499, -0.0686872, -0.501497, 0.21802, 0.0970975, -0.254668, -0.346743, 0.030103, 0.454391, 0.14427, -0.167282, 0.0856206, 0.273394, 0.0707129, 0.0830064, -0.276919, -0.140289, 0.376383, -0.0886053, -0.133392, -0.605991, 0.229732, -0.113578, -0.218965, -0.158457, 0.332803, 0.55916, -0.104618, -0.116877, -0.208639, 0.228147, 0.00164784, 0.0633359, 0.546627, -0.699727, 0.100399, -0.362933, -0.130268, 0.108108, -0.550653, -0.309801, 0.170293, 0.073883, 0.0445615, 0.298542,
-0.238462, 0.52478, -0.435507, 0.05911, -0.11738, 0.167214, 0.442703, -0.0873521, 0.305916, 0.57369, -0.237179, -0.133681, -0.144602, -0.322467, 0.143539, -0.0551687, 0.106049, -0.396603, -0.0743349, 0.175441, 0.180944, -0.423176, -0.0929946, -0.152706, -0.298449, 0.532593, 0.187692, 0.0252459, -0.0101759, -0.712912, -0.103624, 0.210159, -0.0769046, -0.170472, 0.514785, -0.160014, 0.733244, -0.350719, -0.368522, 0.170408, 0.201394, 0.0968266, -0.0644517, 0.174735, -0.0514317, 0.0646325, -0.0253095, 0.0103159, 0.133968, 0.0418562, -0.436127, 0.171215, 0.0740413, -0.079056, -0.210534, 0.294611, 0.178588, 0.0946215, 0.268539, -0.0348631, 0.311776, -0.125352, 0.0251615, -0.179578, -0.279459, -0.14649, 0.0561291, -0.14915, -0.0772619, 0.32764, -0.220281, -0.00481496, -0.591763, 0.214279, 0.192836, -0.319093, -0.073465, -0.0443069, -0.623845, 0.0257495, 0.0675145, -0.757797, -0.0828356, -0.263618, 0.0371315, 0.0776188, 0.0696884, 0.755106, 0.167458, 0.36884, -0.296478, 0.018166, -0.0708012, -0.208653, -0.262378, 0.612111, -0.168697, -0.0246559, 0.498557, -0.440139,
-0.372639, 0.577982, -0.725114, 0.0776024, 0.240902, -0.0359964, -0.401107, 0.236089, 0.285344, -0.294652, 0.0387986, -0.0370218, -0.300149, -0.321582, -0.0134488, -0.678967, 0.0315435, 0.760239, -0.202432, -0.373424, -0.00388459, 0.621154, 0.194996, 0.664259, 0.561339, 0.388899, -0.0191763, 0.170414, -0.514296, 0.526645, 0.37935, 0.039128, -0.203246, 0.309888, -0.392187, -0.276782, 0.41832, 0.178463, -0.163576, 0.0889709, -0.307884, -0.112905, -0.1137, 0.121703, -0.24038, 0.707885, 0.318966, -0.220063, -0.281497, -0.069601, 0.383654, 0.0495172, 0.009063, -0.213285, -0.391132, 0.0824939, 0.392037, -0.211123, -0.20389, 0.0715668, -0.175557, -0.589428, 0.22951, 0.324579, -0.114209, 0.284884, -0.0814387, 0.028787, -0.177602, -0.173193, 0.631357, 0.242058, 0.209042, 0.294147, 0.048379, -0.189253, -0.160831, -0.0524488, 0.34227, 0.159918, 0.115476, -0.191353, 0.0907374, -0.10527, 0.14277, 0.164958, 0.203512, -0.329113, -0.530997, -0.507222, -0.293743, -0.437715, -0.411672, 0.119268, -0.141463, -0.345862, -0.00508044, -0.100487, -0.456815, 0.357847,
0.137838, -0.241101, 0.382406, -0.248389, 0.0946238, 0.426444, 0.0614812, -0.270696, 0.115192, -0.0450254, 0.000288396, -0.0785742, -0.000631831, -0.0949418, 0.243757, -0.0387583, 0.152984, 0.220109, -0.0917675, -0.06219, 0.0600567, 0.298086, -0.0841287, -0.360936, 0.225085, 0.132497, -0.10816, 0.121408, -0.127181, -0.0873745, -0.0346621, -0.277722, 0.124952, 0.329128, 0.699405, 0.336463, -0.455558, 0.376666, 0.165748, 0.170865, -0.195636, -0.185078, 0.130454, 0.274805, 0.0550645, 0.603989, -0.245325, -0.255415, 0.214529, 0.135031, -0.626746, 0.0504001, 0.0757953, -0.0734079, 0.0212764, -0.0660782, -0.175285, -0.161028, -0.783413, -0.0391897, 0.166189, -0.0994555, 0.026224, -0.112509, -0.13331, 0.210555, -0.371085, 0.0552479, -0.0185225, -0.663787, 0.0506143, -0.151971, -0.444358, 0.0953266, 0.244231, 0.691268, -0.150023, -0.271185, 0.177532, 0.213011, 0.178632, -0.540783, -0.144174, 0.159742, 0.0878148, -0.0717884, -0.0303654, -0.0629265, -0.0526514, -0.0282028, -0.166491, -0.127264, 0.161385, -0.178325, 0.0261594, -0.405841, -0.0566029, 0.16267, -0.263521, 0.453505,
-0.761399, -0.081869, 0.350262, -0.656854, -0.13215, 0.464085, 0.0565674, 0.183226, 0.0559764, -0.37739, 0.143986, -0.234017, -0.227459, 0.075242, 0.636877, 0.0299713, 0.0365638, -0.0581221, 0.560143, 0.316863, 0.270141, 0.459578, -0.121296, 0.654772, -0.69873, -0.0374462, 0.246021, 0.0680853, -0.218692, 0.397055, -0.368556, 0.042627, -0.31906, -0.216143, 0.375536, -0.38827, -0.0571647, -0.166837, -0.0348599, 0.0320197, 0.213056, -0.0928005, 0.346849, 0.118698, 0.0284438, -0.304817, -0.139142, 0.0453901, 0.370849, -0.151244, 0.166718, 0.104096, -0.101924, 0.0565374, 0.266687, -0.236646, -0.215538, 0.069813, 0.443868, 0.248517, -0.0848759, -0.19915, 0.0263746, -0.301256, -0.0565568, 0.0700082, 0.496423, -0.120751, -0.158053, 0.0792814, -0.0378074, 0.0624342, 0.0293232, 0.041347, -0.277939, 0.0415749, 0.113046, 0.0971643, -0.426675, -0.476407, -0.0216766, 0.356336, 0.0424682, 0.209168, 0.0251734, 0.0715377, -0.145773, -0.189817, -0.0189854, 0.28789, -0.000804771, -0.117622, 0.337739, 0.414191, 0.501646, -0.48548, 0.0593922, 0.271724, 0.431599, -0.253355,
0.607992, -0.2683, -0.26795, 0.388501, -0.172486, -0.221421, 0.333728, -0.0436855, 0.282151, 0.0596324, -0.170057, 0.292344, -0.128266, 0.369659, -0.113707, 0.176267, 0.189654, -0.278081, 0.425969, -0.0264792, 0.426916, 0.318549, -0.27241, -0.525035, 0.269851, -0.678673, -0.389177, -0.0197289, -0.0728231, -0.205626, 0.402142, -0.202068, 0.320633, -0.0833823, -0.511568, -0.229092, -0.0418015, 0.247975, 0.101737, -0.0120464, -0.74682, -0.194407, -0.00941015, -0.11517, -0.094599, 0.00622762, 0.494488, 0.168484, 0.246285, -0.25639, -0.0430087, 0.0179082, -0.269461, 0.0666728, -0.219428, -0.239988, -0.175121, 0.131256, 0.289853, -0.109704, -0.411376, -0.0384122, 0.0690416, 0.465079, 0.0868793, 0.194569, -0.134339, -0.561134, -0.116864, -0.107559, 0.121715, -0.105621, 0.106309, -0.115425, -0.202266, 0.0597384, -0.0443454, 0.0633824, 0.449193, -0.571005, -0.268429, 0.114107, -0.362189, -0.173765, 0.0707806, -0.0437091, -0.32136, 0.47506, -0.212546, 0.0459401, 0.134996, -0.30974, 0.514199, -0.127388, -0.0793273, -0.0664541, -0.222241, 0.039017, 0.144942, -0.354467,
0.37962, 0.512554, 0.0362093, -0.341029, 0.0159097, -0.191241, -0.64745, 0.942215, 0.397056, -0.339409, 0.16204, -0.121301, 0.302465, -0.171578, -0.452935, 0.148722, 0.264817, 0.138612, -0.179356, 0.501266, -0.840471, -0.79137, 0.179483, -0.273621, 0.143734, 0.00155783, -0.23147, 0.0609803, -0.289878, 0.01346, -0.168936, -0.216012, 0.278548, 0.0258243, -0.261011, 0.357237, 0.675237, 0.00754293, 0.427999, -0.207489, 0.512886, 0.048332, -0.41868, -0.247336, 0.117016, -0.261804, -0.300148, -0.434015, -0.317395, 0.359054, 0.27772, -0.0888531, -0.0724687, -0.021674, -0.0190432, -0.05608, -0.271953, 0.0486493, 0.243094, 0.0670999, -0.271366, 0.189848, -0.0999053, -0.416385, -0.0801124, -0.0569457, -0.183535, -0.0723617, 0.442015, 0.0998597, 0.0554724, 0.250166, -0.0297211, 0.0610039, -0.166792, 0.0375133, -0.12899, -0.115321, 0.609103, -0.303094, 0.118579, 0.0769964, -0.0850968, -0.239362, -0.0632955, 0.102613, 0.0403581, 0.131798, -0.475803, -0.16066, 0.0574348, -0.12467, -0.382925, -0.0810845, -0.137765, 0.347848, 0.348944, -0.240857, -0.242299, 0.206945};
const float b3_784_50_100_10[10] = {-0.241497,
0.0716496,
0.110682,
0.00223341,
-0.36356,
0.170703,
-0.0609609,
0.143773,
0.0947685,
0.204036};


#endif // NN_H
