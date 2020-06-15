/* Copyright  (C) 2010-2014 The RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (file_path.h).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __LIBRETRO_SDK_FILE_PATH_H
#define __LIBRETRO_SDK_FILE_PATH_H

#include <boolean.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <string/string_list.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Order in this enum is equivalent to negative sort order in filelist
 *  (i.e. DIRECTORY is on top of PLAIN_FILE) */
enum
{
   RARCH_FILETYPE_UNSET,
   RARCH_PLAIN_FILE,
   RARCH_COMPRESSED_FILE_IN_ARCHIVE,
   RARCH_COMPRESSED_ARCHIVE,
   RARCH_DIRECTORY,
   RARCH_FILE_UNSUPPORTED,
};


/* path_is_compressed_file also means: The compressed file is supported */
bool path_is_compressed_file(const char *path);

/* Somewhere in the path there might be a compressed file
 * E.g.: /path/to/file.7z#mygame.img
 */
bool path_contains_compressed_file(const char *path);

bool path_is_directory(const char *path);

bool path_file_exists(const char *path);
bool path_is_valid(const char *path);

/* Gets extension of file. Only '.'s after the last slash are considered. */
const char *path_get_extension(const char *path);

bool path_mkdir(const char *dir);

/* Removes all text after and including the last '.'.
 * Only '.'s after the last slash are considered. */
char *path_remove_extension(char *path);

/* Returns basename from path. */
const char *path_basename(const char *path);

/* Extracts base directory by mutating path.
 * Keeps trailing '/'. */
void path_basedir(char *path);

/* Extracts parent directory by mutating path.
 * Assumes that path is a directory. Keeps trailing '/'. */
void path_parent_dir(char *path);

/* Turns relative paths into absolute path.
 * If relative, rebases on current working dir. */
void path_resolve_realpath(char *buf, size_t size);

bool path_is_absolute(const char *path);

/* Path-name operations.
 * If any of these operation are detected to overflow, the application will 
 * be terminated.
 *
 * Replaces filename extension with 'replace' and outputs result to out_path.
 * The extension here is considered to be the string from the last '.'
 * to the end.
 *
 * Only '.'s after the last slash are considered as extensions.
 * If no '.' is present, in_path and replace will simply be concatenated.
 * 'size' is buffer size of 'out_path'.
 * E.g.: in_path = "/foo/bar/baz/boo.c", replace = ".asm" => 
 * out_path = "/foo/bar/baz/boo.asm" 
 * E.g.: in_path = "/foo/bar/baz/boo.c", replace = ""     =>
 * out_path = "/foo/bar/baz/boo" 
 */
void fill_pathname(char *out_path, const char *in_path,
      const char *replace, size_t size);

void fill_dated_filename(char *out_filename,
      const char *ext, size_t size);

/* Appends a filename extension 'replace' to 'in_path', and outputs
 * result in 'out_path'.
 *
 * Assumes in_path has no extension. If an extension is still
 * present in 'in_path', it will be ignored.
 *
 * 'size' is buffer size of 'out_path'. */
void fill_pathname_noext(char *out_path, const char *in_path,
      const char *replace, size_t size);

/* Appends basename of 'in_basename', to 'in_dir', along with 'replace'.
 * Basename of in_basename is the string after the last '/' or '\\',
 * i.e the filename without directories.
 *
 * If in_basename has no '/' or '\\', the whole 'in_basename' will be used.
 * 'size' is buffer size of 'in_dir'.
 *
 * E.g..: in_dir = "/tmp/some_dir", in_basename = "/some_content/foo.c",
 * replace = ".asm" => in_dir = "/tmp/some_dir/foo.c.asm"
 *
 * */
void fill_pathname_dir(char *in_dir, const char *in_basename,
      const char *replace, size_t size);

/* Copies basename of in_path into out_path. */
void fill_pathname_base(char *out_path, const char *in_path, size_t size);

/* Copies base directory of in_path into out_path.
 * If in_path is a path without any slashes (relative current directory),
 * out_path will get path "./". */
void fill_pathname_basedir(char *out_path, const char *in_path, size_t size);

/* Copies parent directory of in_dir into out_dir.
 * Assumes in_dir is a directory. Keeps trailing '/'. */
void fill_pathname_parent_dir(char *out_dir,
      const char *in_dir, size_t size);

/* Joins basedir of in_refpath together with in_path.
 * If in_path is an absolute path, out_path = in_path.
 * E.g.: in_refpath = "/foo/bar/baz.a", in_path = "foobar.cg",
 * out_path = "/foo/bar/foobar.cg". */
void fill_pathname_resolve_relative(char *out_path, const char *in_refpath,
      const char *in_path, size_t size);

/* Joins a directory and path together. Makes sure not to get 
 * two consecutive slashes between dir and path. */
void fill_pathname_join(char *out_path, const char *dir,
      const char *path, size_t size);

/* Joins a directory and path together using the given char. */
void fill_pathname_join_delim(char *out_path, const char *dir,
      const char *path, const char delim, size_t size);

/* Generates a short representation of path. It should only
 * be used for displaying the result; the output representation is not
 * binding in any meaningful way (for a normal path, this is the same as basename)
 * In case of more complex URLs, this should cut everything except for
 * the main image file.
 * E.g.: "/path/to/game.img" -> game.img
 *       "/path/to/myarchive.7z#folder/to/game.img" -> game.img
 */
void fill_short_pathname_representation(char* out_rep,
      const char *in_path, size_t size);

void fill_pathname_expand_special(char *out_path,
      const char *in_path, size_t size);

void fill_pathname_abbreviate_special(char *out_path,
      const char *in_path, size_t size);

void fill_pathname_slash(char *path, size_t size);

#ifndef RARCH_CONSOLE
void fill_pathname_application_path(char *buf, size_t size);
#endif

#ifdef __cplusplus
}
#endif

#endif
