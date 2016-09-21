#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
//#include "zmq.h"
#include <mysql/mysql.h>
#include "json/json.h"

using namespace std;
using namespace Json;



int main()
{

	char sql_z[600];
	char name[100];
	memset(sql_z,0,600);
	memset(name,0,100);
	MYSQL mysql, *connection;
	MYSQL_RES *result;
	MYSQL_ROW row; 

	int maxQuest = 1;

	Json::Value myjson; 
	Json::Reader reader;
	//Json::StyledWriter writer;

	mysql_init(&mysql);
	mysql_options(&mysql,MYSQL_READ_DEFAULT_GROUP,"your_prog_name");
 	connection = mysql_real_connect(&mysql,"localhost","root","1","GPO",3036,NULL,0);
 	if(!connection)
 	{
   		std::cout<<mysql_error(&mysql)<<std::endl;
	 }

	printf("vvedite name personazga:");
	scanf("%s",name);
	int flag;
	char num[10];
	mysql_query(connection, "SELECT MAX(quest_id) AS session_id FROM Quest");
 	result = mysql_store_result(connection);
	row = mysql_fetch_row(result);
	maxQuest = atoi(row[0]);
	std::cout<<maxQuest<<std::endl;

	mysql_query(connection, "SELECT * FROM Quest");
 	result = mysql_store_result(connection);
	for(int i=1; i<=maxQuest; i++)
	{
		row = mysql_fetch_row(result);
		std::cout<<row[1]<<std::endl;
		//printf("num vvodi:");
		scanf("%d",&flag);
		sprintf(num,"%d",i);
		myjson[num] = flag;
		

	}

	std::string jsoninstring;
	Json::FastWriter writer;
	jsoninstring = writer.write(myjson);
	std::cout<<jsoninstring<<std::endl;
	strcat(sql_z,"INSERT INTO Decision (decision_text,decision_json,decision_aprior) VALUES ('");
	strcat(sql_z,name);
	strcat(sql_z,"','");
	strcat(sql_z,jsoninstring.c_str());
	strcat(sql_z,"','0.2')");               //<- izmenit parametr
	mysql_query(connection, sql_z);

	mysql_free_result(result);
	mysql_close(&mysql);

	return 0;
}
