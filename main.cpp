#include "fireproc.h"

/* Path to video file */
constexpr auto FILE_PATH = "footage/footage_02.mp4";

/* MouseListener */
void CallBackFunc(int event, int x, int y, int flags, void* userdata) {
	
	/* Press left click to pause video */
	if (event == EVENT_LBUTTONDOWN)
		waitKey(0);
}

int main(void)
{
	/* Objects for data capture. Set VideCapture to 0 for webcam acces. */
	VideoCapture video(FILE_PATH);
	Mat frame, firstFrame, secondFrame;
	
	/* Return failure and print ERROR */
	if (!video.isOpened()) {
		cerr << "ERROR: Failed to open stream or load file..." << endl;
		return EXIT_FAILURE;
	}

	while (true) {
		/* Objects for storing results */
		Mat moving, detect, fire;

		/* Read frame by frame. Every 3rd frame will be stored as secondFrame for further comparison between frames.*/
		video >> frame;
		video >> secondFrame;
		video >> secondFrame;
		frame.copyTo(firstFrame);

		/* Return failure if frame is empty */
		if (frame.empty())
			return EXIT_FAILURE;

		/* Resize captured footage for faster processing */
		/*Size inHalf(frame.cols / 4, frame.rows / 4);
		resize(frame, frame, inHalf);
		resize(secondFrame, secondFrame, inHalf);
		resize(firstFrame, firstFrame, inHalf);*/

		/* Detect fire regions */
		moving = computeMovement(frame, firstFrame, secondFrame);
		detect = computeROI(frame, 175, CSPACE_YCbCr);
		moving = mClose(moving);

		//bitwise_or(moving, detect, fire, Mat());
		moving.copyTo(fire, detect);
		fire = computeBBox(fire);

		/* Display footage and results */
		imshow("Footage", frame);
		imshow("Motion Detection", moving);
		imshow("ROI Detecion", detect);
		imshow("Fire Detecion", fire);

		/* Free memory after each iteration */
		moving.release();
		detect.release();
		fire.release();

		/* Press ESC to exit */
		if ((char)waitKey(1) == KEY_ESC)
			break;

		/* Pause video using MouseListener */
		setMouseCallback("Footage", CallBackFunc, NULL);
		waitKey(0);
	}

	/* Free memory when done */
	video.release();
	destroyAllWindows();

	/* Return succes */
	return EXIT_SUCCESS;
}