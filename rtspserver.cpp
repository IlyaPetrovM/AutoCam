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
        string vlcCmd = "vlc "+fifoname+" -I dummy --repeat --demux=rawvideo --rawvid-fps="+
                to_string(fps)+
                " --rawvid-width="+to_string(frameWidth)+
                " --rawvid-height="+to_string(frameHeight)+
                " --rawvid-chroma=RV"+to_string(numOfchannels*8);

        string sout=" --sout '#transcode{vcodec="+codec
                +",vb="+to_string(2024)+",fps="+to_string(fps)
                +",scale=Auto,width="+to_string(frameWidth)
                +",height="+to_string(frameHeight)
                +",acodec=none}:rtp{sdp="+adr+"}'"
                +" --sout-x264-keyint=12 --sout-x264-min-keyint=2";

        int ret = system((vlcCmd+sout+" &").c_str());
        Log::print(DEBUG,to_string(ret));
        this_thread::sleep_for(milliseconds(1000));
        openPipe();
        work=true;
        sendFrameThread = new thread(&RtspServer::sendFrameInThread,this);
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
    delete sendFrameThread;
    Log::print(DEBUG,"join send Frame thread");
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

void RtspServer::sendFrame(Frame *frame)
{
    Log::print(DEBUG,string(__FUNCTION__));
    queMtx.lock();
    if(que.size()<=queMaxLen){
        if(frame->cameOnTime()){
            que.push(*frame);
            Log::print(DEBUG,string(__FUNCTION__)+" que size:\t"+to_string(que.size()));
        }else{
            frame->drop();
        }
    }else{
        frame->drop();
    }
    queMtx.unlock();
}

void RtspServer::sendFrameInThread()
{
    static ssize_t written;
    while(work){
        queMtx.lock();
        if(!que.empty()){
            int i=0;
            Log::print(DEBUG,string(__FUNCTION__));
            if(que.front().cameOnTime()){
                for(Pixel &p : cv::Mat_<Pixel>(que.front().getPixels())){
                    frameBuf[i]=p.x;i++;
                    frameBuf[i]=p.y;i++;
                    frameBuf[i]=p.z;i++;
                }
                if(que.front().cameOnTime()){
                    written = write(fdpipe,frameBuf,sizeof(unsigned char)*(buflen));
                    if (written<=0){
                        cerr<<"\nerror with writing bytes:"<< written << " written. errno:"<<errno<<endl;
                    }else{
                        Log::print(DEBUG,"frame written");
                    }
                }else{
                    que.front().drop();
                }
            }else{
                que.front().drop();
            }
            que.pop();
        }
        queMtx.unlock();
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
