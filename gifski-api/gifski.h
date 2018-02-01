#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct gifski;
typedef struct gifski gifski;

/*
 * Settings for creating a new encoder instance. See `gifski_new`
 */
typedef struct {
  /*
   * Resize to max this width if non-0
   */
  uint32_t width;
  /*
   * Resize to max this height if width is non-0. Note that aspect ratio is not preserved.
   */
  uint32_t height;
  /*
   * 1-100, but useful range is 50-100. Recommended to set to 100.
   */
  uint8_t quality;
  /*
   * If true, looping is disabled. Recommended false (looping on).
   */
  bool once;
  /*
   * Lower quality, but faster encode
   */
  bool fast;
} GifskiSettings;

enum GifskiError {
  GIFSKI_OK = 0,
  /** one of input arguments was NULL */
  GIFSKI_NULL_ARG,
  /** a one-time function was called twice, or functions were called in wrong order */
  GIFSKI_INVALID_STATE,
  /** internal error related to palette quantization */
  GIFSKI_QUANT,
  /** internal error related to gif composing */
  GIFSKI_GIF,
  /** internal error related to multithreading */
  GIFSKI_THREAD_LOST,
  /** I/O error: file or directory not found */
  GIFSKI_NOT_FOUND,
  /** I/O error: permission denied */
  GIFSKI_PERMISSION_DENIED,
  /** I/O error: file already exists */
  GIFSKI_ALREADY_EXISTS,
  /** misc I/O error */
  GIFSKI_INVALID_INPUT,
  /** misc I/O error */
  GIFSKI_TIMED_OUT,
  /** misc I/O error */
  GIFSKI_WRITE_ZERO,
  /** misc I/O error */
  GIFSKI_INTERRUPTED,
  /** misc I/O error */
  GIFSKI_UNEXPECTED_EOF,
  /** progress callback returned 0, writing aborted */
  ABORTED,
  /** should not happen, file a bug */
  GIFSKI_OTHER,
};

typedef enum GifskiError GifskiError;

/*
 * Call to start the process
 *
 * See `gifski_add_frame_png_file` and `gifski_end_adding_frames`
 *
 * Returns a handle for the other functions, or `NULL` on error (if the settings are invalid).
 */
gifski *gifski_new(const GifskiSettings *settings);

/*
 * File path must be valid UTF-8. This function is asynchronous.
 *
 * Delay is in 1/100ths of a second
 *
 * Call `gifski_end_adding_frames()` after you add all frames. See also `gifski_write()`
 *
 * Returns 0 (`GIFSKI_OK`) on success, and non-0 `GIFSKI_*` constant on error.
*/
GifskiError gifski_add_frame_png_file(gifski *handle,
                               uint32_t index,
                               const char *file_path,
                               uint16_t delay);

/*
 * Pixels is an array width×height×4 bytes large. The array is copied, so you can free/reuse it immediately.
 *
 * Delay is in 1/100ths of a second
 *
 * Call `gifski_end_adding_frames()` after you add all frames. See also `gifski_write()`
 *
 * Returns 0 (`GIFSKI_OK`) on success, and non-0 `GIFSKI_*` constant on error.
 */
GifskiError gifski_add_frame_rgba(gifski *handle,
                           uint32_t index,
                           uint32_t width,
                           uint32_t height,
                           const unsigned char *pixels,
                           uint16_t delay);

/*
 * You must call it at some point (after all frames are set), otherwise `gifski_write()` will never end!
 *
 * Returns 0 (`GIFSKI_OK`) on success, and non-0 `GIFSKI_*` constant on error.
 */
GifskiError gifski_end_adding_frames(gifski *handle);

/* Get a callback for frame processed, and abort processing if desired.
 *
 * The callback is called once per frame with `NULL`, and then once with non-null message on end.
 *
 * The callback must return `1` to continue processing, or `0` to abort.
 *
 * Must be called before `gifski_write()`
 */
void gifski_set_progress_callback(gifski *handle, int (cb)(const char *));

/*
 * Write frames to `destination` and keep waiting for more frames until `gifski_end_adding_frames` is called.
 *
 * Returns 0 (`GIFSKI_OK`) on success, and non-0 `GIFSKI_*` constant on error.
 */
GifskiError gifski_write(gifski *handle, const char *destination);

/*
 * Call to free all memory
 */
void gifski_drop(gifski *g);
