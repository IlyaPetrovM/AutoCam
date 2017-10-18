#include "director.h"

bool Director::findDubs()
{
    faceDub.clear();
    faceDub.resize(operators.size());

    for(int j=0;j<facesNew.size();j++){
        for(int i=0; i<operators.size();i++){
            if(rules.near(operators[i]->getFace(),facesNew[j])){
                faceDub[i] = facesNew[j];
            }
        }
    }
}

void Director::manageOperators()
{
    while(!stopWork){
        scene.update();
        for(int i=0;i<operators.size();i++){
            operators[i]->work();
        }
        sleep_for(microseconds(1000));
    }
}


Director::Director(Scene s)
    : scene(s),
      stopWork(false),
      rules(5)
{

}

Director::~Director()
{
    for(size_t i=0;i<operators.size();i++){
        delete operators[i];
    }
}


void Director::work()
{
    thread cons(&Director::manageOperators,this);
    string cmd;
    while(!stopWork){
        cin>>cmd;
        if(cmd=="add"){
            operators.push_back(new Operator(&scene,new Camera(&scene,"rtsp://localhost:8056"), new Face()));
        }else if(cmd=="del"){
            cout << "Which one operator you want to fire? "<< endl;
            for (int i=0;i<operators.size();i++){cout << "\tOp #" << operators[i]->getId() << endl;}

            int ch=-1;cin >> ch;
            try {
                for (int i=0;i<operators.size();i++) {
                    if(operators[i]->getId()==ch){ delete operators[i]; operators.erase(operators.begin()+i);
                        break;}
                }
            } catch(exception e) {cerr<<e.what();}
        }else if(cmd=="exit"){
            stopWork=true;
        }else if(cmd=="op"){
            //find operator
            int _id;
            cin>>_id;
            for (int i=0;i<operators.size();i++) {
                if(operators[i]->getId()==_id){
                    char _cmd;
                    cin>>_cmd;
                    operators[i]->sendCommand(_cmd);
                    break;
                }
            }
        }
    }
}
