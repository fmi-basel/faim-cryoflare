//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFlare
//
// Copyright (C) 2017-2018 by the CryoFlare Authors
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
// along with CryoFlare.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#include "epuimageinfo.h"
#include <QFile>

EPUImageInfo::EPUImageInfo(const QString &path):
    dom_document_("EPUImageInfo"),
    x_(),y_(),z_(),
    defocus_(),
    num_frames_(),
    exposure_time_(),
    apix_x_(),apix_y_()
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;
    if (!dom_document_.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    QDomNode custom_data=dom_document_.elementsByTagName("CustomData").at(0);
    QDomNode node = custom_data.firstChild();
    while(!node.isNull()) {
        if(node.firstChild().toElement().text()=="AppliedDefocus"){
            defocus_=node.lastChild().toElement().text().toDouble();
            break;
        }
        node = node.nextSibling();
    }

    QDomNode camera=dom_document_.elementsByTagName("camera").at(0);
    num_frames_=camera.toElement().elementsByTagName("b:NumberOffractions").at(0).toElement().text().toInt();
    exposure_time_=camera.toElement().elementsByTagName("ExposureTime").at(0).toElement().text().toFloat();

    QDomNode pixel_size=dom_document_.elementsByTagName("pixelSize").at(0);
    QDomNodeList pixel_size_values=pixel_size.toElement().elementsByTagName("numericValue");
    apix_x_=pixel_size_values.at(0).toElement().text().toDouble();
    apix_y_=pixel_size_values.at(1).toElement().text().toDouble();

    QDomNode stage=dom_document_.elementsByTagName("stage").at(0);
    x_=stage.toElement().elementsByTagName("X").at(0).toElement().text().toDouble();
    y_=stage.toElement().elementsByTagName("Y").at(0).toElement().text().toDouble();
    z_=stage.toElement().elementsByTagName("Z").at(0).toElement().text().toDouble();

}
double EPUImageInfo::x() const
{
    return x_;
}

double EPUImageInfo::y() const
{
    return y_;
}

double EPUImageInfo::z() const
{
    return z_;
}

double EPUImageInfo::defocus() const
{
    return defocus_;
}

int EPUImageInfo::num_frames() const
{
    return num_frames_;
}

double EPUImageInfo::exposure_time() const
{
    return exposure_time_;
}

double EPUImageInfo::apix_x() const
{
    return apix_x_;
}

double EPUImageInfo::apix_y() const
{
    return apix_y_;
}









