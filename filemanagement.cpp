#include "filemanagement.h"
#include <QFile>
#include <QTextStream>

namespace FileManagement
{
    bool load(const QString &filename, QString &output)
    {
        QFile file(filename);

        if ( !file.open(QFile::ReadOnly) ) return false;

        QTextStream in(&file);
        output = in.readAll();
        file.close();
        return true;
    }

    bool save(const QString &filename, const QString &input)
    {
        QFile file(filename);

        if ( !file.open(QFile::WriteOnly) ) return false;

        QTextStream out(&file);
        out << input;
        file.close();
        return true;
    }

    void remove(const QString &filename)
    {
        QFile file(filename);
        file.remove();
    }

    bool writeKVFile(const QString &filename, const KeyValuesNode &kv)
    {
        QFile file(filename);
        if ( !file.open(QFile::WriteOnly) ) return false;

        QTextStream stream(&file);
        stream << kv;

        file.close();
        return true;
    }

    bool readKVFile(const QString &filename, KeyValuesNode &kv)
    {
        QFile file(filename);
        if ( !file.open(QFile::ReadOnly) ) return false;

        // TODO: validation!
        QTextStream stream(&file);
        kv.clear();
        kv.constructFrom(stream.readAll());

        file.close();
        return true;
    }
}
