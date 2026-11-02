/*
 * Command Line Interface for Partoska.com media sharing service.
 * Copyright (C) 2026 Fabrika Charvat s.r.o. All rights reserved.
 * Developed by Partoska Laboratory team, <https://lab.partoska.com>
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * You can contact the author(s) via email at ask <at> partoska.com.
 */

#ifndef PKC_PLAT_H
#define PKC_PLAT_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** Maximum number of options in a table of pkcOptNext(). */
#define PKC_OPT_MAX 16

typedef struct PKCDir PKCDir;
/** Opaque open file, from pkcFileOpen(); released by pkcFileClose(). */
typedef struct PKCFile PKCFile;

/** Command line option, for pkcOptNext(); a NULL name ends a table. */
typedef struct PKCOpt
{
  const PKChar *name; /**< Long name, without the dashes ("dir"). */
  PKInt key;          /**< Short name, returned when found ('D'). */
  PKBool value;       /**< Takes a value (--dir=x, --dir x, -Dx, -D x). */
} PKCOpt;

/** Receives SIGINT and SIGTERM; see pkcSignals(). */
typedef PKVoid (*PKCSignalFn) (PKInt signal);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - Strings
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Measures a string.
 *
 * @param str NUL-terminated string.
 * @return Number of bytes before the terminating NUL.
 */
PKSize pkcStrLen (const PKChar *str);

/**
 * Compares two strings byte by byte, case-sensitively.
 *
 * @param a NUL-terminated string.
 * @param b NUL-terminated string.
 * @return PK_TRUE when both hold the same bytes.
 */
PKBool pkcStrEq (const PKChar *a, const PKChar *b);

/**
 * Compares at most the first n bytes of two strings, case-sensitively.
 *
 * @param a String; stops at its NUL if shorter than n.
 * @param b String; stops at its NUL if shorter than n.
 * @param n Maximum number of bytes to compare.
 * @return PK_TRUE when the first n bytes (or both strings) are the same.
 */
PKBool pkcStrEqN (const PKChar *a, const PKChar *b, PKSize n);

/**
 * Finds the first occurrence of any character from a set.
 *
 * @param str NUL-terminated string to search.
 * @param set NUL-terminated characters to find.
 * @return Pointer into str at the first match, or NULL when there is none.
 */
PKChar *pkcStrFindAny (const PKChar *str, const PKChar *set);

/**
 * Finds the first occurrence of a substring.
 *
 * @param str NUL-terminated string to search.
 * @param sub NUL-terminated string to find.
 * @return Pointer into str at the first sub, or NULL when there is none.
 */
PKChar *pkcStrFindStr (const PKChar *str, const PKChar *sub);

/**
 * Copies the start of a string into a new one; release it with
 * pkcMemFree().
 *
 * @param str    Bytes to copy; need not be NUL-terminated.
 * @param length Number of bytes to copy.
 * @return New NUL-terminated string of length bytes, or NULL when out of
 *         memory.
 */
PKChar *pkcStrDup (const PKChar *str, PKSize length);

/**
 * Formats into a buffer like snprintf(), truncating to fit.
 *
 * @param dst  Buffer; always NUL-terminated when size is not 0.
 * @param size Capacity of dst in bytes.
 * @param fmt  printf()-style format.
 * @param ...  Arguments of fmt.
 * @return Length of the full result without the NUL, which is size or more
 *         when truncated; negative on an encoding error.
 */
PKInt pkcStrFmt (PKChar *dst, PKSize size, const PKChar *fmt, ...)
    PK_PRINTF (3, 4);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - Memory
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Allocates uninitialized memory; release it with pkcMemFree().
 *
 * @param size Number of bytes; 0 still gives a valid, unique block.
 * @return New block, or NULL when out of memory.
 */
PKVoid *pkcMemAlloc (PKSize size);

/**
 * Allocates zeroed memory; release it with pkcMemFree().
 *
 * @param size Number of bytes; 0 still gives a valid, unique block.
 * @return New block, or NULL when out of memory.
 */
PKVoid *pkcMemZalloc (PKSize size);

/**
 * Releases memory from pkcMemAlloc(), pkcMemZalloc() or pkcStrDup().
 * Accepts NULL.
 *
 * @param ptr Block to release.
 */
PKVoid pkcMemFree (PKVoid *ptr);

/**
 * Sets bytes to zero.
 *
 * @param ptr  Block to clear.
 * @param size Number of bytes.
 */
PKVoid pkcMemZero (PKVoid *ptr, PKSize size);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - Files and console
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Performs platform-specific console initialization (e.g., UTF-8 mode on
 * Windows, with the previous code page restored at exit).
 */
PKVoid pkcConsoleInit (void);

/**
 * Returns whether the given path refers to an existing directory.
 *
 * @param path Path to test.
 * @return PK_TRUE if path is a directory, PK_FALSE otherwise.
 */
PKBool pkcIsDir (const PKChar *path);

