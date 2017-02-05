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
#include <opencv2/video/video.hpp>  // Video write
#include <ctime>

#include "automotion.h"
#include "autozoom.h"
#include "autocamera.h"
#include "arg.h"
#include "detector.h"
#include "3rdparty/asms/colotracker.h"

using namespace std;
using namespace cv;


const double FI=1.61803398; /// Золотое сечение

static void help()
{
    clog << "Build date:" << __DATE__ << " " << __TIME__
            "\n\tDuring execution:\n\tHit any key to quit.\n"
            "\tUsing OpenCV version " << CV_VERSION << "\n" << endl;
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
void drawRects(Mat& img, const vector<Rect>& rects,
               string t="rect", Scalar color=Scalar(255,0,0),
               float fontScale=1.0,
               float textThickness=1.0,
               int textOffset=0,
               int thickness=1,
               int fontFace=CV_FONT_NORMAL){ /// \todo 18.05.2016 class Drawer
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
inline void drawThirds(Mat& img, const Rect2f& r,Scalar color=Scalar(0,255,0),const double& dotsRadius=1){ /// \todo 18.05.2016 class Drawer
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
 * @param[in] r Массив прямоугольников
 * @return Прямоугольник, высота которого - медиана высот, а координаты - медианы координат прямоугольников
 */
Rect median(const vector<Rect>& r){ /// \todo 18.05.2016 class Detector
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
 * @brief Главная функция
 * @param[in] argc Количество аргументов в программе
 * @param[in] argv Массив аргументов
 * @return 0, если программа завершена удачно
 */
int main( int argc, const char** argv )
{

    string inputName; ///< Путь к файлу видео для обработки. Если не введен, то изображение захватывается с камеры /// \todo 18.05.2016 class FileSaver

    clog << "Availible parameters: " << endl;
    /// Параметры детектора лиц
    CascadeClassifier cascadeFull,cascadeProf; ///< Каскады Хаара для детекции лица /// \todo 18.05.2016 class Detector
    string cascadeFullName = "../haarcascade_frontalface_alt.xml";
    string cascadeProfName = "../haarcascade_profileface.xml";
    const string cascadeProfOpt = "--cascadeProf=";
    size_t cascadeProfOptLen = cascadeProfOpt.length();
    const string cascadeFullOpt = "--cascadeFront=";
    size_t cascadeFullOptLen = cascadeFullOpt.length();

    Arg<double> scale(3,"--scale=","%lf", new double(1)); ///< Этот параметр отвечает за то, во сколько раз следует сжать кадр перед тем как приступить к детекции лица
    Arg<int>minNeighbors(1,"--minNeighbors=","%d",new int(1)); ///<  Количество соседних детекций лица в изображении
    Arg<double> scaleFactor(1.1,"--scaleFactor=","%lf", new double(1.1));/**< шаг изменения размеров лица, которое ожидается детектировать
                                                                            в изображении.Чем ближе этот параметр к единице,
                                                                        тем точнее будет определён размер лица, но тем дольше будет работать алгоритм*/
    Arg<int> minFaceHeight(25,"--minFaceHeight=","%d", new int(1));///< Минимальные размеры детектируемого лица
    Arg<int> aimUpdatePer(15,"--aimUpdatePer=","%d",new int(1));///< Период обновления цели (каждые n кадров), к которой будет следоватьвиртуальная камера
    Arg<int> faceDetectPer(1,"--faceDetectPer=","%d", new int(1));///< Период детектирования лиц

    Arg<int> aimCheckerPer(10,"--aimCheckerPeriod=","%d",new int(1)); ///< период редетекции лица внутри прямоугольника, который создаёт трекер/ задаётся в кадрах
    Arg<int> detWarningLimit(3,"--detWarningLimit=","%d",new int(1));
    Arg<unsigned int> focusEx(25,"--aimEx=","%d",0); /// на сколько пикселей расширять область редетекции, чтобы попытаться найти цель

    ///Запись результата
    Arg<int> resultWidth(640,"--resultWidth=","%d", new int(1));///< Высота результирующего видео (ширина рассчитывается автоматически в соответствии с соотношением сторон)
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
    Arg<float> zoomStartThr(0.1,"--zoomThr=","%f",new float(0.001), new float(0.9999));///< zoomStartThr показывает насколько точно масштабирование/зум должны совпасть с целевым 1- должно быть один в один; 3 - может быть чуть больше или чуть меньше aimH>roi.height*zoomThr в относительных единицах
    Arg<double> face2shot(FI,"--face2shot=","%lf",new double(0.1));///< Требуемое отношение высоты лица к высоте кадра
    Arg<double> zoomSpeedMin(0.00,"--zoomSpeedMin=","%lf",new double(0.00));///< Минимальная скорость зума
    Arg<double> zoomSpeedMax(1.00,"--zoomSpeedMax=","%lf",new double(0.001));///< Максимальная скорость зума
    Arg<double> zoomSpeedInc_(0.1,"--zoomSpeedInc=","%lf",new double(0.001));///< Инкремент скорости зума

    Arg<int> streamToStdOut(0,"--streamToStdOut=","%d",new int(0)); ///< Печатать кадры результирующего видео в стандартный вывод
    help();

    /// Чтение аргументов программы
    for( int i = 1; i < argc; i++ )
    {
        clog << "Processing " << i << " " <<  argv[i];

        if( cascadeFullOpt.compare( 0, cascadeFullOptLen, argv[i], cascadeFullOptLen ) == 0 )
        {
            cascadeFullName.assign( argv[i] + cascadeFullOptLen );
            clog << "  from which we have cascadeName= " << cascadeFullName << endl;
        }
        else if( cascadeProfOpt.compare( 0, cascadeProfOptLen, argv[i], cascadeProfOptLen ) == 0 )
        {
            clog << "nc" <<endl;
            if( argv[i][cascadeProfOpt.length()] == '=' );
                cascadeProfName.assign( argv[i] + cascadeProfOpt.length() + 1 );

        }
        else if(scale.input(argv[i]));
        else if(minNeighbors.input(argv[i]));
        else if(scaleFactor.input(argv[i]));
        else if(minFaceHeight.input((argv[i])));

        else if(aimUpdatePer.input(argv[i]));
        else if(faceDetectPer.input(argv[i]));

        else if(resultWidth.input(argv[i]));
        else if(resultHeight.input(argv[i]));
        else if(showPreview.input(argv[i]));
        else if(recordPreview.input(argv[i]));
        else if(maxStepX.input(argv[i]));
        else if(maxStepY.input(argv[i]));

        else if(writeCropFile.input(argv[i]));
        else if(recordResult.input(argv[i]));

        else if(zoomStartThr.input(argv[i]));
        else if(face2shot.input(argv[i]));
        else if(zoomSpeedMin.input(argv[i]));
        else if(zoomSpeedMax.input(argv[i]));
        else if(zoomSpeedInc_.input(argv[i]));
        else if(streamToStdOut.input(argv[i]));
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

    bool isWebcam=false; /// \todo 18.05.2016 class InputMan
    VideoCapture capture;
    while(!capture.isOpened()){
        if( inputName.empty() || (isdigit(inputName.c_str()[0]) && inputName.c_str()[1] == '\0') )
        {
            isWebcam=true;
            int c = inputName.empty() ? 0 : inputName.c_str()[0] - '0' ;
            if(!capture.open(c))
                clog << "Capture from camera #" <<  c << " didn't work" << endl;
        }else{
            if(capture.open(inputName)){
                clog << "Capture file " <<  inputName << endl;
                isWebcam=false;
            }
            else
                clog << "Could not capture file " <<  inputName << endl;
        }
    }

    if( capture.isOpened() )
    {
        if(isWebcam) writeCropFile=false; /// \todo 18.05.2016 class FileSaver
        clog << "Video capturing has been started ..." << endl;

   //    MODEL    //
        // Кадры
        Mat fullFrame; /// \todo 18.05.2016 class InputMan, class Detector
        Mat result; /// \todo 18.05.2016 class FileSaver
        Mat smIm;
        /// Характеристики видео
        const long int videoLength = /*isWebcam ? 1 :*/ capture.get(CAP_PROP_FRAME_COUNT);
        const float aspectRatio = (float)capture.get(CV_CAP_PROP_FRAME_WIDTH)/
                                  (float)capture.get(CV_CAP_PROP_FRAME_HEIGHT);/// \todo 05/12/2016 При стриминге размеры кадра определяются почему-то неправильно
        const Rect2f fullShot(0,0,(int)capture.get(CV_CAP_PROP_FRAME_WIDTH),
                                (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT));
        int fps; ///< Количество кадров в секунду /// \todo 18.05.2016 class FileSaver, class Previewer
        int fourcc; ///< Код кодека, состоящий из 4-х символов (см. \ref fourcc.org http://www.fourcc.org/codecs.php)
        long int frameCounter=0; /// \todo 18.05.2016 class InputMan

        const Size previewSize = Size((float)fullShot.width, (float)fullShot.height);
//        const Size resultSize = Size(resultHeight*aspectRatio,resultHeight);

        const Size resultSize = Size(resultWidth,resultHeight);
        ///Zoom & movement params (driver)
        Detector det(cascadeFullName,cascadeProfName,
                     Size((float)fullShot.width/scale, (float)fullShot.height/scale),
                     aimUpdatePer,faceDetectPer,minNeighbors,minFaceHeight,scaleFactor,scale);
        AutoCamera cam(fullShot.size(),maxStepX,maxStepY,zoomSpeedMin,zoomSpeedMax,zoomStartThr,zoomSpeedInc_,face2shot,1,1); /// \todo 28.01.2017 убрать отсюда параметр scale. См. класс Detector

        ColorTracker* tracker = NULL;
        BBox* bb = NULL;
        bool bTrackerInitialized = false;
        Rect aim;
        Rect focus;


        //file writing
        stringstream outTitleStream;
        string outFileTitle;
        VideoWriter previewVideo;
        VideoWriter outputVideo;

        ///Test items
        fstream logFile;
        stringstream pzoom;


        //    VIEW    //
        /// \todo 18.05.2016 class Drawer ///
        Mat preview; ///<
        Rect2f roiFullSize;
        //drawing
        const float thickness = 0.5*previewSize.width/100.0;
        const int dotsRadius = thickness*2;
        const int textOffset = thickness*2;
        const int textThickness = thickness/2.0;
        const double fontScale = thickness/5;
        string prevWindTitle = "Preview";
        /// \todo 18.05.2016 end class Drawer ///

         // SetUp
        if(isWebcam){ /// \todo 18.05.2016 class InputMan
            time_t t = time(0);   // get time now
            struct tm * now = localtime( & t );
             outTitleStream << "webcam"
                          << (now->tm_year + 1900) << '_'
                          << (now->tm_mon + 1) << '_'
                          << now->tm_mday << "_"
                          << now->tm_hour <<"-"
                          << now->tm_min << "_"
                          << __DATE__ <<"_"<< __TIME__ <<"_arg"
//                          << scale
//                          << minNeighbors
//                          << scaleFactor
//                          << minFaceHeight
//                          << aimUpdatePer
//                          << faceDetectPer
//                          << resultHeight
//                          << showPreview
//                          << recordPreview
//                          << maxStepX
//                         << maxStepY
//                         << writeCropFile
//                         << recordResult
//                         << zoomStartThr
//                         << face2shot
//                         << zoomSpeedMin
//                         << zoomSpeedMax
                         << zoomSpeedInc_;
        }else{
            outTitleStream << inputName.substr(inputName.find_last_of('/')+1)
            << "_" <<__DATE__ <<"_"<< __TIME__<<"_arg"
//            << scale<< "_"
//            << minNeighbors << "_"
//            << scaleFactor << "_"
//            << minFaceHeight << "_"
//            << aimUpdatePer << "_"
//            << faceDetectPer << "_"
//            << resultHeight << "_"
//            << showPreview << "_"
//            << recordPreview << "_"
//            << maxStepX << "_"
//           << maxStepY << "_"
//           << writeCropFile << "_"
//           << recordResult << "_"
//           << zoomStartThr << "_"
//           << face2shot << "_"
//           << zoomSpeedMin << "_"
//           << zoomSpeedMax << "_"
           << zoomSpeedInc_;
        }
        outFileTitle=outTitleStream.str();
        replace(outFileTitle.begin(),outFileTitle.end(),' ','_');
        replace(outFileTitle.begin(),outFileTitle.end(),':','-');

        if(isWebcam){
            fps = capture.get(CAP_PROP_FPS);
            fourcc = VideoWriter::fourcc('M','P','4','2'); // codecs
        }
        else{
            fourcc = capture.get(CV_CAP_PROP_FOURCC); // codecs
            fps = capture.get(CAP_PROP_FPS);
        }
        if(recordResult && !outputVideo.open("closeUp_"+outFileTitle+".avi",fourcc,
                                         fps, resultSize, true)){
            cerr << "Could not open the output video ("
                 << "closeUp_"+outFileTitle+".avi" <<") for writing"<<endl;
            recordResult=false;
//            return -1;
        }
        if(recordPreview){
             if(!previewVideo.open("../results/test_"+outFileTitle+".avi",fourcc,
                                   fps, previewSize, true))
             {
                 cerr << "Could not open the output video ("
                      << "../results/test_"+outFileTitle+".avi" <<") for writing"<<endl;
                 recordPreview=false;
//                 return -1;
             }
        }
        logFile.open(("../results/test_"+outFileTitle+".ods").c_str(), fstream::out); /// \todo 18.05.2016 class StatMan
        if(!logFile.is_open()){
            clog << "Error with opening the file:" << "../results/test_"+outFileTitle+".ods" << endl;
        }
        clog << "!!!";
        vector<Mat> frames;
        bool end=false;
        fstream cropFile;
        if(writeCropFile){ /// \todo 18.05.2016 class FileSaver
            cropFile.open("../results/filter",fstream::out);
        }

        if(streamToStdOut){
        for(int i=0;i<(resultWidth*resultHeight - (float)resultWidth/5.20)*3;i++){ /// \todo 05.12.2016 Это костыль, который сдвигает пиксели в кадре.
            cout<<0;
        }
        }
        unsigned int detWarning=0;
        for(;!end;){

            frames.clear();
            for(register int i=0; i<1;++i){
                capture >> fullFrame;

                char c;
                c=waitKey(1);
                if(!fullFrame.empty() && c!=27) {
                    frames.push_back(fullFrame.clone());
                    switch (c) {
                    case 'r':
                        bTrackerInitialized=false;
                        det.resetAim();
                        aim = det.getAim();
                        break;
                    default:
                        break;
                    }
                }
            }

            // Main cycle
            for(register int i=0; i<1;++i)
            {


                fullFrame = frames[i]; /// \todo 18.05.2016 class InputMan

                if(!fullFrame.empty() ) {
                    if(frameCounter>0)pzoom<<',';
                }
                else{
                    clog << "Frame is empty" << endl;
                    end=true;
                    break;
                }
                if(bTrackerInitialized && tracker != NULL && detWarning < detWarningLimit){
                    if(bb !=NULL) {delete bb; bb=NULL;}
                    bb = new BBox();
                    bb = tracker->track(fullFrame);
                    if(bb!=NULL){
                        aim = Rect(bb->x,
                                   bb->y,
                                   bb->width,
                                   bb->height);

                        if(frameCounter%aimCheckerPer == 0){
                            clog << __LINE__ << endl;

                            det.detect(fullFrame((focus&Rect(0,0,fullShot.width,fullShot.height))));
                            clog << __LINE__ << endl;

                            if(det.foundFaces()){
                                if(detWarning>0){detWarning--;// -1 к предупреждению;
                                    focus+=Point(focusEx,focusEx);
                                    focus+=Size(-focusEx,-focusEx);
                                }
                                /// \todo 30.01.2017 надо подправить размер
                            }else{
                                if(detWarning<255){detWarning++;// +1 предупреждение
                                    focus+=Point(-focusEx,-focusEx);
                                    focus+=Size(focusEx,focusEx);
                                }
                            }
                            /// конкретные действия, что делать если мы теряем лицо
                            if(detWarning >= detWarningLimit){
                                    bTrackerInitialized=false; // запускаем детекцию заново
                                    det.resetAim();
                                    focus=aim;
                            }

                        }
                        cam.update(aim);
                        focus=Rect(aim.x-(focus.width-aim.width)*0.5,aim.y-(focus.height-aim.height)*0.5,focus.width,focus.height);
                    }
                }else{
                    det.detect(fullFrame);
                    if(det.aimDetected()){
                        aim = det.getAim();
                        focus=aim;
                        if(tracker != NULL)delete tracker;
                        tracker = new ColorTracker();
                        tracker->init(fullFrame,
                                     aim.tl().x,
                                     aim.tl().y,
                                     aim.br().x,
                                     aim.br().y);
                        bTrackerInitialized = true;
                        detWarning=0;
                    }
                    cam.update(aim);
                }
                try{
                    /// \todo 18.05.2016 FileSaver
                    if(recordResult || streamToStdOut){
                        resize(fullFrame(cam.getRoi()), result , resultSize, 0,0, INTER_LINEAR );
                        if(recordResult){
                            outputVideo << result;
                        }
                        if(streamToStdOut){
                            Mat array = result.reshape(0,1);
                            string outstr((char *)array.data,array.total()*array.elemSize());
                            cout<<outstr;
                        }
                    }
                }catch(Exception &mvEx){
                    clog << "Result saving: "<< mvEx.msg << endl;
                }
                clog<< __LINE__ << endl;

  //    VIEW    //
                /// \todo 18.05.2016 class Drawer
                if(showPreview || recordPreview){ // Отрисовка области интереса
                    resize(fullFrame, preview, fullShot.size(), 0, 0, INTER_NEAREST ); ///< \todo 28.01.2017 !!! Отрисовка всех превью не должна зависеть от параметра scale, нужен только для детектора Viola Jones

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

                    // Сохранение кадра
                    if(recordPreview)
                        previewVideo << preview;
                    if(showPreview)
                        imshow(prevWindTitle.c_str(),preview);
                } /// \todo 18.05.2016 end class Drawer
                clog<< __LINE__ << endl;


// Сохранение статистики
                /// \todo 18.05.2016 class StatMan
                if(logFile.is_open()) {
                    if(frameCounter<1){
                        logFile << "frame\t";
                        logFile << "det.getFacesFull().x, px"<<"\t"
                                <<"det.getFacesFull().y, px"<<"\t"
                               << "det.getFacesFull().width, px"<<"\t"
                               <<"det.getFacesFull().height, px"<<"\t"
                              << "faceProf.x, px"<<"\t"
                              <<"faceProf.y, px"<<"\t"
                             << "det.getFacesProf().width, px"<<"\t"
                             <<"det.getFacesProf().height, px"<<"\t"
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
                    if(!det.getFacesFull().empty())
                        logFile << det.getFacesFull()[0].x << "\t"
                                                  << det.getFacesFull()[0].y << "\t"
                                                  << det.getFacesFull()[0].width << "\t"
                                                  << det.getFacesFull()[0].height << "\t";
                    else logFile << "\t\t\t\t";
                    if(!det.getFacesProf().empty())
                        logFile << det.getFacesProf()[0].x << "\t"
                                                  << det.getFacesProf()[0].y << "\t"
                                                  << det.getFacesProf()[0].width << "\t"
                                                  << det.getFacesProf()[0].height << "\t";
                    else logFile << "\t\t\t\t";

                    logFile << aim.x << "\t"
                            << aim.y << "\t"
                            << aim.width << "\t"
                            << aim.height << "\t";
                    logFile << cam.getRoi().x << "\t"
                            << cam.getRoi().y << "\t"
                            << cam.getRoi().width << "\t"
                            << cam.getRoi().height << endl;
                }
                if(writeCropFile && cropFile.is_open()){
                    cropFile << "zoompan=enable=eq(n\\,"<< frameCounter
                          << "):z=" << fullShot.width/roiFullSize.width
                          << ":x=" << roiFullSize.x
                          << ":y=" << roiFullSize.y << ":d=1";
                }

                clog <<"frame:"<< frameCounter
                        << " ("<< (int)((100*(float)frameCounter)/(float)videoLength) <<"% of video)"
                        << endl;
                ++frameCounter;
            }
        }
        if(bb!=NULL){
            delete bb;
            bb = NULL;
        }
        if(tracker!=NULL){
            delete tracker;
            clog << __LINE__ <<endl;
        }
        if(logFile.is_open())logFile.close(); /// \todo 18.05.2016 class StatMan
        if(writeCropFile){ /// \todo 18.05.2016 class FileSaver
//            fstream cropFile;
//            cropFile.open(("../results/filter_"/*outFileTitle).c_str()*/,fstream::out);
            if(cropFile.is_open()){
//                cropFile  << pzoom.str();
                cropFile.close();
            }else{
                clog << "Error with opening the file:" << "../results/test_"+outFileTitle+".crop" << endl;
            }
        }

        clog << "The results have been written to " << "''"+outFileTitle+"''" << endl;
        cvDestroyAllWindows();
    }
    return 0;
}
