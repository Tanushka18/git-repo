#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include "zmq.h"
#include <mysql/mysql.h>
#include "json/json.h"
#include <vector>
#include <sstream>
#include "log_fun.h"


using namespace std;
using namespace Json;

MYSQL mysql, *connection;

class SessionGame
{
	public:
	int session_id;
	int session_num;
	int local_decision;
	
	Json::Value session_event;
	Json::Value session_aprior;
//~~~~~
	SessionGame(int s_i)
	{
		std::string data_string;
		Json::Reader reader;	
		session_id = s_i;	
		memset(sql_z,0,600);
		strcat(sql_z,"SELECT * FROM PresentSession WHERE session_id=");
		sprintf(peremenya,"%d",s_i);
		strcat(sql_z,peremenya);
		//catch error!!
		
		mysql_query(connection, sql_z);
		
		result_s = mysql_store_result(connection);
		row_s = mysql_fetch_row(result_s);
		session_num = atoi(row_s[1]);
		memset(sql_z,0,600);
		data_string = row_s[2];
		strcat(sql_z,data_string.c_str());
		bool parsingSuccessful = reader.parse( sql_z, session_event);
		memset(sql_z,0,600);
		strcat(sql_z,row_s[3]);
		parsingSuccessful = reader.parse( sql_z, session_aprior);

		mysql_free_result(result_s);
		
		
	}
//~~~~~
	
	std::string decision_not_found(void)
	{
		Json::Value Inform, Injson;
		Json::FastWriter writer;
		
		
		
		std::vector<std::string> iiiii = session_event.getMemberNames();
		
		for(int i=0; i<iiiii.size(); i++)		
		{
			memset(sql_z,0,600);
			strcat(sql_z,"SELECT * FROM Quest where quest_id='");
			strcat(sql_z,iiiii[i].c_str());
			strcat(sql_z,"'");
			mysql_query(connection, sql_z);
			result_s = mysql_store_result(connection);
			row_s = mysql_fetch_row(result_s);
			Injson["Questions"] = row_s[1];
			Injson["Answers"] = session_event[iiiii[i]];
			Inform[i] = Injson;
			mysql_free_result(result_s);
		}

		Injson.clear();

		std::string hello = writer.write(Inform);
		std::cout<<hello<<std::endl;
		return hello;
	}


	int select_quest()
	{
		Json::Value PeremJson;
		Json::Reader reader;
		float maxVer = 0, peremVer=0;
		std::string deletepotom;

		std::cout<<"Razmer Session_event"<<session_event.size()<<std::endl;
		std::cout<<"Session_event"<<session_event<<std::endl;
		for(int i=1; i<=session_aprior.size(); i++)
		{
			sprintf(peremenya,"%d",i);
			deletepotom = session_aprior[peremenya].asString();
			peremVer = atof(deletepotom.c_str());

			if(maxVer < peremVer)
			{
				maxVer = peremVer;
				local_decision = i;
			}
		}
		
		std::cout<<"local_decision = "<<local_decision<<std::endl;

		
		std::cout<<"ses"<<session_num<<std::endl;
		memset(sql_z,0,600);
		strcat(sql_z,"SELECT * FROM Decision WHERE decision_id=");
		sprintf(peremenya,"%d",local_decision);
		strcat(sql_z,peremenya);
		mysql_query(connection, sql_z);
		result_s = mysql_store_result(connection);
		row_s = mysql_fetch_row(result_s);
		memset(sql_z,0,600);
		std::string data_string = row_s[3];
		strcat(sql_z,data_string.c_str());
		bool parsingSuccessful = reader.parse( sql_z, PeremJson);
		int size_json = PeremJson.size(); ///<---AAAAAAAA!
 
		for(int i=1; i<=size_json; i++) //<- best Quest
		{
			sprintf(peremenya,"%d",i);
			if((PeremJson[peremenya].asInt()==1)&&(session_event[peremenya].empty())) return i;
		}

		for(int i=1; i<=size_json; i++) //<- if not find best quest
		{
			sprintf(peremenya,"%d",i);
			if((PeremJson[peremenya].asInt()==-1)&&(session_event[peremenya].empty())) return i;
		}

		for(int i=1; i<=size_json; i++) //<- if no found :(
		{
			sprintf(peremenya,"%d",i);
			if(session_event[peremenya].empty()) return i;
		}

		return 1; //<- 1-i Quest
	}
//////////////////////////////////////////////////////////////
	void set_event(int qu_id, int an_id)
	{	
		sprintf(peremenya,"%d",qu_id);
		session_event[peremenya] = an_id;
	}


