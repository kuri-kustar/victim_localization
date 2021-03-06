#ifndef VIEW_GENERATOR_IG_H
#define VIEW_GENERATOR_IG_H


#include <iostream>
#include <ros/ros.h>
#include <Eigen/Geometry>
#include <geometry_msgs/Pose.h>
#include <tf_conversions/tf_eigen.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include "rviz_visual_tools/rviz_visual_tools.h"

#include <victim_localization/common.h>
#include <octomap_world/octomap_manager.h>
#include <victim_localization/nbv_history.h>

#include <victim_localization/volumetric_map_manager.h>
#include <grid_map_ros/grid_map_ros.hpp>
#include <grid_map_msgs/GridMap.h>
#include <string.h>


//typedef geometry_msgs::Pose Pose;

using namespace grid_map;

namespace Generator{
enum GeneratorType{
  NONE,
  NN_Generator,
  NN_Adaptive_Generator,
  NN_Adaptive_Frontier_Generator,
  Frontier_Generator,
};
}

class view_generator_IG
{
public:


protected:
   double res_x_, res_y_, res_z_, res_yaw_;

   double nav_bounds_x_max_, nav_bounds_y_max_, nav_bounds_z_max_;
   double nav_bounds_x_min_, nav_bounds_y_min_, nav_bounds_z_min_;
   double dist_to_goal;

   double uav_fixed_height;
   double extensionRange_;
   Eigen::Vector3d boundingbox_;
   double boundingbox_x_,boundingbox_y_,boundingbox_z_;
   double dOvershoot_;

public:
   //....variables....
  volumetric_mapping::OctomapManager *manager_;
  nbv_history *nbv_history_;
  Volumetric_Map *occlusion_map;
  costmap_2d::Costmap2D *costmap_;
  costmap_2d::Costmap2DROS *costmap_ros_;

  double obj_bounds_x_max_, obj_bounds_y_max_, obj_bounds_z_max_;
  double obj_bounds_x_min_, obj_bounds_y_min_, obj_bounds_z_min_;
  double start_x_,start_y_,end_x_,end_y_;

  geometry_msgs::Pose current_pose_;
  std::vector<geometry_msgs::Pose> generated_poses;
  std::vector<geometry_msgs::Pose> rejected_poses;
  int nav_type;
  Generator::GeneratorType generator_type;

  // Visualizer
  int vis_marker_array_prev_size_;
  int vis_sphere_counter_;
  rviz_visual_tools::RvizVisualToolsPtr visualTools;

  //...methods...
  view_generator_IG();
  void setCurrentPose(geometry_msgs::Pose p);
  void setHistory(nbv_history* h);
  void setOcclusionMap(Volumetric_Map* Occ);


  bool isInsideBounds(geometry_msgs::Pose p);
  bool isSafe(geometry_msgs::Pose p);
  bool isCollide(geometry_msgs::Pose p);
  bool isValidViewpoint(geometry_msgs::Pose p , bool check_safety=true);
  void FixCurrentLoc();

  //victim_map parameters
  grid_map::GridMap victim_map;
  std::string map_layer;


  //generate views
  void generateViews(bool check);
  virtual void generateViews(); //viewpoints is  generated at current pose
  virtual std::string getMethodName();
  virtual void resetScaleFactor(){};
  virtual void setSampleResolution(double resX, double resY){};


 //visualize
 virtual void visualize(std::vector<geometry_msgs::Pose> valid_poses, std::vector<geometry_msgs::Pose> invalid_poses, geometry_msgs::Pose selected_pose);
 void visualizeSelectedpose(geometry_msgs::Pose selected_pose);
 void visualizeAllpose(std::vector<geometry_msgs::Pose> valid_poses, std::vector<geometry_msgs::Pose> invalid_poses);


  bool ComparePoses(geometry_msgs::Pose Pose1, geometry_msgs::Pose Pose2);
  void visualizeDrawSphere(geometry_msgs::Pose p, double r);
  void setOctomapManager(volumetric_mapping::OctomapManager *manager);

  void setvictimmap(grid_map::GridMap map,std::string layer_name);
  void setCostMapROS(costmap_2d::Costmap2DROS *CostMapROS_);
};

#endif // VIEW_GENERATOR_IG_H
