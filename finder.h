#ifndef FINDER_H
#define FINDER_H
#include <QObject>

class Finder : public QObject
{
    Q_OBJECT
public:
    Finder(const QString& dir, const QString& s);
signals:
    void onSubstrFound(const QString& file_name, const QString& enterences, int count);
public slots:
    void findSubstr();
private:
    void findFiles(const QString& dir);
    void searchSubstr(const QString& file_name);
private:
    bool NeedToStop = false;
    QString Dir_;
    QString Substr_;
    static const uint32_t P_ = 31;
};

#endif // FINDER_H
