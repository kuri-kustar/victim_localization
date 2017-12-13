#include "victim_localization/victim_thermal_detector.h"


victim_thermal_detector::victim_thermal_detector(ros::NodeHandle &nh_, ros::NodeHandle &pnh_)
{
  ros::param::param<double>("~min_Dist_Between_Blobs", minDistBetweenBlobs_ , 40.0);
  ros::param::param<double>("~thermal_min_Area_Victim", minAreaVictim_ , 40.0);

  image_transport::ImageTransport it(nh_);
  image_transport::ImageTransport p_it(pnh_);
  sub_image = it.subscribe("thermal_camera/image_raw", 1, &victim_thermal_detector::imageCallback,this);
  pub_detection_ = it.advertise("image_detection1", 10);
}


victim_thermal_detector::~victim_thermal_detector(){}

void victim_thermal_detector::imageCallback(const sensor_msgs::ImageConstPtr& img){
  cv_bridge::CvImageConstPtr cv_ptr;
  try
  {
    cv_ptr = cv_bridge::toCvShare(img, sensor_msgs::image_encodings::MONO8);
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("cv_bridge exception: %s", e.what());
    return;
  }
  img_proc(cv_ptr->image);
}


  void victim_thermal_detector::PerformBlobDetection(void){
  victim_found=false;  //initially set victim_found to false

  //Perform blob detection
  cv::SimpleBlobDetector::Params params;
  params.filterByColor = true;
  params.blobColor = 255;
  params.minDistBetweenBlobs = minDistBetweenBlobs_;
  params.filterByArea = true;
  params.minArea = minAreaVictim_;
  params.maxArea = img_proc.rows * img_proc.cols;
  params.filterByCircularity = false;
  params.filterByColor = false;
  params.filterByConvexity = false;
  params.filterByInertia = false;

  cv::Ptr<cv::SimpleBlobDetector> blob_detector = cv::SimpleBlobDetector::create(params);
  std::vector<cv::KeyPoint> keypoints;
  blob_detector->detect(img_proc,keypoints);

  for(unsigned int i=0; i<keypoints.size();i++)
  {
      cv::KeyPoint k = keypoints.at(i);
      std::cout << "blob found at (x,y)= " << k.pt.x << "," << k.pt.y << std::endl;
      ROS_DEBUG("Heat blob found at image coord: (%f, %f)", k.pt.x , k.pt.y);
     // ROS_DEBUG("Heat blob numer: %f", keypoints.size());
      cv::Rect rect_image(0, 0, img_proc.cols, img_proc.rows);
      cv::Rect rect_roi(k.pt.x - (k.size - 1)/2, k.pt.y - (k.size - 1)/2, k.size - 1, k.size - 1);

      //See http://stackoverflow.com/questions/29120231/how-to-verify-if-rect-is-inside-cvmat-in-opencv
      bool is_inside = (rect_roi & rect_image) == rect_roi;

      if (!is_inside){
        ROS_ERROR("ROI image would be partly outside image border, aborting further processing!");
        continue;
      }

      const cv::Mat roi = img_proc(rect_roi);
      int histSize = 256;
      float range[] = { 0, 256 };
      const float* histRange = { range };
      cv::Mat hist;
      cv::calcHist(&roi, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, true, false);

   victim_found=true;
   victim_loc[0]=k.pt.x;
   victim_loc[1]=k.pt.y;
  }

  if (keypoints.size() == 0) {
      std::cout << "No blob detected" << std::endl;
  }

  if(pub_detection_.getNumSubscribers() > 0){

  //Create image with detection frames
      int width = 3;
      int height = 3;

      IplImage ipl_img = img_proc;

     //Display Keypoints
      for(unsigned int i = 0; i < keypoints.size(); i++){
          if (keypoints.at(i).size > 1){

              //Write rectangle into image
              width = (int)(keypoints.at(i).size );
              height = (int)(keypoints.at(i).size );
              for(int j = -width; j <= width;j++){
                   if ((keypoints.at(i).pt.x + j) >= 0  &&  (keypoints.at(i).pt.x + j) < ipl_img.width){
                      //Draw upper line
                      if ((keypoints.at(i).pt.y - height) >= 0){
                           cvSet2D(&ipl_img,(int)(keypoints.at(i).pt.y - height), (int)(keypoints.at(i).pt.x + j),cv::Scalar(0));
                      }
                      //Draw lower line
                      if ((keypoints.at(i).pt.y + height) < ipl_img.height){
                           cvSet2D(&ipl_img,(int)(keypoints.at(i).pt.y + height), (int)(keypoints.at(i).pt.x + j),cv::Scalar(0));
                      }
                   }
              }

              for(int k = -height; k <= height;k++){
                  if ((keypoints.at(i).pt.y + k) >= 0  &&  (keypoints.at(i).pt.y + k) < ipl_img.height){
                      //Draw left line
                      if ((keypoints.at(i).pt.x - width) >= 0){
                           cvSet2D(&ipl_img,(int)(keypoints.at(i).pt.y +k), (int)(keypoints.at(i).pt.x - width),cv::Scalar(0));
                      }
                       //Draw right line
                      if ((keypoints.at(i).pt.x + width) < ipl_img.width){
                           cvSet2D(&ipl_img,(int)(keypoints.at(i).pt.y +k), (int)(keypoints.at(i).pt.x + width),cv::Scalar(0));
                      }
                  }
              }
          }
      }



      cv_bridge::CvImage cvImg;
      cvImg.image = img_proc;



      cvImg.header = img->header;
      cvImg.encoding = sensor_msgs::image_encodings::MONO8;
      pub_detection_.publish(cvImg.toImageMsg());
   }
}

detector_status victim_thermal_detector::GetDetectionResult(){
  detector_status status;
  status.victim_found = victim_found;
  status.victim_loc=victim_loc;
  return status;
}