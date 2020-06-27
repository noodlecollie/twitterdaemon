#ifndef CRYPTO_H
#define CRYPTO_H

#include "simplecrypt.h"
#include <QScopedPointer>

// Right now, cheap way of managing passwords.
// This isn't tremendously secure but it's better than storing the password in plain text.
#define ENCRYPTION_KEY 0xDEADB00B0BEEFDAD   // I'm childish

SimpleCrypt& standardCrypt();

#endif // CRYPTO_H