	void data_to_base()
	{
		Json::FastWriter writer;
		std::string stroka_json;
		stroka_json = writer.write(session_event);
		session_num = session_num+1;	//(in bayes)
		memset(sql_z,0,600);
		sprintf(peremenya,"%d",session_num);
           	strcat(sql_z,"update PresentSession set session_num ='");
		strcat(sql_z,peremenya);
		strcat(sql_z,"',session_event='");
		strcat(sql_z,stroka_json.c_str());
		stroka_json.clear();
		stroka_json = writer.write(session_aprior);
		strcat(sql_z,"',session_aprior='");
		strcat(sql_z,stroka_json.c_str());
		strcat(sql_z,"' where session_id =");
		sprintf(peremenya,"%d",session_id);
		strcat(sql_z,peremenya);
		mysql_query(connection, sql_z);
		//catch error!!

	}
	



	int FunctionBayes(int q_id, int a_id)
	{
		int id_decision = 0;
		float ver_decision = 0;

		Json::Value ApostrVer;
		Json::Value PeremJson;
		Json::Reader reader;
		Json::FastWriter writer;
		int maxDecis;
		float PB = 0;

		memset(sql_z,0,600);	
		mysql_query(connection,"select decision_id,decision_json from Decision");
		std::string deletepotom;
		result_s = mysql_store_result(connection);
		
		int flag;

		 while ((row_s = mysql_fetch_row(result_s))!=NULL) 
 		{
			
			reader.parse( row_s[1], PeremJson);
			sprintf(peremenya,"%d",q_id);
			if (PeremJson[peremenya].empty()) flag = 0;
			else   flag = PeremJson[peremenya].asInt();
			
			if (flag == a_id) ApostrVer[row_s[0]] = 0.8;
			if (flag != a_id) ApostrVer[row_s[0]] = 0.2;
			if (flag==0) ApostrVer[row_s[0]] = 0.5;

   			maxDecis = atoi(row_s[0]);
			memset(peremenya,0,12);
		 }	
		mysql_free_result(result_s);

		float apr=0, apost=0;
		for(int i=1; i<=maxDecis; i++)
		{
			
			sprintf(peremenya,"%d",i);
			deletepotom = session_aprior[peremenya].asString();
			apr = atof(deletepotom.c_str());
	
			deletepotom = ApostrVer[peremenya].asString();
			apost = atof(deletepotom.c_str());

			PB = PB+(apr * apost);
		}	

		float endNumber;



		for(int i=1; i<=maxDecis; i++)
		{
			deletepotom.clear();
			sprintf(peremenya,"%d",i);
			deletepotom = session_aprior[peremenya].asString();
			apr = atof(deletepotom.c_str());
			deletepotom = ApostrVer[peremenya].asString();
			apost = atof(deletepotom.c_str());
			deletepotom.clear();
			endNumber = (apr * apost)/PB;
			session_aprior[peremenya] = endNumber;

			if(ver_decision<endNumber)
			{
				local_decision = i; //for quest select function
				 ver_decision = endNumber;
				id_decision  = i;	
			}
		}	

		std::cout<<"Aprior:"<<session_aprior<<std::endl;
		if (ver_decision>0.7) return id_decision; //id_decision != 1 (in BD)
		return 0;
	//	mysql_free_result(result_s);
	}
//================================================================================
	private:
	char sql_z[600];
	MYSQL_RES *result_s;	
	MYSQL_ROW row_s; 
	char peremenya[12];	
//	answer_handler

};



