#ifndef RVIZ_TOOLS_HPP_
#define RVIZ_TOOLS_HPP_

#ifndef Q_MOC_RUN
#include <QObject>
#include <rclcpp/rclcpp.hpp>
#include <rviz_common/tool.hpp>
#include <rviz_default_plugins/tools/pose/pose_tool.hpp>
#include <rviz_common/properties/string_property.hpp>
#include <rviz_common/display_context.hpp>
#include <visualization_msgs/msg/marker_array.hpp>
#include <geometry_msgs/msg/point_stamped.hpp>
#include <interactive_markers/interactive_marker_server.hpp>
#include <QtWidgets>
#include <std_msgs/msg/string.hpp>
#include <rviz_common/panel.hpp>
#include <rviz_common/interaction/selection_manager.hpp>
#endif

#include <map_tools/msg/way_point.hpp>
#include <map_tools/msg/way_point_array.hpp>
#include <map_tools/srv/delete_point.hpp>

#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/transform_listener.h>

namespace rviz_common
{
    class DisplayContext;

    namespace properties
    {
        class StringProperty;
    }

}

namespace map_tools
{
    // 添加航点控件
    class AddWaypointTools : public rviz_default_plugins::tools::PoseTool
    {
        Q_OBJECT
    public:
        AddWaypointTools();
        ~AddWaypointTools() override;

        void onInitialize() override;

    protected:
        void onPoseSet(double x, double y, double theta) override;

    private Q_SLOTS:
        void updateTopic();

    private:
        rclcpp::Node::SharedPtr raw_node_;
        rclcpp::Publisher<map_tools::msg::WayPoint>::SharedPtr pub_; // 发布航点
        rviz_common::properties::StringProperty *topic_property_;    // 发布话题
    };

    // 删除航点控件
    class DeleteWaypointTools : public rviz_common::Tool
    {
        Q_OBJECT
    public:
        DeleteWaypointTools();
        ~DeleteWaypointTools() override;

        void onInitialize() override;
        void activate() override;
        void deactivate() override;
        int processMouseEvent(rviz_common::ViewportMouseEvent &event) override;

    private Q_SLOTS:
        void updateTopic();

    private:
        rclcpp::Node::SharedPtr raw_node_;
        rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr maker_pub_; // 发布删除标记
        rclcpp::Client<map_tools::srv::DeletePoint>::SharedPtr delete_client_;         // 删除服务客户端
        rviz_common::properties::StringProperty *topic_property_;                      // 删除服务话题
        rclcpp::Subscription<map_tools::msg::WayPointArray>::SharedPtr waypoint_sub_;  // 订阅所有点位信息

        std::vector<map_tools::msg::WayPoint> current_waypoints_;                  // 当前所有航点
        void waypointCallback(const map_tools::msg::WayPointArray::SharedPtr msg); // 点位回调函数
        bool findNearestWaypoint(int x, int y, map_tools::msg::WayPoint &find_wp); // 查找最近点位
        void updateMakers();                                                       // 更新删除标记
    };

    // 保存点位控件
    class ShowWaypointsTools : public rviz_common::Panel
    {
        Q_OBJECT
    public:
        ShowWaypointsTools(QWidget *parent = nullptr);
        ~ShowWaypointsTools() override;

        void load(const rviz_common::Config &config) override;
        void save(rviz_common::Config config) const override;
        void onInitialize() override;
    protected Q_SLOTS:
        void onGetInfoButtonClicked();
        void updateTopic();

    private:
        // gui 控件
        QPushButton *get_info_button_;
        QTextEdit *info_display_;
        QLineEdit *topic_edit_;
        QLabel *status_label_;

        // rviz
        rviz_common::DisplayContext *context_;

        // ros
        rclcpp::Node::SharedPtr raw_node_;
        rclcpp::Subscription<std_msgs::msg::String>::SharedPtr string_sub_;
        rclcpp::Subscription<map_tools::msg::WayPointArray>::SharedPtr waypoint_sub_;

        // config
        rviz_common::properties::StringProperty *topic_property_;

        // data
        std::string latest_info_;
        bool has_new_info_;

        // callbacks
        void infoCallback(const std_msgs::msg::String::SharedPtr msg);
        void waypointCallback(const map_tools::msg::WayPointArray::SharedPtr msg);
        void updateDisplay();
    };

} // namespace map_tools

#endif // RVIZ_TOOLS_HPP_