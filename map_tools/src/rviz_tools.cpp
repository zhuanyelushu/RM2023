#include <map_tools/rviz_tools.hpp>
#include "rviz_common/load_resource.hpp"
#include "rviz_common/display_context.hpp"
#include "rviz_common/viewport_mouse_event.hpp"
#include "rviz_common/interaction/view_picker_iface.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>

static int nWaypointCount = 0; // 全局航点计数器

namespace map_tools
{
    // 添加航点控件
    AddWaypointTools::AddWaypointTools() : rviz_default_plugins::tools::PoseTool()
    {
        shortcut_key_ = 'a'; // 快捷键
        topic_property_ = new rviz_common::properties::StringProperty("Topic", "/rviz_tools/addwaypoint", "add waypoints",
                                                                      getPropertyContainer(), SLOT(updateTopic()), this);
    }

    AddWaypointTools::~AddWaypointTools()
    {
    }

    void AddWaypointTools::onInitialize()
    {
        rviz_default_plugins::tools::PoseTool::onInitialize();
        setName("Add Waypoint");
        setIcon(rviz_common::loadPixmap("package://map_tools/meshes/pointTools.png"));
        updateTopic();
    }

    void AddWaypointTools::updateTopic()
    {
        raw_node_ = context_->getRosNodeAbstraction().lock()->get_raw_node();
        RCLCPP_INFO(raw_node_->get_logger(), "Add waypoint topic: %s", topic_property_->getStdString().c_str());
        pub_ = raw_node_->create_publisher<map_tools::msg::WayPoint>(topic_property_->getStdString(), 1);

        // todo 添加点位后需要将添加的点位数据发布  当前所有点位应该会在/rviz_tools/waypoints中记录
    }

    void AddWaypointTools::onPoseSet(double x, double y, double theta)
    {
        RCLCPP_INFO(raw_node_->get_logger(), "Add waypoint: (%.2f, %.2f, %.2f)", x, y, theta);
        // theta -> q4
        std::string frame_id = context_->getFixedFrame().toStdString();
        tf2::Quaternion qtn;
        qtn.setRPY(0.0, 0.0, theta);
        geometry_msgs::msg::PoseStamped new_pos;
        new_pos.header.frame_id = frame_id;
        geometry_msgs::msg::Quaternion qtn_msg;
        tf2::convert(qtn, qtn_msg);
        new_pos.pose.orientation = qtn_msg;
        new_pos.pose.position.x = x;
        new_pos.pose.position.y = y;

        // todo： 设置点位后，应该有一个点位计数器，计数器自增，并且push_back到/rviz_tools/waypoints,所以需要有一个waypointarray 并发布这个消息
        // 临时先用全局变量处理一下
        nWaypointCount++;
        std::ostringstream ss;
        ss << nWaypointCount;
        map_tools::msg::WayPoint waypoint_msg;
        waypoint_msg.frame_id = frame_id;
        waypoint_msg.name = ss.str();
        waypoint_msg.pose = new_pos.pose;

        pub_->publish(waypoint_msg);
    }

    // 删除航点控件
    DeleteWaypointTools::DeleteWaypointTools() : rviz_common::Tool()
    {
        shortcut_key_ = 'd'; // 快捷键
        topic_property_ = new rviz_common::properties::StringProperty("Topic", "/rviz_tools/deletewaypoint", "delete waypoints",
                                                                      getPropertyContainer(), SLOT(updateTopic()), this);
    }
    DeleteWaypointTools::~DeleteWaypointTools()
    {
    }

    void DeleteWaypointTools::onInitialize()
    {

        rviz_common::Tool::onInitialize();
        setName("Delete Waypoint");
        setIcon(rviz_common::loadPixmap("package://map_tools/meshes/deleteTools.png"));
        updateTopic();
    }

    void DeleteWaypointTools::updateTopic()
    {
        raw_node_ = context_->getRosNodeAbstraction().lock()->get_raw_node();
        // 删除客户端
        delete_client_ = raw_node_->create_client<map_tools::srv::DeletePoint>("/rviz_tools/deletewaypoint");
        // 订阅所有点位信息
        waypoint_sub_ = raw_node_->create_subscription<map_tools::msg::WayPointArray>(
            "/rviz_tools/waypoint_array", 10,
            std::bind(&DeleteWaypointTools::waypointCallback, this, std::placeholders::_1));

        // 标记发布器，高亮显示可以删除的点位
        maker_pub_ = raw_node_->create_publisher<visualization_msgs::msg::MarkerArray>(
            "/rviz_tools/delete_marker", 1);
    }

