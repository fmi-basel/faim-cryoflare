//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2018 by the CryoFLARE Authors
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
#include "lastimagetimer.h"

LastImageTimer::LastImageTimer():
    timer_(),
    time_()
{
    setText("");
    connect(&timer_,&QTimer::timeout,this,&LastImageTimer::update_);
}

void LastImageTimer::reset()
{
    timer_.start(1000);
    time_.start();
}

void LastImageTimer::stop()
{
    timer_.stop();
    setText("");
}

void LastImageTimer::update_()
{
    setText(QString("%1s since last image").arg(time_.elapsed()/1000));
}

