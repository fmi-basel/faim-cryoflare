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

#ifndef EPUIMAGEINFO_H
#define EPUIMAGEINFO_H

#include <QDomDocument>

class EPUImageInfo
{
public:
    EPUImageInfo(const QString& path);
    double x() const;

    double y() const;

    double z() const;

    double defocus() const;

    int num_frames() const;

    double exposure_time() const;

    double apix_x() const;

    double apix_y() const;

private:
    QDomDocument dom_document_;
    double x_,y_,z_;
    double defocus_;
    int num_frames_;
    double exposure_time_;
    double apix_x_,apix_y_;
};

#endif // EPUIMAGEINFO_H
