The script here is just an example but isn't usable for actual webcam calib. I used calibtools library instead;
Usage:

1- Record a video of a 8x6 chessboard with the webcam (using Camera app for instance)
2- in the folder of the video, open cmd and do:

calibtools calib calib_webcam.mp4 output.json

3- Result is in a JSON file