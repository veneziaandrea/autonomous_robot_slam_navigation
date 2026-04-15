#include "ros/ros.h"
#include "nav_msgs/Odometry.h"
#include <tf/transform_broadcaster.h>
#include <cmath>
#include <geometry_msgs/PointStamped.h> 

void odomCallback(const nav_msgs::Odometry::ConstPtr& msg) {

    static tf::TransformBroadcaster broadcaster;
    tf::Transform transform;

    transform.setOrigin(tf::Vector3(msg->pose.pose.position.x, msg->pose.pose.position.y, msg->pose.pose.position.z));
    
    tf::Quaternion q;
    tf::quaternionMsgToTF(msg->pose.pose.orientation, q);
    transform.setRotation(q);

    // Ho messo odom come parent frame e base_link come child
    broadcaster.sendTransform(
        tf::StampedTransform(transform, msg->header.stamp, "odom", "base_link")
    );
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "odom_to_tf");
    ros::NodeHandle n;

    ros::Subscriber sub = n.subscribe("/odometry", 100, odomCallback);

    ros::spin();
    return 0;
}