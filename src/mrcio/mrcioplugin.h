#ifndef MRCIOPLUGIN_H
#define MRCIOPLUGIN_H

#include <QImageIOPlugin>

class MRCIOPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "mrcioplugin.json")

public:
    MRCIOPlugin();
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

#endif // MRCIOPLUGIN_H
