/*
 * Copyright 2007-2017 Content Management AG
 * All rights reserved.
 *
 * author: Max Kellermann <mk@cm4all.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NETSTRING_SERVER_HXX
#define NETSTRING_SERVER_HXX

#include "NetstringInput.hxx"
#include "NetstringGenerator.hxx"
#include "io/MultiWriteBuffer.hxx"
#include "net/UniqueSocketDescriptor.hxx"
#include "event/SocketEvent.hxx"

#include <exception>
#include <cstddef>

/**
 * A server that receives netstrings
 * (http://cr.yp.to/proto/netstrings.txt) from its clients and
 * responds with another netstring.
 */
class NetstringServer {
    UniqueSocketDescriptor fd;

    SocketEvent event;

    NetstringInput input;
    NetstringGenerator generator;
    MultiWriteBuffer write;

public:
    NetstringServer(EventLoop &event_loop, UniqueSocketDescriptor &&_fd);
    ~NetstringServer();

protected:
    int GetFD() const {
        return fd.Get();
    }

    bool SendResponse(const void *data, size_t size);
    bool SendResponse(const char *data);

    /**
     * A netstring has been received.
     *
     * @param payload the netstring value; for the implementation's
     * convenience, the netstring is writable
     */
    virtual void OnRequest(AllocatedArray<uint8_t> &&payload) = 0;
    virtual void OnError(std::exception_ptr ep) = 0;
    virtual void OnDisconnect() = 0;

private:
    void OnEvent(unsigned events);
};

#endif
