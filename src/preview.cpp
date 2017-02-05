#include "preview.h"

Mat Preview::getPreview() const
{
    return preview;
}

Preview::Preview(const unsigned int width, const unsigned int height, string windowTitle):
    size(Size(width,height)),
    thickness(0.5*width/100.0),
    dotsRadius(thickness*2),
    textOffset(thickness*2),
    textThickness(thickness/2.0),
    fontScale(thickness/5),
    title(windowTitle)
{

}

void Preview::show()
{
    imshow(title.c_str(),preview);
}

void Preview::drawPreview(
        const Mat & fullFrame,
        const AutoCamera &cam,
        const bool bTrackerInitialized,
        const Rect &focus,
        const Rect &aim,
        Detector &det,
        const int &detWarning,
        const int &aimCheckerPer,
        const unsigned int frameCounter)
{
    resize(fullFrame, preview, this->size, 0, 0, INTER_NEAREST ); ///< \todo 28.01.2017 !!! Отрисовка всех превью не должна зависеть от параметра scale, нужен только для детектора Viola Jones

    // Рисовать кадр захвата
    rectangle(preview,cam.getRoi(),Scalar(0,0,255),thickness);
    Scalar colorX;
    switch (cam.getMoveX().getState()){
    case BEGIN:
        colorX = Scalar(127,255,127);break;
    case MOVE:
        colorX = Scalar(255,255,255);break;
    case END:
        colorX = Scalar(127,127,255);break;
    }
    Scalar colorY;
    switch (cam.getMoveY().getState()){
    case BEGIN:
        colorY = Scalar(127,255,127);break;
    case MOVE:
        colorY = Scalar(255,255,255);break;
    case END:
        colorY = Scalar(127,127,255);break;
    }
    if(cam.getMoveX().getState()!=STOP){
        if(cam.getMoveX().getSign()<0)
            arrowedLine(preview,
                        Point(cam.getRoi().x,cam.getRoi().y+cam.getRoi().height/2),
                        Point(cam.getRoi().x-cam.getMoveX().getSpeed()*2,cam.getRoi().y+cam.getRoi().height/2),
                        colorX,thickness,8,0,1);
        else
            arrowedLine(preview,
                        Point(cam.getRoi().x+cam.getRoi().width,cam.getRoi().y+cam.getRoi().height/2),
                        Point(cam.getRoi().x+cam.getRoi().width+cam.getMoveX().getSpeed()*2,cam.getRoi().y+cam.getRoi().height/2),
                        colorX,thickness,8,0,1);
    }
    if(cam.getMoveY().getState()!=STOP){
        if(cam.getMoveY().getSign()<0)
            arrowedLine(preview,
                        Point(cam.getRoi().x+cam.getRoi().width/2,cam.getRoi().y),
                        Point(cam.getRoi().x+cam.getRoi().width/2,cam.getRoi().y-cam.getMoveY().getSpeed()*2),
                        colorY,thickness,8,0,1);
        else
            arrowedLine(preview,
                        Point(cam.getRoi().x+cam.getRoi().width/2,cam.getRoi().br().y),
                        Point(cam.getRoi().x+cam.getRoi().width/2,cam.getRoi().br().y+cam.getMoveY().getSpeed()*2),
                        colorY,thickness,8,0,1);
    }
    drawThirds(preview,cam.getRoi(),Scalar(0,255,0),dotsRadius);
    // Рисовать цель
    if(bTrackerInitialized){
        stringstream strDetWarn;
        strDetWarn << detWarning;
        rectangle(preview,aim,Scalar(255-detWarning,255-detWarning,255),thickness);
        if(frameCounter % aimCheckerPer!=0){
            putText(preview, "aim tracking"+strDetWarn.str(),aim.tl(),CV_FONT_NORMAL,fontScale,Scalar(255,255,255),textThickness);
        }
        else{
            putText(preview, "redetection",aim.tl(),CV_FONT_NORMAL,fontScale,Scalar(0,255,255),textThickness);
        }
    }else{
        rectangle(preview,aim,Scalar(0,255,0),thickness);
        putText(preview, "aim detection",aim.tl(),CV_FONT_NORMAL,fontScale,Scalar(0,255,0),textThickness);
        //Отрисовка распознаных объектов на превью
        drawRects(preview,det.getFacesFull(),"Full face",Scalar(255,0,0),fontScale,textThickness,textOffset,thickness);
        drawRects(preview,det.getFacesProf(),"Profile",Scalar(255,127,0),fontScale,textThickness,textOffset,thickness);
    }
    // Рисовать фокус детекции
    rectangle(preview,focus,Scalar(0,255,255),thickness*0.8);

    /// Вывести время в превью
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    stringstream timestring;
    timestring
            << __DATE__ <<" "<< __TIME__
               "|"<< (now->tm_year + 1900) << '.'
            << (now->tm_mon + 1) << '.'
            << now->tm_mday << " "
            << now->tm_hour <<":"
            << now->tm_min << ":"
            << now->tm_sec;
    //                           << frameCounter;
//                    putText(preview,timestring.str(),Point(0,previewSize.height-3),CV_FONT_NORMAL,fontScale*1.35,Scalar(0,0,0),textThickness*10);
//                    putText(preview,timestring.str(),Point(0,previewSize.height-3),CV_FONT_NORMAL,fontScale*1.35,Scalar(255,255,255),textThickness*2);
    stringstream frame;
    frame << frameCounter;
    putText(preview,(frame.str()),Point(0,fontScale*20),CV_FONT_NORMAL,fontScale*1.45,Scalar(0,0,0,100),textThickness*10);
    putText(preview,(frame.str()),Point(0,fontScale*20),CV_FONT_NORMAL,fontScale*1.45,Scalar(255,255,255),textThickness*5);

}

