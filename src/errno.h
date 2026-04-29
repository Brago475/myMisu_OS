#ifndef ERRNO_H
#define ERRNO_H

/* POSIX-style error codes returned as negative values from syscalls.
 * A successful syscall returns >= 0. A failed syscall returns -E<NAME>.
 *
 * Numbers match Linux i386 errno values.
 */

#define EPERM        1   /* Operation not permitted        */
#define ENOENT       2   /* No such file or directory      */
#define ESRCH        3   /* No such process                */
#define EINTR        4   /* Interrupted system call        */
#define EIO          5   /* I/O error                      */
#define EBADF        9   /* Bad file descriptor            */
#define ECHILD      10   /* No child processes             */
#define EAGAIN      11   /* Try again / would block        */
#define ENOMEM      12   /* Out of memory                  */
#define EACCES      13   /* Permission denied              */
#define EFAULT      14   /* Bad address (NULL pointer etc) */
#define EBUSY       16   /* Device or resource busy        */
#define EEXIST      17   /* File exists                    */
#define ENOTDIR     20   /* Not a directory                */
#define EISDIR      21   /* Is a directory                 */
#define EINVAL      22   /* Invalid argument               */
#define ENFILE      23   /* File table overflow            */
#define EMFILE      24   /* Too many open files            */
#define ENOSPC      28   /* No space left on device        */
#define ESPIPE      29   /* Illegal seek                   */
#define ENAMETOOLONG 36  /* File name too long             */
#define ENOSYS      38   /* Function not implemented       */
#define ENOTEMPTY   39   /* Directory not empty            */

#endif
