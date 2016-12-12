

#ifndef TEXTPROGRESSBAR_H
#define TEXTPROGRESSBAR_H

#include <QString>

#include "build_download.h"
class DOWNLOAD_EXPORT TextProgressBar
{
public:
    TextProgressBar();

    void clear();
    void update();
    void setMessage(const QString &message);
    void setStatus(qint64 value, qint64 maximum);

private:
    QString message;
    qint64 value;
    qint64 maximum;
    int iteration;
};

#endif
