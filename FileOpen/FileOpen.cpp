#include "pch.h"
#include <io.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>  
#include <iomanip>
using namespace std;


//获取特定格式的文件名
void GetAllFormatFiles(string path, vector<string>& files)
{
	//文件句柄  
	intptr_t   hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					//files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
					GetAllFormatFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
}

//日期转换为timestamp
int standard_to_stamp(const char *str_time)
{
	struct tm stm;
	int iY, iM, iD, iH, iMin, iS;

	memset(&stm, 0, sizeof(stm));
	iY = atoi(str_time);
	iM = atoi(str_time + 5);
	iD = atoi(str_time + 8);
	iH = atoi(str_time + 11);
	iMin = atoi(str_time + 14);
	iS = atoi(str_time + 17);

	stm.tm_year = iY - 1900;
	stm.tm_mon = iM - 1;
	stm.tm_mday = iD;
	stm.tm_hour = iH;
	stm.tm_min = iMin;
	stm.tm_sec = iS;

	//printf("%d-%0d-%0d %0d:%0d:%0d\n", iY, iM, iD, iH, iMin, iS);
	return (int)mktime(&stm);

}
//经纬度转换
double lonlat_transform(double x)
{
	return (floor(x / 100) + (x - floor(x / 100) * 100) / 60);
}
//utc转换为秒数
int second_transform(int x)
{
	//return (floor(x / 10000) * 3600 + floor((x - floor(x / 10000) * 10000) / 100) * 60 + fmod(x,100));
	return ((x / 10000) * 3600 + ((x - (x / 10000) * 10000) / 100) * 60 + (x % 100));

}


// 该函数有两个参数，第一个为路径字符串(string类型，最好为绝对路径)；
// 第二个参数为文件夹与文件名称存储变量(vector类型,引用传递)。
// 在主函数中调用格式(并将结果保存在文件"AllFiles.txt"中，第一行为总数)：

int main()
{
	//startTime
	clock_t startTime, endTime;
	startTime = clock();

	vector<vector<string>> strArray;
	string filePath = "E:\\Huawei\\Data\\Data";
	vector<string> files;
	const char * distAll = "E:\\Huawei\\Data\\AllFiles.txt";


	GetAllFormatFiles(filePath, files);
	ofstream ofn(distAll);
	//	int size = files.size();
		//ofn << size << endl; 输出文件个数
	for (unsigned int i = 0; i < files.size(); i++)
	{
		ofn << files[i] << endl;
		//cout << files[i] << endl;
	}
	ofn.close();

	vector<int> utc;
	vector<double> longitude;
	vector<double> latitude;
	vector<double> angle;
	vector<double> speed;

	int sum = 0;
	int count = 0;
	size_t namelength;//文件名长度
	int stampTime;

	string YY;
	string MM;
	string DD;
	string timestamp1;
	const char * timestamp;

	for (unsigned int i = 0; i < files.size(); i++)
	{
		namelength = files[i].size();
		if (files[i].substr(namelength - 3, 3) == "txt"&&files[i].substr(namelength - 23, 2) == "20"&&files[i].substr(namelength - 45, 12) == "BDSView_nmea")
		{
			string line;
			count = 0;
			ifstream Files(files[i]);
			//获取文件日期

			YY = files[i].substr(namelength - 23, 4);
			MM = files[i].substr(namelength - 18, 2);
			DD = files[i].substr(namelength - 15, 2);
			timestamp1 = YY + "-" + MM + "-" + DD + " 00;00;00";//获取数据日期
			timestamp = timestamp1.c_str();
			stampTime = standard_to_stamp(timestamp);

			if (Files)
			{
				Files.seekg(0, Files.end);
				auto length = Files.tellg() / 1024.00;
				Files.clear();
				Files.seekg(0, Files.beg);
			//	cout << length << endl;
				while (length>50&&getline(Files, line))//按行读取文件路径
				{
					//if (length >50)
					//	cout << "Binary length: " << length << endl;
					//	if (line.substr(0, 8) == "PB$GNRMC" || line.substr(0, 6) == "$GNRMC")
					if( line.substr(0, 8) == "PB$GNRMC")
					{
						stringstream ss(line);
						string str;
						vector<string> lineArray;
						count++;
						//if (count % 10 == 1)
						{
							while (getline(ss, str, ','))
								lineArray.push_back(str);
							strArray.push_back(lineArray);
							//	cout << line << endl; 
							sum++;
						}
					}
				}
				for (int i = 0; i < sum; i++)
				{

					//代表是否定位 A定位 V未定位 速度大于1 角度不为空
					if (strArray[i][2] == "A" && atof(strArray[i][8].c_str()) >= 0&& (atof(strArray[i][7].c_str())*0.5144444)>= 1)
					{
						//	PJ_COORD xy = project_to_xy(atof(strArray[i][2].c_str()), atof(strArray[i][1].c_str()));
						//	origin_x.push_back(xy.xy.x);
						//	origin_y.push_back(xy.xy.y);
						utc.push_back(stampTime + (atoi(strArray[i][1].c_str())));
						latitude.push_back(lonlat_transform(atof(strArray[i][3].c_str())));
						longitude.push_back(lonlat_transform(atof(strArray[i][5].c_str())));
						speed.push_back(atof(strArray[i][7].c_str())*0.5144444);
						angle.push_back(atof(strArray[i][8].c_str()));
					}
				}
				sum = 0;
				count = 0;
				vector<vector<string>>().swap(strArray);//清空容器
				Files.close();
				Files.clear();
			}

			else
			{
				cout << "No Such Files!" << endl;
			}
		}
	}
	ofstream outFile;
	outFile.open("C:\\Users\\Administrator\\Desktop\\Date_test2.csv", ios::out);
	for (unsigned int i = 0; i < latitude.size(); i++)
	{
		//cout << setprecision(13) << longitude[i] << endl;
		outFile << setprecision(13) << utc[i] << "," << latitude[i] << "," <<longitude[i] << ","  << speed[i] << "," << angle[i] << endl;
	}
	outFile.close();
	cout << utc.size() << endl;
	cout << latitude.size() << endl;
	endTime = clock();
	cout << "运行时间 : " << setprecision(5) << ((double)(endTime - startTime) / CLOCKS_PER_SEC) / 60 << "min" << '\n' << endl;
	return 0;
}
