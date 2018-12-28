/*======
This file is part of Percona Server for MongoDB.

Copyright (C) 2018-present Percona and/or its affiliates. All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the Server Side Public License, version 1,
    as published by MongoDB, Inc.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    Server Side Public License for more details.

    You should have received a copy of the Server Side Public License
    along with this program. If not, see
    <http://www.mongodb.com/licensing/server-side-public-license>.

    As a special exception, the copyright holders give permission to link the
    code of portions of this program with the OpenSSL library under certain
    conditions as described in each individual source file and distribute
    linked combinations including the program with the OpenSSL library. You
    must comply with the Server Side Public License in all respects for
    all of the code used other than as permitted herein. If you modify file(s)
    with this exception, you may extend this exception to your version of the
    file(s), but you are not obligated to do so. If you do not wish to do so,
    delete this exception statement from your version. If you delete this
    exception statement from all source files in the program, then also delete
    it in the license file.
======= */

#pragma once

#include <openssl/evp.h>

#include "mongo/db/storage/encryption_hooks.h"

namespace mongo {

class WiredTigerEncryptionHooks: public EncryptionHooks
{
public:
    WiredTigerEncryptionHooks();
    virtual ~WiredTigerEncryptionHooks();

    /**
     * Returns true if the encryption hooks are enabled.
     */
    virtual bool enabled() const override;

    /**
     * Perform any encryption engine initialization/sanity checking that needs to happen after
     * storage engine initialization but before the server starts accepting incoming connections.
     *
     * Returns true if the server needs to be rebooted because of configuration changes.
     */
    virtual bool restartRequired() override;

protected:
    static constexpr int _key_len{32};
    unsigned char _masterkey[_key_len];
    const EVP_CIPHER *_cipher{nullptr};
    int _iv_len;
};

class WiredTigerEncryptionHooksCBC: public WiredTigerEncryptionHooks
{
public:
    WiredTigerEncryptionHooksCBC();
    virtual ~WiredTigerEncryptionHooksCBC();

    /**
     * Transform temp data to non-readable form before writing it to disk.
     */
    virtual Status protectTmpData(
        const uint8_t* in, size_t inLen, uint8_t* out, size_t outLen, size_t* resultLen) override;

    /**
     * Tranforms temp data back to readable form, after reading from disk.
     */
    virtual Status unprotectTmpData(
        const uint8_t* in, size_t inLen, uint8_t* out, size_t outLen, size_t* resultLen) override;

    /**
     * Returns the maximum size addition when doing transforming temp data.
     */
    virtual size_t additionalBytesForProtectedBuffer() override;

    /**
     * Get the data protector object
     */
    virtual std::unique_ptr<DataProtector> getDataProtector() override;

    /**
     * Get an implementation specific path suffix to tag files with
     */
    virtual boost::filesystem::path getProtectedPathSuffix() override;

private:
    static constexpr int _chksum_len{sizeof(uint32_t)};
    uint32_t (*wiredtiger_checksum_crc32c)(const void *, size_t);
};

class WiredTigerEncryptionHooksGCM: public WiredTigerEncryptionHooks
{
public:
    WiredTigerEncryptionHooksGCM();
    virtual ~WiredTigerEncryptionHooksGCM();

    /**
     * Transform temp data to non-readable form before writing it to disk.
     */
    virtual Status protectTmpData(
        const uint8_t* in, size_t inLen, uint8_t* out, size_t outLen, size_t* resultLen) override;

    /**
     * Tranforms temp data back to readable form, after reading from disk.
     */
    virtual Status unprotectTmpData(
        const uint8_t* in, size_t inLen, uint8_t* out, size_t outLen, size_t* resultLen) override;

    /**
     * Returns the maximum size addition when doing transforming temp data.
     */
    virtual size_t additionalBytesForProtectedBuffer() override;

    /**
     * Get the data protector object
     */
    virtual std::unique_ptr<DataProtector> getDataProtector() override;

    /**
     * Get an implementation specific path suffix to tag files with
     */
    virtual boost::filesystem::path getProtectedPathSuffix() override;

private:
    static constexpr int _gcm_tag_len{16};
};

}  // namespace mongo
