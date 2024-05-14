/*=======================
SeaGroup 1.0 Source
For 清华大学经管学院学术部

Created by Aliebc 
(C) 2022
=======================*/

//FUNC PART
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//FUNC PART END
//FLTK PART
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Cairo.H>
#include <FL/platform.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Browser.H>
//FLTK PART END
#include "SeaGroup.H"
typedef struct
{
    Fl_Secret_Input * token;
    Fl_Input * gid;
    Fl_Input * url;
    Fl_Box * output;
    SeaGroup * sag_m;
} TUG_Group;

typedef struct
{
    TUG_Group * TUG1;
    Fl_Choice * cho;
    Fl_Browser * output;
    Fl_Text_Buffer * output_txt;
} TUGS_Group;

typedef struct
{
    Fl_Browser * display;
    Fl_Text_Buffer * buffer;
} Text_Group;

typedef struct
{
    bool is_added;
    char * err;
} SG_Status;


extern size_t curl_memory_write(char *buffer,size_t size,size_t nitems,void *userdata);

extern configor::json xj;

SeaGroup * sag_main=NULL;
json sag_list;

void test_config(Fl_Widget * o, void *p)
{
    char info[128]="设置失败,原因:";
    strcpy(info,"设置失败,原因:");
    TUG_Group * tug=(TUG_Group *)p;
    if(sag_main==NULL){
        sag_main=new SeaGroup(tug->token->value(),tug->url->value(),tug->gid->value());
    }else{
        sag_main->change(tug->token->value(),tug->url->value(),tug->gid->value());
    }
    char *_i2=info+strlen(info);
    int x=sag_main->check(_i2);
    if(x==0){
        tug->output->copy_label(_i2);
    }else{
        tug->output->copy_label(info);
    }
    tug->output->redraw();
}

void sg_config(Fl_Widget * o,void *p)
{
    Fl_Button * mb=(Fl_Button *)o;
    TUG_Group * tug=(TUG_Group *)p;
    Fl_Native_File_Chooser fc;
    fc.title("读取配置文件");
    fc.filter("JSON配置文件\t*.json");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fc.directory(".");

    switch (fc.show())
    {
    case -1:
        break;
    case 1:
        break;
    default:
        FILE * fp=fopen(fc.filename(),"r");
        char hvalue[1024];
        if(fp==NULL){
            fl_alert("文件不存在!");
            return;
        }
        fread(hvalue,1024,1,fp);
        try{
            json xx=json::parse(hvalue);
            tug->url->value(xx["url"].get<string>().c_str());
            tug->gid->value(xx["gid"].get<string>().c_str());
            tug->token->value(xx["token"].get<string>().c_str());
            tug->url->redraw();
            tug->gid->redraw();
            tug->token->redraw();
        }catch(configor::configor_deserialization_error &e){
            fl_alert("%s",e.what());
            return;
        }catch(configor::configor_type_error &e){
            fl_alert("%s",e.what());
            return;
        }
        
    }

}

void sg_data(Fl_Widget * o,void *p)
{
    Fl_Button * mb=(Fl_Button *)o;
    TUGS_Group * tugs=(TUGS_Group *)p;
    Fl_Native_File_Chooser fc;
    fc.title("读取数据");
    fc.filter("文本文件\t*.txt");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fc.directory(".");

    switch (fc.show())
    {
    case -1:
        break;
    case 1:
        break;
    default:
        FILE * fp=fopen(fc.filename(),"r");
        char hvalue[1024];
        if(fp==NULL){
            fl_alert("文件不存在!");
            return;
        }
        sag_list.clear();
        int j=0;
        while(fgets(hvalue,sizeof(hvalue),fp)!=NULL){
            sag_list[j]={{"status",false},{"err_msg","等待执行"},{"id",string(hvalue,strlen(hvalue)-1)}};
            j++;
        }
        fclose(fp);
        fl_alert("已读取数据!");
    }
    tugs->output->clear();
    tugs->output->redraw();
    Fl::check();
    for(auto i=sag_list.begin();i!=sag_list.end();i++){
        char hhv[1024];
        snprintf(hhv,sizeof(hhv),"%s\t%s\t%s",
        i.value()["id"].get<string>().c_str(),
        i.value()["status"].get<bool>()?"True":"False",
        i.value()["err_msg"].get<string>().c_str()
        );
        tugs->output->add(hhv);
    }
    tugs->output->redraw();
    //puts(sag_list[0]["id"].get<string>().c_str());
    fflush(stdout);
}

