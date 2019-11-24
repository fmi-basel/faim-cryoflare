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
#include "diskusagewidget.h"
#include <cmath>

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
   int p=static_cast<int>(round(100.0*(storage_info_.bytesTotal()-storage_info_.bytesAvailable())/storage_info_.bytesTotal()));
   if(p>90){
       setStyleSheet("QProgressBar::chunk {background-color: #AF0000 } QProgressBar { text-align: center}");
   }else{
       setStyleSheet("QProgressBar::chunk {background-color: #00AF00 } QProgressBar { text-align: center}");
   }
   setValue(p);
   setToolTip(QString("Available: %1 GB\nUsed: %2 GB\nTotal: %3 GB").arg(storage_info_.bytesAvailable()/1024.0/1024.0/1024.0).arg((storage_info_.bytesTotal()-storage_info_.bytesAvailable())/1024.0/1024.0/1024.0).arg(storage_info_.bytesTotal()/1024.0/1024.0/1024.0));
}