/**
 * Returns whether the given path refers to an existing regular file.
 *
 * @param path Path to test.
 * @return PK_TRUE if path is a file, PK_FALSE otherwise.
 */
PKBool pkcIsFile (const PKChar *path);

/**
 * Creates a directory, including any missing parent directories.
 *
 * @param path Directory path to create.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkcCreateDir (const PKChar *path);

/**
 * Opens a file in binary mode, so bytes pass unchanged on every platform.
 *
 * @param path  File to open, in UTF-8.
 * @param write PK_FALSE to read; PK_TRUE to write, creating the file or
 *              truncating it.
 * @return Open file, or NULL on failure.
 */
PKCFile *pkcFileOpen (const PKChar *path, PKBool write);

/**
 * Reads from a file.
 *
 * @param file File opened for reading.
 * @param data Buffer to fill.
 * @param size Capacity of the buffer in bytes.
 * @return Number of bytes stored; less than size at the end of the file or
 *         on failure, which pkcFileFailed() tells apart.
 */
PKSize pkcFileRead (PKCFile *file, PKVoid *data, PKSize size);

/**
 * Writes to a file.
 *
 * @param file File opened for writing.
 * @param data Bytes to write.
 * @param size Number of bytes.
 * @return Number of bytes written; anything but size is a failure.
 */
PKSize pkcFileWrite (PKCFile *file, const PKVoid *data, PKSize size);

/**
 * Reports whether a read or write on a file failed.
 *
 * @param file Open file.
 * @return PK_TRUE once any read or write on file has failed.
 */
PKBool pkcFileFailed (PKCFile *file);

/**
 * Closes a file, writing out buffered data; the file is released even when
 * that fails.
 *
 * @param file Open file; not NULL.
 * @return PK_FALSE when buffered data could not be written.
 */
PKBool pkcFileClose (PKCFile *file);

/**
 * Renames a file or directory.
 *
 * @param from Current path, in UTF-8.
 * @param to   New path, in UTF-8.
 * @return PK_TRUE on success.
 */
PKBool pkcFileRename (const PKChar *from, const PKChar *to);

/**
 * Opens a directory for iteration.
 *
 * @param path Directory path to open.
 * @return Opaque PKCDir handle, or NULL on failure.
 */
PKCDir *pkcOpenDir (const PKChar *path);

/**
 * Returns the name of the next entry in an open directory.
 *
 * @param dir Open directory handle.
 * @return Pointer to the next entry name, or NULL when the directory is
 * exhausted.
 */
const PKChar *pkcReadDir (PKCDir *dir);

/**
 * Closes a directory handle opened with pkcOpenDir.
 *
 * @param dir Directory handle to close.
 */
PKVoid pkcCloseDir (PKCDir *dir);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - Options
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Starts parsing options again from argv[1], with any argument vector.
 */
PKVoid pkcOptReset (void);

/**
 * Finds the next option of an argument vector, like getopt_long().
 *
 * @param argc  Number of arguments.
 * @param argv  Arguments; argv[0] is skipped.
 * @param opts  Known options, ended by an entry with a NULL name; at most
 *              PKC_OPT_MAX.
 * @param value Receives the value of an option that takes one, else NULL.
 * @return Key of the option; '?' for an unknown option, a missing value or
 *         too many options; -1 after the last option.
 */
PKInt pkcOptNext (PKInt argc, PKChar **argv, const PKCOpt *opts,
                  const PKChar **value);

/**
 * Tells where the arguments after the options start, once pkcOptNext()
 * returned -1.
 *
 * @return Index into argv of the first argument that is not an option;
 *         argc when there is none.
 */
PKInt pkcOptIndex (void);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - Process
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Prints formatted text to the standard output or error.
 *
 * @param error PK_TRUE for the standard error, PK_FALSE for the output.
 * @param fmt   printf()-style format.
 * @param args  Arguments of fmt.
 * @return Number of characters printed, or a negative value on error.
 */
PKInt pkcPrintV (PKBool error, const PKChar *fmt, PKVaList args);

/**
 * Writes out what is buffered for the standard output, so a reader on the
 * other end of a pipe sees it now.
 */
PKVoid pkcFlush (void);

/**
 * Routes SIGINT and SIGTERM to a handler, or back to their defaults.
 *
 * @param handler Receives the signals; NULL restores the defaults.
 */
PKVoid pkcSignals (PKCSignalFn handler);

/**
 * Gets the calendar time.
 *
 * @return Seconds since the epoch.
 */
PKTime pkcTimeNow (void);

/**
 * Parses a date at the start of a string, "YYYY-MM-DD".
 *
 * @param text  NUL-terminated string.
 * @param year  Receives the year.
 * @param month Receives the month, 1 to 12.
 * @param day   Receives the day of the month.
 * @return PK_TRUE when the string starts with a date.
 */
PKBool pkcDateParse (const PKChar *text, PKInt *year, PKInt *month,
                     PKInt *day);

#endif
