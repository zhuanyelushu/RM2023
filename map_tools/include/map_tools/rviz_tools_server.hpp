#include"map_tools/rviz_tools.hpp"
#include"rclcpp/timer.hpp"
#include"rclcpp/rclcpp.hpp"
#include <interactive_markers/interactive_marker_server.hpp>
#include <interactive_markers/menu_handler.hpp>
#include <visualization_msgs/msg/interactive_marker.hpp>
#include <visualization_msgs/msg/interactive_marker_control.hpp>
#include <geometry_msgs/msg/pose.hpp>

namespace map_tools
{
    class RvizToolsServer : public rclcpp::Node
    {
    public:
        RvizToolsServer();
        ~RvizToolsServer() override;

    private:
        rclcpp::Subscription<map_tools::msg::WayPoint>::SharedPtr waypoint_sub_; // 订阅rviz发布的航点
        rclcpp::Publisher<map_tools::msg::WayPointArray>::SharedPtr waypoint_pub_; // 发布所有航点
        map_tools::msg::WayPointArray waypoints_; // 存储所有航点
        rclcpp::TimerBase::SharedPtr timer_; // 定时器  
        interactive_markers::InteractiveMarkerServer* waypoint_server_p_{nullptr};
        interactive_markers::MenuHandler* menu_waypoint_p_{nullptr};
        void waypointCallback(const map_tools::msg::WayPoint::SharedPtr msg); // 处理rviz发布的航点
        void timerCallback(); // 定时器回调函数
        void WaypointInterMarker(interactive_markers::InteractiveMarkerServer* server,std::string name,geometry_msgs::msg::Pose pose);  // 创建 interactive_marker
        void processWaypointFeedback(const visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr &feedback);
    };
}