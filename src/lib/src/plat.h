/*
 * Partoska Library - Client library for Partoska.com media sharing service.
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

#ifndef PK_PLAT_H
#define PK_PLAT_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** Opaque open file, from pkFileOpen(); released by pkFileClose(). */
typedef struct PKFile PKFile;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - Memory
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Allocates zeroed memory; release it with pkMemFree().
 *
 * @param size Number of bytes; 0 still gives a valid, unique block.
 * @return New block, or NULL when out of memory.
 */
PKVoid *pkMemZalloc (PKSize size);

/**
 * Allocates a zeroed array; release it with pkMemFree().
 *
 * @param count Number of elements.
 * @param size  Size of an element in bytes.
 * @return New block, or NULL when out of memory or when count * size
 *         overflows.
 */
PKVoid *pkMemZallocArray (PKSize count, PKSize size);

/**
 * Resizes a block, keeping its contents up to the smaller size; bytes
 * beyond the old size are uninitialized.
 *
 * @param ptr  Block from pkMemZalloc() or pkMemRealloc(), or NULL for a new
 *             one.
 * @param size New size in bytes; 0 still gives a valid, unique block.
 * @return Resized block, possibly moved, or NULL when out of memory; ptr
 *         then stays valid and unchanged.
 */
PKVoid *pkMemRealloc (PKVoid *ptr, PKSize size);

/**
 * Releases memory from pkMemZalloc(), pkMemRealloc() or pkStrDup().
 * Accepts NULL.
 *
 * @param ptr Block to release.
 */
PKVoid pkMemFree (PKVoid *ptr);

/**
 * Copies bytes between blocks that do not overlap.
 *
 * @param dst  Destination, at least size bytes.
 * @param src  Source, at least size bytes.
 * @param size Number of bytes.
 */
PKVoid pkMemCopy (PKVoid *dst, const PKVoid *src, PKSize size);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - Strings
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Measures a string.
 *
 * @param str NUL-terminated string.
 * @return Number of bytes before the terminating NUL.
 */
PKSize pkStrLen (const PKChar *str);

/**
 * Compares two strings, byte by byte or ASCII case-insensitively; locale
 * settings play no part.
 *
 * @param a     NUL-terminated string.
 * @param b     NUL-terminated string.
 * @param icase PK_TRUE to treat A to Z as a to z.
 * @return Negative, zero or positive as a sorts before, equal to or after
 *         b.
 */
PKInt pkStrCmp (const PKChar *a, const PKChar *b, PKBool icase);

/**
 * Compares at most the first n bytes of two strings, case-sensitively.
 *
 * @param a String; stops at its NUL if shorter than n.
 * @param b String; stops at its NUL if shorter than n.
 * @param n Maximum number of bytes to compare.
 * @return Negative, zero or positive as a sorts before, equal to or after
 *         b.
 */
PKInt pkStrCmpN (const PKChar *a, const PKChar *b, PKSize n);

/**
 * Finds the first occurrence of a character.
 *
 * @param str NUL-terminated string to search.
 * @param c   Character to find; '\0' finds the terminator.
 * @return Pointer into str at the first c, or NULL when there is none.
 */
PKChar *pkStrFind (const PKChar *str, PKChar c);

/**
 * Finds the last occurrence of a character.
 *
 * @param str NUL-terminated string to search.
 * @param c   Character to find; '\0' finds the terminator.
 * @return Pointer into str at the last c, or NULL when there is none.
 */
PKChar *pkStrFindLast (const PKChar *str, PKChar c);

/**
 * Finds the first occurrence of any character from a set.
 *
 * @param str NUL-terminated string to search.
 * @param set NUL-terminated characters to find.
 * @return Pointer into str at the first match, or NULL when there is none.
 */
PKChar *pkStrFindAny (const PKChar *str, const PKChar *set);

/**
 * Copies the start of a string into a new one; release it with
 * pkMemFree().
 *
 * @param str    Bytes to copy; need not be NUL-terminated.
 * @param length Number of bytes to copy.
 * @return New NUL-terminated string of length bytes, or NULL when out of
 *         memory.
 */
PKChar *pkStrDup (const PKChar *str, PKSize length);

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
PKInt pkStrFmt (PKChar *dst, PKSize size, const PKChar *fmt, ...)
    PK_PRINTF (3, 4);

/**
 * Formats into a buffer like vsnprintf(), as pkStrFmt() does.
 *
 * @param dst  Buffer; always NUL-terminated when size is not 0.
 * @param size Capacity of dst in bytes.
 * @param fmt  printf()-style format.
 * @param args Arguments of fmt.
 * @return Length of the full result without the NUL, which is size or more
 *         when truncated; negative on an encoding error.
 */
