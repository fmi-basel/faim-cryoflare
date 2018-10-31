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
#include "diskusagewidget.h"


DiskUsageWidget::DiskUsageWidget(const QString &name):
    storage_info_(),
    timer_()
{
    setFormat(name+": %p%");
    connect(&timer_,&QTimer::timeout,this,&DiskUsageWidget::update);
    setMinimum(0);
    setMaximum(100);
}

void DiskUsageWidget::start(const QString &path)
{
    storage_info_.setPath(path);
    timer_.start(1000*60*5);
    update();
}

void DiskUsageWidget::stop()
{
    timer_.stop();
}

void DiskUsageWidget::update()
{
   storage_info_.refresh();
   int p=static_cast<int>(100.0*storage_info_.bytesAvailable()/storage_info_.bytesTotal());
   if(p>90){
       setStyleSheet("QProgressBar::chunk {background-color: #FF0000; text-align: center}");
   }else{
       setStyleSheet("QProgressBar::chunk {background-color: #00FF00; text-align: center}");
   }
   setValue(p);
}
