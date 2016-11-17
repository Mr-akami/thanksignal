
/*
This program generate .xml text for ThankSignal images.
*/



#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>

using namespace std;

int main() {

	string fileName;
	int num;


	cout << "ファイル名を入力してください．" << endl;
	cin >> fileName;
	cout << "ファイルの数を入力してください．" << endl;
	cin >> num;

	ofstream outputfile(fileName + ".txt");

	for (int i = 0; i < num; i++) {
		outputfile << "<File  path=\""<<  fileName << "_" <<setfill('0') << setw(5) << right << i << ".jpg\" " << "type=\"graphics_image_t*\" "  << "symbol=\"" << fileName << "_" << setfill('0') << setw(5) << right << i <<"\"/>"<< endl;
	}

	for (int i = 0; i < 3; i++) {
		outputfile << endl;
	}

	outputfile << "static const graphics_image_t* " << fileName << "[" << num << "] = {" << endl;
	for (int i = 0; i < num; i++) {
		if(i<num-1) {
			outputfile << fileName << "_" << setfill('0') << setw(5) << right << i << "," << endl;
		}
		else {
			outputfile << fileName << "_" << setfill('0') << setw(5) << right << i << endl;
		}
	}
	outputfile << "};" << endl;

	outputfile.close();



	cout << "テキストの生成が終了しました．" << endl;
	cout << "何かキーを押してください．" << endl;
	cin >> fileName;
}