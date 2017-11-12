#include "rtspserver.h"
int RtspServer::cnt=0;
RtspServer::RtspServer(string _adr, string _codec, int _fps=25, int _frameWidth=640, int _frameHeight=480,int numOfchannels=3)
    : adr(_adr),codec(_codec),fps(_fps),frameWidth(_frameWidth),frameHeight(_frameHeight)
{
    buflen=frameHeight*frameWidth*numOfchannels;
    buf = new uint8_t[buflen];


    cnt++;
    fifoname = string("videofifo_")+to_string(cnt);
    if(!mkfifo(fifoname.c_str(),0666) ){
        clog<<"RtspServer start init" << endl;
//        this_thread::sleep_for(chrono::milliseconds(1000));
        fdpipe = open(fifoname.c_str(),O_WRONLY);
        clog<<"RtspServer fopen" << endl;
        if(fdpipe==NULL){
            cerr<<"Unable to open named pipe '" << fifoname << endl;
        }
    }else{
        cerr<<"Unable to make named pipe "<< fifoname <<":\n\t";
        switch (errno) {
        case EACCES:
            cerr<<"One of the directories in pathname did not allow search (execute) permission."<< endl;
            break;
              case EDQUOT:
              cerr<<"The user's quota of disk blocks or inodes on the file system has been exhausted."<< endl;
               break;
              case EEXIST:
              clog<<"pathname already exists. This includes the case where pathname is a symbolic link, dangling or not."<< endl;
              fdpipe = open(fifoname.c_str(),O_WRONLY);
              clog<<"RtspServer fopen" << endl;
              if(fdpipe==NULL){
                  cerr<<"buflenUnable to open named pipe '" << fifoname << endl;
              }
               break;
              case ENAMETOOLONG:
              cerr<<"Either the total length of pathname is greater than PATH_MAX, or an individual filename component has a length greater than NAME_MAX. In the GNU system, there is no imposed limit on overall filename length, but some file systems may place limits on the length of a component."<< endl;
               break;
              case ENOENT:
              cerr<<"A directory component in pathname does not exist or is a dangling symbolic link."<< endl;
               break;
              case ENOSPC:
              cerr<<"The directory or file system has no room for the new file."<< endl;
               break;
              case ENOTDIR:
              cerr<<"A component used as a directory in pathname is not, in fact, a directory."<< endl;
               break;
              case EROFS:
              cerr<<"pathname refers to a read-only file system."<< endl;
               break;
        default:
              cerr<<"unknown error"<< endl;
            break;
        }
         cerr << endl;
    }
}

RtspServer::~RtspServer()
{
    close(fdpipe);
//    delete fdpipe;
    if(!unlink(fifoname.c_str())) {
        cerr<<"Unable to unlink "<<fifoname<<"errno is "<<errno<<endl;
    }
    delete buf;
}

void RtspServer::sendFrame(Mat &frame) const
{

    if(!frame.empty()){
        int i=0;
        for(Pixel &p : cv::Mat_<Pixel>(frame)){
            buf[i]=p.x;i++;
            buf[i]=p.y;i++;
            buf[i]=p.z;i++;
        }
        static ssize_t written;
        written = write(fdpipe,buf,sizeof(uint8_t)*buflen);
        if (written<=0){
            cerr<<"\nerror with writing bytes:"<< written << " written. errno:"<<errno<<endl;
        }
    }
}
