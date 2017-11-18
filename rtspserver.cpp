#include "rtspserver.h"
int RtspServer::cnt=0;

RtspServer::RtspServer(int _frameWidth, int _frameHeight, string _adr, string _codec, int _fps=25, int numOfchannels=3, unsigned int _queLen)
    : Output(_frameWidth,_frameHeight), adr(_adr),codec(_codec),fps(_fps), queMaxLen(_queLen)
{
    buflen=frameHeight*frameWidth*numOfchannels;
    frameBuf = new unsigned char[buflen];

    firstFrameSent=false;
    cnt++;
    fifoname = string("/tmp/videofifo_")+to_string(cnt);


    int success = !mkfifo(fifoname.c_str(),0666);
    if(success || errno==EEXIST){
        clog<<"RtspServer start init" << endl;
        string vlcCmd = "vlc -I dummy --repeat "+fifoname
                +" --demux=rawvideo --rawvid-fps="+to_string(fps)
                +" --rawvid-width="+to_string(frameWidth)
                +" --rawvid-height="+to_string(frameHeight)
                +" --rawvid-chroma=RV"+to_string(numOfchannels*8)
                +" --sout '#transcode{vcodec="+codec
               +",vb="+to_string(1024)+",fps="+to_string(fps)
               +",scale=Auto,width="+to_string(frameWidth)
               +",height="+to_string(frameHeight)
               +",acodec=none}:rtp{sdp="+adr+"}'"+" 2>logVlc.txt &";
        clog << "RUN:\n\t" << vlcCmd << endl;
        clog << "return is " << system(vlcCmd.c_str()) << endl;
        this_thread::sleep_for(milliseconds(1000));
        openPipe();
        work=true;
        sendFrameThread = new thread(&RtspServer::sendFrameInThread,this);
        sendFrameThread->detach();
    }else{
        clog<<"Unable to make named pipe "<< fifoname <<":\n\t";
        switch (errno) {
        case EACCES:
            cerr<<"One of the directories in pathname did not allow search (execute) permission."<< endl;
            break;
              case EDQUOT:
              cerr<<"The user's quota of disk blocks or inodes on the file system has been exhausted."<< endl;
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
    }
}

RtspServer::~RtspServer()
{
    work=false;
    sendFrameThread->join();
    close(fdpipe);

    string killcmd="killall -s 9 vlc";
    int ret = system(killcmd.c_str());
    if(ret!=0){
        cerr<<"Unable to kill vlc process with command <"+killcmd << ">"<<clog;
    }
    delete frameBuf;

    if(unlink(fifoname.c_str())==-1)
        cerr<<"Unable to unlink "<<fifoname<<" errno is "<<errno<<endl;
    else
        clog << "\tRtspServer:" << fifoname << " deleted" << endl;
}

void RtspServer::sendFrame(const Mat &frame)
{
    if(que.size()<=queMaxLen){
        que.push(frame);
        Log::print(INFO,string(__FUNCTION__)+" que size:\t"+to_string(que.size()));
    }else{
        Log::print(WARN,string(__FUNCTION__)+" drop frame due to queue length");
    }
}

void RtspServer::sendFrameInThread()
{
    while(work){
        Log::print(INFO,string(__FUNCTION__)+" 1");
        if(!que.empty()){
            int i=0;
            for(Pixel &p : cv::Mat_<Pixel>(que.front())){
                frameBuf[i]=p.x;i++;
                frameBuf[i]=p.y;i++;
                frameBuf[i]=p.z;i++;
            }
            static ssize_t written;
            written = write(fdpipe,frameBuf,sizeof(unsigned char)*(buflen));
            if (written<=0){
                cerr<<"\nerror with writing bytes:"<< written << " written. errno:"<<errno<<endl;
            }
            que.pop();
            Log::print(INFO,string(__FUNCTION__)+" que size:\t"+to_string(que.size()));
        }
        Log::print(INFO,string(__FUNCTION__)+" 2");
    }
}


void RtspServer::openPipe()
{
    fdpipe = open(fifoname.c_str(),O_WRONLY);
    if(fdpipe==NULL){
        cerr<<"Unable to open named pipe '" << fifoname << endl;
    }else{
        clog<<fifoname << " opened" << endl;
    }
}
