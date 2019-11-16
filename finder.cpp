#include "finder.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QQueue>
#include <QTextStream>
#include <QThread>

Finder::Finder(const QString &dir, const QString& s)
    : Dir_(dir), Substr_(s)
{
}

void Finder::findSubstr()
{
    findFiles(Dir_);
    emit QThread::currentThread()->quit();
    emit completeDir();
}

void Finder::findFiles(const QString &dir)
{
    QFileInfo info(dir);
    if (info.exists() && info.isFile()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }
        searchSubstr(dir);
        return;
    }
    QDir d(dir);
    QStringList files = d.entryList(QDir::Files);
    for(const auto& file: files) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }
        searchSubstr(dir + "/" + file);
    }

    QStringList dirs = d.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (const auto& item: dirs) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }
        findFiles(dir + "/" + item);
    }
}

void Finder::searchSubstr(const QString &file_name)
{
    QFile file(file_name);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        uint32_t hash = 0;
        uint32_t hash_substr = 0;
        uint32_t max_power = 1;
        int count = 0;
        int line_count = 0;
        QString indexes = "Entrances in format \"line:number in string\" ";
        QString symbol;
        QVector<uint32_t> pr;
        for(int i = 0; i < Substr_.size(); ++i) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                return;
            }
            QChar a = Substr_[Substr_.size()-i - 1];
            hash_substr += a.unicode() * max_power;
            pr.push_back(max_power);
            max_power *= P_;
        }
        max_power = pr.back();
        QQueue<QChar> q;
        uint32_t power = max_power;
        int k = 0;
        for(int i = 0; i < Substr_.size(); ++i, ++k) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                return;
            }
            if ((symbol = stream.read(1)).isEmpty()) {
                return;
            }
            if (symbol == "\n" || symbol == "\r") {
                line_count++;
                k = 0;
            }
            hash += pr[Substr_.size() - i - 1] * symbol[0].unicode();
            q.enqueue(symbol[0]);
        }
        if (hash == hash_substr) {
            count++;
            indexes += QString::number(line_count) + ":" + QString::number(k) + " ";
        }

        while(!(symbol = stream.read(1)).isEmpty()) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                if (count > 0) {
                    emit onSubstrFound(file_name, indexes, count);
                }
                return;
            }
            hash -= q.dequeue().unicode()*max_power;
            hash *= P_;
            hash += symbol[0].unicode();
            q.enqueue(symbol[0].unicode());
            if (hash == hash_substr) {
                count++;
                indexes += QString::number(line_count) + ":" + QString::number(k) + " ";
                if (count == MAX_ENTERENCES) {
                    break;
                }
            }
            ++k;
            if (symbol == "\n" || symbol == "\r") {
                line_count++;
                k = 0;
            }
        }
        if (count > 0) {
            emit onSubstrFound(file_name, indexes, count);
        }
    } else {
        qDebug() << "Can't open file" + file_name;
    }
}

