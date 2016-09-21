#include <time.h>

void Log(std::string s)
{
char data[50];
  FILE *f;
  std::string nf;
  std::string t;
  time_t tt;
  tm* aTm;
  int i;
  
  tt = time(NULL);
  aTm = localtime(&tt);
  sprintf(data,"%04d/%02d/%02d %02d:%02d:%02d",aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
  t=data;
  
  //  t = DateTimeToStr(now())+": ";
    s = t + " " +s;
        s = s + "\n";
        f=fopen("log.txt","a+");
        fwrite(s.c_str(),s.length(),1,f);
        fclose(f);
}