#include "stringconvert.h"

std::string toString(const QString &str)
{
#ifdef win32
    return std::string(str.toLocal8Bit().constData());
#else
    return std::string(str.toUtf8().constData());
#endif
}
