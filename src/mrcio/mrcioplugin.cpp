//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2019 by the CryoFLARE Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------
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
    if(!device){
        if (format == "mrc"){
            return Capabilities(CanRead | CanWrite);
        }else {
            return Capabilities();
        }
    }
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

