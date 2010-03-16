/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-18
 * Description : Free rotation threaded image filter.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "freerotationfilter.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Local includes

#include "dimg.h"
#include "pixelsaliasfilter.h"

namespace Digikam
{

class FreeRotationFilterPriv
{
public:

    FreeRotationFilterPriv() :
        DEG2RAD(0.017453292519943)
    {
    }

    // Degrees to radian conversion coeff (PI/180). To optimize computation.
    const double DEG2RAD; 

    FreeRotationContainer settings;
};

FreeRotationFilter::FreeRotationFilter(DImg* orgImage, QObject* parent, const FreeRotationContainer& settings)
                  : DImgThreadedFilter(orgImage, parent, "FreeRotation"),
                    d(new FreeRotationFilterPriv)
{
    d->settings = settings;
    initFilter();
}

FreeRotationFilter::~FreeRotationFilter()
{
    delete d;
};

QSize FreeRotationFilter::getNewSize() const 
{
    return d->settings.newSize;
}

double FreeRotationFilter::calculateAngle(int x1, int y1, int x2, int y2)
{
    QPoint p1(x1, y1);
    QPoint p2(x2, y2);

    return calculateAngle(p1, p2);
}

double FreeRotationFilter::calculateAngle(const QPoint& p1, const QPoint& p2)
{
    // check for invalid points. This should have been handled by the calling method,
    // but we want to be really sure here

    if (p1.x() < 0 ||
        p2.y() < 0 ||
        p1.x() < 0 ||
        p2.y() < 0)
    {
        return 0.0;
    }

    // check if points are equal
    if (p1 == p2)
        return 0.0;

    // if y() is equal, no angle needs to be calculated
    if (p1.y() == p2.y())
        return 0.0;

    // if x() is equal, angle equals 90°
    if (p1.x() == p2.x())
        return 90.0;

    // do we rotate to the left (counter clock wise)?
    bool ccw = ((p1.x() < p2.x()) && (p2.y() > p1.y())) ||
               ((p1.x() > p2.x()) && (p2.y() < p1.y()));

    // calculate the angle
    double angle = 0.0;
    double ly    = fabs((double)p2.y() - p1.y());
    double lx    = fabs((double)p2.x() - p1.x());

    angle = atan2(ly, lx) * 180.0 / M_PI;
    angle = ccw ? -angle : angle;

    return angle;
}

void FreeRotationFilter::filterImage()
{
    int          progress;
    register int w, h, nw, nh, j, i = 0;
    int          nNewHeight, nNewWidth;
    int          nhdx, nhdy, nhsx, nhsy;
    double       lfSin, lfCos, lfx, lfy;

    int nWidth  = m_orgImage.width();
    int nHeight = m_orgImage.height();

    uchar* pBits            = m_orgImage.bits();
    unsigned short *pBits16 = (unsigned short*)m_orgImage.bits();

    // first of all, we need to calculate the sin and cos of the given angle

    lfSin = sin(d->settings.angle * -d->DEG2RAD);
    lfCos = cos(d->settings.angle * -d->DEG2RAD);

    // now, we have to calc the new size for the destination image

    if ((lfSin * lfCos) < 0)
    {
        nNewWidth  = lround (fabs (nWidth * lfCos - nHeight * lfSin));
        nNewHeight = lround (fabs (nWidth * lfSin - nHeight * lfCos));
    }
    else
    {
        nNewWidth  = lround (fabs (nWidth * lfCos + nHeight * lfSin));
        nNewHeight = lround (fabs (nWidth * lfSin + nHeight * lfCos));
    }

    // getting the destination's center position

    nhdx = nNewWidth  / 2;
    nhdy = nNewHeight / 2;

    // getting the source's center position

    nhsx = nWidth  / 2;
    nhsy = nHeight / 2;

    // now, we have to alloc a new image

    bool sixteenBit = m_orgImage.sixteenBit();
    m_destImage     = DImg(nNewWidth, nNewHeight, sixteenBit, m_orgImage.hasAlpha());

    if (m_destImage.isNull())
        return;

    m_destImage.fill( DColor(d->settings.backgroundColor.rgb(), sixteenBit) );

    uchar* pResBits            = m_destImage.bits();
    unsigned short* pResBits16 = (unsigned short *)m_destImage.bits();

    PixelsAliasFilter alias;

    // main loop

    for (h = 0; !m_cancel && (h < nNewHeight); ++h)
    {
        nh = h - nhdy;

        for (w = 0; !m_cancel && (w < nNewWidth); ++w)
        {
            nw = w - nhdx;

            i = setPosition (nNewWidth, w, h);

            lfx = (double)nw * lfCos - (double)nh * lfSin + nhsx;
            lfy = (double)nw * lfSin + (double)nh * lfCos + nhsy;

            if (isInside (nWidth, nHeight, (int)lfx, (int)lfy))
            {
                if (d->settings.antiAlias)
                {
                    if (!sixteenBit)
                        alias.pixelAntiAliasing(pBits, nWidth, nHeight, lfx, lfy,
                                                &pResBits[i+3], &pResBits[i+2],
                                                &pResBits[i+1], &pResBits[i]);
                    else
                        alias.pixelAntiAliasing16(pBits16, nWidth, nHeight, lfx, lfy,
                                                  &pResBits16[i+3], &pResBits16[i+2],
                                                  &pResBits16[i+1], &pResBits16[i]);
                }
                else
                {
                    j = setPosition (nWidth, (int)lfx, (int)lfy);

                    for (int p = 0 ; p < 4 ; ++p)
                    {
                        if (!sixteenBit)
                            pResBits[i] = pBits[j];
                        else
                            pResBits16[i] = pBits16[j];

                        ++i;
                        ++j;
                    }
                }
            }
        }

        // Update the progress bar in dialog.
        progress = (int) (((double) h * 100.0) / nNewHeight);
        if (progress % 5 == 0)
            postProgress(progress);
    }

    // Compute the rotated destination image size using original image dimensions.
    int    W, H;
    double absAngle = fabs(d->settings.angle);

    // stop here when no angle was set
    if (absAngle == 0.0)
    {
        return;
    }

    if (absAngle < 90.0)
    {
        W = (int)(d->settings.orgW * cos(absAngle * d->DEG2RAD) + d->settings.orgH * sin(absAngle * d->DEG2RAD));
        H = (int)(d->settings.orgH * cos(absAngle * d->DEG2RAD) + d->settings.orgW * sin(absAngle * d->DEG2RAD));
    }
    else
    {
        H = (int)(d->settings.orgW * cos((absAngle-90.0) * d->DEG2RAD) + d->settings.orgH * sin((absAngle-90.0) * d->DEG2RAD));
        W = (int)(d->settings.orgH * cos((absAngle-90.0) * d->DEG2RAD) + d->settings.orgW * sin((absAngle-90.0) * d->DEG2RAD));
    }

    // Auto-cropping destination image without black holes around.
    QRect autoCrop;

    switch (d->settings.autoCrop)
    {
        case FreeRotationContainer::WidestArea:
        {
            // 'Widest Area' method (by Renchi Raju).

            autoCrop.setX((int) (nHeight * sin(absAngle * d->DEG2RAD)));
            autoCrop.setY((int) (nWidth  * sin(absAngle * d->DEG2RAD)));
            autoCrop.setWidth((int)  (nNewWidth  - 2* nHeight * sin(absAngle * d->DEG2RAD)));
            autoCrop.setHeight((int) (nNewHeight - 2* nWidth  * sin(absAngle * d->DEG2RAD)));

            if (!autoCrop.isValid())
            {
                m_destImage = DImg(m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit(),  
                                   m_orgImage.hasAlpha());
                m_destImage.fill(DColor(d->settings.backgroundColor.rgb(), sixteenBit));
                d->settings.newSize   = QSize();
            }
            else
            {
                m_destImage = m_destImage.copy(autoCrop);
                d->settings.newSize.setWidth((int)  (W - 2* d->settings.orgH * sin(absAngle * d->DEG2RAD)));
                d->settings.newSize.setHeight((int) (H - 2* d->settings.orgW * sin(absAngle * d->DEG2RAD)));
            }
            break;
        }

        case FreeRotationContainer::LargestArea:
        {
            // 'Largest Area' method (by Gerhard Kulzer).

            float gamma = 0.0f;

            if (nHeight > nWidth)
            {
                gamma = atan((float) nWidth / (float) nHeight);

                if (absAngle < 90.0)
                {
                    autoCrop.setHeight((int) ((float) nWidth / cos(absAngle * d->DEG2RAD) / (tan(gamma) +
                                       tan(absAngle * d->DEG2RAD))));
                    autoCrop.setWidth((int) ((float) autoCrop.height() * tan(gamma)));
                }
                else
                {
                    autoCrop.setWidth((int) ((float) nWidth / cos((absAngle - 90.0) * d->DEG2RAD) / (tan(gamma) +
                                      tan((absAngle - 90.0) * d->DEG2RAD))));
                    autoCrop.setHeight((int) ((float) autoCrop.width() * tan(gamma)));
                }
            }
            else
            {
                gamma = atan((float) nHeight / (float) nWidth);

                if (absAngle < 90.0)
                {
                    autoCrop.setWidth((int) ((float) nHeight / cos(absAngle * d->DEG2RAD) / (tan(gamma) + 
                                      tan(absAngle * d->DEG2RAD))));
                    autoCrop.setHeight((int) ((float) autoCrop.width() * tan(gamma)));
                }
                else
                {
                    autoCrop.setHeight((int) ((float) nHeight / cos((absAngle - 90.0) * d->DEG2RAD) /
                                       (tan(gamma) + tan((absAngle - 90.0) * d->DEG2RAD))));
                    autoCrop.setWidth((int) ((float) autoCrop.height() * tan(gamma)));
                }
            }

            autoCrop.moveCenter(QPoint(nNewWidth / 2, nNewHeight / 2));

            if (!autoCrop.isValid())
            {
                m_destImage = DImg(m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit(),
                                   m_orgImage.hasAlpha());
                m_destImage.fill(DColor(d->settings.backgroundColor.rgb(), sixteenBit));
                d->settings.newSize   = QSize();
            }
            else
            {
                m_destImage = m_destImage.copy(autoCrop);
                gamma       = atan((float) d->settings.orgH / (float) d->settings.orgW);

                if (absAngle < 90.0)
                {
                    d->settings.newSize.setWidth((int) ((float) d->settings.orgH / cos(absAngle * d->DEG2RAD) / (tan(gamma) + 
                                       tan(absAngle * d->DEG2RAD))));
                    d->settings.newSize.setHeight((int) ((float) d->settings.newSize.width() * tan(gamma)));
                }
                else
                {
                    d->settings.newSize.setHeight((int) ((float) d->settings.orgH / cos((absAngle - 90.0) * d->DEG2RAD) /
                                        (tan(gamma) + tan((absAngle - 90.0) * d->DEG2RAD))));
                    d->settings.newSize.setWidth((int) ((float) d->settings.newSize.height() * tan(gamma)));
                }
            }
            break;
        }
        
        default: // No auto cropping.
        {
            d->settings.newSize.setWidth(W);
            d->settings.newSize.setHeight(H);
            break;
        }
    }
}

int FreeRotationFilter::setPosition (int Width, int X, int Y)
{
    return (Y *Width*4 + 4*X);
}

bool FreeRotationFilter::isInside (int Width, int Height, int X, int Y)
{
    bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
    bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
    return (bIsWOk && bIsHOk);
}

}  // namespace Digikam
