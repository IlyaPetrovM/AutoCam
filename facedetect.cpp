/**
  * \brief Программа для детекции лиц в видео с повышенным разрешением
  * \author Ilya Petrov
  * \date Май 2016 года
  */
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <opencv2/videoio/videoio.hpp>  // Video write
#include <ctime>

#include "automotion.h"
#include "autozoom.h"

using namespace std;
using namespace cv;
const double FI=1.61803398; /// Золотое сечение

static void help()
{
    cout << "Build date:" << __DATE__ << " " << __TIME__
            "\n\tDuring execution:\n\tHit any key to quit.\n"
            "\tUsing OpenCV version " << CV_VERSION << "\n" << endl;
}
/**
 * @brief topMiddleDec Ищет точку посередине прямоугольника, отстоящую от верха на одну треть (в относительных координатах)
 * @param [in]r Прямоугольник, в котором ищется точка
 * @return Точка, у которой x и y - отступы от левого верхнего края прямоугольника
 */
inline Point topMiddleDec(const Rect2f& r) {return Point(cvRound((double)r.width*0.5) , cvRound((double)r.height/3.0));}
/**
 * @brief topLeftDec Ищет левую верхнюю точку по правилу третей (в относительных координатах)
 * @param [in]r Прямоугольник, в котором ищется точка
 * @return Точка, у которой x и y - отступы от левого верхнего края прямоугольника
 */
inline Point topLeftDec(const Rect2f& r) {return Point(cvRound((double)r.width/3.0) , cvRound((double)r.height/3.0));}
/**
 * @brief topRightDec Ищет правую верхнюю точку по правилу третей (в относительных координатах)
 * @param [in]r Прямоугольник, в котором ищется точка
 * @return Точка, у которой x и y - отступы от левого верхнего края прямоугольника
 */
inline Point topRightDec(const Rect2f& r){return Point(cvRound((double)r.width*2.0/3.0) , cvRound((double)r.height/3.0));}
/**
 * @brief getGoldenPoint Ищет координаты прямоугольника так, чтобы внитри него располагалось лицо по правилу третей
 * @param [in]roi Область интереса, кадр
 * @param [in]face лицо
 * @return Абсолютные координаты нового положения кадра
 */
Point getGoldenPoint(const Rect2f& roi,const Rect& face){
    Point target;
    if(cvRound((float)roi.width/3.0) - face.width < 0 ) /// если лицо крупное, то держать его в центре кадра
        target = topMiddleDec(roi);
    else if(face.x+cvRound((float)face.width/2.0) < topMiddleDec(roi).x
            && face.x < roi.x+topLeftDec(roi).x)
        target = topLeftDec(roi);
    else if(face.x+face.width > roi.x+topRightDec(roi).x) // Камера посередине не будет реагировать
        target = topRightDec(roi);
    else
        target = topMiddleDec(roi);

    Point result = (face+topMiddleDec(face) - target).tl();// Должна быть зависимость только от размеров ROI
    return result;
}
/**
 * @brief Нарисовать прямоугольники
 * Рисует несколько прямоугольников, добавляя к ним подпись в виде текста и номера
 * @param [in,out]img Кадр, в котором будут нарисованы прямоугольники
 * @param [in]rects Массив прямоугольников
 * @param [in]t Подпись каждого прямоугольника
 * @param [in]color Цвет прямоугольников и подписей
 * @param [in]fontScale Кегль подписи
 * @param [in]textThickness Толщина линии подписи
 * @param [in]textOffset Отступ подписи от прямоугольника
 * @param [in]thickness Толщина прямоугольника
 * @param [in]fontFace Шрифт
 */
