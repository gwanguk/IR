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

#define stopword_CNT 1000 // �ҿ�� ����
#define stopword_LENGTH 20 //�ҿ�� �ִ� ���
#define MAX_READLINE_LENGTH 10000 //�ִ���� �б� ��

using namespace std;
using namespace index;

void Stopword(std::string filename);
void Stemming(std::string filename);

static string ResultFileDir = "./text/"; // �������� ��ġ ����
static string StemmedFileDir = "./stemmed/"; //���׹� ���� ����
static string IndexingFileDir = "./indexing/"; // �ε��� ���� ����
const char mark[6] = { ',','.','-',';','"','`'}; // �ҿ�� ���Ž� ���� ��ȣ
vector<string> stopword_list;


int main() {

	_finddata_t fd; // ���� ���� ��ȸ �ҷ����̱�
	long handle;
	int result = 1;
	handle = _findfirst("./text/*.*", &fd);  //���� ���� �� ��� ������ ã�´�.
	char inputString[MAX_READLINE_LENGTH];
	Index indexing = Index::Index();
	clock_t begin, stop_end, stem_end, index_one_end, index_two_end;

	ifstream stopFile("stopword.txt"); //�ҿ�� ���� ����

	while (!stopFile.eof()) { //�ҿ�� ���� �ҷ����̱�
		stopFile.getline(inputString, stopword_LENGTH);
		stopword_list.push_back(inputString);
	}
	stopFile.close(); // �ҿ�� ���� �ݱ�

	if (handle == -1) // ���� ������
	{
		printf("There were no files.\n");
	}
	else // �������� �� 
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

	/*���������� ����*/
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

	ifstream textFile; // ���� �ؽ�Ʈ����
	ofstream output_textFile; // ó�� �� ��� ����
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
	output_textFile.open(ResultFileDir+filename+"_stopword"); // 1�� �Ľ̵� ���� �̸�


	while (!textFile.eof()) {
		textFile.getline(inputString, MAX_READLINE_LENGTH);
		str = inputString;
		if (sizeof(inputString) > MAX_READLINE_LENGTH)
		{
			printf("ex!!!!!!!!!!\n");
		}

		if ((s_index = str.find("<DOCNO>")) != string::npos) // <DOCNO>�� �����ϰ� ������
		{
			l_index = str.find("</DOCNO>");
			out_str = str.substr(s_index + 7, l_index - s_index - 7); // "</DOCNO>ũ�� 7
			out_str.insert(0, "[DOCNO] :");
			output_textFile << out_str << endl;
		}

		if (str.find("<TEXT>") != string::npos) //<TEXT>�� ���� �ϰ����� �� �÷��װ��� ���� ��Ŵ
		{
			curTypeIsText = 1;
			out_str = "[TEXT] : ";
			output_textFile << out_str << endl;
			continue;
		}

		if ((s_index = str.find("<HEADLINE>")) != string::npos) // <HEADLINE>�� ���� �ϰ� ���� ��
		{
			l_index = str.find("</HEADLINE>");
			out_str = str.substr(s_index + 10, l_index - s_index - 10); // "</HEADLINE>ũ�� 10
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
				cur_stopword.insert(0, " "); // �ҿ�� �յ� �����߰�
				cur_stopword.insert(cur_stopword.length(), " ");

				s_index = 0;
				while ((s_index = out_str.find(cur_stopword, s_index)) != string::npos) // �ҿ�� ��ȸ �ϸ� ����
				{
					out_str.erase(s_index + 1, cur_stopword.length() - 1);
				}
				s_index = 0;
				while ((s_index = out_str.find(StringFunction::replaceFirstCapital(cur_stopword),s_index)) != string::npos) // ù �ܾ� �빮�ڷ� �����Ͽ� ����
				{
					out_str.erase(s_index + 1, cur_stopword.length() - 1);
				}
			}
			output_textFile << out_str << endl;
		}

		if (curTypeIsText == 1 && str.find("</TEXT>") == string::npos) // TEXT������ ��� HEADLINE�������ϰ�ó��
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

	ifstream parsingFile(ResultFileDir+ filename+"_stopword"); // 1�� parsing�� ���� ����
	ofstream stemmedFile; //stemming �� ����
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
		parsingFile.getline(inputstring, MAX_READLINE_LENGTH); // ���ڿ� �б�
		str = inputstring;
		stringstream stream(str);
		while (stream >> token) // stream�� �̿��Ͽ� ���ڿ� ��ūȭ
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