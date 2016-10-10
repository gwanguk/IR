#include <iostream>d
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <io.h>
#include <math.h>
#include <vector>
#include <unordered_map>
#include <tuple>
#include "Index.h"
#define MAX_READLINE_LENGTH 10000
#define MAX_INDEXFILE_LENGTH 100000000
using namespace std;
using namespace index;

Index::Index()
{
	allocated_doc_id = 0;
	tf_file_size = 0;
}

/*total weight 미리 구해 놓기*/
void Index::Total_weight(unordered_map<int, float> &total_weight, string IndexFileDir)
{
	float weight;
	ifstream indexfile(IndexFileDir);
	char inputstring[1000];
	string doc_name = "";
	string term = "";
	int value;
	int df;
	float sum = 0;
	int start = 1;

	string str;
	string pre_str;
	stringstream stream;
	while (!indexfile.eof())
	{
		indexfile.getline(inputstring, MAX_READLINE_LENGTH);
		str = inputstring;
		stream.str(inputstring);
		stream >> doc_name; stream >> term; stream >> value;

		if (doc_name.compare(pre_str) && start == 0)//문서가 변경되면 total weight 저장
		{
			sum = sqrtf(sum);
			total_weight[ID_DOC_map[pre_str]] = sum;
			sum = 0;
			start = 0;
		}
		pre_str = doc_name;
		df = DF[term];
		sum += pow(((log10f(value) + 1.0)*(log10f((float)(allocated_doc_id + 1) / (float)df))), 2);
		start = 0;
		stream.clear();
	}
	sum = sqrtf(sum); //마지막 저장
	total_weight[ID_DOC_map[pre_str]] = sum;
}

/*처음 추출된 index 파일을 읽어들여 역색인함*/
void Index::InvertedIndexing(string OutputDir)
{
	int* DOC_ID;
	int* TERM_ID;
	int* TF;
	float weight;
	unordered_map<string, int>::iterator it;
	ifstream indexfile(OutputDir + "Index");
	char inputstring[100];
	string doc_name = "";
	string term = "";
	int value;
	float cfidf;
	int df = 0;
	int cur_TERM_ID;
	int i = 0;
	int j = 1;
	int total_tf_cnt;

	int offset = 0;
	char wordinput[100];
	stringstream wordstream;
	string w_id; string w_term; string w_df; string w_cf;

	char DOC_ID_FORMAT[10];
	char TERM_ID_FORMAT[10];
	char WEIGHT_FORMAT[6];
	char TF_FORMAT[3];
	DOC_ID = (int*)malloc(sizeof(int)*MAX_INDEXFILE_LENGTH);
	TERM_ID = (int*)malloc(sizeof(int)*MAX_INDEXFILE_LENGTH);
	TF = (int*)malloc(sizeof(int)*MAX_INDEXFILE_LENGTH);
	printf("Memory allocated complete!!\n");

	Total_weight(total_weight, OutputDir + "Index"); //total_weighting 먼저 구하기
	printf("Total weight calculated!!\n");

	ifstream wordtxt(OutputDir + "Word.txt");
	ofstream wordtxt_re(OutputDir + "Word_offset.txt");
	ofstream TF_File((OutputDir + "Index").append("_inverted.txt"));

	stringstream stream;
	while (!indexfile.eof())
	{
		indexfile.getline(inputstring, MAX_READLINE_LENGTH);
		stream.str(inputstring);
		stream >> doc_name; stream >> term; stream >> value;
		DOC_ID[i] = ID_DOC_map[doc_name];
		TERM_ID[i] = ID_TERM_map[term];
		TF[i] = value;
		stream.clear();
		i++;
	}
	indexfile.close();
	printf("index file read OK!!\n");

	total_tf_cnt = i;
	it = ID_TERM_map.begin();
	while (it != ID_TERM_map.end()) // ID_TERM_map 의 순서대로 역색인
	{
		cur_TERM_ID = (*it).second;
		wordtxt.getline(wordinput, 100);
		wordstream.str(wordinput);
		wordstream >> w_id;	wordstream >> w_term;	wordstream >> w_df;	wordstream >> w_cf;
		wordtxt_re << w_id << "\t" << w_term << "\t" << w_df << "\t" << w_cf << "\t" << offset << endl; // 최초 시작부분을 offset으로 하여 Word.txt파일에 정보 추가
		wordstream.clear();
		for (i = 0; i < total_tf_cnt; i++)
		{
			if (TERM_ID[i] == cur_TERM_ID)
			{
				df = DF[(*it).first];
				cfidf = (log10f(value) + 1.0)*(log10f((float)(allocated_doc_id + 1) / (float)df));
				weight = cfidf / total_weight[DOC_ID[i]];
				sprintf(DOC_ID_FORMAT, "%010d", DOC_ID[i]);
				sprintf(TERM_ID_FORMAT, "%010d", cur_TERM_ID);
				sprintf(WEIGHT_FORMAT, "%5.4f", weight);
				sprintf(TF_FORMAT, "%03d", TF[i]);
				TF_File << TERM_ID_FORMAT << DOC_ID_FORMAT << TF_FORMAT << WEIGHT_FORMAT << endl;
				offset++;
			}
		}
		it++;
		j++;
	}

	free(DOC_ID);
	free(TERM_ID);
	free(TF);

	TF_File.close();
	wordtxt.close();
	wordtxt_re.close();
	printf("InvertedIndexing done!\n");

}

void Index::Counting_DF(unordered_map<string, int> map) //현재 문서에 해당하는 임시 CF를 이용하여 DF 구하기
{
	unordered_map<string, int>::iterator it;
	unordered_map<string, int>::iterator tmp;
	it = map.begin();
	while (it != map.end())
	{
		if ((tmp = DF.find((*it).first)) == DF.end())
		{
			DF[(*it).first] = 1;
		}
		else
		{
			DF[(*it).first]++;
		}
		it++;
	}
}

