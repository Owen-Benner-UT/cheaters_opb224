#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <queue>
#include <math.h>
#include <algorithm>

using namespace std;

const int TABLE_SIZE = 0x1000000;
vector<int> table [TABLE_SIZE];

//Gets vector of filenames.
int getdir(string dir, vector<string> &files)
{
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir(dir.c_str())) == NULL)
	{
		cout << "Error(" << errno << ") opening " << dir << endl;
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		files.push_back(string(dirp->d_name));
	}
	closedir(dp);
	return 0;
}

//Hash function, multiplies current value of out by ASCII value of next
//character, then adds with value of next character.
unsigned long hash(queue<string> sequence)
{
	unsigned long out = 0;
	while(!sequence.empty())
	{
		string word = sequence.front();
		sequence.pop();
		for(int i = 0; i < word.length(); ++i)
		{
			out = out * word[i] + word[i];
		}
	}
	return out;
}

//Given hash code and file index, records collision in table.
void mark(unsigned long code, const int index)
{
	code %= TABLE_SIZE;
	vector<int>::iterator itr;
	itr = find(table[code].begin(), table[code].end(), index);
	if(itr == table[code].end())
		table[code].push_back(index);
}

//Parses files and hashes segments.
void parse(const string dir, const vector<string> files, const int index,
	const int seqLength)
{
	//cout << files[index] << " " << seqLength << endl;
	ifstream file;
	file.open((dir + "/" + files[index]).c_str());
	if(file.is_open())
	{
		string word;
		queue<string> words;
		for(int i = 0; i < seqLength; ++i)
		{
			getline(file, word, ' ');
			words.push(word);
		}
		mark(hash(words), index);
		while(file >> word)
		{
			words.pop();
			words.push(word);
			mark(hash(words), index);
		}
		file.close();
		//cout << endl;
	}
}

int main(const int argc, const char * argv[])
{
	string dir = argv[1];
	vector<string> files = vector<string>();

	getdir(dir,files);
	
	//Delete filenames which start with '.' from vector.
	cout << "Clearing hidden files...." << endl;
	for(int i = 0; i < files.size();)
	{
		if((files[i])[0] == '.')
			files.erase(files.begin() + i);
		else ++i;
	}

	//Parse each file.
	cout << "Parsing files...." << endl;
	for(unsigned int i = 0; i < files.size(); ++i)
	{
		if((files[i])[0] != '.')
			parse(dir, files, i, atoi(argv[2]));
	}

	//Find collisions.
	cout << "Analyzing table...." << endl;
	int cheaters [files.size()][files.size()];
	for(int i = 0; i < TABLE_SIZE; ++i)
	{
		if(table[i].empty()) continue;
		for(int j = 0; j < table[i].size(); ++j)
		{
			for(int k = j + 1; k < table[i].size(); ++k)
			{
				++cheaters[j][k];
				++cheaters[k][j];
			}
		}
	}

	//Report cheaters.
	for(int i = 0; i < files.size(); ++i)
		for(int j = i + 1; j < files.size(); ++j)
			if(files[i][j] > atoi(argv[3]))
			{
				cout << "Found " << (int) files[i][j];
				cout << " collisions between files \"";
				cout << files[i] << "\" and \"" << files[j];
				cout << "\"." << endl;
			}

	return 0;
}