void sg_savecfg(Fl_Widget * o,void *p)
{
    Fl_Native_File_Chooser fc;
    TUG_Group * tug=(TUG_Group *)p;
    fc.title("保存配置文件");
    fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fc.preset_file("seagroup.json");
    fc.filter("JSON配置文件\t*.json");
    fc.directory(".");
    switch (fc.show())
    {
    case -1:
        break;
    case 1:
        break;
    default:
        FILE * fp=fopen(fc.filename(),"w");
        if(fp!=NULL){
            json fpj;
            fpj["token"]=tug->token->value();
            fpj["gid"]=tug->gid->value();
            fpj["url"]=tug->url->value();
            fputs(fpj.dump(4,' ').c_str(),fp);
            fclose(fp);
        }else{
            fl_alert("无法打开文件!");
        }
    }
}

void sg_savef(Fl_Widget * o,void * p){
    Fl_Native_File_Chooser fc;
    TUGS_Group * tugs=(TUGS_Group *)p;
    fc.title("保存结果");
    fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fc.preset_file("SeaGroup_output.txt");
    fc.filter("文本文件\t*.txt");
    fc.directory(".");
    switch (fc.show())
    {
    case -1:
        break;
    case 1:
        break;
    default:
        FILE * fp=fopen(fc.filename(),"w");
        if(fp!=NULL){
            for(int j=1;j<=tugs->output->size();j++){
                fprintf(fp,"%s\n",tugs->output->text(j));
            }
            fclose(fp);
        }else{
            fl_alert("无法打开文件!");
        }
    }
}

void rz(Fl_Widget * o, void * p)
{
    Fl_Window * wo=(Fl_Window *)o;
    Fl_Menu_Bar * wp=(Fl_Menu_Bar *)p;
}

void sg_version(Fl_Widget * o,void * p){
    Fl_Window *sg_m=(Fl_Window *)p;
    Fl_Window sg_v(120,120,300,200,"关于SeaGroup");
    sg_v.begin();
    Fl_Help_View hv(10,10,280,180);
    hv.scrollbar_size(-1);
    hv.value("<center><h1>SeaGroup - v1.0<br>For " __X_SYSTEM__ "</h1><hr><p>批量管理Seafile网盘群组成员的工具</p><p>By AliebcX</p><center>");
    sg_v.end();
    sg_v.show();
    Fl::run();
}

int sg_showm(int _issuccess, long _now, long _total, const char * _err, void * _d)
{
    Text_Group * output=(Text_Group *)_d;
    if(_issuccess<=0){
        puts(_err);
        output->display->clear();
        if(_issuccess<0){
            output->display->add("ERROR");
            output->display->redraw();
            return 0;
        }
        try{
            json mb=json::parse(_err);
            char hvalue[1024];
            if(mb.is_array()){
                int j=0;
                for(auto i=mb.begin();i!=mb.end();i++){
                    sprintf(hvalue,"%d\t%s(%s)",++j,
                    i.value()["name"].get<string>().c_str(),
                    i.value()["contact_email"].get<string>().c_str()
                    );
                    output->display->add(hvalue);
                    memset(hvalue,0,sizeof(hvalue));
                }
            }
        }catch(configor::configor_deserialization_error &e){
            output->display->add(e.what());
        }catch(configor::configor_type_error &e){
            output->display->add(e.what());
        }
        output->display->redraw();
        return 0;
    }
    char str[1024];
    snprintf(str,1024,"[Progress]%03f%%(Total%02fKB)",(float)_now/_total*100,_total/1024.0f);
    output->display->copy_label(str);
    output->display->redraw();
    Fl::check();
    return 0;
}

int sg_addm(int _issuccess, long _now, long _total, const char * _err, void * _d)
{
    Text_Group * output=(Text_Group *)_d;
    if(_issuccess<0){
        printf("EDone %ld\n",_now);
        sag_list[_now]["status"].get<bool&>()=false;
        sag_list[_now]["err_msg"].get<string&>()=string(_err);
    }else if(_issuccess==0){
        printf("Done %ld\n",_now);
        sag_list[_now]["status"].get<bool&>()=true;
        sag_list[_now]["err_msg"].get<string&>()=string(_err);
    }else if(_issuccess>0){
        
    }
    char hhv[1024];
    snprintf(hhv,sizeof(hhv),"%s\t%s\t%s",
    sag_list[_now]["id"].get<string>().c_str(),
    sag_list[_now]["status"].get<bool>()?"True":"False",
    sag_list[_now]["err_msg"].get<string>().c_str()
    );
    fflush(stdout);
    char str[1024];
    snprintf(str,1024,"[Progress]%03f%%(第%ld个/共%ld个)",(float)(_now+1)/_total*100,_now+1,_total);
    output->display->copy_label(str);
    output->display->text(_now+1,hhv);
    output->display->redraw();
    Fl::check();
    return 0;
}

