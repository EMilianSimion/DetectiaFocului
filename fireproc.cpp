#include "fireproc.h"

Mat computeROI(Mat inputMatrix, double binFrame, int type) {

	unsigned int height = inputMatrix.rows;
	unsigned int width = inputMatrix.cols;
	Mat outputMatrix(inputMatrix.rows, inputMatrix.cols, inputMatrix.type());

	switch (type)
	{
	case CSPACE_HSV:
		for (unsigned int y = 0; y < height; y++)
			for (unsigned int x = 0; x < width; x++) {
				byte r, g, b;
				float R, G, B;
				float H, S, V;
				float Cmax, Cmin;
				float delta;

				b = inputMatrix.at<Vec3b>(y, x)[0];
				g = inputMatrix.at<Vec3b>(y, x)[1];
				r = inputMatrix.at<Vec3b>(y, x)[2];

				/* The R, G, B, values are divided by 255 to change the range from 0..255 to 0..1 */
				R = r / 255.0f;
				G = g / 255.0f;
				B = b / 255.0f;

				Cmax = MAX(R, MAX(G, B));
				Cmin = MIN(R, MIN(G, B));
				delta = Cmax - Cmin;

				if (delta == 0) {
					H = 0;
					S = 0;
				}
				else {
					if (Cmax == R)
						H = 60.0f * ((G - B) / delta);
					if (Cmax == G)
						H = 60.0f * (2.0f + (B - R) / delta);
					if (Cmax == B)
						H = 60.0f * (4.0f + (R - G) / delta);
					S = (Cmax == 0) ? 0 : 100.0f * delta / Cmax;
				}

				H = (H < 0) ? H += 360.0f : H;
				V = Cmax * 100;

				B = H / 2;
				G = S * 2;
				R = V * 2;

				if ((R > binFrame) && (R > G) && (G > B)) {
					outputMatrix.at<Vec3b>(y, x)[0] = 255;
					outputMatrix.at<Vec3b>(y, x)[1] = 255;
					outputMatrix.at<Vec3b>(y, x)[2] = 255;
				}
				else {
					outputMatrix.at<Vec3b>(y, x)[0] = 0;
					outputMatrix.at<Vec3b>(y, x)[1] = 0;
					outputMatrix.at<Vec3b>(y, x)[2] = 0;
				}
			}

		return outputMatrix;
		break;

	case CSPACE_YCbCr:
		for (unsigned int y = 0; y < height; y++)
			for (unsigned int x = 0; x < width; x++) {
				
				float delta = 0.5f;

				float B = inputMatrix.at<Vec3b>(y, x)[0];
				float G = inputMatrix.at<Vec3b>(y, x)[1];
				float R = inputMatrix.at<Vec3b>(y, x)[2];

				float Y = 0.299f * R + 0.587f * G + 0.114f * B;
				float Cb = (B - Y) * 0.564f + delta;
				float Cr = (R - Y) * 0.713f + delta;

				if ((Y > binFrame) && (Y >= Cb) && (Cr >= Cb) && (255.0 * abs(Cb - Cr) > binFrame)) {
					outputMatrix.at<Vec3b>(y, x)[0] = 255;
					outputMatrix.at<Vec3b>(y, x)[1] = 255;
					outputMatrix.at<Vec3b>(y, x)[2] = 255;
				}
				else {
					outputMatrix.at<Vec3b>(y, x)[0] = 0;
					outputMatrix.at<Vec3b>(y, x)[1] = 0;
					outputMatrix.at<Vec3b>(y, x)[2] = 0;
				}
			}

		return outputMatrix;
		break;

	default:
		cerr << "WARNING: Couldn't compute region of interest." << endl;
		return inputMatrix;
		break;
	}
}

Mat computeMovement(Mat inputMatrix, Mat firstFrame, Mat secondFrame)
{
	Mat f_gray, s_gray, deltaFrame, binFrame;
	vector<vector<Point>> contours;

	if (!firstFrame.empty() && !secondFrame.empty())
	{
		/* Preprocess both frames with noise reduction */
		GaussianBlur(firstFrame, firstFrame, Size(21, 21), 0);
		GaussianBlur(secondFrame, secondFrame, Size(21, 21), 0);

		/* Change color space to GRAYSCALE */
		cvtColor(firstFrame, f_gray, COLOR_BGR2GRAY);
		cvtColor(secondFrame, s_gray, COLOR_BGR2GRAY);

		/* Compute difference between first and second frame */
		absdiff(f_gray, s_gray, deltaFrame);

		/* Threshold segmentation on deltaFrame */
		threshold(deltaFrame, binFrame, 25, 255, THRESH_BINARY);

		dilate(binFrame, binFrame, Mat(), Point(-1, -1), 2);
		findContours(binFrame, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		Mat outputMatrix;
		secondFrame.copyTo(outputMatrix, binFrame);

		return outputMatrix;
	}

	/* If empty frames are fed into the function, raise warning and make no changes */
	cerr << "WARNING: One or more empty frames." << endl;
	return inputMatrix;
}


Mat computeBBox(Mat inputMatrix)
{
	Mat grayMatrix, cannyMatrix;
	Mat outputMatrix = inputMatrix.clone();
	vector<vector<Point>> contours;

	cvtColor(inputMatrix, grayMatrix, COLOR_RGB2GRAY);
	Canny(grayMatrix, cannyMatrix, 30, 128, 3, false);
	findContours(cannyMatrix, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	/*Rect BBox = boundingRect(cannyMatrix);
	rectangle(outputMatrix, BBox, Scalar(0, 0, 255), 2);*/

	for (int i = 0; i < contours.size(); ++i)
	{
		Rect BBox = boundingRect(contours[i]);
		rectangle(outputMatrix, BBox, Scalar(0, 0, 255), 2);
	}

	return outputMatrix;
}

Mat mClose(Mat inputMatrix)
{
	Mat outputMatrix(inputMatrix.rows, inputMatrix.cols, inputMatrix.type());
	Mat elem = getStructuringElement(MORPH_RECT, Size(5, 5));
	
	morphologyEx(inputMatrix, outputMatrix, MORPH_CLOSE, elem);
	dilate(outputMatrix, outputMatrix, elem);
	
	return outputMatrix;
}