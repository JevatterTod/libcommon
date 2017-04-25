/*
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "BindMount.hxx"
#include "system/Error.hxx"

#include <sys/mount.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

void
BindMount(const char *source, const char *target, int flags)
{
    if (mount(source, target, nullptr, MS_BIND, nullptr) < 0)
        throw FormatErrno("bind_mount('%s', '%s') failed", source, target);

    /* wish we could just pass additional flags to the first mount
       call, but unfortunately that doesn't work, the kernel ignores
       these flags */
    if (flags != 0 &&
        mount(nullptr, target, nullptr, MS_REMOUNT|MS_BIND|flags,
              nullptr) < 0 &&

        /* after EPERM, try again with MS_NOEXEC just in case this
           missing flag was the reason for the kernel to reject our
           request */
        (errno != EPERM ||
         (flags & MS_NOEXEC) != 0 ||
         mount(nullptr, target, nullptr, MS_REMOUNT|MS_BIND|MS_NOEXEC|flags,
               nullptr) < 0))
        throw FormatErrno("remount('%s') failed", target);
}