#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>

#include <math.h>

#include <stdio.h>
#include <sndfile.h>

#define ISCLIPPED(x) fabs(x) >= 0.999f

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString filename = QString::fromUtf8(argv[1]);

    if (filename.isEmpty()) {
        qFatal("No filename specified!");

    }

    QFileInfo *info = new QFileInfo(filename);

    if (!(info->exists())) {
        qFatal(QString("File \"%1\" does not exist").arg(filename).toUtf8());
    }

    qDebug() << "Opening file" << filename;

    SF_INFO ininfo;
    SNDFILE* infile =  sf_open(filename.toUtf8(), SFM_READ, &ininfo);

    if (!infile) {
        qFatal(QString("Failed to open file. Error: \"%1\"").arg(sf_strerror(infile)).toUtf8());
    }

    if (ininfo.channels != 1) {
        qFatal("Too many channels in file! Only mono files are supported yet!");
    }

    SF_INFO outinfo;
    outinfo.frames = ininfo.frames;
    outinfo.samplerate = ininfo.samplerate;
    outinfo.channels = 1;
    outinfo.format = SF_FORMAT_WAV + SF_FORMAT_FLOAT;
    outinfo.sections = ininfo.sections;

    SNDFILE* outfile = sf_open(QString("%1_declipped.wav").arg(filename).toUtf8(), SFM_WRITE, &outinfo);

    if (!outfile) {
        qFatal("Cannot create output file");
    }

    sf_count_t scount = 1;
    float sample;
    float presample = 0.0;
    float firstclipsample = 0.0;
    float lastclipsample  = 0.0;
    float calcsample = 0.0;
    float m1, m2;

    uchar length = 0;
    uchar i = 1;

    while (scount) {
        scount = sf_read_float(infile, &sample, 1);
        if (!scount) {
            break;
        }
        qDebug() << sample;
        if (ISCLIPPED(sample)) {
            qDebug() << "clip start";
            firstclipsample = sample;
            while (ISCLIPPED(sample)) {
                lastclipsample = sample;
                sf_read_float(infile, &sample, 1);
                qDebug() << sample;

                if (!scount) {
                    break;
                }
                length++;
            }
            qDebug() << "clip end. length: " << length << "presample:" << presample << "firstclip" << firstclipsample << "lastclip" << lastclipsample << "postsample:" << sample;

            m1 = firstclipsample - presample;
            m2 = sample - lastclipsample;

            qDebug() << "m1" << m1 << "m2" << m2;

            // 1st half of clip samples, derived from firstclipsample and m1
            for (i = 1; i <= length/2; i++) {
                calcsample = (firstclipsample + i*m1) * 0.5;
                qDebug() << "clipsample i" << calcsample;
                sf_write_float(outfile, &calcsample, 1);
            }
            // 2nd half of clip samples, derived from lastclipsample  and m2
            for (i = (length - length/2); i >= 1; i--) {
                calcsample = (lastclipsample - i*m2) * 0.5;
                qDebug() << "clipsample i" << calcsample;
                sf_write_float(outfile, &calcsample, 1);
            }


            // Presample has been written! Towrite: LENGTH samples. (post) sample will be written after loop

            length = 0;
        }

        presample = sample;
        sample = sample * 0.5;
        sf_write_float(outfile, &sample, 1);
    }

    sf_close(outfile);
    sf_close(infile);

    return(0);
}
