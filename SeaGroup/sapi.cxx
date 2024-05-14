/*=======================
SeaGroup 1.0 Source
For 清华大学经管学院学术部

Created by Aliebc 
(C) 2022
=======================*/

#include <stdio.h>
#include <string.h>
#include "SeaGroup.H"

extern json sag_list;

size_t curl_memory_write(char *buffer,size_t size,size_t nitems,void *userdata)
{
    size_t true_size=size*nitems;
    mem_buf *ud=(mem_buf *)userdata;
    memcpy(ud->buf+*ud->len,buffer,true_size);
    *ud->len+=true_size;
    printf("W:%lld\n",true_size);
    return true_size;
}

typedef struct 
{
    int(*callback)(int, long, long, const char *, void *);
    void * data;
} curl_xfer;


int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    curl_xfer*cp=(curl_xfer*)clientp;
    cp->callback(1,dlnow,dltotal,NULL,cp->data);
    return 0;
}


SeaGroup::SeaGroup(const char * token,const char * url,const char * gid)
{
    buffer=(char*)malloc(1024*1024*sizeof(char));
    bp=0;
    memset(this->buffer,0,1024*1024);
    snprintf(this->p_token,SG_TOKEN_SIZE,"Authorization: Token %s",token);
    snprintf(this->p_url,SG_URL_SIZE,"%s",url);
    snprintf(this->p_url2,SG_URL_SIZE,"%s",url);
    strncpy(this->p_gid,gid,SG_GID_SIZE);
    this->sg_api=curl_easy_init();
    curl_easy_setopt(this->sg_api,CURLOPT_URL,this->p_url2);
    header=curl_slist_append(NULL,this->p_token);
    header=curl_slist_append(header,"User-Agent: SeaGroup/1.0");
    curl_easy_setopt(this->sg_api,CURLOPT_HTTPHEADER,header);
    curl_easy_setopt(this->sg_api,CURLOPT_SSL_VERIFYPEER,0);
    curl_easy_setopt(this->sg_api,CURLOPT_SSL_VERIFYHOST,0);
    curl_easy_setopt(this->sg_api,CURLOPT_TIMEOUT,2L);
    curl_easy_setopt(this->sg_api,CURLOPT_PROXY,getenv("ALL_PROXY"));
    curl_easy_setopt(this->sg_api,CURLOPT_WRITEFUNCTION,curl_memory_write);
    buff={&bp,buffer};
    curl_easy_setopt(this->sg_api,CURLOPT_WRITEDATA,(void *)&this->buff);
}

int SeaGroup::change(const char * token,const char * url,const char * gid)
{
    snprintf(this->p_token,SG_TOKEN_SIZE,"Authorization: Token %s",token);
    bp=0;
    memset(this->buffer,0,1024*1024);
    snprintf(this->p_url,SG_URL_SIZE,"%s",url);
    snprintf(this->p_url2,SG_URL_SIZE,"%s",url);
    strncpy(this->p_gid,gid,SG_GID_SIZE);
    curl_easy_setopt(this->sg_api,CURLOPT_URL,this->p_url2);
    header=curl_slist_append(NULL,this->p_token);
    header=curl_slist_append(header,"User-Agent: SeaGroup/1.0");
    curl_easy_setopt(this->sg_api,CURLOPT_HTTPHEADER,header);
    return 0;
}

SeaGroup::~SeaGroup(void)
{
    curl_easy_cleanup(this->sg_api);
}

int SeaGroup::check(void)
{
    CURLcode st;
    memset(this->p_url2,0,SG_URL_SIZE);
    snprintf(this->p_url2,SG_URL_SIZE,"%s/api/v2.1/groups/%s/",this->p_url,this->p_gid);
    curl_easy_setopt(this->sg_api,CURLOPT_URL,this->p_url2);
    bp=0;
    memset(this->buffer,0,1024*1024);
    st=curl_easy_perform(this->sg_api);
    if(st==CURLE_OK){
        if(strcmp(buffer,"\"pong\"")==0){
            return 0;
        }else{
            return -1;
        }
    }else{
        return -2;
    }
    return -1;
}

int SeaGroup::check(char * _dest)
{
    CURLcode st;
    memset(this->p_url2,0,SG_URL_SIZE);
    snprintf(this->p_url2,SG_URL_SIZE,"%s/api/v2.1/groups/%s/",this->p_url,this->p_gid);
    curl_easy_setopt(this->sg_api,CURLOPT_URL,this->p_url2);
    bp=0;
    memset(this->buffer,0,1024*1024);
    st=curl_easy_perform(this->sg_api);
    if(st==CURLE_OK){
        long code;
        curl_easy_getinfo(sg_api,CURLINFO_RESPONSE_CODE,&code);
        if(code==200){
            printf("%s\n",buffer);
            json buf=json::parse(buffer);
            if(!buf["name"].is_string()){
                strcpy(_dest,"Unknown Error");
                return -1;
            }
            snprintf(_dest,64,"目标: %s",buf["name"].get<std::string>().c_str());
            return 0;
        }else{
            snprintf(_dest,64,"%ld(ERROR_CODE)",code);
            return -1;
        }
    }else{
        strncpy(_dest,curl_easy_strerror(st),32);
        return -2;
    }
    return -1;
}

