#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include "zmq.h"
#include <mysql/mysql.h>
#include "json/json.h"
#include <vector>
#include <sstream>



using namespace std;
using namespace Json;

MYSQL mysql, *connection;

class SessionGame
{
	public:
	int session_id;
	int session_num;
	
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
	//	if(session_num>1)
	//	{
			memset(sql_z,0,600);
			//std::cout<<row_s[2]<<std::endl;
			data_string = row_s[2];
			strcat(sql_z,data_string.c_str());
			bool parsingSuccessful = reader.parse( sql_z, session_event);
			memset(sql_z,0,600);
			strcat(sql_z,row_s[3]);
			parsingSuccessful = reader.parse( sql_z, session_aprior);

	//	}

	//	else 


		mysql_free_result(result_s);
		
		//std::cout<<session_id<<" "<<session_num<<std::endl;
		///????
		
	}
//~~~~~
	


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
	



	void FunctionBayes(int q_id)
	{
		Json::Value ApostrVer;
		Json::Value PeremJson;
		Json::Reader reader;
		Json::FastWriter writer;
		int maxDecis;
		float PB;
		std::cout<<"work1"<<std::endl;
		memset(sql_z,0,600);	
		mysql_query(connection,"select decision_id,decision_json from Decision");
		std::string deletepotom;
		result_s = mysql_store_result(connection);
		std::cout<<"work2"<<std::endl;
		int flag;
		 while ((row_s = mysql_fetch_row(result_s))!=NULL) 
 		{
			reader.parse( row_s[1], PeremJson);
			std::cout<<PeremJson<<std::endl;
			deletepotom = PeremJson[row_s[q_id]].asString();
			strcat(peremenya,deletepotom.c_str());
			flag = atoi(peremenya);
			std::cout<<flag<<std::endl;
			/*
				Error! sravnivat otv(tekyshii) with znacheniem from tables!!!!	
			*/
			if( flag>0 ) ApostrVer[row_s[0]] = "0.8";
			if( flag==0 ) ApostrVer[row_s[0]] = "0.5";
			if( flag<0 ) ApostrVer[row_s[0]] = "0.2";
   			maxDecis = atoi(row_s[0]);
			memset(peremenya,0,12);
			deletepotom.clear();
		 }	
		std::cout<<"work3"<<std::endl;
		std::string forexample;
		forexample = writer.write(session_aprior);
		std::cout<<forexample<<std::endl;
		forexample = writer.write(ApostrVer);
		std::cout<<forexample<<std::endl;
		float apr, apost;
		for(int i=1; i<maxDecis; i++)
		{
			deletepotom.clear();
			sprintf(peremenya,"%d",i);
			deletepotom = session_aprior[peremenya].asString();
			std::cout<<deletepotom<<std::endl;
			apr = atof(deletepotom.c_str());
			deletepotom.clear();
			std::cout<<apr<<std::endl;
			deletepotom = ApostrVer[peremenya].asString();
			apost = atof(deletepotom.c_str());
			deletepotom.clear();
			PB = PB+(apr * apost);

		}	
		std::cout<<"work4"<<std::endl;
		float endNumber;
		for(int i=1; i<maxDecis; i++)
		{
			deletepotom.clear();
			sprintf(peremenya,"%d",i);
			deletepotom = session_aprior[peremenya].asString();
			apr = atof(deletepotom.c_str());
			deletepotom.clear();
			deletepotom = ApostrVer[peremenya].asString();
			apost = atof(deletepotom.c_str());
			deletepotom.clear();
			endNumber = (apr * apost)/PB;
			session_aprior[peremenya] = endNumber;
			

		}	

		
		mysql_free_result(result_s);
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

	char sql_zapros[1000]; 
	Json::Value funjson;
	Json::Value myjson; 
	Json::Value ResponseJson;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse( s, funjson);
	bool mysqlError;
	Json::FastWriter writer;
	std::string function;
	if(funjson["params"]!=0)  function = writer.write(funjson["params"]);
	std::string textQuest;
	int s_id = 0;
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
				//std::cout<<row[0]<<std::endl;
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
			char peremen[100];
			int number_quest = 1; //<----INDEX QUEST

			s_id = funjson["params"].get("session_id", 3).asInt();
			session = new SessionGame(s_id);
			number_quest = session->session_num;
			//opred_min_quest()???
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
				//std::cout<<row[1]<<std::endl; //delete this text!!!
				ResponseJson["result"] = myjson;
				
			}
			else 
			{
				myjson.clear();
				myjson["code"] = -32000;
				myjson["message"] = "There was an error reading to the database.";
				ResponseJson["error"] = myjson;	
			} 	

		//	session->data_to_base();
			delete session;

	}
//=====================================================================
	if(funjson["method"]=="send_answer") 
	{
		myjson.clear();
		s_id = funjson["params"].get("session_id", 3).asInt();
		//std::cout<<s_id<<std::endl;
		session = new SessionGame(s_id);
		int qi,ai;
		qi=funjson["params"].get("quest_id", 11).asInt();
		ai=funjson["params"].get("answer_id", 3).asInt();
		session->set_event(qi, ai);                                    
		//if(-1 == FunctionBayes()) decision not found;
		//else decision = ResultFunctionBayes;
		myjson.clear();

		session->FunctionBayes(qi);

		if((session->session_num) < 8) myjson["sesion_state"] = "next";
		else
		{
			myjson["session_state"] = "find";
			myjson["decision_id"] = 1;  ///<---Ispravit!!!
			myjson["decision_text"] = "It is the text of decision for User Problem! ";
		}


		myjson["session_id"] = s_id;
		ResponseJson["result"] = myjson;
		session->data_to_base();
		delete session;
		//std::cout<<"set_session_answer"<<std::endl;
	}
//=====================================================================
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
			} 
		
	}

//=====================================================================
	if(!funjson["method"]) std::cout<<"Error: method is not found!"<<std::endl;
	
	
//ANSWER
	
	ResponseJson["jsonrpc"] = "2.0";
	ResponseJson["id"] = funjson["id"];
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

		std::string answerStr = JsonToString(stroka);		
		
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
	return 0;
}