const std::string JsonToString(const std::string &s)
{
	MYSQL_RES *result;	
	MYSQL_ROW row; 	

	SessionGame *session;	

	Json::Value data_session;
	char peremen[100]; //prosto peremennya
	char sql_zapros[1000]; 
	Json::Value funjson;
	Json::Value myjson; 
	Json::Value ResponseJson;
	Json::Reader reader;
	bool mysqlError;
	std::string textQuest;
	std::string function;
	int s_id = 0;
	Json::FastWriter writer;
	bool parsingSuccessful = reader.parse( s, funjson);

	if( parsingSuccessful==true)
	{

		if(funjson["params"]!=0)  function = writer.write(funjson["params"]);
	
//=====================================================================	
		if(funjson["method"]=="start_session")
		{
			//myjson.clear();
			mysqlError = mysql_query(connection, "SELECT MAX(session_id) AS session_id FROM PresentSession");
		        if(mysqlError==false)
			{
				result = mysql_store_result(connection);
				row = mysql_fetch_row(result);
				memset(sql_zapros,0,1000); //??
           			strcat(sql_zapros,"INSERT INTO PresentSession (session_num,session_event,session_aprior) VALUES ('1','{}','{}')");
				mysqlError = mysql_query(connection, sql_zapros);
				s_id = atoi(row[0]);
				myjson["session_id"] = s_id+1;//row[0];
			}
			else {myjson["session_id"] =1;
			      s_id = 1;
			      }
			ResponseJson["result"] = myjson;
			
			
			mysqlError = mysql_query(connection,"Select decision_id,decision_aprior from Decision ");
			result = mysql_store_result(connection);
 			myjson.clear();
			 while ((row = mysql_fetch_row(result))!=NULL) 
 			{
				myjson[row[0]] = row[1];
   				//std::cout<<row[0]<<" - "<<row[1]<<std::endl;
			 }	

			char peremenya[10];
			memset(peremenya,0,10); 
			sprintf(peremenya,"%d",s_id+1);
			memset(sql_zapros,0,1000); 
			strcat(sql_zapros, "update PresentSession set session_aprior ='");
			textQuest = writer.write(myjson);
			strcat(sql_zapros,textQuest.c_str());
			strcat(sql_zapros,"' where session_id='");
			strcat(sql_zapros,peremenya);
			strcat(sql_zapros,"'");
			mysqlError = mysql_query(connection, sql_zapros);
			mysql_free_result(result);
		}	
		//...........

//=====================================================================
		if(funjson["method"]=="next_quest")
		{
			int number_quest = 1; //<----INDEX QUEST

			s_id = funjson["params"].get("session_id", 3).asInt();
			session = new SessionGame(s_id);
			number_quest = session->select_quest();
			memset(sql_zapros,0,1000);
           		strcat(sql_zapros,"SELECT * FROM Quest WHERE quest_id="); 
			sprintf(peremen,"%d",number_quest);
			strcat(sql_zapros,peremen);
			mysqlError = mysql_query(connection, sql_zapros);
			if (mysqlError==false)
			{	
				myjson.clear();
				result = mysql_store_result(connection);
				row = mysql_fetch_row(result);
				mysql_free_result(result);
				myjson["session_id"] = s_id;
				myjson["quest_id"] = number_quest;
				myjson["quest_text"] = row[1];
				ResponseJson["result"] = myjson;
				
			}
			else 
			{
				myjson.clear();
				myjson["code"] = -32000;
				myjson["message"] = "There was an error reading to the database.";
				ResponseJson["error"] = myjson;	
				Log("Error : -32000 : There was an error reading to the database.");
			} 	

			delete session;

		}
//=====================================================================
		if(funjson["method"]=="send_answer") 
		{
			int id_decision = 0;		
			
			myjson.clear();
			s_id = funjson["params"].get("session_id", 3).asInt();
			session = new SessionGame(s_id);
			int qi,ai;
			qi=funjson["params"].get("quest_id", 11).asInt();
			ai=funjson["params"].get("answer_id", 3).asInt();
			session->set_event(qi, ai);                                    
			myjson.clear();
	
			id_decision = session->FunctionBayes(qi,ai);
			std::cout<<id_decision<<std::endl;
			//if((session->session_num) < 8) myjson["sesion_state"] = "next";
			if(((session->session_num) < 11)&&(id_decision==0)) 
			{
				myjson["sesion_state"] = "next";
				myjson["session_id"] = s_id;
				ResponseJson["result"] = myjson;
			}
			else
			{
				
				std::string string_data = session->decision_not_found();
				//if(id_decision==0) id_decision = session->local_decision;
				if(id_decision==0)
				{
					myjson.clear();
					myjson["code"] = -32001;                  //<----Error code!!!!
					myjson["message"] ="Decision not found";
					ResponseJson["error"] = myjson;	
					Log("Error : -32001 : Decision not found");	
				}	
				else
				{
					myjson["session_state"] = "find";
					myjson["decision_id"] = id_decision; 
					memset(sql_zapros,0,1000);
           				strcat(sql_zapros,"SELECT * FROM Decision WHERE decision_id="); 
					sprintf(peremen,"%d",id_decision);
					strcat(sql_zapros,peremen);
					mysqlError = mysql_query(connection, sql_zapros);
					result = mysql_store_result(connection);
					row = mysql_fetch_row(result);
					mysql_free_result(result);
					myjson["decision_text"] = row[1];
					myjson["session_id"] = s_id;
					ResponseJson["result"] = myjson;

				}
			}

	
			session->data_to_base();
			delete session;
		}
//=====================================================================
		if(funjson["method"]=="get_session")
		{
			myjson.clear();
			s_id = funjson["params"].get("session_id", 3).asInt();
			session = new SessionGame(s_id);
			std::string string_data = session->decision_not_found();
			bool parsingSuccessful = reader.parse(  string_data, myjson);
			ResponseJson["result"] = myjson;
			delete session;

		}	



	
		if(funjson["method"]=="question")
		{ 
			parsingSuccessful = reader.parse( function, myjson);
			if (parsingSuccessful==false) 
			{
				//{"result": 1, "error": null, "id": 99}
				myjson.clear();
				myjson["code"] = -32700;
				myjson["message"] = "Error parse Json";
				ResponseJson["error"] = myjson;	
				Log("Error : -32700 : Error parse Json");
				
			}

			else{
				textQuest = myjson["question"].asString();		
				std::cout<<textQuest<<std::endl;
				memset(sql_zapros,0,1000);
           			strcat(sql_zapros,"INSERT INTO Quest (quest_text) VALUES ('");
				strcat(sql_zapros,textQuest.c_str());
              		  	strcat(sql_zapros,"')");
				mysqlError = mysql_query(connection, sql_zapros);
			}

			if (mysqlError==false) ResponseJson["result"] = 1;
			else 
			{
				myjson.clear();
				myjson["code"] = -32000;
				myjson["message"] = "There was an error writing to the database.";
				ResponseJson["error"] = myjson;	
				Log("Error : -32000 : There was an error writing to the database.");
			} 
		
		}

//=====================================================================
		if(!funjson["method"])
		{
			std::cout<<"Error: method is not found!"<<std::endl;
			ResponseJson.clear();
			myjson.clear();
			myjson["code"] = -32002;
			myjson["message"] = "the \"method\" not found!";
			ResponseJson["error"] = myjson;
			Log("Error : -32002 : The method not found!");
		}	
	}
///<---END "IF"	
	else 
	{
		std::cout<<"Error: Json not parsing!"<<std::endl;
		ResponseJson.clear();
		myjson.clear();
		myjson["code"] = -32003;
		myjson["message"] = "Json not parsing!";
		ResponseJson["error"] = myjson;
	}
//ANSWER
	
	ResponseJson["jsonrpc"] = "2.0";
	ResponseJson["id"] = funjson["id"].asInt();
	std::cout<<ResponseJson<<std::endl;
	function = writer.write(ResponseJson);
	return function;
}


