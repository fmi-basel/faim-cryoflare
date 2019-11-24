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
#include <QColor>
#include <QVariant>
#include <QDataStream>
#include <QImage>
#include <iostream>
#include <QDebug>
#include "mrciohandler.h"
#include <cmath>

class MRCHeader
{
public:
    quint32 NX,NY,NZ;
    quint32 MODE;
    quint32 NXSTART,NYSTART,NZSTART;
    quint32 MX,MY,MZ;
    float CELL_A,CELL_B,CELL_C;
    float CELL_ALPHA,CELL_BETA,CELL_GAMMA;
    quint32 MAPC,MAPR,MAPS;
    float DMIN,DMAX,DMEAN;
    quint32 ISPG;
    quint32 NSYMBT;
    quint32 EXTRA1,EXTRA2,EXTTYP,NVERSION;
    char EXTRA[21*4];
    quint32 ORIGIN_X,ORIGIN_Y,ORIGIN_Z;
    quint32 MAP;
    quint32 MACHST;
    quint32 RMS;
    quint32 NLABL;
    char LABEL[200*4];
    QByteArray EXTHEADER;
};

template< class T>
void read_data_(const MRCHeader& header, QDataStream &stream,QImage *image){
    QImage result(header.NX,header.NY, QImage::Format_ARGB32);
    quint32 data_len=header.NX*header.NY;
    QVector<T> data_vec(data_len);
    quint32 count=0;
    float mean=0,m2=0;
    for(quint32 i=0; i<data_len;++i){
        stream >> data_vec[i];
        ++count;
        float delta=float(data_vec[i])-mean;
        mean+=delta/count;
        float delta2=float(data_vec[i])-mean;
        m2+=delta*delta2;
    }
    float stdev=sqrt(m2/count);
    float width=6.0;
    float s_min=mean-width*0.5*stdev;
    float iscale=255.0/std::max(width*stdev,0.00001f);
    QRgb * rgb_data=(QRgb *)(result.bits());
    for(quint32 i=0; i<data_len;++i){
        int iv=std::min<float>(std::max<float>(static_cast<int>((float( data_vec[i])-s_min)*iscale),0.0),255.0);
        rgb_data[i]=qRgb(iv,iv,iv);
    }
    if (stream.status() == QDataStream::Ok){
        *image = result;
    }
}


QDataStream &operator<<(QDataStream& s, const MRCHeader& h)
{
    s <<"NXYZ: "<< h.NX <<","<< h.NY <<","<< h.NZ<<"\n";
    s <<"MODE: "<< h.MODE<<"\n";
    s <<"NXYZSTART: "<< h.NXSTART <<","<< h.NYSTART <<","<< h.NZSTART<<"\n";
    s <<"MXYZ: "<< h.MX <<","<< h.MY <<","<< h.MZ<<"\n";
    s <<"CELLA: "<< h.CELL_A <<","<< h.CELL_B <<","<< h.CELL_C<<"\n";
    s <<"CELLB: "<< h.CELL_ALPHA <<","<< h.CELL_BETA <<","<< h.CELL_GAMMA<<"\n";
    s <<"MAPCRS: "<< h.MAPC <<","<< h.MAPR <<","<< h.MAPS<<"\n";
    s <<"DMINMAXMEAN: "<< h.DMIN <<","<< h.DMAX <<","<< h.DMEAN<<"\n";
    s <<"ISPG: "<< h.ISPG<<"\n";
    s <<"NSYMBT: "<< h.NSYMBT<<"\n";
    s <<"EXTRA: "<< h.EXTRA1 <<","<< h.EXTRA2 <<","<< h.EXTTYP <<","<< h.NVERSION<<"\n";
    s <<"ORIGINXYZ: "<< h.ORIGIN_X <<","<< h.ORIGIN_Y <<","<< h.ORIGIN_Z<<"\n";
    s <<"MAP: "<< h.MAP<<"\n";
    s <<"MACHST: "<< h.MACHST<<"\n";
    s <<"RMS: "<< h.RMS<<"\n";
    s <<"NLABL: "<< h.NLABL<<"\n";
    return s;
}

QDataStream &operator>>(QDataStream& s, MRCHeader& h)
{
    s >> h.NX >> h.NY >> h.NZ;
    s >> h.MODE;
    s >> h.NXSTART >> h.NYSTART >> h.NZSTART;
    s >> h.MX >> h.MY >> h.MZ;
    s >> h.CELL_A >> h.CELL_B >> h.CELL_C;
    s >> h.CELL_ALPHA >> h.CELL_BETA >> h.CELL_GAMMA;
    s >> h.MAPC >> h.MAPR >> h.MAPS;
    s >> h.DMIN >> h.DMAX >> h.DMEAN;
    s >> h.ISPG;
    s >> h.NSYMBT;
    s >> h.EXTRA1 >> h.EXTRA2 >> h.EXTTYP >> h.NVERSION;
    s.readRawData(h.EXTRA,21*4);
    s >> h.ORIGIN_X >> h.ORIGIN_Y >> h.ORIGIN_Z;
    s >> h.MAP;
    s >> h.MACHST;
    s >> h.RMS;
    s >> h.NLABL;
    s.readRawData(h.LABEL,200*4);
    h.EXTHEADER.resize(h.NSYMBT);
    s.readRawData(h.EXTHEADER.data(),h.NSYMBT);
    return s;
}