void Index::Counting_Map(unordered_map<string, int> &map, string str)
{
	unordered_map<string, int>::iterator it;
	it = map.find(str);
	if (it == map.end())
	{
		map[str] = 1;
	}
	else
	{
		(*it).second++;
	}
}
/*1차 인덱싱*/
void Index::ExtractIndexFile(string InputFileDir, string OutputDir)
{
	vector<string> stemmedfile_list = StemmedFileListRead(InputFileDir);
	vector<string>::size_type it;
	ifstream fileopen;
	int doc_start = 0;
	int doc_end = 0;
	int cur_DOCNO;
	stringstream stream;
	int DOC_ID;
	int TERM_ID;
	string DOC_NAME;
	int allocated_term_id = 0;
	int doc_len = 0;
	unordered_map<string, int> cur_TF;

	unordered_map<string, int>::iterator map_itor;

	string prev_DOCNAME;
	int pre_DOCID;
	int start = 0;

	ofstream TF_txt(OutputDir + "Index");
	ofstream DOC_txt(OutputDir + "Doc.txt");
	ofstream TERM_txt(OutputDir + "Word.txt");

	char inputString[MAX_READLINE_LENGTH];
	unordered_map<string, int>::iterator it_;

	for (it = 0; it < stemmedfile_list.size(); it++)
	{
		printf("%s file processing...\n", stemmedfile_list[it]);
		fileopen.open(InputFileDir + stemmedfile_list[it]);
		while (!fileopen.eof()) {
			fileopen.getline(inputString, MAX_READLINE_LENGTH);
			stream.clear();
			stream.str(inputString);
			string str;
			int cur_term_ID;
			if (strstr(inputString, "[DOCNO]"))
			{
				stream >> str;	stream >> str; stream >> DOC_NAME;
				if (start == 1 && DOC_NAME.compare(prev_DOCNAME))
				{
					/*현재 문서 TF 파일 만들기*/
					it_ = cur_TF.begin();
					while (it_ != cur_TF.end())
					{
						TF_txt << prev_DOCNAME << "\t" << (*it_).first << "\t" << (*it_).second << endl;
						it_++;
						tf_file_size++;
					}
					Counting_DF(cur_TF);
					cur_TF.clear();
					/*문서 정보 파일 내용 추가*/
					DOC_txt << pre_DOCID << "\t" << prev_DOCNAME << "\t" << doc_len << endl;
					doc_len = 0;
				}
				DOC_ID = allocated_doc_id++;
				ID_DOC_map[DOC_NAME] = DOC_ID; //문서 ID 할당
				pre_DOCID = DOC_ID;
				prev_DOCNAME = DOC_NAME;
				start = 1;
			}
			else
			{
				if ((strstr(inputString, "[HEADLINE]")))
				{
					stream >> str;	stream >> str;
				}
				if (strstr(inputString, "[TEXT]"))
					continue;
				while (stream >> str)
				{
					it_ = ID_TERM_map.find(str);
					if (it_ == ID_TERM_map.end())
					{
						ID_TERM_map[str] = allocated_term_id++;
						cur_term_ID = ID_TERM_map[str];
					}
					else
					{
						cur_term_ID = ID_TERM_map[str];
					}

					Counting_Map(CF, str);
					Counting_Map(cur_TF, str);
					doc_len++;
				}
			}
		}
		fileopen.close();
	}

	/*마지막 문서 TF 파일 만들기*/
	it_ = cur_TF.begin();
	while (it_ != cur_TF.end())
	{
		TF_txt << prev_DOCNAME << "\t" << (*it_).first << "\t" << (*it_).second << endl;
		it_++;
		tf_file_size++;
	}
	Counting_DF(cur_TF);
	cur_TF.clear();
	/*문서 정보 파일 내용 추가*/
	DOC_txt << pre_DOCID << "\t" << prev_DOCNAME << "\t" << doc_len << endl;
	doc_len = 0;

	printf("Doc.txt file creating...\n");
	DOC_txt.close();
	cur_TF.clear();

	string str;
	int id;
	int df;
	int cf;
	map_itor = ID_TERM_map.begin();
	unordered_map<string, int>::iterator tmp;
	printf("Word.txt file creating...\n");
	while (map_itor != ID_TERM_map.end())
	{
		str = (*map_itor).first;
		id = (*map_itor).second;
		if ((tmp = CF.find(str)) == CF.end()) //CD
		{
			printf("cf not found !\n");
		}
		else
		{
			cf = CF[str];
		}

		if ((tmp = DF.find(str)) == DF.end()) //DF
		{
			printf("df not found !\n");
		}
		else
		{
			df = DF[str];
		}

		TERM_txt << id << "\t" << str << "\t" << df << "\t" << cf << endl;
		map_itor++;
	}
	TF_txt.close();
	TERM_txt.close();


}
vector<string> Index::StemmedFileListRead(string InputFileDir)
{
	_finddata_t fd;
	long handle;
	int result = 1;
	int file_cnt = 0;
	char str[50];
	strcpy(str, (InputFileDir + "*.*").c_str());
	handle = _findfirst(str, &fd);
	file_cnt;

	vector<string> stemmedfile_list;

	if (handle == -1) // 파일 없을시
	{
		printf("There were no files.\n");
	}
	else // 파일있을 시 
	{
		while (result != -1)
		{
			if (strstr(fd.name, "_stemmed")) {
				stemmedfile_list.push_back(fd.name);
			}
			result = _findnext(handle, &fd);
		}
	}
	_findclose(handle);
	return stemmedfile_list;
}