    void DeleteWaypointTools::waypointCallback(const map_tools::msg::WayPointArray::SharedPtr msg)
    {
        current_waypoints_ = msg->way_points;
        updateMakers();
    }

    void DeleteWaypointTools::activate()
    {
        updateMakers();
    }

    void DeleteWaypointTools::deactivate()
    {
        // 清空标记
        visualization_msgs::msg::MarkerArray clear_markers;
        visualization_msgs::msg::Marker marker;
        marker.action = visualization_msgs::msg::Marker::DELETEALL;
        clear_markers.markers.push_back(marker);
        maker_pub_->publish(clear_markers);
    }

    int DeleteWaypointTools::processMouseEvent(rviz_common::ViewportMouseEvent &event)
    {

        if (event.left() && event.leftDown()) // 左键点击
        {
            Ogre::Vector3 pos;
            if (context_->getViewPicker()->get3DPoint(event.panel, event.x, event.y, pos))
            {
                map_tools::msg::WayPoint target_wp;
                if (findNearestWaypoint(event.x, event.y, target_wp))
                {
                    // 调用删除服务
                    auto request = std::make_shared<map_tools::srv::DeletePoint::Request>();
                    request->name = target_wp.name;

                    auto result_future = delete_client_->async_send_request(request);

                    if (rclcpp::spin_until_future_complete(raw_node_, result_future) ==
                        rclcpp::FutureReturnCode::SUCCESS)
                    {
                        if (result_future.get()->success)
                        {
                            RCLCPP_INFO(raw_node_->get_logger(), "Deleted waypoint: %s", target_wp.name.c_str());
                            return rviz_common::Tool::Finished; // 删除成功
                        }
                    }
                }
            }
        }
        return rviz_common::Tool::Render; // 继续渲染
    }

    bool DeleteWaypointTools::findNearestWaypoint(int x, int y, map_tools::msg::WayPoint &find_wp)
    {

        double min_distance = 0.1; // 最大删除距离阈值
        bool found = false;
        for (const auto &wp : current_waypoints_)
        {
            double distance = std::sqrt(std::pow(wp.pose.position.x - x, 2) +
                                        std::pow(wp.pose.position.y - y, 2));
            if (distance < min_distance)
            {
                min_distance = distance;
                find_wp = wp;
                found = true;
            }
        }
        return found;
    }

    void DeleteWaypointTools::updateMakers()
    {
        visualization_msgs::msg::MarkerArray markers;
        for (size_t i = 0; i < current_waypoints_.size(); i++)
        {
            visualization_msgs::msg::Marker marker;
            marker.header.frame_id = context_->getFixedFrame().toStdString();
            marker.header.stamp = raw_node_->get_clock()->now();
            marker.ns = "delete_waypoints"; // 命名空间
            marker.id = i;
            marker.type = visualization_msgs::msg::Marker::SPHERE;
            marker.action = visualization_msgs::msg::Marker::ADD;
            marker.pose = current_waypoints_[i].pose;
            marker.scale.x = 0.3;
            marker.scale.y = 0.3;
            marker.scale.z = 0.3;
            marker.color.a = 1.0;
            marker.color.r = 0.0;
            marker.color.g = 0.0;
            marker.color.b = 0.5;
            markers.markers.push_back(marker);
        }

        maker_pub_->publish(markers);
    }

