#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_datatypes.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// action client
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

// struttura di un goal letto dal file csv
struct Goal // Corrected struct name to uppercase 'Goal'
{
  double x;
  double y;
  double theta;
};

class Goal_Pub
{
public:
  Goal_Pub()
      : nh_("~"), 
        ac_("move_base", true), 
        current_goal_idx_(0)
  {
    // path del CSV
    nh_.param<std::string>("csv_file_path", csv_path_, "csv/goals.csv"); 
    Load_Goals_from_CSV(csv_path_); 

    if (goals_.empty()) 
    {
      ROS_ERROR("Nessun goal caricato nel CSV");
      ros::shutdown();
      return;
    }

    goal_ready_ = false;
    goal_timer_ = nh_.createTimer(ros::Duration(0.5), &Goal_Pub::goal_timer_callback, this, false, false);

    ROS_INFO("Connesso a move_base, invio primo goal.");
    ac_.waitForServer();
    next_Goal();
  }

private:
  ros::NodeHandle nh_; 
  MoveBaseClient ac_; 
  std::vector<Goal> goals_; 
  int current_goal_idx_; 
  std::string csv_path_; 
  ros::Timer goal_timer_;
  bool goal_ready_;

  void Load_Goals_from_CSV(const std::string &filename)
  {
    // Apertura file
    std::ifstream file(filename);
    if (!file.is_open())
    {
      ROS_ERROR("Impossibile aprire file: %s", filename.c_str());
      return;
    }

    std::string line;
    while (std::getline(file, line))
    {
      std::istringstream ss(line); // lettura di una riga come line
      std::string x, y, theta;
      if (std::getline(ss, x, ',') &&
          std::getline(ss, y, ',') &&
          std::getline(ss, theta)) // Suddivisione dei campi divisi da virgola
      {
        Goal g; // Corrected to Goal
        g.x = std::stod(x); // conversione da stringa a double
        g.y = std::stod(y);
        g.theta = std::stod(theta);
        goals_.push_back(g); 
      }
    }
  }

  void goal_timer_callback(const ros::TimerEvent&)
{
  if (goal_ready_) {
    goal_ready_ = false;
    goal_timer_.stop(); // ferma il timer finché non serve di nuovo
    next_Goal();
  }
}


  void next_Goal() {

    if (current_goal_idx_ >= goals_.size()) 
    {
      ROS_INFO("Tutti i goal sono stati raggiunti. Spegnimento del nodo.");
      ros::shutdown();
      return;
    }

    Goal g = goals_[current_goal_idx_]; 

    move_base_msgs::MoveBaseGoal mb_goal;
    mb_goal.target_pose.header.frame_id = "map";
    mb_goal.target_pose.header.stamp = ros::Time::now();
    mb_goal.target_pose.pose.position.x = g.x;
    mb_goal.target_pose.pose.position.y = g.y;

    tf::Quaternion q = tf::createQuaternionFromYaw(g.theta);
    mb_goal.target_pose.pose.orientation.x = q.x();
    mb_goal.target_pose.pose.orientation.y = q.y();
    mb_goal.target_pose.pose.orientation.z = q.z();
    mb_goal.target_pose.pose.orientation.w = q.w();

    ROS_INFO("Goal [%d]: x=%.2f, y=%.2f, θ=%.2f",
             current_goal_idx_, g.x, g.y, g.theta); 
    ac_.sendGoal(
        mb_goal,
        boost::bind(&Goal_Pub::done_callback, this, _1, _2)); 
  }

  void done_callback(const actionlib::SimpleClientGoalState& state,
                     const move_base_msgs::MoveBaseResultConstPtr& result) // result parameter
  {
    if (state == actionlib::SimpleClientGoalState::SUCCEEDED) {
      ROS_INFO("Goal [%d] reached", current_goal_idx_);
      current_goal_idx_++;
    } 
    else if (state == actionlib::SimpleClientGoalState::ABORTED) {
      ROS_WARN("Goal [%d] aborted, going to the next", current_goal_idx_);
      current_goal_idx_++;
    } 
    else {
      ROS_WARN("Goal [%d] terminated with state: %s",
             current_goal_idx_, state.toString().c_str());
      return;
    }

  goal_timer_.start();
  goal_ready_= true;
 }

};
  

int main(int argc, char **argv)
{
  ros::init(argc, argv, "goal_publisher");
  Goal_Pub gp;
  ros::spin();
  return 0;
}