std::ostream& operator<<(std::ostream& s, const MRCHeader& h)
{
    s <<"NXYZ: "<< h.NX <<","<< h.NY <<","<< h.NZ<<"\n";
    s <<"MODE: "<< h.MODE<<"\n";
    s <<"NXYZSTART: "<< h.NXSTART <<","<< h.NYSTART <<","<< h.NZSTART<<"\n";
    s <<"MXYZ: "<< h.MX <<","<< h.MY <<","<< h.MZ<<"\n";
    s <<"CELLA: "<< h.CELL_A <<","<< h.CELL_B <<","<< h.CELL_C<<"\n";
    s <<"CELLB: "<< h.CELL_ALPHA <<","<< h.CELL_BETA <<","<< h.CELL_GAMMA<<"\n";
    s <<"MAPCRS: "<< h.MAPC <<","<< h.MAPR <<","<< h.MAPS<<"\n";
    s <<"DMINMAXMEAN: "<< h.DMIN <<","<< h.DMAX <<","<< h.DMEAN<<"\n";
    s <<"ISPG: "<< h.ISPG<<"\n";
    s <<"NSYMBT: "<< h.NSYMBT<<"\n";
    s <<"EXTRA: "<< h.EXTRA1 <<","<< h.EXTRA2 <<","<< h.EXTTYP <<","<< h.NVERSION<<"\n";
    s <<"ORIGINXYZ: "<< h.ORIGIN_X <<","<< h.ORIGIN_Y <<","<< h.ORIGIN_Z<<"\n";
    s <<"MAP: "<< h.MAP<<"\n";
    s <<"MACHST: "<< h.MACHST<<"\n";
    s <<"RMS: "<< h.RMS<<"\n";
    s <<"NLABL: "<< h.NLABL<<"\n";
    return s;
}

MRCIOHandler::MRCIOHandler()
{

}

bool MRCIOHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("mrc");
        return true;
    }
    return false;
}

bool MRCIOHandler::read(QImage *image)
{
    QByteArray data=device()->readAll();
    QDataStream input(data);
    input.setFloatingPointPrecision(QDataStream::SinglePrecision);
    if(isBigEndian_()){
        input.setByteOrder(QDataStream::BigEndian);
    }else{
        input.setByteOrder(QDataStream::LittleEndian);
    }
    MRCHeader header;
    input >> header;
    if (input.status() != QDataStream::Ok){
        return false;
    }
    switch(header.MODE){
    case 0:
    default:
        read_data_<uchar>(header,input,image);
        break;
    case 1:
        read_data_<short>(header,input,image);
        break;
    case 2:
        read_data_<float>(header,input,image);
        break;
    case 6:
        read_data_<unsigned short>(header,input,image);
        break;
    }
    return input.status() == QDataStream::Ok;
}

bool MRCIOHandler::write(const QImage &/*image*/)
{
    return true;
}

QByteArray MRCIOHandler::name() const
{
    return "mrc";
}

bool MRCIOHandler::canRead(QIODevice *device)
{
    return device->peek(53*4).right(4) == "MAP ";
}

QVariant MRCIOHandler::option(QImageIOHandler::ImageOption option) const
{
    if (option == Size) {
        QByteArray bytes = device()->peek(8);
        QDataStream input(bytes);
        if(isBigEndian_()){
            input.setByteOrder(QDataStream::BigEndian);
        }else{
            input.setByteOrder(QDataStream::LittleEndian);
        }
        quint32  width, height;
        input >> width >> height;
        if (input.status() == QDataStream::Ok )
            return QSize(width, height);
    }
    return QVariant();
}

void MRCIOHandler::setOption(QImageIOHandler::ImageOption option, const QVariant &value)
{
    Q_UNUSED(option);
    Q_UNUSED(value);
}

bool MRCIOHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
    return option == Size;
}

bool MRCIOHandler::isBigEndian_() const
{
    qint64 old_pos=device()->pos();
    device()->seek(53*4);
    QByteArray machst=device()->peek(2);
    device()->seek(old_pos);
    if(machst==QByteArray("\x44\x41") || machst==QByteArray("\x44\x44")){
        return false;
    }else if(machst==QByteArray("\x11\x11")){
        return true;
    }else{
        return true;
    }

}


