#include "map_tools/rviz_tools_server.hpp"

namespace map_tools
{
    RvizToolsServer::RvizToolsServer() : Node("rviz_tools_server")
    {

        waypoint_server_p_ = new interactive_markers::InteractiveMarkerServer("waypoint_server", this);
        menu_waypoint_p_ = new interactive_markers::MenuHandler();
        waypoint_sub_ = this->create_subscription<map_tools::msg::WayPoint>(
            "/rviz_tools/addwaypoint", 10, std::bind(&RvizToolsServer::waypointCallback, this, std::placeholders::_1));
        waypoint_pub_ = this->create_publisher<map_tools::msg::WayPointArray>("/rviz_tools/waypoint_array", 10);
        timer_ = this->create_wall_timer(std::chrono::milliseconds(100), std::bind(&RvizToolsServer::timerCallback, this)); // 10hz
    }
    RvizToolsServer::~RvizToolsServer()
    {
    }


    void RvizToolsServer::waypointCallback(const map_tools::msg::WayPoint::SharedPtr msg)
    {
        if (msg != nullptr)
        {

            RCLCPP_INFO(this->get_logger(), "get waypoint message");
            waypoints_.header.stamp = this->now();
            waypoints_.header.frame_id = msg->frame_id;
            waypoints_.way_points.push_back(*msg);
            WaypointInterMarker(waypoint_server_p_, msg->name, msg->pose);
            menu_waypoint_p_->apply(*waypoint_server_p_, msg->name);
            waypoint_server_p_->applyChanges();
        }
    }

    void RvizToolsServer::timerCallback()
    {
        if (waypoints_.way_points.size() > 0)
        {
            waypoint_pub_->publish(waypoints_);
        }
    }

    void RvizToolsServer::WaypointInterMarker(interactive_markers::InteractiveMarkerServer *server, std::string name, geometry_msgs::msg::Pose pose)
    {
        visualization_msgs::msg::InteractiveMarker wp_itr_marker;
        visualization_msgs::msg::InteractiveMarkerControl wp_dis_ctrl;
        visualization_msgs::msg::InteractiveMarkerControl move_control;
        wp_itr_marker.header.stamp = this->get_clock()->now();
        wp_itr_marker.name = name;
        wp_itr_marker.description = name;
        wp_itr_marker.pose = pose;
        wp_itr_marker.header.frame_id = "map";


        visualization_msgs::msg::Marker wp_dis_marker;
        wp_dis_marker.action = visualization_msgs::msg::Marker::ADD;
        wp_dis_marker.type = visualization_msgs::msg::Marker::MESH_RESOURCE;
        wp_dis_marker.mesh_resource = "package://map_tools/meshes/waypoint.dae";
        wp_dis_marker.scale.x = 1;
        wp_dis_marker.scale.y = 1;
        wp_dis_marker.scale.z = 1;
        wp_dis_marker.color.r = 1.0;
        wp_dis_marker.color.g = 0.0;
        wp_dis_marker.color.b = 1.0;
        wp_dis_marker.color.a = 1.0;
        wp_dis_ctrl.markers.push_back(wp_dis_marker);


        visualization_msgs::msg::Marker text_marker;
        text_marker.type = visualization_msgs::msg::Marker::TEXT_VIEW_FACING;
        text_marker.scale.z = 0.3;
        text_marker.color.r = 0;
        text_marker.color.g = 0;
        text_marker.color.b = 1;
        text_marker.color.a = 1.0;
        text_marker.text = name;
        text_marker.pose.position.z = 0.8;
        wp_dis_ctrl.markers.push_back(text_marker);

        wp_dis_ctrl.always_visible = true;
        wp_itr_marker.controls.push_back(wp_dis_ctrl);


        move_control.name = "move_x";
        move_control.orientation.w = 1.0;
        move_control.orientation.x = 1.0;
        move_control.orientation.y = 0.0;
        move_control.orientation.z = 0.0;
        move_control.interaction_mode = visualization_msgs::msg::InteractiveMarkerControl::MOVE_AXIS;
        wp_itr_marker.controls.push_back(move_control);
        move_control.name = "move_z";
        move_control.orientation.x = 0.0;
        move_control.orientation.z = 1.0;
        wp_itr_marker.controls.push_back(move_control);
        move_control.name = "rotate_z";
        move_control.orientation.x = 0.0;
        move_control.orientation.y = 1.0;
        move_control.orientation.z = 0.0;
        move_control.interaction_mode = visualization_msgs::msg::InteractiveMarkerControl::ROTATE_AXIS;
        wp_itr_marker.controls.push_back(move_control);


        visualization_msgs::msg::Marker menu_marker;
        menu_marker.type = visualization_msgs::msg::Marker::CUBE;
        menu_marker.scale.x = 0.5;
        menu_marker.scale.y = 0.5;
        menu_marker.scale.z = 0.5;
        menu_marker.color.r = 0.9;
        menu_marker.color.g = 0.9;
        menu_marker.color.b = 0.9;
        menu_marker.color.a = 0.0; // 全透明
        visualization_msgs::msg::InteractiveMarkerControl menu_control;
        menu_control.interaction_mode = visualization_msgs::msg::InteractiveMarkerControl::BUTTON;
        menu_control.always_visible = true;
        menu_control.markers.push_back(menu_marker);
        wp_itr_marker.controls.push_back(menu_control);

        server->insert(wp_itr_marker, std::bind(&RvizToolsServer::processWaypointFeedback, this,std::placeholders::_1));
        server->applyChanges();
    }

    void RvizToolsServer::processWaypointFeedback(const visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr &feedback)
    {
        const int wp_size = waypoints_.way_points.size();
        for (int i = 0; i < wp_size; i++)
        {
            if (feedback->marker_name == waypoints_.way_points[i].name)
            {
                waypoints_.way_points[i].pose = feedback->pose;
            }
        }
    }

}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<map_tools::RvizToolsServer>());
    rclcpp::shutdown();
    return 0;
}