int main (int argc, char const *argv[]) 
{
        
   	MYSQL_RES *result;
	MYSQL_ROW row; 

	mysql_init(&mysql);
	mysql_options(&mysql,MYSQL_READ_DEFAULT_GROUP,"your_prog_name");
 	connection = mysql_real_connect(&mysql,"localhost","root","1","GPO",3036,NULL,0);
 	if(!connection)
 	{
   		std::cout<<mysql_error(&mysql)<<std::endl;
	 }


	void* context = zmq_ctx_new();
	void* respond = zmq_socket(context, ZMQ_REP);
	zmq_bind(respond, "tcp://*:49200");
	Log("Service run...");
	printf("Starting…\n");
	char stroka[600];
	memset(stroka,0,600);

	
	for(;;) 
	{
		
		zmq_msg_t request; //zayavka
		zmq_msg_init(&request);
		zmq_msg_recv(&request, respond, 0); //otvet
                memcpy(stroka, zmq_msg_data(&request), zmq_msg_size(&request));
		std::cout<<"From BackEnd:"<<stroka<<std::endl;
		Log("~~ Get message ~~ :");
		Log(stroka);
		std::string answerStr = JsonToString(stroka);		
		Log("~~ Send message ~~:");
		Log(answerStr);
                zmq_msg_close(&request);	
		zmq_msg_t reply; //otvet


		zmq_msg_init_size(&reply, strlen(answerStr.c_str())); //opredelili razmer 
		memcpy(zmq_msg_data(&reply),answerStr.c_str(), strlen(answerStr.c_str()));
/*
		zmq_msg_init_size(&reply, strlen(stroka)); //opredelili razmer 
		memcpy(zmq_msg_data(&reply),stroka, strlen(stroka));
*/
		answerStr.clear();
		memset(stroka,0,600);
		zmq_msg_send(&reply, respond, 0);
		zmq_msg_close(&reply);
	
	}
	zmq_close(respond);
	zmq_ctx_destroy(context);
 	mysql_close(&mysql);
	Log("Service was close.");
	return 0;
}
