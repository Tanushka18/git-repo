#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include "zmq.h"
#include "json/json.h"

using namespace std;
using namespace Json;

int main (int argc, char const *argv[]) 
{
	void* context = zmq_ctx_new();
	printf("Client Starting….\n");
	
	void* request = zmq_socket(context, ZMQ_REQ);
	//zmq_connect(request, "tcp://gpo2.tomsksoft.com:49200");
	zmq_connect(request, "tcp://localhost:49200");
	
	int count = 0;
	Json::Value myjson;
	Json::Value funjson;
	funjson["jsonrpc"] = "2.0";
	
	Json::Value answerJson; 		
	Json::Reader reader;
	Json::StyledWriter writer;
	int flag = 0;
	int s_id = 0;
	int quest_id = 0;
	count = 0;
	for(;;) 
	{	
		if(count==32001)
		{
			myjson.clear();
			funjson["method"] = "get_session";
			myjson["session_id"] = s_id;
			funjson["params"] = myjson;	
		}

		if(count == 3)
		{
			myjson.clear();
			std::string str;
  			std::cout<<"Enter text:"<<std::endl;
  			std::getline(std::cin,str);
			myjson ["question"] =str.c_str();
			funjson["params"] = myjson;
			funjson["method"] = "question";
			str.clear();
		}

		if(count == 2)  ///peredacha otveta na vopros
		{
			int param_answ;
			myjson.clear();
			funjson["method"] = "send_answer";
			myjson["session_id"] = s_id;
			myjson["quest_id"] = quest_id;
		//	std::cout<<"printf answer_id:"<<std::endl;
			scanf("%d",&param_answ);
			myjson["answer_id"] = param_answ;
			funjson["params"] = myjson;

			count = 5;
	
		}


		if(count == 1)
		{
			myjson.clear();
			funjson["method"] = "next_quest";
			myjson["session_id"] = s_id;
			funjson["params"] = myjson;
			count = 2;
		}
		
		if(count == 0)
		{
			funjson["method"] = "start_session";
			count = 1;
			std::cout<<"~~~~~Start session~~~~~~"<<std::endl;
		}

		if(count==5) count = 1;
		
		zmq_msg_t req;
		
		std::string jsString = writer.write(funjson);
		zmq_msg_init_size(&req, jsString.size());		
		memcpy(zmq_msg_data(&req), jsString.c_str(), jsString.size());
		//printf("Sending!-number %d\n", count);
		zmq_msg_send(&req, request, 0);
		zmq_msg_close(&req);
		zmq_msg_t reply;
		zmq_msg_init(&reply);
		zmq_msg_recv(&reply, request, 0);

		myjson.clear();
		char stroka[1000];
		memset(stroka,0,1000);
		memcpy(stroka, zmq_msg_data(&reply), zmq_msg_size(&reply));
	
		bool parsingSuccessful = reader.parse( stroka, answerJson);
		if(answerJson["result"].size()!=0){
			jsString = writer.write(answerJson["result"]);
			parsingSuccessful = reader.parse( jsString, myjson);
			if(count==32001)
			{
				std::cout<<"tyt eshe rabotaet..."<<std::endl;
				std::cout<<"All session: "<<answerJson["result"]<<std::endl;
				std::cout<<"~~~~~End session~~~~~~"<<std::endl;
				break;
			}
			s_id = myjson["session_id"].asInt();
			if(myjson["session_state"]== "find") 
			{
				std::cout<<"id decision: "<<myjson["decision_id"].asString()<<std::endl;
				std::cout<<"decision: "<<myjson["decision_text"].asString()<<std::endl;
				flag = 1;
			}

			else
			{
				 quest_id = myjson["quest_id"].asInt();	
				if(count==2) std::cout<<s_id<<"] "<<"Quest| "<<quest_id<<":"<<myjson["quest_text"].asString()<<std::endl; 

			}
		}
		if(answerJson["error"].size()!=0){
			count = 32001;
		}
		myjson.clear();
		answerJson.clear();
		jsString.clear();
	
		zmq_msg_close(&reply);
		if(flag == 1) 
		{
			std::cout<<"~~~~~End session~~~~~~"<<std::endl;
			break;
		}
 	
	}
	// We never get here though.
	zmq_close(request);
	zmq_ctx_destroy(context);
	
	return 0;
}
