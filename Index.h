#pragma once
#include <unordered_map>
#include <vector>
using namespace std;
namespace index {
	class Index {
	public:
		unordered_map<int, float> total_weight;
		int allocated_doc_id;
		unordered_map<string, int> ID_TERM_map;
		unordered_map<string, int> ID_DOC_map;
		unordered_map<string, int> DF;
		unordered_map<string, int> CF;
		int tf_file_size;
		int indexfile_cnt;
		tuple<string, string, int> Indexfile;


		Index();
		void Index::Counting_Map(unordered_map<string, int> &map, string str);
		void Total_weight(unordered_map<int, float> &total_weight, string TF_fileDir);
		void InvertedIndexing( string OutputDir);
		void Index::Counting_DF(unordered_map<string, int> map);
		void Index::ExtractIndexFile(string InputFileDir, string OutputDir);
		vector<string> Index::StemmedFileListRead(string InputFileDir);
	};
}