    // 保存航点控件
    ShowWaypointsTools::ShowWaypointsTools(QWidget *parent) : rviz_common::Panel(parent)
    {
        // 创建gui控件
        get_info_button_ = new QPushButton("显示点位", this);
        topic_edit_ = new QLineEdit("/rviz_tools/waypoint_array", this);
        info_display_ = new QTextEdit(this);
        status_label_ = new QLabel("Status: ok", this);

        // gui config
        info_display_->setReadOnly(true);
        info_display_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        get_info_button_->setMaximumHeight(100);

        // 布局
        QVBoxLayout *main_layout = new QVBoxLayout;

        QHBoxLayout *topic_layout = new QHBoxLayout;
        topic_layout->addWidget(new QLabel("Topic:"));
        topic_layout->addWidget(topic_edit_);
        topic_layout->addWidget(get_info_button_);

        main_layout->addLayout(topic_layout);
        main_layout->addWidget(status_label_);
        main_layout->addWidget(new QLabel("Waypoints Info:"));
        main_layout->addWidget(info_display_);

        setLayout(main_layout);

        // choose sinals and slots
        connect(get_info_button_, &QPushButton::clicked, this, &ShowWaypointsTools::onGetInfoButtonClicked);
        connect(topic_edit_, &QLineEdit::editingFinished, this, &ShowWaypointsTools::updateTopic);

        // init ros node
        // raw_node_ = std::make_shared<rclcpp::Node>("save_waypoints_tools");
        has_new_info_ = false;
    }
    ShowWaypointsTools::~ShowWaypointsTools()
    {
    }

    void ShowWaypointsTools::onInitialize()
    {
        Panel::onInitialize();
        context_ = getDisplayContext();
        auto node_abstraction_weak = getDisplayContext()->getRosNodeAbstraction();
        auto node_abstraction = node_abstraction_weak.lock();
        if (node_abstraction)
        {
            raw_node_ = node_abstraction->get_raw_node();
                    updateTopic();
        }

    }

    void ShowWaypointsTools::load(const rviz_common::Config &config)
    {
        Panel::load(config);
        QString topic;
        if (config.mapGetString("topic", &topic))
        {
            topic_edit_->setText(topic);
            updateTopic();
        }
    }

    void ShowWaypointsTools::save(rviz_common::Config config) const
    {
        Panel::save(config);
        config.mapSetValue("topic", topic_edit_->text());
    }

    void ShowWaypointsTools::updateTopic()
    {
        std::string topic_name = topic_edit_->text().toStdString();
        // 取消旧的订阅
        waypoint_sub_.reset();
        // 创建新的订阅
        if (!topic_name.empty())
        {
            waypoint_sub_ = raw_node_->create_subscription<map_tools::msg::WayPointArray>(
                topic_name, 10,
                std::bind(&ShowWaypointsTools::waypointCallback, this, std::placeholders::_1));
            status_label_->setText(QString("Subscribed to topic: %1").arg(topic_name.c_str()));
        }
    }

    void ShowWaypointsTools::onGetInfoButtonClicked()
    {
        if (has_new_info_)
        {
            info_display_->setText(QString::fromStdString(latest_info_));
            status_label_->setText("已获取最新信息");
            has_new_info_ = false;
        }
        else
        {
            info_display_->setText("暂无新消息，等待话题数据...");
            status_label_->setText("等待数据中...");
        }
    }
    void ShowWaypointsTools::infoCallback(const std_msgs::msg::String::SharedPtr msg)
    {
        latest_info_ = msg->data;
        has_new_info_ = true;
        status_label_->setText(QString("get new info: %1").arg(QString::fromStdString(latest_info_).left(20) + "..."));
    }

    void ShowWaypointsTools::waypointCallback(const map_tools::msg::WayPointArray::SharedPtr msg)
    {
        std::stringstream ss;
        ss << "point num: " << msg->way_points.size() << "\n\n";

        for (size_t i = 0; i < msg->way_points.size(); ++i)
        {
            const auto &wp = msg->way_points[i];
            ss << "waypoint " << i + 1 << ": \n";
            ss << "  name: " << wp.name << "\n";
            ss << "  位置: (" << wp.pose.position.x << ", " << wp.pose.position.y << ", " << wp.pose.position.z << ")\n";
            ss << "  方向: (" << wp.pose.orientation.x << ", " << wp.pose.orientation.y << ", "
               << wp.pose.orientation.z << ", " << wp.pose.orientation.w << ")\n\n";
        }

        latest_info_ = ss.str();
        has_new_info_ = true;
    }

} // namespace map_tools

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(map_tools::AddWaypointTools, rviz_common::Tool)
PLUGINLIB_EXPORT_CLASS(map_tools::DeleteWaypointTools, rviz_common::Tool)
PLUGINLIB_EXPORT_CLASS(map_tools::ShowWaypointsTools, rviz_common::Panel)