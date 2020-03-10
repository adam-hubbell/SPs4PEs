#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib> 
#include <chrono>
#include <ctime>
#include <omp.h>
#include  <tgmath.h>
using namespace std;
using namespace std::chrono;


/*
All matrix arrays are oriented [width][height]

*/

double calc_conv(int x, int y, int *** matrix, int mat_height, int mat_width){
	double conv_matrix[3][3]= {
			  {0, 0, 0},
			  {0, 0, 0},
			  {0, 0, 0}
			};
	int xloc;
	int yloc; 
	double xdiff;
	double ydiff;
	//this nested loop fills the array to calculate the convolution from
	//it handles overflow locations by setting their value to the value on the edge nearest the overflow
	for(int i=0; i<3;i++){ 
		for(int j=0;j<3;j++){
			xloc = (i-1) + x;
			if(xloc < 0){
				xloc = 0;
			}else if(xloc == mat_width){
				xloc = mat_width - 1 ;
			}
			yloc = (j-1) + y;
			if(yloc < 0){
				yloc = 0;
			}else if(yloc == mat_height){
				yloc = mat_height - 1;
			}
			conv_matrix[i][j] = (*matrix)[xloc][yloc];
		}
	}
	ydiff = (double) abs(conv_matrix[0][0]+2*conv_matrix[0][1]+conv_matrix[0][2] - conv_matrix[2][0] - 2*conv_matrix[2][1] - conv_matrix[2][2]);
	xdiff = (double) abs(conv_matrix[0][0]+2*conv_matrix[1][0]+conv_matrix[2][0] - conv_matrix[0][2] - 2*conv_matrix[1][2] - conv_matrix[2][2]);
	return xdiff + ydiff;

}

void rgb_from_int(int output[3], int input){
	output[0] = input/(int)pow(2,16);
	output[1] = (input % (int) pow(2,16))/pow(2,8);
	output[2] = input % (int) pow(2,8);
}

double calc_conv_rgb(int x, int y, int *** matrix, int mat_height, int mat_width){

	int xloc;
	int yloc; 
	double xdiff = 0;
	double ydiff = 0;
	int rgb_matrix[3][3][3] = {
		{{0,0,0},{0,0,0},{0,0,0}},
		{{0,0,0},{0,0,0},{0,0,0}},
		{{0,0,0},{0,0,0},{0,0,0}}
	};
	//this nested loop fills the array to calculate the convolution from
	//it handles overflow locations by setting their value to the value on the edge nearest the overflow
	for(int i=0; i<3;i++){ 
		for(int j=0;j<3;j++){
			xloc = (i-1) + x;
			if(xloc < 0){
				xloc = 0;
			}else if(xloc == mat_width){
				xloc = mat_width - 1 ;
			}
			yloc = (j-1) + y;
			if(yloc < 0){
				yloc = 0;
			}else if(yloc == mat_height){
				yloc = mat_height - 1;
			}
			rgb_from_int(rgb_matrix[i][j],(*matrix)[xloc][yloc]);
		}
	}

	for(int i=0;i<3;i++){
		ydiff += (double) abs(rgb_matrix[0][0][i]+2*rgb_matrix[0][1][i]+rgb_matrix[0][2][i] - rgb_matrix[2][0][i] - 2*rgb_matrix[2][1][i] - rgb_matrix[2][2][i]);
		xdiff += (double) abs(rgb_matrix[0][0][i]+2*rgb_matrix[1][0][i]+rgb_matrix[2][0][i] - rgb_matrix[0][2][i] - 2*rgb_matrix[1][2][i] - rgb_matrix[2][2][i]);
	}
	return xdiff + ydiff;

}


