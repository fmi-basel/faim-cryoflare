#ifndef MRCIOHANDLER_H
#define MRCIOHANDLER_H

#include <QImageIOHandler>

class MRCIOHandler : public QImageIOHandler
{
public:
    MRCIOHandler();
    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const;
    void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;
protected:
    bool isBigEndian_() const;
};

#endif // MRCIOHANDLER_H