PKInt pkStrFmtV (PKChar *dst, PKSize size, const PKChar *fmt, PKVaList args);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - Files
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Returns whether the given path refers to an existing directory.
 *
 * @param path Path to test.
 * @return PK_TRUE if path is a directory, PK_FALSE otherwise.
 */
PKBool pkIsDir (const PKChar *path);

/**
 * Creates a directory, including any missing parent directories.
 *
 * @param path Directory path to create.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkCreateDir (const PKChar *path);

/**
 * Returns whether the given path refers to an existing regular file.
 *
 * @param path Path to test.
 * @return PK_TRUE if path is a file, PK_FALSE otherwise.
 */
PKBool pkIsFile (const PKChar *path);

/**
 * Sets file permissions to owner-read/write only (mode 0600).
 *
 * @param path Path to the file.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkChmod600 (const PKChar *path);

/**
 * Opens a file in binary mode, so bytes pass unchanged on every platform.
 *
 * @param path  File to open, in UTF-8.
 * @param write PK_FALSE to read; PK_TRUE to write, creating the file or
 *              truncating it.
 * @return Open file, or NULL on failure, with pkSysError() telling why.
 */
PKFile *pkFileOpen (const PKChar *path, PKBool write);

/**
 * Reads from a file.
 *
 * @param file File opened for reading.
 * @param data Buffer to fill.
 * @param size Capacity of the buffer in bytes.
 * @return Number of bytes stored; less than size at the end of the file or
 *         on failure, which pkFileFailed() tells apart.
 */
PKSize pkFileRead (PKFile *file, PKVoid *data, PKSize size);

/**
 * Writes to a file.
 *
 * @param file File opened for writing.
 * @param data Bytes to write.
 * @param size Number of bytes.
 * @return Number of bytes written; anything but size is a failure.
 */
PKSize pkFileWrite (PKFile *file, const PKVoid *data, PKSize size);

/**
 * Reports whether a read or write on a file failed.
 *
 * @param file Open file.
 * @return PK_TRUE once any read or write on file has failed.
 */
PKBool pkFileFailed (PKFile *file);

/**
 * Gets the size of a file opened for reading, leaving it at the start.
 *
 * @param file File opened for reading.
 * @param size Receives the size in bytes.
 * @return PK_TRUE on success, PK_FALSE otherwise (pkSysError() tells why).
 */
PKBool pkFileSize (PKFile *file, PKLong *size);

/**
 * Closes a file, writing out buffered data; the file is released even when
 * that fails.
 *
 * @param file Open file; not NULL.
 * @return PK_FALSE when buffered data could not be written.
 */
PKBool pkFileClose (PKFile *file);

/**
 * Removes a file.
 *
 * @param path File to remove, in UTF-8.
 * @return PK_TRUE when removed, PK_FALSE otherwise (errno tells why).
 */
PKBool pkFileRemove (const PKChar *path);

/**
 * Sleeps.
 *
 * @param ms Milliseconds to sleep.
 */
PKVoid pkSleep (PKLong ms);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - Time
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Gets the calendar time.
 *
 * @return Seconds since the epoch.
 */
PKTime pkTimeNow (void);

/**
 * Gets a monotonic clock in milliseconds. It wraps around every 49 days, so
 * only differences of two readings (unsigned, wrapping) are meaningful.
 *
 * @return Milliseconds since an unspecified start.
 */
PKDword pkClockMs (void);

/**
 * Formats a calendar time as decimal seconds since the epoch, in full on
 * every platform.
 *
 * @param dst  Buffer; always NUL-terminated when size is not 0.
 * @param size Capacity of dst in bytes.
 * @param time Time to format.
 * @return As pkStrFmt().
 */
PKInt pkStrFmtTime (PKChar *dst, PKSize size, PKTime time);

/**
 * Parses decimal seconds since the epoch, as pkStrFmtTime() writes them.
 *
 * @param str NUL-terminated string.
 * @return The time, or 0 when the string does not start with a number.
 */
PKTime pkStrToTime (const PKChar *str);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - Environment
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Gets an environment variable.
 *
 * @param name Variable name.
 * @return Value owned by the C library, or NULL when unset.
 */
const PKChar *pkEnvGet (const PKChar *name);

/**
 * Gets the current directory.
 *
 * @param buf  Receives the path.
 * @param size Capacity of buf in bytes.
 * @return PK_TRUE on success, PK_FALSE otherwise (pkSysError() tells why).
 */
PKBool pkCwd (PKChar *buf, PKSize size);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations - System errors
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Gets the code of the last system error.
 *
 * @return errno, as left by the last failed system call.
 */
PKInt pkSysError (void);

/**
 * Describes a system error code.
 *
 * @param code System error code, e.g. from pkSysError().
 * @return Human-readable text owned by the C library; valid until the next
 *         call.
 */
const PKChar *pkSysErrorText (PKInt code);

#endif