void SeaGroup::exec(int flag,int(*callback)(int _issuccess, long _now, long _total, const char * _err, void * _d),void * data)
{
    switch (flag)
    {
    case SG_VIEW:
        {
            memset(this->p_url2,0,SG_URL_SIZE);
            snprintf(this->p_url2,SG_URL_SIZE,"%s/api/v2.1/groups/%s/members/?page=1&per_page=300&is_admin=false",this->p_url,this->p_gid);
            curl_easy_setopt(this->sg_api,CURLOPT_URL,this->p_url2);
            curl_xfer xpro={callback,data};
            curl_easy_setopt(this->sg_api,CURLOPT_NOPROGRESS,0);
            curl_easy_setopt(this->sg_api,CURLOPT_TIMEOUT,10L);
            curl_easy_setopt(this->sg_api,CURLOPT_XFERINFOFUNCTION,progress_callback);
            curl_easy_setopt(this->sg_api,CURLOPT_XFERINFODATA,&xpro);
            bp=0;
            memset(this->buffer,0,1024*1024);
            CURLcode st=curl_easy_perform(this->sg_api);
            if(st==CURLE_OK){
                long code;
                curl_easy_getinfo(sg_api,CURLINFO_RESPONSE_CODE,&code);
                if(code==200){
                    callback(0,0,0,buffer,data);
                }else{
                    callback(-2,0,0,buffer,data);
                }
            }else{
                callback(-1,0,0,curl_easy_strerror(st),data);
            }
        }
        break;
    
    case SG_ADD:
        {
            memset(this->p_url2,0,SG_URL_SIZE);
            snprintf(this->p_url2,SG_URL_SIZE,"%s/api/v2.1/groups/%s/members/",this->p_url,this->p_gid);
            curl_easy_setopt(this->sg_api,CURLOPT_URL,this->p_url2);
            int j=0;
            for(auto i=sag_list.begin();i!=sag_list.end();i++){
                char hhv[1024];
                snprintf(hhv,sizeof(hhv),"email=%s",i.value()["id"].get<string>().c_str());
                curl_easy_setopt(this->sg_api,CURLOPT_CUSTOMREQUEST,"POST");
                curl_easy_setopt(this->sg_api,CURLOPT_POSTFIELDS,hhv);
                curl_easy_setopt(this->sg_api,CURLOPT_POSTFIELDSIZE,strlen(hhv));
                curl_easy_setopt(this->sg_api,CURLOPT_POST,1);
                bp=0;
                memset(this->buffer,0,1024*1024);
                CURLcode st=curl_easy_perform(this->sg_api);
                if(st==CURLE_OK){
                    printf("%s\n",buffer);
                    long code;
                    curl_easy_getinfo(sg_api,CURLINFO_RESPONSE_CODE,&code);
                    try{
                        json buf=json::parse(buffer);
                        if(buf["error_msg"].is_string()){
                            callback(-2,j,sag_list.size(),buf["error_msg"].get<string>().c_str(),data);
                        }else if(buf["name"].is_string()){
                            callback(0,j,sag_list.size(),buf["name"].get<string>().c_str(),data);
                        }else{
                            callback(-2,j,sag_list.size(),buffer,data);
                        }
                    }catch(configor::configor_deserialization_error &e){
                        callback(-2,j,sag_list.size(),e.what(),data);
                    }
                }else{
                    callback(-1,j,sag_list.size(),curl_easy_strerror(st),data);
                }
                j++;
            }
        }
        break;

    case SG_DEL:
        {
            int j=0;
            for(auto i=sag_list.begin();i!=sag_list.end();i++){
                memset(this->p_url2,0,SG_URL_SIZE);
                snprintf(this->p_url2,SG_URL_SIZE,"%s/api/v2.1/groups/%s/members/%s/",this->p_url,this->p_gid,i.value()["id"].get<string>().c_str());
                curl_easy_setopt(this->sg_api,CURLOPT_URL,this->p_url2);
                curl_easy_setopt(this->sg_api,CURLOPT_CUSTOMREQUEST,"DELETE");
                bp=0;
                memset(this->buffer,0,1024*1024);
                CURLcode st=curl_easy_perform(this->sg_api);
                if(st==CURLE_OK){
                    printf("%s\n",buffer);
                    long code;
                    curl_easy_getinfo(sg_api,CURLINFO_RESPONSE_CODE,&code);
                    try{
                        json buf=json::parse(buffer);
                        if(buf["error_msg"].is_string()){
                            callback(-2,j,sag_list.size(),buf["error_msg"].get<string>().c_str(),data);
                        }else if(buf["success"].is_bool()){
                            callback(0,j,sag_list.size(),buf["success"].as_bool()?"True":"False",data);
                        }else{
                            callback(-2,j,sag_list.size(),buffer,data);
                        }
                    }catch(configor::configor_deserialization_error &e){
                        callback(-2,j,sag_list.size(),e.what(),data);
                    }
                }else{
                    callback(-1,j,sag_list.size(),curl_easy_strerror(st),data);
                }
                j++;
            }
        }
        break;
    default:
        int i=0;
        break;
    }
    curl_easy_setopt(this->sg_api,CURLOPT_CUSTOMREQUEST,"GET");
    //curl_easy_setopt(this->sg_api,CURLOPT_POSTFIELDS,NULL);
    curl_easy_setopt(this->sg_api,CURLOPT_NOPROGRESS,1);
    curl_easy_setopt(this->sg_api,CURLOPT_TIMEOUT,2L);
}