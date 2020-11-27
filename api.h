#ifndef API_H
#define API_H
#include <QDebug>
#include <QImage>
#include<opencv2/opencv.hpp>
using namespace cv;
class api
{
public:
    api();

    static Mat QImageToMat(QImage image)
    {
        Mat mat;
        switch (image.format()) {
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32_Premultiplied:
            mat = Mat(image.height(), image.width(), CV_8UC4, (void *)image.constBits(), static_cast<size_t>(image.bytesPerLine()));
            break;
        case QImage::Format_RGB888:
            mat = Mat(image.height(), image.width(), CV_8UC3, (void *)image.constBits(), static_cast<size_t>(image.bytesPerLine()));
            cvtColor(mat, mat, COLOR_BGR2RGB);
            break;
        case QImage::Format_Indexed8:
            mat = Mat(image.height(), image.width(), CV_8UC1, (void *)image.constBits(), static_cast<size_t>(image.bytesPerLine()));
            break;
        default:
            break;
        }

        return mat;
    }
    static QImage MatToQImage(const Mat &mat)
    {
        // 8-bits unsigned, NO. OF CHANNELS = 1
        if (mat.type() == CV_8UC1) {
            QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
            // Set the color table (used to translate colour indexes to qRgb values)
            image.setColorCount(256);
            for (int i = 0; i < 256; i++) {
                image.setColor(i, qRgb(i, i, i));
            }
            // Copy input Mat
            uchar *pSrc = mat.data;
            for (int row = 0; row < mat.rows; row ++) {
                uchar *pDest = image.scanLine(row);
                memcpy(pDest, pSrc, static_cast<size_t>(mat.cols));
                pSrc += mat.step;
            }
            return image;
        }
        // 8-bits unsigned, NO. OF CHANNELS = 3
        else if (mat.type() == CV_8UC3) {
            // Copy input Mat
            const uchar *pSrc = static_cast<const uchar *>(mat.data);
            // Create QImage with same dimensions as input Mat
            QImage image(pSrc, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888);
            return image.rgbSwapped();
        } else if (mat.type() == CV_8UC4) {
            qDebug() << "CV_8UC4";
            // Copy input Mat
            const uchar *pSrc = mat.data;
            // Create QImage with same dimensions as input Mat
            QImage image(pSrc, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_ARGB32);
            return image.copy();
        } else {
            qDebug() << "ERROR: Mat could not be converted to QImage.";
            return QImage();
        }
    }

