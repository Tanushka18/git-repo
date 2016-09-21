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
	zmq_connect(request, "tcp://localhost:81");
	
	int count = 0;
	
	Json::Value myjson;
	Json::Value funjson;
	funjson["jsonrpc"] = "2.0";
	//funjson["id"] = 15;
	//myjson ["flag"] = 5;
	
	Json::Value answerJson; 		
	Json::Reader reader;
	Json::StyledWriter writer;
	int flag = 5;
	int s_id = 0;
/*
	std::cout<<"Enter flag:";
  	std::cin>>flag;
*/

	if(flag == 1)
		{
			std::cout<<"Enter ID:";
  			std::cin>>s_id;
			myjson["SessionID"] = s_id;	
		}



	for(;;) 
	{	
		std::cout<<"printf cout (0 - newSes; 1 - getQuest; 2 - answ; 3 - giveQuest):"<<std::endl;
		//std::cin>>count;
		scanf("%d",&count);
		myjson["number"] = count;
	

		if(count == 3)
		{
			myjson.clear();
			std::string str;
  			std::cout<<"Enter text:"<<std::endl;
  			std::getline(std::cin,str);
			myjson ["TextQuest"] =str.c_str();
			funjson["params"] = myjson;
			funjson["method"] = "question";
			str.clear();
		}

		
		if(count == 0)
		{
			funjson["method"] = "start_session";
		}

		if(count == 1)
		{
			myjson.clear();
			funjson["method"] = "next_quest";
			myjson["session_id"] = 15;
			funjson["params"] = myjson;
		}

		if(count == 2)  ///peredacha otveta na vopros
		{
			int param_answ;
			std::cout<<"printf quest_id, answer_id"<<std::endl;
			myjson.clear();
			funjson["method"] = "send_answer";
			scanf("%d",&param_answ);
			myjson["session_id"] = param_answ;
			scanf("%d",&param_answ);
			myjson["quest_id"] = param_answ;
			scanf("%d",&param_answ);
			myjson["answer_id"] = param_answ;
			funjson["params"] = myjson;
		/*
			std::cout<<"Enter yes(1)/no(0):";
  			std::cin>>s_id;
			myjson["Answer"] = s_id;
			funjson["param"] = myjson;	
                  */
		}
		
		zmq_msg_t req;
		
	//	std::string jsString = writer.write(myjson);
		std::string jsString = writer.write(funjson);
		zmq_msg_init_size(&req, jsString.size());		
		memcpy(zmq_msg_data(&req), jsString.c_str(), jsString.size());
		//memcpy(zmq_msg_data(&req), myjson,  10);
		printf("Sending!-number %d\n", count);
		zmq_msg_send(&req, request, 0);
		zmq_msg_close(&req);
		//zmq_send(request,myjson,100);
               // zmq_send_close(request);
		zmq_msg_t reply;
		zmq_msg_init(&reply);
		zmq_msg_recv(&reply, request, 0);

		char stroka[200];
		memset(stroka,0,200);
		memcpy(stroka, zmq_msg_data(&reply), zmq_msg_size(&reply));
		std::cout<<stroka<<std::endl;
		bool parsingSuccessful = reader.parse( stroka, answerJson);
/*
		std::cout<<answerJson["flag"].asString()<<std::endl;
		std::cout<<answerJson["text"].asString()<<std::endl;
		printf("Received!- %d\n", count);
*/		
		zmq_msg_close(&reply);
		//count++;
 	
	}
	// We never get here though.
	zmq_close(request);
	zmq_ctx_destroy(context);
	
	return 0;
}