/**
 * @brief Нарисовать прямоугольники
 * Рисует несколько прямоугольников, добавляя к ним подпись в виде текста и номера
 * @param[in,out] img Кадр, в котором будут нарисованы прямоугольники
 * @param[in] rects Массив прямоугольников
 * @param[in] t Подпись каждого прямоугольника
 * @param[in] color Цвет прямоугольников и подписей
 * @param[in] fontScale Кегль подписи
 * @param[in] textThickness Толщина линии подписи
 * @param[in] textOffset Отступ подписи от прямоугольника
 * @param[in] thickness Толщина прямоугольника
 * @param[in] fontFace Шрифт
 */
void Preview::drawRects(
       Mat& img,
       vector<Rect> rects,
       string t,
       Scalar color,
       float fontScale,
       float textThickness,
       int textOffset,
       int thickness,
       int fontFace) { /// \todo 18.05.2016 class Drawer
    for (int i = 0; i < rects.size(); ++i)
    {
        stringstream title;
        title << t<<" "<< i;
        putText(img, title.str(),
                Point(rects[i].x,
                      rects[i].y-textOffset),
                fontFace, fontScale,color,textThickness);
        rectangle(img,rects[i],
                  color, thickness, 8, 0);
    }
}
/**
 * @brief Рисовать точки правила третей
 * Рисует точки по правилу третей в заданном прямоугольнике \c r
 * @param[in,out] img Изображение, на которое наносятся точки правила третей
 * @param[in] r Прямоугольник, в котором определяется правило третей
 * @param[in] color Цвет точек
 * @param[in] dotsRadius Радиус точек
 */
inline void Preview::drawThirds(Mat& img, const Rect2f& r,Scalar color,const double& dotsRadius){ /// \todo 18.05.2016 class Drawer
    circle(img,Point(r.x + r.width/3.0,
                              r.y + r.height/3.0), 1,color, dotsRadius);
    circle(img,Point(r.x + 2.0*r.width/3.0,
                              r.y + r.height/3.0),1,color, dotsRadius);
    circle(img,Point(r.x + r.width/3.0,
                              r.y + 2.0*r.height/3.0),1,color,dotsRadius);
    circle(img,Point(r.x + 2.0*r.width/3.0,
                              r.y + 2.0*r.height/3.0),1,color,dotsRadius);
}
/**
 * @brief Медиана набора прямоугольников
 * @param[in] r Массив прямоугольников
 * @return Прямоугольник, высота которого - медиана высот, а координаты - медианы координат прямоугольников
 */
Rect Preview::median(const vector<Rect>& r){ /// \todo 18.05.2016 class Detector
    static vector<int> x,y,h;
    x.resize(r.size());
    y.resize(r.size());
    h.resize(r.size());
    for (int i = 0; i < r.size(); ++i) {
        x[i]=r[i].x;
        y[i]=r[i].y;
        h[i]=r[i].height;
    }
    sort(x.begin(),x.end());
    sort(y.begin(),y.end());
    sort(h.begin(),h.end());
    return Rect(x[x.size()/2],y[y.size()/2],h[h.size()/2],h[h.size()/2]);
}