int main(int argc, char *argv[]){

	std::ifstream pgm_file;
	std::ofstream out_file;
	std::string ofname;
	std::string line;
	int img_width;
	int img_height;
	int ** img_matrix;
	double ** conv_matrix;
	int threshold;
	int pxl;
	char buff[1000];
	bool grayscale;

  	high_resolution_clock::time_point start;
  	high_resolution_clock::time_point end;
  	typedef std::chrono::duration<float> float_seconds;
  	float_seconds singleT;
  	float_seconds multiT;

	if(argc < 3){
		printf("args should be ./programname filename threshold\n");
		exit(-1);
	}


	pgm_file.open(argv[1]);
	std::getline(pgm_file, line);
	std::getline(pgm_file, line);
	sscanf(line.c_str(), "%d %d\n", &img_width, &img_height);

	std::getline(pgm_file, line);
	if(std::stoi(line.c_str()) == 255){
		grayscale = true;
	}else{
		grayscale = false;
	}

	img_matrix = new int * [img_width];
	conv_matrix = new double * [img_width];
	for(int i=0;i<img_width;i++){
		img_matrix[i] = new int[img_height];
		conv_matrix[i] = new double[img_height];
	}


	for(int h=0; h<img_height; h++){
		std::getline(pgm_file, line);
		stringstream ssin(line);
		for(int w=0;w<img_width;w++){
			if(ssin.good() == false){
				break;
			}
			ssin >> pxl;
			img_matrix[w][h] = pxl;
		}
	}

/*
SINGLE THREADED
*/

	start = high_resolution_clock::now();
	for(int w=0; w<img_width;w++){
		for(int h=0;h<img_height;h++){
			if(grayscale){
				conv_matrix[w][h] = calc_conv(w, h, &img_matrix, img_height, img_width);
			}else{
				conv_matrix[w][h] = calc_conv_rgb(w, h, &img_matrix, img_height, img_width);
			}
		}
	}
	end = high_resolution_clock::now();
	singleT = end - start;
	printf("Single thread took %f seconds\n", singleT.count());

/*
MULTI THREADED
*/

	start = high_resolution_clock::now();
	#pragma omp parallel for
	for(int w=0; w<img_width;w++){
		for(int h=0;h<img_height;h++){
			if(grayscale){
				conv_matrix[w][h] = calc_conv(w, h, &img_matrix, img_height, img_width);
			}else{
				conv_matrix[w][h] = calc_conv_rgb(w, h, &img_matrix, img_height, img_width);
			}
		}
	}
	end = high_resolution_clock::now();
	multiT = end - start;
	printf("%d threads took %f seconds\n",omp_get_max_threads() , multiT.count());
	printf("for a speedup of %f\n",singleT.count()/multiT.count());

/* ________________________________________________________________

	USED THIS BIT TO CALCULATE SOME STATS TO FIND A GOOD THRESHOLD
__________________________________________________________________

*/

	double sum = 0;
	double max = -1;
	double sd = 0;
	double cnt = 0;
	double mean = 0;

	for(int w=0; w<img_width;w++){
		for(int h=0;h<img_height;h++){
			sum += conv_matrix[w][h];
			cnt += 1.0;
			if(conv_matrix[w][h] > max){
				max = conv_matrix[w][h];
			}
		}
	}

	mean = sum/cnt;
	for(int w=0; w<img_width;w++){
		for(int h=0;h<img_height;h++){
			sd += pow(mean - conv_matrix[w][h],2);
		}
	}
	sd /= cnt;
	printf("mean:%f, sd:%f, max:%f\n",mean,sd,max);
//*/	

	threshold = stoi(argv[2]);
	sprintf(buff,"OUT_%d_%s",threshold,argv[1]);
	printf("%s\n",buff);
	out_file.open(buff);

	out_file << "P2\n";
	out_file << img_width;
	out_file << " ";
	out_file << img_height;
	out_file << "\n";
	out_file << "255\n";

	for(int h=0;h<img_height;h++){
		for(int w=0; w<img_width;w++){
			if(conv_matrix[w][h] > threshold){
				out_file << 0;
			}else{
				out_file << 255;
			}
			if(w < img_width-1){
				out_file << " ";
			}
		}
		out_file << "\n";
	}

}