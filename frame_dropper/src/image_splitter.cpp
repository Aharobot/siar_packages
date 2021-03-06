#include <ros/ros.h>
#include <std_msgs/Bool.h>
#include <image_transport/image_transport.h>
// OpenCV stuff
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
// OpenCV to ROS stuff
#include <cv_bridge/cv_bridge.h>

class ImageSplitter
{
public:
  ImageSplitter(void) : it(nh)
  {  
    // Resolve the camera topic
    cameraTopic = nh.resolveName("in");
    imageTopic = cameraTopic+"/rgb/image_raw";
    depthTopic = cameraTopic + "/depth_registered/image_raw";
    cameraTopic = nh.resolveName("out");
    imageOutTopic = cameraTopic+"/rgb/image_raw";
    depthOutTopic = cameraTopic + "/depth_registered/image_raw";
    
    cameraTopic_2 = nh.resolveName("in_2");
    imageTopic_2 = cameraTopic_2 +"/rgb/image_raw";
    depthTopic_2 = cameraTopic_2 + "/depth_registered/image_raw";
    cameraTopic_2 = nh.resolveName("out_2");
    imageOutTopic_2 = cameraTopic_2 + "/rgb/image_raw";
    depthOutTopic_2 = cameraTopic_2 + "/depth_registered/image_raw";
    
    reverseTopic = "/reverse";
    allCamerasTopic = "/all_cameras";
    publishDepthTopic = "/publish_depth";
    
    // Load parameters
    ros::NodeHandle lnh("~");
    if(!lnh.getParam("frame_skip", frameSkip))
      frameSkip = 3;     
    if(!lnh.getParam("use_depth", use_depth)) 
      use_depth = false;
    if(!lnh.getParam("publish_depth", publish_depth)) 
      publish_depth = false;
    if(!lnh.getParam("scale", scale))
      scale = 1.0;
    if(!lnh.getParam("publish_all", publish_all))
      publish_all = false;
    
    reverse = false;
    
    
    // Create bool subscribers
    sub_all = nh.subscribe(allCamerasTopic, 1, &ImageSplitter::publishAllCallback, this);
    sub_reverse = nh.subscribe(reverseTopic, 1, &ImageSplitter::reverseCallback, this);
    sub_depth = nh.subscribe(publishDepthTopic, 1, &ImageSplitter::publishDepthCallback, this);
    
    // Create image subscribers
    sub = it.subscribe(imageTopic, 1, &ImageSplitter::imageCb, this); 
    if (use_depth) 
      depth_sub = it.subscribe(depthTopic, 1, &ImageSplitter::depthCb, this);
    sub_2 = it.subscribe(imageTopic_2, 1, &ImageSplitter::imageCb_2, this); 
    if (use_depth) 
      depth_sub_2 = it.subscribe(depthTopic_2, 1, &ImageSplitter::depthCb_2, this);
    
    // Advertise image topics
//     ROS_INFO("Depth topic: %s Depth out topic: %s PUblish depth = %d", depthTopic.c_str(), depthOutTopic.c_str(), publish_depth);
    pub = it.advertise(imageOutTopic, 1);
    if (use_depth)
      depth_pub = it.advertise(depthOutTopic, 1);
    pub_2 = it.advertise(imageOutTopic_2, 1);
    if (use_depth)
      depth_pub_2 = it.advertise(depthOutTopic_2, 1);
    
    imgCount = 0;
    depthCount = 0;
    imgCount_2 = 0;
    depthCount_2 = 0;
  }
  
  ~ImageSplitter(void)
  {
  }
  
  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {
      
    imgCount++;
    if(imgCount < frameSkip)
    {
      return;
    }
    imgCount = 0;
    cv_bridge::CvImagePtr img_cv = cv_bridge::toCvCopy(msg, "bgr8");
    cv::Mat dst(msg->width * scale,msg->height * scale,img_cv->image.type());
    cv::resize(img_cv->image, dst, cv::Size(0,0), scale, scale);
    img_cv->image = dst;
    
    sensor_msgs::ImagePtr pt = img_cv->toImageMsg();
    if (reverse && publish_all) 
      pub_2.publish(pt);
    if (!reverse)
      pub.publish(pt);
  }
  