void drawRects(Mat& img, const vector<Rect>& rects,
               string t="rect", Scalar color=Scalar(255,0,0),
               float fontScale=1.0,
               float textThickness=1.0,
               int textOffset=0,
               int thickness=1,
               int fontFace=CV_FONT_NORMAL){
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
 * Рисует точки по правилу третей в заданном прямоугольнике r
 * @param [in,out]img Изображение, на которое наносятся точки правила третей
 * @param [in]r Прямоугольник, в котором определяется правило третей
 * @param [in]color Цвет точек
 * @param [in]dotsRadius Радиус точек
 */
inline void drawThirds(Mat& img, const Rect2f& r,Scalar color=Scalar(0,255,0),const double& dotsRadius=1){
    circle(img,Point(r.x + r.width/3.0,
                              r.y + r.height/3.0), 1,Scalar(0,255,0), dotsRadius);
    circle(img,Point(r.x + 2.0*r.width/3.0,
                              r.y + r.height/3.0),1,Scalar(0,255,0), dotsRadius);
    circle(img,Point(r.x + r.width/3.0,
                              r.y + 2.0*r.height/3.0),1,Scalar(0,255,0),dotsRadius);
    circle(img,Point(r.x + 2.0*r.width/3.0,
                              r.y + 2.0*r.height/3.0),1,Scalar(0,255,0),dotsRadius);
}
/**
 * @brief Медиана набора прямоугольников
 * @param [in]r Массив прямоугольников
 * @return Прямоугольник, высота которого - медиана высот, а координаты - медианы координат прямоугольников
 */
Rect median(const vector<Rect>& r){
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

/**
 * @class Arg
 * Этот класс предназначен для ввода числовых параметров программы различных типов
 */
template <typename T>
class Arg
{
    T val; ///< Значение
    T *valDef; ///< Значение по-умолчанию
    string *opt;///< Как должен выглядеть аргумент при запуске программы
    string *format;///< Спецификатор для ввода параметров (см. документацию [scanf](http://www.cplusplus.com/reference/cstdio/scanf/))
    T *gt;///< нижняя граница для параметра (Greater Than)
    T *lt;///< верхняя граница для параметра (Less Than)
public:
    /**
     * @brief Конструктор
     * @param [in]defVal Значение параметра по-умолчанию
     * @param [in]opt_ Как должен выглядеть аргумент при запуске программы
     * @param [in]format_ Спецификатор (см. документацию [scanf](http://www.cplusplus.com/reference/cstdio/scanf/))
     * @param [in]greater Нижняя граница для числа. По-умолчанию границы нет.
     * @param [in]less Верхняя граница для числа. По-умолчанию границы нет.
     */
    Arg(const T defVal, const string opt_, const string format_,  const T* greater=NULL,const T* less=NULL)
        : val(defVal) {
        opt = new string(opt_);
        format = new string(format_);
        valDef = new T(defVal);
        if(greater) gt = new T(*greater); else gt=NULL;
        if(less)    lt = new T(*less);    else lt=NULL;
        cout << "\t" << *opt << "[" << val << "]" << endl;
    }
    /**
     * @brief Ввод параметра
     * Определяет, является ли текущая строка нужным идентефикатором
     * @param [in]argv один аргумент программы
     * @return true, если была распознана строка-идентификатор и знчение удалось прочитать
     */
    bool input(const char* argv){
        if(exists(argv))
        {
            if(sscanf( argv + opt->length(), format->c_str(), &val)){
                if((gt!=NULL && *gt>val))
                    val=*valDef;
                else if((lt!=NULL && *lt<val))
                    val=*valDef;
                cout << " and " << val << " assigned."<<endl;
                delete format, opt;
                delete valDef;
                return true;
            }
        }
        return false;
    }
    /**
     * @brief operator T
     * Маскировка данного класса под используемый тип
     */
    operator T(){
        return val;
    }
    /**
     * @brief operator =
     * Маскировка данного класса под используемый тип
     * @param [in]newVal
     * @return Новое значение параметра
     */
    T operator =(T newVal){
        val=newVal;
        return val;
    }
    /**
     * @brief Поиск аргумента
     * Ищет аргумент в заданной строке
     * @param [in]argv Один аргумент программы
     * @return true, если аргумент найден
     */
    bool exists(const char* argv){
        return (opt->compare(0,opt->length(),argv,opt->length())==0);
    }
    ~Arg(){
        delete gt,lt;
    }
};
/**
 * @brief Главная функция
 * @param [in]argc Количество аргументов в программе
 * @param [in]argv Массив аргументов
 * @return 0, если программа завершена удачно
 */
int main( int argc, const char** argv )
{

    string inputName; ///< Путь к файлу видео для обработки. Если не введен, то изображение захватывается с камеры

    cout << "Availible parameters: " << endl;
    /// Параметры детектора лиц
    CascadeClassifier cascadeFull,cascadeProf; ///< Каскады Хаара для детекции лица
    string cascadeFullName = "../../data/haarcascades/haarcascade_frontalface_alt.xml";
    string cascadeProfName = "../../data/haarcascades/haarcascade_profileface.xml";
    const string nestedCascadeOpt = "--nested-cascade";
    size_t nestedCascadeOptLen = nestedCascadeOpt.length();
    const string cascadeOpt = "--cascade=";
    size_t cascadeOptLen = cascadeOpt.length();

    Arg<double> scale(3,"--scale=","%lf", new double(1)); ///< Этот параметр отвечает за то, во сколько раз следует сжать кадр перед тем как приступить к детекции лица
    Arg<int>minNeighbors(1,"--minNeighbors=","%d",new int(1)); ///<  Количество соседних детекций лица в изображении
    Arg<double> scaleFactor(1.1,"--scaleFactor=","%lf", new double(1.1));/**< шаг изменения размеров лица, которое ожидается детектировать
                                                                            в изображении.Чем ближе этот параметр к единице,
                                                                        тем точнее будет определён размер лица, но тем дольше будет работать алгоритм*/
    Arg<int> minFaceHeight(25,"--minFaceHeight=","%d", new int(1));///< Минимальные размеры детектируемого лица
    Arg<int> aimUpdatePer(15,"--aimUpdatePer=","%d",new int(1));///< Период обновления цели (каждые n кадров), к которой будет следоватьвиртуальная камера
    Arg<int> faceDetectPer(1,"--faceDetectPer=","%d", new int(1));///< Период детектирования лиц

    ///Запись результата
    Arg<int> resultHeight(480,"--resultHeight=","%d", new int(1));///< Высота результирующего видео (ширина рассчитывается автоматически в соответствии с соотношением сторон)
    Arg<int> recordResult(1,"--recordResult=","%d",new int(0));///< Записывать результирующее видео.
    Arg<int> writeCropFile(0,"--writeCropFile=","%d",new int(0));///< Записывать фильтр-скрипт для обработки исходного видео в ffmpeg (см. [filter_script](http://ffmpeg.org/ffmpeg.html#Main-options))

    /// Визуализация
    Arg<int> showPreview(0,"--showPreview=","%d",new int(0));///< Показывать в реальном времени процесс обработки видео с отрисовкой виртуальной камеры и детектированных лиц
    Arg<int> recordPreview(0,"--recordPreview=","%d",new int(0));///< Записывать процесс обработки видео в отдельный файл

    /// Перемещение виртуальной камеры
    Arg<float>maxStepX(1,"--maxStepX=","%f",new float(0.2));///< Максимальная скорость по координате Х
    Arg<float>maxStepY(1,"--maxStepY=","%f",new float(0.2));///< Максимальная скорость по координате У
    /// Зум
    Arg<float> zoomStopThr_ (10.0,"--zoomStopThr=","%f",new float(1));///< Триггерное значение окончания зуммирования
    Arg<float> zoomThr(FI,"--zoomThr=","%f",new float(1));///< Триггер начала зуммирования
    Arg<double> face2shot(FI,"--face2shot=","%lf",new double(0.1));///< Требуемое отношение высоты лица к высоте кадра
    Arg<double> zoomSpeedMin(0.00,"--zoomSpeedMin=","%lf",new double(0.00));///< Минимальная скорость зума
    Arg<double> zoomSpeedMax(0.03,"--zoomSpeedMax=","%lf",new double(0.001));///< Максимальная скорость зума
    Arg<double> zoomSpeedInc_(15.0,"--zoomSpeedInc=","%lf",new double(0.001));///< Инкремент скорости зума

    help();

    /// Чтение аргументов программы
    for( int i = 1; i < argc; i++ )
    {
        cout << "Processing " << i << " " <<  argv[i];

        if( cascadeOpt.compare( 0, cascadeOptLen, argv[i], cascadeOptLen ) == 0 )
        {
            cascadeFullName.assign( argv[i] + cascadeOptLen );
            cout << "  from which we have cascadeName= " << cascadeFullName << endl;
        }
        else if( nestedCascadeOpt.compare( 0, nestedCascadeOptLen, argv[i], nestedCascadeOptLen ) == 0 )
        {
            cout << "nc" <<endl;
            if( argv[i][nestedCascadeOpt.length()] == '=' );
//                cascadeLEyeName.assign( argv[i] + nestedCascadeOpt.length() + 1 );

        }
        else if(scale.input(argv[i]));
        else if(minNeighbors.input(argv[i]));
        else if(scaleFactor.input(argv[i]));
        else if(minFaceHeight.input((argv[i])));

        else if(aimUpdatePer.input(argv[i]));
        else if(faceDetectPer.input(argv[i]));

        else if(resultHeight.input(argv[i]));
        else if(showPreview.input(argv[i]));
        else if(recordPreview.input(argv[i]));
        else if(maxStepX.input(argv[i]));
        else if(maxStepY.input(argv[i]));
        else if(zoomStopThr_.input(argv[i]));

        else if(writeCropFile.input(argv[i]));
        else if(recordResult.input(argv[i]));

        else if(zoomThr.input(argv[i]));
        else if(face2shot.input(argv[i]));
        else if(zoomSpeedMin.input(argv[i]));
        else if(zoomSpeedMax.input(argv[i]));
        else if(zoomSpeedInc_.input(argv[i]));

        else if(argv[i][0] == '-' )
        {
            cerr << "WARNING: UnkoneIterEndn option %s" << argv[i] << endl;
        }
        else inputName.assign( argv[i] );
    }
    if( !cascadeFull.load( cascadeFullName ) )
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        help();
        return -1;
    }
    if( !cascadeProf.load( cascadeProfName ) )
    {
        cerr << "ERROR: Could not load classifier 2 cascade" << endl;
        help();
        return -1;
    }
    bool isWebcam=false;
    VideoCapture capture;
    if( inputName.empty() || (isdigit(inputName.c_str()[0]) && inputName.c_str()[1] == '\0') )
    {
        isWebcam=true;
        int c = inputName.empty() ? 0 : inputName.c_str()[0] - '0' ;
        if(!capture.open(c))
            cout << "Capture from camera #" <<  c << " didn't work" << endl;
    }else{
        if(capture.open(inputName)){
            cout << "Capture file " <<  inputName << endl;
            isWebcam=false;
        }
        else
            cout << "Could not capture file " <<  inputName << endl;
    }

    if( capture.isOpened() )
    {
        if(isWebcam) writeCropFile=false;
        cout << __LINE__ <<endl;
        cout << "Video capturing has been started ..." << endl;

   //    MODEL    //
        // Кадры
        Mat fullFrame;
        Mat smallImg;
        Mat graySmall;
        Mat result;

        // Face detection
        const double fx = 1 / scale;
        const Size minfaceSize=Size(minFaceHeight,minFaceHeight);

        bool foundFaces=false;
        vector<Rect> facesFull,facesProf,faceBuf;

        /// Характеристики видео
        const long int videoLength = capture.get(CAP_PROP_FRAME_COUNT);
        const float aspectRatio = (float)capture.get(CV_CAP_PROP_FRAME_WIDTH)/
                                  (float)capture.get(CV_CAP_PROP_FRAME_HEIGHT);
        const Rect2f fullShot(0,0,(int)capture.get(CV_CAP_PROP_FRAME_WIDTH),
                                (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT));
        int fps; ///< Количество кадров в секунду
        int fourcc; ///< Код кодека, состоящий из 4-х символов (см. \ref fourcc.org http://www.fourcc.org/codecs.php)
        long int frameCounter=0;


        const Size smallImgSize = Size((float)fullShot.width/scale, (float)fullShot.height/scale);
        const Size maxRoiSize = smallImgSize;
        const Size previewSize = smallImgSize;
        const Size resultSize = Size(resultHeight*aspectRatio,resultHeight);

        ///Zoom & movement params (driver)
        const double onePerc =(double)smallImgSize.width/100.0; // onePercent

        Point gp;
        Rect aim=Rect(Point(0,0),maxRoiSize);
        Rect2f roi = Rect2f(Point(0,0),maxRoiSize);

        const bool bZoom = true;
        const bool bMove = true;

        DYNAMIC_STATES zoomState = STOP;
        AutoMotion moveX(0,maxStepX*onePerc),moveY(0,maxStepY*onePerc);
        AutoZoom zoom(zoomSpeedMin,zoomSpeedMax,maxRoiSize,zoomThr,cvRound(zoomStopThr_*onePerc),zoomSpeedInc_,face2shot);

        //file writing
        stringstream outTitleStream;
        string outFileTitle;
        VideoWriter previewVideo;
        VideoWriter outputVideo;

        ///Test items
        const double ticksPerMsec=cvGetTickFrequency() * 1.0e3;
        vector<int64> tmr;
        vector<int> lines;
        fstream logFile;
        stringstream pzoom;


   //    VIEW    //
        Mat preview;
        Rect2f roiFullSize;
        //drawing
        const float thickness = 0.5*previewSize.width/100.0;
        const int dotsRadius = thickness*2;
        const int textOffset = thickness*2;
        const int textThickness = thickness/2.0;
        const double fontScale = thickness/5;
        string prevWindTitle = "Preview";

         // SetUp
        if(isWebcam){
            time_t t = time(0);   // get time now
            struct tm * now = localtime( & t );
             outTitleStream << "webcam"
                          << (now->tm_year + 1900) << '_'
                          << (now->tm_mon + 1) << '_'
                          << now->tm_mday << "_"
                          << now->tm_hour <<"-"
                          << now->tm_min << "_"
                          << __DATE__ <<"_"<< __TIME__ <<"_arg"
                          << scale
                          << minNeighbors
                          << scaleFactor
                          << minFaceHeight
                          << aimUpdatePer
                          << faceDetectPer
                          << resultHeight
                          << showPreview
                          << recordPreview
                          << maxStepX
                         << maxStepY
                         << zoomStopThr_
                         << writeCropFile
                         << recordResult
                         << zoomThr
                         << face2shot
                         << zoomSpeedMin
                         << zoomSpeedMax
                         << zoomSpeedInc_;
        }else{
            outTitleStream << inputName.substr(inputName.find_last_of('/')+1)
            << "_" <<__DATE__ <<"_"<< __TIME__<<"_arg"
            << scale<< "_"
            << minNeighbors << "_"
            << scaleFactor << "_"
            << minFaceHeight << "_"
            << aimUpdatePer << "_"
            << faceDetectPer << "_"
            << resultHeight << "_"
            << showPreview << "_"
            << recordPreview << "_"
            << maxStepX << "_"
           << maxStepY << "_"
           << zoomStopThr_ << "_"
           << writeCropFile << "_"
           << recordResult << "_"
           << zoomThr << "_"
           << face2shot << "_"
           << zoomSpeedMin << "_"
           << zoomSpeedMax << "_"
           << zoomSpeedInc_;
        }
        outFileTitle=outTitleStream.str();
        replace(outFileTitle.begin(),outFileTitle.end(),' ','_');
        replace(outFileTitle.begin(),outFileTitle.end(),':','-');

        if(isWebcam){
            fps = capture.get(CAP_PROP_FPS)/5.0;
            fourcc = VideoWriter::fourcc('M','J','P','G'); // codecs
        }
        else{
            fourcc = capture.get(CV_CAP_PROP_FOURCC); // codecs
            fps = capture.get(CAP_PROP_FPS);
        }
        if(recordResult && !outputVideo.open("results/closeUp_"+outFileTitle+".avi",fourcc,
                                         fps, resultSize, true)){
            cout << "Could not open the output video ("
                 << "results/closeUp_"+outFileTitle+".avi" <<") for writing"<<endl;
            return -1;
        }
        if(recordPreview){
             if(!previewVideo.open("results/test_"+outFileTitle+".avi",fourcc,
                                   fps, previewSize, true))
             {
                 cout << "Could not open the output video ("
                      << "results/test_"+outFileTitle+".avi" <<") for writing"<<endl;
                 return -1;
             }
        }
        logFile.open(("results/test_"+outFileTitle+".ods").c_str(), fstream::out);
        if(!logFile.is_open()){
            cout << "Error with opening the file:" << "results/test_"+outFileTitle+".ods" << endl;
        }

        //// Main cycle
        for(;;)
        {

            tmr.push_back(cvGetTickCount());if(frameCounter<2)lines.push_back(__LINE__);
            try{
                capture >> fullFrame;
                if(!fullFrame.empty() && waitKey(1) != 27) {
                    if(frameCounter>0)pzoom<<',';
                }
                else{
                    clog << "Frame is empty" << endl;
                    break;
                }


                tmr.push_back(cvGetTickCount());if(frameCounter<2)lines.push_back(__LINE__);
                if(frameCounter%faceDetectPer==0){
                    resize( fullFrame, smallImg, smallImgSize, 0, 0, INTER_LINEAR );
                    cvtColor( smallImg, graySmall, COLOR_BGR2GRAY );
                /// \todo 04/05/2016 Искать лица только в некоторой области вокруг уже обнаруженного лица
                    /* Поиск лиц в анфас */
                    cascadeFull.detectMultiScale(graySmall, facesFull,
                                                 scaleFactor, minNeighbors, 0|CASCADE_SCALE_IMAGE,minfaceSize);
                    /// Поиск лиц в профиль
                    cascadeProf.detectMultiScale( graySmall, facesProf,
                                                  scaleFactor, minNeighbors, 0|CASCADE_SCALE_IMAGE,minfaceSize);
                    foundFaces = !(facesFull.empty() && facesProf.empty());
                }else foundFaces = false;
                tmr.push_back(cvGetTickCount());if(frameCounter<2)lines.push_back(__LINE__);

                if(foundFaces){
                    if(!facesFull.empty() && ((facesFull[0]&aim).area()>0))faceBuf.push_back(facesFull[0]);
                    if(!facesProf.empty() && ((facesProf[0]&aim).area()>0))faceBuf.push_back(facesProf[0]);
                }
                if(frameCounter%aimUpdatePer == 0){
                    if(!faceBuf.empty()) {
                        aim = median(faceBuf);
                        faceBuf.clear();
                    }
                }
            }catch(Exception &mvEx){
                cout << "Detection block: "<< mvEx.msg << endl;
            }
            /// Motion and zoom
            tmr.push_back(cvGetTickCount());if(frameCounter<2)lines.push_back(__LINE__);
            try{
                if(bZoom){
                    zoom.update(aim,roi);
                }
                tmr.push_back(cvGetTickCount());if(frameCounter<2)lines.push_back(__LINE__);

                if(bMove) {
                    bool outOfRoi = ((Rect2f)aim&roi).area()<aim.area();
                    gp = getGoldenPoint(roi,aim);
                    moveX.update(roi.x,gp.x,roi.width/15.0,outOfRoi);
                    moveY.update(roi.y,gp.y,roi.height/3.0,outOfRoi);
                }
                tmr.push_back(cvGetTickCount());if(frameCounter<2)lines.push_back(__LINE__);

                if(roi.x<0) roi.x = 0;
                if(maxRoiSize.width < roi.x+roi.width)
                    roi.x = maxRoiSize.width-roi.width;
                if(roi.y<0)roi.y = 0;
                if(maxRoiSize.height < roi.y+roi.height)
                    roi.y=maxRoiSize.height-roi.height;
                /// end Motion and zoom
            }catch(Exception &mvEx){
                cout << "Motion and zoom block: "<< mvEx.msg << endl;
            }

            tmr.push_back(cvGetTickCount());if(frameCounter<2)lines.push_back(__LINE__);
            try{
                roiFullSize = Rect2f(Point(roi.x*scale,roi.y*scale),Size(roi.width*scale,roi.height*scale));
                if(recordResult){
                    resize(fullFrame(roiFullSize), result , resultSize, 0,0, INTER_LINEAR );
                    outputVideo << result ;
                }
            }catch(Exception &mvEx){
                cout << "Result saving: "<< mvEx.msg << endl;
            }
            tmr.push_back(cvGetTickCount());if(frameCounter<2)lines.push_back(__LINE__);
            if(showPreview || recordPreview){ // Отрисовка области интереса
                resize(fullFrame, preview, smallImgSize, 0, 0, INTER_NEAREST );

                // Рисовать кадр захвата
                if(zoom.getState()==BEGIN)rectangle(preview,roi,Scalar(100,255,100),thickness+zoom.getStopThr());
                if(zoom.getState()==MOVE)rectangle(preview,roi,Scalar(255,255,255),thickness+zoom.getStopThr());
                if(zoom.getState()==END)rectangle(preview,roi,Scalar(100,100,255),thickness+zoom.getStopThr());
                rectangle(preview,roi,Scalar(0,0,255),thickness);
                Scalar colorX;
                switch (moveX.getState()){
                case BEGIN:
                    colorX = Scalar(127,255,127);break;
                case MOVE:
                    colorX = Scalar(255,255,255);break;
                case END:
                    colorX = Scalar(127,127,255);break;
                }
                Scalar colorY;
                switch (moveY.getState()){
                case BEGIN:
                    colorY = Scalar(127,255,127);break;
                case MOVE:
                    colorY = Scalar(255,255,255);break;
                case END:
                    colorY = Scalar(127,127,255);break;
                }
                if(moveX.getState()!=STOP){
                    if(moveX.getSign()<0)
                        arrowedLine(preview,
                                    Point(roi.x,roi.y+roi.height/2),
                                    Point(roi.x-moveX.getSpeed()*2,roi.y+roi.height/2),
                                    colorX,thickness,8,0,1);
                    else
                        arrowedLine(preview,
                                    Point(roi.x+roi.width,roi.y+roi.height/2),
                                    Point(roi.x+roi.width+moveX.getSpeed()*2,roi.y+roi.height/2),
                                    colorX,thickness,8,0,1);
                }
                if(moveY.getState()!=STOP){
                    if(moveY.getSign()<0)
                        arrowedLine(preview,
                                    Point(roi.x+roi.width/2,roi.y),
                                    Point(roi.x+roi.width/2,roi.y-moveY.getSpeed()*2),
                                    colorY,thickness,8,0,1);
                    else
                        arrowedLine(preview,
                                    Point(roi.x+roi.width/2,roi.br().y),
                                    Point(roi.x+roi.width/2,roi.br().y+moveY.getSpeed()*2),
                                    colorY,thickness,8,0,1);
                }
                drawThirds(preview,roi,Scalar(0,255,0),dotsRadius);
                // Рисовать цель
                rectangle(preview,aim,Scalar(0,255,0),thickness);
                putText(preview, "aim",aim.tl(),CV_FONT_NORMAL,fontScale,Scalar(0,255,0),textThickness);
                //Отрисовка распознаных объектов на превью
                drawRects(preview,facesFull,"Full face",Scalar(255,0,0),fontScale,textThickness,textOffset,thickness);
                drawRects(preview,facesProf,"Profile",Scalar(255,127,0),fontScale,textThickness,textOffset,thickness);

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
                putText(preview,timestring.str(),Point(0,smallImgSize.height-3),CV_FONT_NORMAL,fontScale*1.35,Scalar(0,0,0),textThickness*100);
                putText(preview,timestring.str(),Point(0,smallImgSize.height-3),CV_FONT_NORMAL,fontScale*1.35,Scalar(255,255,255),textThickness*2);
                stringstream frame;
                frame << frameCounter;
                putText(preview,(frame.str()),Point(0,fontScale*20),CV_FONT_NORMAL,fontScale*1.45,Scalar(0,0,0,100),textThickness*10);
                putText(preview,(frame.str()),Point(0,fontScale*20),CV_FONT_NORMAL,fontScale*1.45,Scalar(255,255,255),textThickness*5);
                tmr.push_back(cvGetTickCount());if(frameCounter<2)lines.push_back(__LINE__);
                // Сохранение кадра
                if(recordPreview)
                    previewVideo << preview;
                if(showPreview)
                    imshow(prevWindTitle.c_str(),preview);
            }
            tmr.push_back(cvGetTickCount());if(frameCounter<2)lines.push_back(__LINE__);


            /// Запись статистики
            if(logFile.is_open()) {
                if(frameCounter<1){
                    logFile << "frame\t";
                    for (int i = 0; i < lines.size()-1; ++i) {
                        logFile << lines[i] << "-" << lines[i+1] << "\t";
                    }
                    logFile << "facesFull.x, px"<<"\t"
                            <<"facesFull.y, px"<<"\t"
                           << "facesFull.width, px"<<"\t"
                           <<"facesFull.height, px"<<"\t"
                          << "faceProf.x, px"<<"\t"
                          <<"faceProf.y, px"<<"\t"
                         << "facesProf.width, px"<<"\t"
                         <<"facesProf.height, px"<<"\t"
                        << "aim.x, px"<<"\t"
                        <<"aim.y, px"<<"\t"
                       << "aim.width, px"<<"\t"
                       <<"aim.height, px"<<"\t"
                      << "roi.x, px\t"
                      <<"roi.y, px"<<"\t"
                     << "roi.width, px\t"
                     <<"roi.height, px"<< endl;
                }
                logFile  << frameCounter <<"\t";
                for (int i = 0; i < tmr.size()-1; ++i) {
                    logFile << (double)(tmr[i+1]-tmr[i])/ticksPerMsec <<"\t";
                }
                if(!facesFull.empty())
                    logFile << facesFull[0].x << "\t"
                            << facesFull[0].y << "\t"
                            << facesFull[0].width << "\t"
                            << facesFull[0].height << "\t";
                else logFile << "\t\t\t\t";
                if(!facesProf.empty())
                    logFile << facesProf[0].x << "\t"
                            << facesProf[0].y << "\t"
                            << facesProf[0].width << "\t"
                            << facesProf[0].height << "\t";
                else logFile << "\t\t\t\t";

                logFile << aim.x << "\t"
                        << aim.y << "\t"
                        << aim.width << "\t"
                        << aim.height << "\t";
                logFile << roi.x << "\t"
                        << roi.y << "\t"
                        << roi.width << "\t"
                        << roi.height << endl;
            }
            if(writeCropFile){ /// \todo 9.05.2016 Не только вырезать, но и изменять размер кадра
                pzoom << "zoompan=enable=eq(n\\,"<< frameCounter
                      << "):z=" << fullShot.width/roiFullSize.width
                      << ":x=" << roiFullSize.x
                      << ":y=" << roiFullSize.y << ":d=1";
            }

            cout <<"frame:"<< frameCounter
                << " fps:" << (int)(1000*(float)ticksPerMsec/(float)(tmr[tmr.size()-1]-tmr[0]))
                << " ("<< (int)((100*(float)frameCounter)/(float)videoLength) <<"% of video)"
                << endl;
            ++frameCounter;

            tmr.clear();
            lines.clear();
        }
        if(logFile.is_open())logFile.close();
        if(writeCropFile){
            fstream cropFile;
            cropFile.open(("results/filter_"+outFileTitle).c_str(),fstream::out);
            if(cropFile.is_open()){
                cropFile  << pzoom.str();
                cropFile.close();
            }else{
                clog << "Error with opening the file:" << "results/test_"+outFileTitle+".crop" << endl;
            }
        }
        cout << "The results have been written to " << "''"+outFileTitle+"''" << endl;
        cvDestroyAllWindows();
    }
    return 0;
}
