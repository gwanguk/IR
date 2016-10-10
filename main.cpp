#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <io.h>
#include "porter2stemmer.h"
#include "Util.h"
#include "KrovetzStemmer.h"
#include "Index.h"
#include <time.h>

#define stopword_CNT 1000 // 불용어 개수
#define stopword_LENGTH 20 //불용어 최대 길기
#define MAX_READLINE_LENGTH 10000 //최대라인 읽기 수

using namespace std;
using namespace index;

void Stopword(std::string filename);
void Stemming(std::string filename);

static string ResultFileDir = "./text/"; // 원본파일 위치 폴더
static string StemmedFileDir = "./stemmed/"; //스테밍 파일 폴더
static string IndexingFileDir = "./indexing/"; // 인덱싱 파일 폴더
const char mark[6] = { ',','.','-',';','"','`'}; // 불용어 제거시 삭제 기호
vector<string> stopword_list;


int main() {

	_finddata_t fd; // 뉴스 파일 순회 불러들이기
	long handle;
	int result = 1;
	handle = _findfirst("./text/*.*", &fd);  //현재 폴더 내 모든 파일을 찾는다.
	char inputString[MAX_READLINE_LENGTH];
	Index indexing = Index::Index();
	clock_t begin, stop_end, stem_end, index_one_end, index_two_end;

	ifstream stopFile("stopword.txt"); //불용어 파일 오픈

	while (!stopFile.eof()) { //불용어 파일 불러들이기
		stopFile.getline(inputString, stopword_LENGTH);
		stopword_list.push_back(inputString);
	}
	stopFile.close(); // 불용어 파일 닫기

	if (handle == -1) // 파일 없을시
	{
		printf("There were no files.\n");
	}
	else // 파일있을 시 
	{
		while (result != -1)
		{
			if (!(fd.name[0] == '.'))
			{
				string filename = fd.name;
				begin = clock();
				Stopword(filename); // parsing & stopword
				stop_end = clock();
				Stemming(filename);// stemming
				stem_end = clock();
			}
			result = _findnext(handle, &fd);
		}
	}
	_findclose(handle);

	/*역색인파일 생성*/
	printf("Indexing start!\n");
	indexing.ExtractIndexFile(StemmedFileDir,IndexingFileDir);
	printf("ReversedIndex.txt file creating...\n");
	indexing.InvertedIndexing(IndexingFileDir);
	
	index_one_end = clock();

	//cout << (stop_end - begin) / CLOCKS_PER_SEC << endl;
	//cout << (stem_end - stop_end) / CLOCKS_PER_SEC << endl;
	//cout << (index_one_end - stem_end) / CLOCKS_PER_SEC << endl;
	return 0;
}


