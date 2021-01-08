#pragma once
#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;
void cartoonifyImage(InputArray src, OutputArray dst, bool isAlien = false);
void cartoonDevImage(InputArray src, OutputArray dst);
void cartoonAlien(InputArray src, OutputArray dst);