  void depthCb(const sensor_msgs::ImageConstPtr& msg)
  {
      
    depthCount++;
    if(depthCount < frameSkip)
    {
      return;
    }
    depthCount = 0;
    cv_bridge::CvImagePtr img_cv = cv_bridge::toCvCopy(msg, "32FC1");
    cv::Mat dst(msg->width * scale,msg->height * scale,img_cv->image.type());
    cv::resize(img_cv->image, dst, cv::Size(0,0), scale, scale);
    img_cv->image = dst;
    
    sensor_msgs::ImagePtr pt = img_cv->toImageMsg();
    if (publish_depth)
    {
      if (reverse && publish_all) 
	depth_pub_2.publish(pt);
      if (!reverse)
	depth_pub.publish(pt);
    }
  }
  
  void imageCb_2(const sensor_msgs::ImageConstPtr& msg)
  {
      
    imgCount_2++;
    if(imgCount_2 < frameSkip)
    {
      return;
    }
    imgCount_2 = 0;
    
    cv_bridge::CvImagePtr img_cv = cv_bridge::toCvCopy(msg, "bgr8");
    cv::Mat dst(msg->width * scale,msg->height * scale,img_cv->image.type());
    cv::resize(img_cv->image, dst, cv::Size(0,0), scale, scale);
    img_cv->image = dst;
     
    sensor_msgs::ImagePtr pt = img_cv->toImageMsg();
    
    if (reverse)
      pub.publish(pt);
    if (!reverse && publish_all)
      pub_2.publish(pt);
  }
  
  void depthCb_2(const sensor_msgs::ImageConstPtr& msg)
  {
    depthCount_2++;
    if(depthCount_2 < frameSkip)
    {
      return;
    }
    depthCount_2 = 0;
    cv_bridge::CvImagePtr img_cv = cv_bridge::toCvCopy(msg, "32FC1");
    cv::Mat dst(msg->width * scale,msg->height * scale,img_cv->image.type());
    cv::resize(img_cv->image, dst, cv::Size(0,0), scale, scale);
    img_cv->image = dst;
    
    sensor_msgs::ImagePtr pt = img_cv->toImageMsg();
    if (publish_depth) 
    {
      if (reverse)
	depth_pub.publish(pt);
      else if (publish_all)
	depth_pub_2.publish(pt);
    }
  }
  
  void reverseCallback(const std_msgs::BoolConstPtr &msg)
  {
//     ROS_INFO("Changing the cameras. New value = %d", msg->data);
    reverse = msg->data;
  }
  
  void publishAllCallback(const std_msgs::BoolConstPtr &msg)
  {
    publish_all = msg->data;
  }
  
  void publishDepthCallback(const std_msgs::BoolConstPtr &msg)
  {
    publish_depth = msg->data;
  }
protected:

  // ROS handler and subscribers
  ros::NodeHandle nh;
  image_transport::ImageTransport it; 
  image_transport::Publisher pub;
  image_transport::Subscriber sub;
  image_transport::Publisher depth_pub;
  image_transport::Subscriber depth_sub;
  image_transport::Publisher pub_2;
  image_transport::Subscriber sub_2;
  image_transport::Publisher depth_pub_2;
  image_transport::Subscriber depth_sub_2;
  ros::Subscriber sub_reverse, sub_depth, sub_all;
  
  // Topics 
  std::string cameraTopic, imageTopic, imageOutTopic;
  std::string depthTopic, depthOutTopic;
  std::string cameraTopic_2, imageTopic_2, imageOutTopic_2;
  std::string depthTopic_2, depthOutTopic_2;
  std::string publishDepthTopic, allCamerasTopic, reverseTopic;
  
  int frameSkip;
  int imgCount, depthCount;
  int imgCount_2, depthCount_2;
  double scale; // Scaling in the image
  bool publish_depth, use_depth, reverse, publish_all;
};

int main(int argc, char **argv)
{
  ros::init(argc, argv, "image_splitter");  
  ImageSplitter node;
  ros::spin();
}
