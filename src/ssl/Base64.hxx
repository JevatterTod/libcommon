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

/*
 * OpenSSL BIO_f_base64() wrapper.
 */

#ifndef SSL_BASE64_HXX
#define SSL_BASE64_HXX

#include "MemBio.hxx"
#include "Unique.hxx"
#include "Error.hxx"

#include <string>

#include <string.h>

/**
 * Call a function that writes into a memory BIO and return the BIO
 * memory as Base64-encoded #AllocatedString instance.
 */
template<typename W>
static inline AllocatedString<>
BioWriterToBase64String(W &&writer)
{
	return BioWriterToString([&writer](BIO &bio){
			UniqueBIO b64(BIO_new(BIO_f_base64()));
			if (!b64)
				throw SslError("BIO_new() failed");

			BIO_push(b64.get(), &bio);
			BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
			writer(*b64);
			(void)BIO_flush(b64.get());
		});
}

static inline AllocatedString<>
Base64(ConstBuffer<void> data)
{
	return BioWriterToBase64String([data](BIO &bio){
			BIO_write(&bio, data.data, data.size);
		});
}

static inline AllocatedString<>
Base64(const std::string &s)
{
	return Base64(ConstBuffer<void>(s.data(), s.length()));
}

static inline AllocatedString<>
Base64(const BIGNUM &bn)
{
	return BioWriterToBase64String([&bn](BIO &bio){
			size_t size = BN_num_bytes(&bn);
			std::unique_ptr<unsigned char[]> data(new unsigned char[size]);
			BN_bn2bin(&bn, data.get());
			BIO_write(&bio, data.get(), size);
		});
}

static inline AllocatedString<>
Base64(X509_REQ &req)
{
	return BioWriterToBase64String([&req](BIO &bio){
			i2d_X509_REQ_bio(&bio, &req);
		});
}

template<typename T>
static inline AllocatedString<>
UrlSafeBase64(T &&t)
{
	auto s = Base64(std::forward<T>(t));
	char *p = s.data();
	char *end = p + strlen(p);
	while (end > p && end[-1] == '=')
		*--end = 0;

	std::replace(p, end, '+', '-');
	std::replace(p, end, '/', '_');

	return s;
}

static inline AllocatedString<>
UrlSafeBase64SHA256(ConstBuffer<void> data)
{
	unsigned char buffer[SHA256_DIGEST_LENGTH];
	SHA256((const unsigned char *)data.data, data.size, buffer);
	return UrlSafeBase64(ConstBuffer<void>(buffer, sizeof(buffer)));
}

static inline AllocatedString<>
UrlSafeBase64SHA256(const std::string &s)
{
	return UrlSafeBase64SHA256(ConstBuffer<void>(s.data(), s.length()));
}

#endif