void sg_run(Fl_Widget * o,void * p){
    TUGS_Group * tugs=(TUGS_Group *)p;
    printf("Choice:%d\n",tugs->cho->value());
    int (*funcs[])(int, long, long, const char *, void *)={sg_showm,sg_addm,sg_addm};
    if(sag_main==NULL){
        tugs->output->label("请先进行测试!");
    }else{
        if(tugs->cho->value()==-1){
        tugs->output->label("请选择一个操作!");
        }else{
            tugs->output->label(" ");
            tugs->output->redraw();
            Fl::check();
            Text_Group optxt={tugs->output,tugs->output_txt};
            sag_main->exec(tugs->cho->value(),funcs[tugs->cho->value()],&optxt);
        }
    }
    tugs->output->redraw();
}

void sg_exit(Fl_Widget * o,void * p){
    exit(0);
}


extern Fl_Window sg_main;
configor::json xj={{"x",1},{"y",2},{"z","你好"}};

int main()
{
    Fl_Window sg_main(100,100,600,400,"SeaGroup - v1.0");
    #ifdef __MACH__
    Fl_Mac_App_Menu::about = "关于 SeaGroup";
    Fl_Mac_App_Menu::print = "";
    Fl_Mac_App_Menu::services = "服务";
    Fl_Mac_App_Menu::hide = "隐藏";
    Fl_Mac_App_Menu::hide_others = "隐藏其他";
    Fl_Mac_App_Menu::show = "全部显示";
    Fl_Mac_App_Menu::quit = "退出 SeaGroup";
    fl_mac_set_about((Fl_Callback *)sg_version,&sg_main);
    #endif
    TUG_Group TUG1;
    TUGS_Group TUG2;
    sg_main.color(FL_WHITE);
    sg_main.begin();
    Fl_Sys_Menu_Bar sg_menu(0,0,600,20,nullptr);
    sg_menu.box(FL_FLAT_BOX);
    sg_menu.add("文件/保存配置\t", FL_COMMAND + 's', (Fl_Callback*)sg_savecfg,&TUG1);
    sg_menu.add("文件/读取配置\t", FL_COMMAND + 'o', (Fl_Callback*)sg_config,&TUG1);
    sg_menu.add("文件/导出结果\t", FL_COMMAND + 'p', (Fl_Callback*)sg_savef,&TUG2);
    sg_menu.add("操作/确定配置\t", FL_COMMAND + 't', (Fl_Callback*)test_config,&TUG1);
    sg_menu.add("操作/运行\t", FL_COMMAND + 'r', (Fl_Callback*)sg_run,&TUG2);
    sg_menu.add("操作/退出\t", FL_COMMAND + 'q', (Fl_Callback*)sg_exit,&TUG2);
    #ifndef __APPLE__
    sg_menu.add("关于", FL_COMMAND + 'a', sg_version, &sg_main);
    #endif
    Fl_Box sg_confb(10,30,580,90);
    Fl_Box sg_confb_c(10,120,580,30);
    sg_confb.box(FL_FLAT_BOX);
    sg_confb.color(FL_DARK_GREEN);
    sg_confb_c.box(FL_FLAT_BOX);
    Fl_Secret_Input sg_token(70,40,300,25,"Token:");
    Fl_Input sg_gid(450,40,100,25,"Group ID:");
    Fl_Input sg_url(70,80,350,25,"URL:");
    Fl_Choice sg_action(70,190,100,25,"操作:");
    sg_action.add("查看成员");
    sg_action.add("添加成员");
    sg_action.add("删除成员");
    Fl_Button sg_actionb(450,190,100,25,"执行!");
    Fl_Button sg_chkconfig(450,80,100,25,"确定配置");
    Fl_Box sg_text1(400,40,170,30,NULL);
    Fl_Browser sg_tab(10,230,580,150);
    Fl_Text_Buffer sg_tab_txt;
    sg_tab_txt.text("这里是内容");
    sg_tab.add("内容区");
    sg_main.end();
    TUG1={&sg_token,&sg_gid,&sg_url,&sg_confb_c,sag_main};
    sg_chkconfig.callback(test_config,&TUG1);
    TUG2={&TUG1,&sg_action,&sg_tab,&sg_tab_txt};
    sg_actionb.callback(sg_run,&TUG2);
    sg_menu.add("文件/选择数据\t", FL_COMMAND + 'd', (Fl_Callback*)sg_data,&TUG2);
    sg_main.show();
    return Fl::run();
}
