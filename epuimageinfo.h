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
