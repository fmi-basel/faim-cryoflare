#include "mrciohandler.h"
#include "mrcioplugin.h"

MRCIOPlugin::MRCIOPlugin()
{
}

QStringList MRCIOPlugin::keys() const
{
	return QStringList() << "mrc";
}

QImageIOPlugin::Capabilities MRCIOPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
	if (format == "mrc")
		return Capabilities(CanRead | CanWrite);
	if (!(format.isEmpty() && device->isOpen()))
		return 0;

	Capabilities cap;
	if (device->isReadable() && MRCIOHandler::canRead(device))
		cap |= CanRead;
	if (device->isWritable())
		cap |= CanWrite;
	return cap;
}

QImageIOHandler *MRCIOPlugin::create(QIODevice *device, const QByteArray &format) const
{
	QImageIOHandler *handler = new MRCIOHandler;
	handler->setDevice(device);
	handler->setFormat(format);
	return handler;
}

