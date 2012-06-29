/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : a command line tool to test qt PGF interface
 *
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// C++ includes

#include <ctime>

// Qt includes

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QImage>

// Local includes

#include "pgfutils.h"

using namespace Digikam;

int main(int /*argc*/, char** /*argv*/)
{
    clock_t    start, end;
    QImage     img;
    QByteArray pgfData, jpgData, pngData;

    // QImage => PGF conversion

    qDebug() << "Using LibPGF version: " << libPGFVersion() << endl;

    img.load("test.png");

    qDebug() << "Generate PGF file using file stream" << endl;

    // First, write QImage as PGF file using file stream
    if (!writePGFImageFile(img, "test-filestream.pgf", 0, true))
    {
        qDebug() << "writePGFImageData failed..." << endl;
        return -1;
    }

    qDebug() << "Generate PGF file using data stream" << endl;

    start = clock();

    // Second, write QImage as PGF file using data stream
    if (!writePGFImageData(img, pgfData, 0, true))
    {
        qDebug() << "writePGFImageData failed..." << endl;
        return -1;
    }

    end   = clock();

    qDebug() << "PGF Encoding time: " << double(end - start)/CLOCKS_PER_SEC << " s" << endl;

    // Write PGF file.

    QFile file("test-datastream.pgf");

    if ( !file.open(QIODevice::WriteOnly) )
    {
        qDebug() << "Cannot open PGF file to write..." << endl;
        return -1;
    }

    QDataStream stream(&file);
    stream.writeRawData(pgfData.data(), pgfData.size());
    file.close();

    // PGF => QImage conversion

    qDebug() << "Load PGF file generated by data stream" << endl;

    start = clock();

    if (!readPGFImageData(pgfData, img, true))
    {
        qDebug() << "readPGFImageData failed..." << endl;
        return -1;
    }

    end   = clock();

    img.save("test2.png", "PNG");

    qDebug() << "PGF Decoding time: " << double(end - start)/CLOCKS_PER_SEC << " s" << endl;

    // JPEG tests for comparisons.

    img.load("test.png");

    qDebug() << "Generate JPG file to compare performances" << endl;

    start = clock();

    QBuffer buffer(&jpgData);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "JPEG", 85);  // Here we will use JPEG quality = 85 to reduce artifacts.

    if (jpgData.isNull())
    {
        qDebug() << "Save JPG image data to byte array failed..." << endl;
        return -1;
    }

    end   = clock();

    qDebug() << "JPG Encoding time: " << double(end - start)/CLOCKS_PER_SEC << " s" << endl;

    start = clock();

    buffer.open(QIODevice::ReadOnly);
    img.load(&buffer, "JPEG");

    if (jpgData.isNull())
    {
        qDebug() << "Load JPG image data from byte array failed..." << endl;
        return -1;
    }

    end   = clock();

    qDebug() << "JPG Decoding time: " << double(end - start)/CLOCKS_PER_SEC << " s" << endl;

    // PNG tests for comparisons.

    img.load("test.png");

    qDebug() << "Generate PNG file to compare performances" << endl;

    start = clock();

    QBuffer buffer2(&pngData);
    buffer2.open(QIODevice::WriteOnly);
    img.save(&buffer2, "PNG", 100);

    if (pngData.isNull())
    {
        qDebug() << "Save PNG image data to byte array failed..." << endl;
        return -1;
    }

    end   = clock();

    qDebug() << "PNG Encoding time: " << double(end - start)/CLOCKS_PER_SEC << " s" << endl;

    start = clock();

    buffer2.open(QIODevice::ReadOnly);
    img.load(&buffer2, "PNG");

    if (pngData.isNull())
    {
        qDebug() << "Load PNG image data from byte array failed..." << endl;
        return -1;
    }

    end   = clock();

    qDebug() << "PNG Decoding time: " << double(end - start)/CLOCKS_PER_SEC << " s" << endl;

    return 0;
}