void Stopword(std::string filename)
{
	int stopword_cnt = 0;

	ifstream textFile; // 현재 텍스트파일
	ofstream output_textFile; // 처리 후 결과 파일
	char inputString[MAX_READLINE_LENGTH];
	string str;
	string out_str;
	string::size_type s_index;
	string::size_type l_index;
	string::size_type m_index;
	int curTypeIsText = 0;
	int curTypeIsSubheadline = 0;
	int result = 0;

	
	textFile.open(ResultFileDir+ filename);
	output_textFile.open(ResultFileDir+filename+"_stopword"); // 1차 파싱된 파일 이름


	while (!textFile.eof()) {
		textFile.getline(inputString, MAX_READLINE_LENGTH);
		str = inputString;
		if (sizeof(inputString) > MAX_READLINE_LENGTH)
		{
			printf("ex!!!!!!!!!!\n");
		}

		if ((s_index = str.find("<DOCNO>")) != string::npos) // <DOCNO>를 포함하고 있을시
		{
			l_index = str.find("</DOCNO>");
			out_str = str.substr(s_index + 7, l_index - s_index - 7); // "</DOCNO>크기 7
			out_str.insert(0, "[DOCNO] :");
			output_textFile << out_str << endl;
		}

		if (str.find("<TEXT>") != string::npos) //<TEXT>를 포함 하고있을 시 플래그값을 변경 시킴
		{
			curTypeIsText = 1;
			out_str = "[TEXT] : ";
			output_textFile << out_str << endl;
			continue;
		}

		if ((s_index = str.find("<HEADLINE>")) != string::npos) // <HEADLINE>를 포함 하고 있을 시
		{
			l_index = str.find("</HEADLINE>");
			out_str = str.substr(s_index + 10, l_index - s_index - 10); // "</HEADLINE>크기 10
			out_str.insert(0, "[HEADLINE] :");
			out_str.append(" ");

			m_index = string::npos;
			for (int i = 0; i < size(mark); i++)
			{
				while ((m_index = out_str.find(mark[i])) != string::npos)
				{
					out_str.erase(m_index, 1);
				}
			}

			for (vector<string>::size_type i = 0; i < stopword_list.size(); i++)
			{
				string cur_stopword = stopword_list[i];
				cur_stopword.insert(0, " "); // 불용어 앞뒤 공백추가
				cur_stopword.insert(cur_stopword.length(), " ");

				s_index = 0;
				while ((s_index = out_str.find(cur_stopword, s_index)) != string::npos) // 불용어 순회 하며 삭제
				{
					out_str.erase(s_index + 1, cur_stopword.length() - 1);
				}
				s_index = 0;
				while ((s_index = out_str.find(StringFunction::replaceFirstCapital(cur_stopword),s_index)) != string::npos) // 첫 단어 대문자로 변경하여 삭제
				{
					out_str.erase(s_index + 1, cur_stopword.length() - 1);
				}
			}
			output_textFile << out_str << endl;
		}

		if (curTypeIsText == 1 && str.find("</TEXT>") == string::npos) // TEXT내용일 경우 HEADLINE과동일하게처리
		{
			out_str = str;
			out_str.insert(0, " ");
			out_str.append(" ");

			if (!str.compare("<P>") || !str.compare("</P>") || !str.compare("<SUBHEAD>") || !str.compare("</SUBHEAD>"))
				continue;
			else if (str.find("<P>") != string::npos || str.find("</P>") != string::npos || str.find("<SUBHEAD>") != string::npos || str.find("</SUBHEAD>") != string::npos)
				continue;

			m_index = string::npos;
			for (int i = 0; i < size(mark); i++)
			{
				while ((m_index = out_str.find(mark[i])) != string::npos)
				{
					out_str.erase(m_index, 1);
				}
			}

			for (vector<string>::size_type i = 0; i < stopword_list.size(); i++)
			{
				string cur_stopword = stopword_list[i];
				cur_stopword.insert(0, " ");
				cur_stopword.insert(cur_stopword.length(), " ");

				s_index = 0;
				while ((s_index = out_str.find(cur_stopword, s_index)) != string::npos)
				{
					out_str.erase(s_index +1, cur_stopword.length()-1);
				}
				s_index = 0;
				while ((s_index = out_str.find(StringFunction::replaceFirstCapital(cur_stopword), s_index)) != string::npos)
				{
					out_str.erase(s_index + 1, cur_stopword.length() - 1);
				}
			}
			output_textFile << out_str << endl;
			//cout << out_str << endl;
		}
		else
			curTypeIsText = 0;


		result = 1;
	}
	textFile.close();
	output_textFile.close();

}

void Stemming(std::string filename)
{

	ifstream parsingFile(ResultFileDir+ filename+"_stopword"); // 1차 parsing된 파일 열기
	ofstream stemmedFile; //stemming 된 파일
	stemmedFile.open(StemmedFileDir+filename+"_stemmed");

	stem::KrovetzStemmer * stemmer = new stem::KrovetzStemmer();
	char word[80];
	char thestem[80];

	char inputstring[MAX_READLINE_LENGTH];
	string str;
	string token;
	int ret;
	while (!parsingFile.eof())
	{
		parsingFile.getline(inputstring, MAX_READLINE_LENGTH); // 문자열 읽기
		str = inputstring;
		stringstream stream(str);
		while (stream >> token) // stream을 이용하여 문자열 토큰화
		{
				//strcpy(word, token.c_str());
				//ret=stemmer->kstem_stem_tobuffer(word, thestem); //krovetz stemming
				//if(ret>0) 
				//	token = thestem;
				Porter2Stemmer::stem(token); //porter2 stemming
				stemmedFile << token << " ";
		}
		stemmedFile << endl;

	}
	parsingFile.close();
	stemmedFile.close();

}