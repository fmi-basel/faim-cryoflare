#include <QColor>
#include <QVariant>
#include <QDataStream>
#include <QImage>
#include <iostream>
#include <QDebug>
#include "mrciohandler.h"

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

QDataStream &operator<<(QDataStream& s, const MRCHeader& h)
{
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
    // hack to get usable grey levels
    h.DMIN=h.DMEAN-3;
    h.DMAX=h.DMEAN+3;
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
    QByteArray machst=device()->peek(54*4).right(4).left(2);
    QByteArray data=device()->readAll();
    QDataStream input(data);
    input.setFloatingPointPrecision(QDataStream::SinglePrecision);
    if(machst==QByteArray("\x44\x41")){
        input.setByteOrder(QDataStream::LittleEndian);
    }else if(machst==QByteArray("\x11\x11")){
        input.setByteOrder(QDataStream::BigEndian);
    }else{
        input.setByteOrder(QDataStream::BigEndian);
    }
    MRCHeader header;
    input >> header;
    if (input.status() != QDataStream::Ok){
        return false;
    }
    QImage result(header.NX,header.NY, QImage::Format_ARGB32);
    float iscale=255.0/std::max(header.DMAX-header.DMIN,0.00001f);
    for (quint32 y = 0; y < header.NY ; ++y) {
        QRgb *scanLine = (QRgb *)result.scanLine(y);
        for (quint32 x = 0; x < header.NX ; ++x){
            float v;
            input >> v;
            int iv=static_cast<int>((v-header.DMIN)*iscale);
            scanLine[x]=qRgb(iv,iv,iv);
        }
    }
    if (input.status() == QDataStream::Ok){
        *image = result;
    }
    return input.status() == QDataStream::Ok;
}

bool MRCIOHandler::write(const QImage &image)
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

