
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


	cout << "�t�@�C��������͂��Ă��������D" << endl;
	cin >> fileName;
	cout << "�t�@�C���̐�����͂��Ă��������D" << endl;
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



	cout << "�e�L�X�g�̐������I�����܂����D" << endl;
	cout << "�����L�[�������Ă��������D" << endl;
	cin >> fileName;
}