    static void QImageD_RunBEEPSHorizontalVertical(QImage *src,QImage *toSrc,double spatialDecay=0.02,double photometricStandardDeviation=10)
    {
        if(!src){
            return ;
        }
        if(!toSrc)
        {
            toSrc=new QImage(*src);
        }
        double c=-0.5/(photometricStandardDeviation * photometricStandardDeviation); //-1/2 *光度标准偏差的平方
        double mu=spatialDecay/(2-spatialDecay);

        double *exptable=new double[256];;
        double *g_table=new double[256];;
        for (int i=0;i<=255;i++)
        {
            exptable[i]=(1-spatialDecay)* exp(c*i*i);
            g_table[i]=mu*i;
        }
        int width=src->width();
        int height=src->height();
            int length=width*height;
        double* data2Red= new double[length];
        double* data2Green= new double[length];
        double* data2Blue= new double[length];

        int i=0;

        for(int y=0;y<height;y++)
        {
            for(int x=0;x<width;x++)
            {
                QRgb rgb=src->pixel(x,y);
                data2Red[i]=qRed(rgb);
                data2Green[i]=qGreen(rgb);
                data2Blue[i]=qBlue(rgb);
                i++;
            }
        }


        double* gRed = new double[length];
        double* pRed = new double[length];
        double* rRed = new double[length];

        double* gGreen = new double[length];
        double* pGreen = new double[length];
        double* rGreen = new double[length];

        double* gBlue = new double[length];
        double* pBlue = new double[length];
        double* rBlue = new double[length];
        memcpy(pRed,data2Red, sizeof(double) * length);
        memcpy(rRed,data2Red, sizeof(double) * length);

        memcpy(pGreen,data2Green, sizeof(double) * length);
        memcpy(rGreen,data2Green, sizeof(double) * length);

        memcpy(pBlue,data2Blue, sizeof(double) * length);
        memcpy(rBlue,data2Blue, sizeof(double) * length);


        double rho0=1.0/(2-spatialDecay);
        for (int k2 = 0;k2 < height;++k2)
        {
            int startIndex=k2 * width;
            double mu=0.0;
            for (int k=startIndex+1,K=startIndex+width;k<K;++k)
            {
                int div0Red=fabs(pRed[k]-pRed[k-1]);
                mu =exptable[div0Red];
                pRed[k] = pRed[k - 1] * mu + pRed[k] * (1.0 - mu);//公式1

                int div0Green=fabs(pGreen[k]-pGreen[k-1]);
                mu =exptable[div0Green];
                pGreen[k] = pGreen[k - 1] * mu + pGreen[k] * (1.0 - mu);//公式1

                int div0Blue=fabs(pBlue[k]-pBlue[k-1]);
                mu =exptable[div0Blue];
                pBlue[k] = pBlue[k - 1] * mu + pBlue[k] * (1.0 - mu);//公式1

            }

            for (int k =startIndex + width - 2;startIndex <= k;--k)
            {
                int div0Red=fabs(rRed[k]-rRed[k+1]);
                double mu =exptable[div0Red];
                rRed[k] = rRed[k + 1] * mu + rRed[k] * (1.0 - mu);//公式3

                int div0Green=fabs(rGreen[k]-rGreen[k+1]);
                mu =exptable[div0Green];
                rGreen[k] = rGreen[k + 1] * mu + rGreen[k] * (1.0 - mu);//公式3

                int div0Blue=fabs(rBlue[k]-rBlue[k+1]);
                mu =exptable[div0Blue];
                rBlue[k] = rBlue[k + 1] * mu + rBlue[k] * (1.0 - mu);//公式3
            }
            for (int k =startIndex,K=startIndex+width;k<K;k++)
            {
                rRed[k]=(rRed[k]+pRed[k])*rho0- g_table[(int)data2Red[k]];
                rGreen[k]=(rGreen[k]+pGreen[k])*rho0- g_table[(int)data2Green[k]];
                rBlue[k]=(rBlue[k]+pBlue[k])*rho0- g_table[(int)data2Blue[k]];
            }
        }

        int m = 0;
        for (int k2=0;k2<height;k2++)
        {
            int n = k2;
            for (int k1=0;k1<width;k1++)
            {
                gRed[n] = rRed[m];
                gGreen[n] = rGreen[m];
                gBlue[n] = rBlue[m];
                m++;
                n += height;
            }
        }

        memcpy(pRed, gRed, sizeof(double) * height * width);
        memcpy(rRed, gRed, sizeof(double) * height * width);

        memcpy(pGreen, gGreen, sizeof(double) * height * width);
        memcpy(rGreen, gGreen, sizeof(double) * height * width);

        memcpy(pBlue, gBlue, sizeof(double) * height * width);
        memcpy(rBlue, gBlue, sizeof(double) * height * width);

        for (int k1=0;k1<width;++k1)
        {
            int startIndex=k1 * height;
            double mu = 0.0;
            for (int k =startIndex+1,K =startIndex+height;k<K;++k)
            {
                int div0Red=fabs(pRed[k]-pRed[k-1]);
                mu =exptable[div0Red];
                pRed[k] = pRed[k - 1] * mu + pRed[k] * (1.0 - mu);

                int div0Green=fabs(pGreen[k]-pGreen[k-1]);
                mu =exptable[div0Green];
                pGreen[k] = pGreen[k - 1] * mu + pGreen[k] * (1.0 - mu);

                int div0Blue=fabs(pBlue[k]-pBlue[k-1]);
                mu =exptable[div0Blue];
                pBlue[k] = pBlue[k - 1] * mu + pBlue[k] * (1.0 - mu);
            }
            for (int k=startIndex+height-2;startIndex<=k;--k)
            {
                int div0Red=fabs(rRed[k]-rRed[k+1]);
                mu =exptable[div0Red];
                rRed[k] = rRed[k + 1] * mu + rRed[k] * (1.0 - mu);

                int div0Green=fabs(rGreen[k]-rGreen[k+1]);
                mu =exptable[div0Green];
                rGreen[k] = rGreen[k + 1] * mu + rGreen[k] * (1.0 - mu);

                int div0Blue=fabs(rBlue[k]-rBlue[k+1]);
                mu =exptable[div0Blue];
                rBlue[k] = rBlue[k + 1] * mu + rBlue[k] * (1.0 - mu);
            }
        }

        double init_gain_mu=spatialDecay/(2-spatialDecay);
        for (int k = 0;k <length;++k)
        {
            rRed[k]= (rRed[k]+pRed[k])*rho0- gRed[k]*init_gain_mu;

            rGreen[k]= (rGreen[k]+pGreen[k])*rho0- gGreen[k]*init_gain_mu;

            rBlue[k]= (rBlue[k]+pBlue[k])*rho0- gBlue[k]*init_gain_mu;
        }

        m = 0;
        for (int k1=0;k1<width;++k1)
        {
            int n = k1;
            for (int k2=0;k2<height;++k2)
            {

                data2Red[n]=rRed[m];
                data2Green[n]=rGreen[m];
                data2Blue[n]=rBlue[m];

                m++;
                n += width;
            }
        }

        int index=0;
        for (int k1=0;k1<height;++k1)
        {
            for (int k2=0;k2<width;++k2)
            {

                toSrc->setPixel(k2,k1,qRgb(data2Red[index],data2Green[index],data2Blue[index]));
                index++;
            }
        }


        delete pRed;
        delete rRed;
        delete gRed;

        delete pGreen;
        delete rGreen;
        delete gGreen;

        delete pBlue;
        delete rBlue;
        delete gBlue;
    }
    static QVector<QRect> getFaceRect(QImage img,int RectRange =0)
    {
        QVector<QRect> rectVec;

        CascadeClassifier faceCascader;//人脸识别
        String filename1 = "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt2.xml";//必须调用，人脸识别数据采集对比
        if (!faceCascader.load(filename1))
        {
            printf("can not load the face feature data \n");
            return rectVec;
        }
        Mat frame = QImageToMat(img);
        Mat gray, faceROI, src;
        std::vector<Rect> faces;
        int k = 0;
        src = frame;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        equalizeHist(gray, gray);
        faceCascader.detectMultiScale(gray, faces,1.2, 3, 0, Size(200, 100));//获得face数量和区域范围，通过feces的Rect可以获得区域范围

        for (int i = 0; i < faces.size(); i++)
        {

            int x1 = faces[static_cast<int>(i)].x + 2;
            int y1 = faces[static_cast<int>(i)].y + 2;
            int x2 = faces[static_cast<int>(i)].width - 2;
            int y2 = faces[static_cast<int>(i)].height - 2;
            if(img.width()>(x2+RectRange))
            {
                x2=x2+RectRange;
            }
            else if(img.width()>(x2+RectRange/2))
            {
                x2=x2+RectRange;
            }
            if(img.height()>(y2+RectRange))
            {
                y2=y2+RectRange/2;
            }
            else if(img.height()>(y2+RectRange))
            {
                y2=y2+RectRange/2;
            }
            QRect rect(x1,y1,x2,y2);
            rectVec.push_back(rect);

            Rect roi;
            roi.x = x1;
            roi.y = y1;
            roi.width = x2;
            roi.height = y2;

            faceROI=frame(roi);
        }

        return rectVec;

    }

};

#endif // API_H
