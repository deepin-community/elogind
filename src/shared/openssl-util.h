/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#if HAVE_OPENSSL
#  include <openssl/pem.h>

DEFINE_TRIVIAL_CLEANUP_FUNC(X509*, X509_free);
DEFINE_TRIVIAL_CLEANUP_FUNC(X509_NAME*, X509_NAME_free);
DEFINE_TRIVIAL_CLEANUP_FUNC(EVP_PKEY_CTX*, EVP_PKEY_CTX_free);
DEFINE_TRIVIAL_CLEANUP_FUNC(EVP_CIPHER_CTX*, EVP_CIPHER_CTX_free);

#endif
