#include "crypto.h"

QScopedPointer<SimpleCrypt> g_Crypt;

SimpleCrypt& standardCrypt()
{
    if ( !g_Crypt )
    {
        g_Crypt.reset(new SimpleCrypt(ENCRYPTION_KEY));
        g_Crypt->setCompressionMode(SimpleCrypt::CompressionNever);
        g_Crypt->setIntegrityProtectionMode(SimpleCrypt::ProtectionChecksum);
    }
    return *g_Crypt;
}
