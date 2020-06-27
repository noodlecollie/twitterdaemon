#ifndef FILEMANAGEMENT_H
#define FILEMANAGEMENT_H

#include <QString>
#include "keyvaluesnode.h"

namespace FileManagement
{
    bool load(const QString &filename, QString &output);
    bool save(const QString &filename, const QString &input);
    void remove(const QString &filename);
    bool writeKVFile(const QString &filename, const KeyValuesNode &kv);
    bool readKVFile(const QString &filename, KeyValuesNode &kv);
}

#endif // FILEMANAGEMENT_H
